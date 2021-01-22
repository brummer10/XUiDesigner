/*
 *                           0BSD 
 * 
 *                    BSD Zero Clause License
 * 
 *  Copyright (c) 2021 Hermann Meyer
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted.

 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 */

#include <stdint.h>
#include <unistd.h>
#include <limits.h>

#include <lilv/lilv.h>
#include "lv2/lv2plug.in/ns/ext/presets/presets.h"
#include "lv2/lv2plug.in/ns/ext/state/state.h"
#include "lv2/lv2plug.in/ns/ext/urid/urid.h"
#include <lv2/lv2plug.in/ns/ext/atom/atom.h>
#include <lv2/lv2plug.in/ns/ext/buf-size/buf-size.h>
#include <lv2/lv2plug.in/ns/ext/options/options.h>
#include <lv2/lv2plug.in/ns/ext/uri-map/uri-map.h>
#include <lv2/lv2plug.in/ns/ext/port-props/port-props.h>

#ifndef LV2_CORE__enabled
#define LV2_CORE__enabled LV2_CORE_PREFIX "enabled"
#endif

#include "xwidgets.h"
#include "xmessage-dialog.h"

/*---------------------------------------------------------------------
-----------------------------------------------------------------------    
                define maximum allowed controller numbers
-----------------------------------------------------------------------
----------------------------------------------------------------------*/

#define MAX_CONTROLS 225

/*---------------------------------------------------------------------
-----------------------------------------------------------------------    
                enums
-----------------------------------------------------------------------
----------------------------------------------------------------------*/

enum {
    XUI_NONE       = 0,
    XUI_POSITION   = 1,
    XUI_SIZE       = 2,
    XUI_WIDTH      = 3,
    XUI_HEIGHT     = 4,
};
 
typedef enum {
    IS_KNOB           ,
    IS_HSLIDER        ,
    IS_VSLIDER        ,
    IS_BUTTON         ,
    IS_TOGGLE_BUTTON  ,
    IS_COMBOBOX       ,
    IS_VALUE_DISPLAY  ,
    IS_LABEL          ,
    IS_VMETER         ,
    IS_HMETER         ,
} WidgetType;

/*---------------------------------------------------------------------
-----------------------------------------------------------------------    
                struct to hold the info for printout
-----------------------------------------------------------------------
----------------------------------------------------------------------*/

typedef struct {
    Widget_t * wid;
    WidgetType is_type;
    uint32_t port_index;
    bool have_adjustment;
    const char* type;
} Controller;

/*---------------------------------------------------------------------
-----------------------------------------------------------------------    
                struct to hold the info for the designer
-----------------------------------------------------------------------
----------------------------------------------------------------------*/

typedef struct {
    bool is_input_port;
    bool is_output_port;
    bool is_toggle_port;
    bool is_enum_port;
    bool is_int_port;
    bool is_trigger_port;
    bool is_log_port;
    bool have_adjustment;
    int Port_Index;
    const char *name;
    float min;
    float max;
    float def;
} LV2_CONTROLLER;

typedef struct {
    LilvWorld* world;
    const LilvPlugins* lv2_plugins;        
    
    Widget_t *w;
    Widget_t *ui;
    Widget_t *controller_label;
    Widget_t *widgets;
    Widget_t *index;
    Widget_t *set_index;
    Widget_t *combobox_settings;
    Widget_t *add_entry;
    Widget_t *combobox_entry;
    Widget_t *controller_settings;
    Widget_t *controller_entry[4];
    Widget_t *set_adjust;
    Widget_t *lv2_uris;
    int active_widget_num;
    int pos_x;
    int pos_y;
    int width;
    int height;
    int modify_mod;
    bool run;
    int wid_counter;
    int select_widget_num;
    char** new_label ;
    LV2_CONTROLLER lv2c;
    Controller controls[MAX_CONTROLS];
} XUiDesigner;

Widget_t *x_axis = NULL;
Widget_t *y_axis = NULL;
Widget_t *w_axis = NULL;
Widget_t *h_axis = NULL;
Widget_t *active_widget = NULL;

/*---------------------------------------------------------------------
-----------------------------------------------------------------------    
                print widgets on exit
-----------------------------------------------------------------------
----------------------------------------------------------------------*/

static void remove_from_list(XUiDesigner *designer, Widget_t *wid) {
    designer->controls[wid->data].wid = NULL;
    designer->controls[wid->data].have_adjustment = false;
}

static void add_to_list(XUiDesigner *designer, Widget_t *wid, const char* type,
                                    bool have_adjustment, WidgetType is_type) {
    designer->controls[wid->data].wid = wid;
    designer->controls[wid->data].type = type;
    designer->controls[wid->data].have_adjustment = have_adjustment;
    designer->controls[wid->data].is_type = is_type;
}

static void print_list(XUiDesigner *designer) {
    int i = 0;
    int j = 0;
    for (;i<MAX_CONTROLS;i++) {
        if (designer->controls[i].wid != NULL) {
            j++;
        }
    }
    if (j) {
        printf ("#define CONTROLS %i\n", j);
        Window w = (Window)designer->ui->widget;
        char *name;
        XFetchName(designer->ui->app->dpy, w, &name);
        if (name != NULL) printf ("name = %s\n", name);
        printf ("width = %i;\nheight = %i;\n", designer->ui->width, designer->ui->height);
    } else {
        return;
    }
    i = 0;
    j = 0;
    for (;i<MAX_CONTROLS;i++) {
        if (designer->controls[i].wid != NULL) {
            Widget_t * wid = designer->controls[i].wid;
            printf ("ui->widget[%i] = %s (ui->widget[%i], %i, \"%s\", %i,  %i, %i, %i);\n", 
                j, designer->controls[i].type, j,
                designer->controls[i].port_index, designer->controls[i].wid->label,
                designer->controls[i].wid->x, designer->controls[i].wid->y,
                designer->controls[i].wid-> width, designer->controls[i].wid->height);
            if (designer->controls[i].is_type == IS_COMBOBOX) {
                Widget_t *menu = wid->childlist->childs[1];
                Widget_t* view_port =  menu->childlist->childs[0];
                ComboBox_t *comboboxlist = (ComboBox_t*)view_port->parent_struct;
                unsigned int k = 0;
                for(; k<comboboxlist->list_size;k++) {
                    printf ("combobox_add_entry (ui->widget[%i], \"%s\");\n", j, comboboxlist->list_names[k]);
                }
            }
            if (designer->controls[i].have_adjustment) {
                printf ("set_adjustment(ui->widget[%i]->adj, %.3f, %.3f, %.3f, %.3f, %.3f, %i);\n", 
                    j, wid->adj->std_value, wid->adj->std_value, wid->adj->min_value, wid->adj->max_value,
                    wid->adj->step, wid->adj->type);
            }
            j++;
        }
    }
}


/*---------------------------------------------------------------------
-----------------------------------------------------------------------    
                label input widget
-----------------------------------------------------------------------
----------------------------------------------------------------------*/

void utf8ncpy(char* dst, const char* src, size_t sizeDest ) {
    if( sizeDest ){
        size_t sizeSrc = strlen(src);
        while( sizeSrc >= sizeDest ){
            const char* lastByte = src + sizeSrc;
            while( lastByte-- > src )
                if((*lastByte & 0xC0) != 0x80)
                    break;
            sizeSrc = lastByte - src;
        }
        memcpy(dst, src, sizeSrc);
        dst[sizeSrc] = '\0';
    }
}

static void entry_set_text(XUiDesigner *designer, const char* label) {
    memset(designer->controller_label->input_label, 0,
        32 * (sizeof designer->controller_label->input_label[0]));
    if (strlen(label))
        utf8ncpy(designer->controller_label->input_label, label, 30);
    strcat(designer->controller_label->input_label, "|");
    expose_widget(designer->controller_label);
}

static void draw_entry(void *w_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    if (!w) return;
    XWindowAttributes attrs;
    XGetWindowAttributes(w->app->dpy, (Window)w->widget, &attrs);
    int width = attrs.width;
    int height = attrs.height;
    if (attrs.map_state != IsViewable) return;

    use_base_color_scheme(w, NORMAL_);
    cairo_rectangle(w->cr,0,0,width,height);
    cairo_fill_preserve (w->cr);
    use_text_color_scheme(w, NORMAL_);
    cairo_set_line_width(w->cr, 2.0);
    cairo_stroke(w->cr);

    cairo_set_font_size (w->cr, 9.0);

    cairo_move_to (w->cr, 2, 9);
    cairo_show_text(w->cr, " ");
}

static void entry_add_text(void  *w_, void *label_) {
    Widget_t *w = (Widget_t*)w_;
    if (!w) return;
    char *label = (char*)label_;
    if (!label) {
        label = (char*)"";
    }
    draw_entry(w,NULL);
    cairo_text_extents_t extents;
    use_text_color_scheme(w, NORMAL_);
    cairo_set_font_size (w->cr, 11.0);
    if (strlen( w->input_label))
         w->input_label[strlen( w->input_label)-1] = 0;
    if (strlen( w->input_label)<30) {
        if (strlen(label))
        strcat( w->input_label, label);
    }
    w->label = w->input_label;
    strcat( w->input_label, "|");
    cairo_set_font_size (w->cr, 12.0);
    cairo_text_extents(w->cr, w->input_label , &extents);

    cairo_move_to (w->cr, 2, 12.0+extents.height);
    cairo_show_text(w->cr,  w->input_label);

}

static void entry_clip(Widget_t *w) {
    draw_entry(w,NULL);
    cairo_text_extents_t extents;
    use_text_color_scheme(w, NORMAL_);
    cairo_set_font_size (w->cr, 11.0);

    // check for UTF 8 char
    if (strlen( w->input_label)>=2) {
        int i = strlen( w->input_label)-1;
        int j = 0;
        int u = 0;
        for(;i>0;i--) {
            if(IS_UTF8(w->input_label[i])) {
                 u++;
            }
            j++;
            if (u == 1) break;
            if (j>2) break;
        }
        if (!u) j =2;

        memset(&w->input_label[strlen( w->input_label)-(sizeof(char)*(j))],0,sizeof(char)*(j));
        strcat( w->input_label, "|");
    }
    cairo_set_font_size (w->cr, 12.0);
    cairo_text_extents(w->cr, w->input_label , &extents);

    cairo_move_to (w->cr, 2, 12.0+extents.height);
    cairo_show_text(w->cr,  w->input_label);

}

static void entry_get_text(void *w_, void *key_, void *user_data) {
    Widget_t *w = (Widget_t*)w_;
    if (!w) return;
    XUiDesigner *designer = (XUiDesigner*)w->parent_struct;
    XKeyEvent *key = (XKeyEvent*)key_;
    if (!key) return;
    int nk = key_mapping(w->app->dpy, key);
    if (nk == 11) {
        entry_clip(w);
    } else {
        Status status;
        KeySym keysym;
        char buf[32];
        Xutf8LookupString(w->xic, key, buf, sizeof(buf) - 1, &keysym, &status);
        if (keysym == XK_Return) {
            if (active_widget != NULL) {
                if (strlen(w->input_label)) {
                    asprintf (&designer->new_label[designer->active_widget_num], "%s", w->input_label);
                    designer->new_label[designer->active_widget_num][strlen( w->input_label)-1] = 0;
                    active_widget->label = (const char*)designer->new_label[designer->active_widget_num];
                    expose_widget(active_widget);
                }
            }
            return;
        }
        if(status == XLookupChars || status == XLookupBoth){
            entry_add_text(w, buf);
            if (active_widget != NULL) {
                if (strlen(w->input_label)) {
                    asprintf (&designer->new_label[designer->active_widget_num], "%s", w->input_label);
                    designer->new_label[designer->active_widget_num][strlen( w->input_label)-1] = 0;
                    active_widget->label = (const char*)designer->new_label[designer->active_widget_num];
                    expose_widget(active_widget);
                }
            }
        }
    }
}

/*---------------------------------------------------------------------
-----------------------------------------------------------------------    
                text/numeric entry input widget
-----------------------------------------------------------------------
----------------------------------------------------------------------*/

static void box_entry_set_text(Widget_t *w, float value) {
    memset(w->input_label, 0, 32 * (sizeof w->input_label[0]));
    char buffer[30];
    snprintf(buffer, sizeof buffer, "%.3f", value);
    strcat(w->input_label, buffer);
    strcat(w->input_label, "|");
    expose_widget(w);
}

static void draw_box_entry(void *w_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    if (!w) return;
    XWindowAttributes attrs;
    XGetWindowAttributes(w->app->dpy, (Window)w->widget, &attrs);
    int width = attrs.width;
    int height = attrs.height;
    if (attrs.map_state != IsViewable) return;

    use_base_color_scheme(w, NORMAL_);
    cairo_rectangle(w->cr,0,0,width,height);
    cairo_fill_preserve (w->cr);
    use_text_color_scheme(w, NORMAL_);
    cairo_set_line_width(w->cr, 2.0);
    cairo_stroke(w->cr);

    cairo_set_font_size (w->cr, 9.0);

    cairo_move_to (w->cr, 2, 9);
    cairo_show_text(w->cr, " ");
}

static void box_entry_add_text(void  *w_, void *label_) {
    Widget_t *w = (Widget_t*)w_;
    if (!w) return;
    char *label = (char*)label_;
    if (!label) {
        label = (char*)"";
    }
    draw_box_entry(w,NULL);
    cairo_text_extents_t extents;
    use_text_color_scheme(w, NORMAL_);
    cairo_set_font_size (w->cr, 11.0);
    if (strlen( w->input_label))
         w->input_label[strlen( w->input_label)-1] = 0;
    if (strlen( w->input_label)<30) {
        if (strlen(label))
        strcat( w->input_label, label);
    }
    w->label = w->input_label;
    strcat( w->input_label, "|");
    cairo_set_font_size (w->cr, 12.0);
    cairo_text_extents(w->cr, w->input_label , &extents);

    cairo_move_to (w->cr, 2, 12.0+extents.height);
    cairo_show_text(w->cr,  w->input_label);

}

static void box_entry_clip(Widget_t *w) {
    draw_entry(w,NULL);
    cairo_text_extents_t extents;
    use_text_color_scheme(w, NORMAL_);
    cairo_set_font_size (w->cr, 11.0);

    // check for UTF 8 char
    if (strlen( w->input_label)>=2) {
        int i = strlen( w->input_label)-1;
        int j = 0;
        int u = 0;
        for(;i>0;i--) {
            if(IS_UTF8(w->input_label[i])) {
                 u++;
            }
            j++;
            if (u == 1) break;
            if (j>2) break;
        }
        if (!u) j =2;

        memset(&w->input_label[strlen( w->input_label)-(sizeof(char)*(j))],0,sizeof(char)*(j));
        strcat( w->input_label, "|");
    }
    cairo_set_font_size (w->cr, 12.0);
    cairo_text_extents(w->cr, w->input_label , &extents);

    cairo_move_to (w->cr, 2, 12.0+extents.height);
    cairo_show_text(w->cr,  w->input_label);

}

static void box_entry_get_text(void *w_, void *key_, void *user_data) {
    Widget_t *w = (Widget_t*)w_;
    if (!w) return;
    XKeyEvent *key = (XKeyEvent*)key_;
    if (!key) return;
    int nk = key_mapping(w->app->dpy, key);
    if (nk == 11) {
        box_entry_clip(w);
    } else {
        Status status;
        KeySym keysym;
        char buf[32];
        Xutf8LookupString(w->xic, key, buf, sizeof(buf) - 1, &keysym, &status);
        if (keysym == XK_Return) {
            if (strlen(w->input_label)>1) {
                w->func.user_callback(w, (void*)w->input_label);
            }
            return;
        }
        if(status == XLookupChars || status == XLookupBoth){
            if (w->data) {
                if (key->keycode == XKeysymToKeycode(w->app->dpy, XK_0) ||
                    key->keycode == XKeysymToKeycode(w->app->dpy, XK_1) ||
                    key->keycode == XKeysymToKeycode(w->app->dpy, XK_2) ||
                    key->keycode == XKeysymToKeycode(w->app->dpy, XK_3) ||
                    key->keycode == XKeysymToKeycode(w->app->dpy, XK_4) ||
                    key->keycode == XKeysymToKeycode(w->app->dpy, XK_5) ||
                    key->keycode == XKeysymToKeycode(w->app->dpy, XK_6) ||
                    key->keycode == XKeysymToKeycode(w->app->dpy, XK_7) ||
                    key->keycode == XKeysymToKeycode(w->app->dpy, XK_8) ||
                    key->keycode == XKeysymToKeycode(w->app->dpy, XK_9) ||
                    key->keycode == XKeysymToKeycode(w->app->dpy, XK_period)) {
                    box_entry_add_text(w, buf);
                }
            } else {
                box_entry_add_text(w, buf);
            }
        }
    }
}

// data = 0; textinput / data = 1; numeric input
Widget_t *add_input_box(Widget_t *parent, int data, int x, int y, int width, int height) {
    Widget_t *wid = create_widget(parent->app, parent, x, y, width, height);
    memset(wid->input_label, 0, 32 * (sizeof wid->input_label[0]) );
    wid->data = data;
    wid->func.expose_callback = box_entry_add_text;
    wid->func.key_press_callback = box_entry_get_text;
    wid->flags &= ~USE_TRANSPARENCY;
    //wid->scale.gravity = EASTWEST;
    Cursor c = XCreateFontCursor(parent->app->dpy, XC_xterm);
    XDefineCursor (parent->app->dpy, wid->widget, c);
    XFreeCursor(parent->app->dpy, c);
    return wid;
}

/*---------------------------------------------------------------------
-----------------------------------------------------------------------    
                designer function calls
-----------------------------------------------------------------------
----------------------------------------------------------------------*/

static void draw_window(void *w_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    use_bg_color_scheme(w, NORMAL_);
    cairo_paint (w->crb);
}

static void set_port_index(void *w_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    XUiDesigner *designer = (XUiDesigner*)w->parent_struct;
    if (w->flags & HAS_POINTER && !adj_get_value(w->adj_y)) {
        if (active_widget != NULL) {
            designer->controls[designer->active_widget_num].port_index =
                (int)adj_get_value(designer->index->adj);
        }
    }
}

static void set_combobox_entry(void *w_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    XUiDesigner *designer = (XUiDesigner*)w->parent_struct;
    if (designer->controls[designer->active_widget_num].is_type == IS_COMBOBOX ) {
        if (strlen(designer->combobox_entry->input_label)>1) {
            designer->combobox_entry->input_label[strlen(designer->combobox_entry->input_label)-1] = 0;
            combobox_add_entry(designer->controls[designer->active_widget_num].wid,
                                            designer->combobox_entry->input_label);
            memset(designer->combobox_entry->input_label, 0, 32 *
                (sizeof designer->combobox_entry->input_label[0]));
            strcat(designer->combobox_entry->input_label, "|");
            expose_widget(designer->combobox_entry);
        }
    }
}

static void add_combobox_entry(void *w_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    XUiDesigner *designer = (XUiDesigner*)w->parent_struct;
    if (w->flags & HAS_POINTER && !adj_get_value(w->adj_y)) {
        if (designer->controls[designer->active_widget_num].is_type == IS_COMBOBOX ) {
            if (strlen(designer->combobox_entry->input_label)>1) {
                designer->combobox_entry->input_label[strlen(designer->combobox_entry->input_label)-1] = 0;
                combobox_add_entry(designer->controls[designer->active_widget_num].wid,
                                                designer->combobox_entry->input_label);
                memset(designer->combobox_entry->input_label, 0, 32 *
                    (sizeof designer->combobox_entry->input_label[0]));
                strcat(designer->combobox_entry->input_label, "|");
                expose_widget(designer->combobox_entry);
            }
        }
    }
}

static void set_controller_adjustment(void *w_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    XUiDesigner *designer = (XUiDesigner*)w->parent_struct;
    if (w->flags & HAS_POINTER && !adj_get_value(w->adj_y)) {
        if (designer->controls[designer->active_widget_num].have_adjustment) {
            if (strlen(designer->controller_entry[0]->input_label)>1) {
                designer->controller_entry[0]->input_label[strlen(designer->controller_entry[0]->input_label)-1] = 0;
                active_widget->adj->min_value = atof(designer->controller_entry[0]->input_label);
                strcat(designer->controller_entry[0]->input_label, "|");
            }
            if (strlen(designer->controller_entry[1]->input_label)>1) {
                designer->controller_entry[1]->input_label[strlen(designer->controller_entry[1]->input_label)-1] = 0;
                active_widget->adj->max_value = atof(designer->controller_entry[1]->input_label);
                strcat(designer->controller_entry[1]->input_label, "|");
            }
            if (strlen(designer->controller_entry[2]->input_label)>1) {
                designer->controller_entry[2]->input_label[strlen(designer->controller_entry[2]->input_label)-1] = 0;
                active_widget->adj->value = atof(designer->controller_entry[2]->input_label);
                active_widget->adj->std_value = atof(designer->controller_entry[2]->input_label);
                strcat(designer->controller_entry[2]->input_label, "|");
            }
            if (strlen(designer->controller_entry[3]->input_label)>1) {
                designer->controller_entry[3]->input_label[strlen(designer->controller_entry[3]->input_label)-1] = 0;
                active_widget->adj->step = atof(designer->controller_entry[3]->input_label);
                strcat(designer->controller_entry[3]->input_label, "|");
            }
        }
    }
}

static void set_widget_callback(void *w_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    XUiDesigner *designer = (XUiDesigner*)w->parent_struct;
    designer->select_widget_num = (int)adj_get_value(w->adj);
}

static void set_x_axis_callback(void *w_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    int v = (int)adj_get_value(w->adj);
    if (active_widget != NULL)
        XMoveWindow(active_widget->app->dpy,active_widget->widget,v, (int)adj_get_value(y_axis->adj));
}

static void set_y_axis_callback(void *w_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    int v = (int)adj_get_value(w->adj);
    if (active_widget != NULL)
        XMoveWindow(active_widget->app->dpy,active_widget->widget, (int)adj_get_value(x_axis->adj), v);
}

static void set_w_axis_callback(void *w_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    int v = (int)adj_get_value(w->adj);
    if (active_widget != NULL)
        XResizeWindow(active_widget->app->dpy,active_widget->widget, v, (int)adj_get_value(h_axis->adj));
}

static void set_h_axis_callback(void *w_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    int v = (int)adj_get_value(w->adj);
    if (active_widget != NULL)
        XResizeWindow(active_widget->app->dpy,active_widget->widget, (int)adj_get_value(w_axis->adj), v);
}

static void move_wid(void *w_, void *xmotion_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    Widget_t *p = (Widget_t*)w->parent;
    XUiDesigner *designer = (XUiDesigner*)p->parent_struct;
    XMotionEvent *xmotion = (XMotionEvent*)xmotion_;
    static int is_curser = 2;
    switch(designer->modify_mod) {
        case XUI_POSITION:
            XMoveWindow(w->app->dpy,w->widget,w->x + (xmotion->x_root-designer->pos_x), w->y+ (xmotion->y_root-designer->pos_y));
            adj_set_value(x_axis->adj, w->x + (xmotion->x_root-designer->pos_x));
            adj_set_value(y_axis->adj, w->y + (xmotion->y_root-designer->pos_y));
        break;
        case XUI_SIZE:
            XResizeWindow(w->app->dpy, w->widget, max(10,w->width + (xmotion->x_root-designer->pos_x)), max(10,w->height+ (xmotion->x_root-designer->pos_x)));
            adj_set_value(w_axis->adj, w->width + ((xmotion->x_root-designer->pos_x)));
            adj_set_value(h_axis->adj, w->height+ ((xmotion->x_root-designer->pos_x)));
            w->scale.ascale = 1.0;
            designer->pos_x = xmotion->x_root;
        break;
        case XUI_WIDTH:
            XResizeWindow(w->app->dpy, w->widget, max(10,w->width + (xmotion->x_root-designer->pos_x)), w->height);
            adj_set_value(w_axis->adj, w->width + ((xmotion->x_root-designer->pos_x)));
            w->scale.ascale = 1.0;
            designer->pos_x = xmotion->x_root;
        break;
        case XUI_HEIGHT:
            XResizeWindow(w->app->dpy, w->widget, w->width, max(10,w->height + (xmotion->y_root-designer->pos_y)));
            adj_set_value(h_axis->adj, w->height + ((xmotion->y_root-designer->pos_y)));
            w->scale.ascale = 1.0;
            designer->pos_y = xmotion->y_root;
        break;
        case XUI_NONE:
            if (xmotion->x > w->width/1.2 && xmotion->y > w->height/1.2 && is_curser != 1) {
                is_curser = 1;
                Cursor c = XCreateFontCursor(w->app->dpy, XC_bottom_right_corner);
                XDefineCursor (w->app->dpy, w->widget, c);
                XFreeCursor(w->app->dpy, c);
            } else if ( xmotion->y > w->height/1.2 && is_curser != 4) {
                is_curser = 4;
                Cursor c = XCreateFontCursor(w->app->dpy, XC_bottom_side);
                XDefineCursor (w->app->dpy, w->widget, c);
                XFreeCursor(w->app->dpy, c);
            } else if (xmotion->x > w->width/1.2 && is_curser != 3) {
                is_curser = 3;
                Cursor c = XCreateFontCursor(w->app->dpy, XC_right_side);
                XDefineCursor (w->app->dpy, w->widget, c);
                XFreeCursor(w->app->dpy, c);
            } else if ((xmotion->x <= w->width/1.2 && xmotion->y <= w->height/1.2) && is_curser != 2) {
                is_curser = 2;
                Cursor c = XCreateFontCursor(w->app->dpy, XC_hand2);
                XDefineCursor (w->app->dpy, w->widget, c);
                XFreeCursor(w->app->dpy, c);
            } else {
                is_curser = 0;
            }
        break;
        default:
        break;
    }
}

static void set_pos_wid(void *w_, void *button_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    Widget_t *p = (Widget_t*)w->parent;
    XWindowAttributes attrs;
    XGetWindowAttributes(w->app->dpy, (Window)w->widget, &attrs);
    int width = attrs.width;
    int height = attrs.height;
    XUiDesigner *designer = (XUiDesigner*)p->parent_struct;
    XButtonEvent *xbutton = (XButtonEvent*)button_;
    if(xbutton->button == Button1) {
        active_widget = (Widget_t*)w_;
        designer->active_widget_num = w->data;
        designer->pos_x = xbutton->x_root;
        designer->pos_y = xbutton->y_root;
        entry_set_text(designer, w->label);
        adj_set_value(w_axis->adj, w->width);
        adj_set_value(h_axis->adj, w->height);
        adj_set_value(designer->index->adj, 
            (float)designer->controls[designer->active_widget_num].port_index);

        if (designer->controls[designer->active_widget_num].have_adjustment) {
            widget_show_all(designer->controller_settings);
            box_entry_set_text(designer->controller_entry[0], active_widget->adj->min_value);
            box_entry_set_text(designer->controller_entry[1], active_widget->adj->max_value);
            box_entry_set_text(designer->controller_entry[2], active_widget->adj->std_value);
            box_entry_set_text(designer->controller_entry[3], active_widget->adj->step);

        } else {
            widget_hide(designer->controller_settings);
        }
        if (designer->controls[designer->active_widget_num].is_type == IS_COMBOBOX) {
            widget_show_all(designer->combobox_settings);
        } else {
            widget_hide(designer->combobox_settings);
        }
    }
    if (xbutton->x > width/1.2 && xbutton->y > height/1.2) {
        designer->modify_mod = XUI_SIZE;
    } else if (xbutton->y > height/1.2) {
        designer->modify_mod = XUI_HEIGHT;
    } else if (xbutton->x > width/1.2) {
        designer->modify_mod = XUI_WIDTH;
    } else {
        designer->modify_mod = XUI_POSITION;
    }
}

static void fix_pos_wid(void *w_, void *button_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    Widget_t *p = (Widget_t*)w->parent;
    XWindowAttributes attrs;
    XGetWindowAttributes(w->app->dpy, (Window)w->widget, &attrs);
    int x = attrs.x;
    int y = attrs.y;
    int width = attrs.width;
    int height = attrs.height;
    XUiDesigner *designer = (XUiDesigner*)p->parent_struct;
    XButtonEvent *xbutton = (XButtonEvent*)button_;
    if(xbutton->button == Button1) {
        w->x = x;
        w->y = y;
        w->scale.init_x   = x;
        w->scale.init_y   = y;
        w->width = width;
        w->height = height;
        w->scale.init_width = width;
        w->scale.init_height = height;
        w->scale.ascale = 1.0;
        designer->modify_mod = XUI_NONE;
        active_widget = (Widget_t*)w_;
        designer->active_widget_num = w->data;
        adj_set_value(x_axis->adj, w->x);
        adj_set_value(y_axis->adj, w->y);
    } else if(xbutton->button == Button3) {
        designer->modify_mod = XUI_NONE;
        active_widget = NULL;
        remove_from_list(designer, w);
        destroy_widget(w, w->app);
        entry_set_text(designer, "");
        adj_set_value(x_axis->adj, 0.0);
        adj_set_value(y_axis->adj, 0.0);
        adj_set_value(w_axis->adj, 10.0);
        adj_set_value(h_axis->adj, 10.0);
        widget_hide(designer->combobox_settings);
        widget_hide(designer->controller_settings);
    }
}

static void null_callback(void *w_, void* user_data) {
    
}

static void set_designer_callbacks(float x, float y, float w, float h) {
    xevfunc store = x_axis->func.value_changed_callback;
    x_axis->func.value_changed_callback = null_callback;
    adj_set_value(x_axis->adj, x);
    x_axis->func.value_changed_callback = store;
    store = y_axis->func.value_changed_callback;
    y_axis->func.value_changed_callback = null_callback;
    adj_set_value(y_axis->adj, y);
    y_axis->func.value_changed_callback = store;
    store = w_axis->func.value_changed_callback;
    w_axis->func.value_changed_callback = null_callback;
    adj_set_value(w_axis->adj, w);
    w_axis->func.value_changed_callback = store;
    store = h_axis->func.value_changed_callback;
    h_axis->func.value_changed_callback = null_callback;
    adj_set_value(h_axis->adj, h);
    h_axis->func.value_changed_callback = store;
}

static void set_controller_callbacks(XUiDesigner *designer, Widget_t *wid) {
    XSelectInput(wid->app->dpy, wid->widget,StructureNotifyMask|ExposureMask|KeyPressMask 
                    |EnterWindowMask|LeaveWindowMask|ButtonReleaseMask
                    |ButtonPressMask|Button1MotionMask|PointerMotionMask);
    wid->data = designer->wid_counter;
    wid->scale.gravity = NONE;
    //wid->parent_struct = designer;
    wid->func.button_press_callback = set_pos_wid;
    wid->func.button_release_callback = fix_pos_wid;
    wid->func.motion_callback = move_wid;
    designer->active_widget_num = designer->wid_counter;
    entry_set_text(designer, wid->label);
    active_widget = wid;
    adj_set_value(designer->index->adj, adj_get_value(designer->index->adj)+1.0);
    designer->controls[designer->wid_counter].port_index = adj_get_value(designer->index->adj);
    set_designer_callbacks((float)wid->x, (float)wid->y, (float)wid->width, (float)wid->height);
    designer->wid_counter++;
    Cursor c = XCreateFontCursor(wid->app->dpy, XC_hand2);
    XDefineCursor (wid->app->dpy, wid->widget, c);
    XFreeCursor(wid->app->dpy, c);
    widget_show_all(designer->ui);
}

static void button_released_callback(void *w_, void *button_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    XUiDesigner *designer = (XUiDesigner*)w->parent_struct;
    if (designer->wid_counter >= MAX_CONTROLS) {
        Widget_t *dia = open_message_dialog(w, INFO_BOX, _("INFO"), _("MAX CONTROL COUNTER OVERFLOW"),NULL);
        XSetTransientForHint(w->app->dpy, dia->widget, w->widget);
        return;
    }
    //fprintf(stderr, "%i\n", designer->select_widget_num);
    XButtonEvent *xbutton = (XButtonEvent*)button_;
    Widget_t *wid = NULL;
    if(xbutton->button == Button1) {
        switch(designer->select_widget_num) {
            case 1:
                wid = add_knob(w, "Knob", xbutton->x, xbutton->y, 60, 80);
                set_controller_callbacks(designer, wid);
                add_to_list(designer, wid, "add_lv2_knob", true, IS_KNOB);
            break;
            case 2:
                wid = add_hslider(w, "HSlider", xbutton->x, xbutton->y, 120, 30);
                set_controller_callbacks(designer, wid);
                add_to_list(designer, wid, "add_lv2_hslider", true, IS_HSLIDER);
            break;
            case 3:
                wid = add_vslider(w, "VSlider", xbutton->x, xbutton->y, 30, 120);
                set_controller_callbacks(designer, wid);
                add_to_list(designer, wid, "add_lv2_vslider", true, IS_VSLIDER);
            break;
            case 4:
                wid = add_button(w, "Button", xbutton->x, xbutton->y, 60, 60);
                set_controller_callbacks(designer, wid);
                add_to_list(designer, wid, "add_lv2_button", false, IS_BUTTON);
            break;
            case 5:
                wid = add_toggle_button(w, "Switch", xbutton->x, xbutton->y, 60, 60);
                set_controller_callbacks(designer, wid);
                add_to_list(designer, wid, "add_lv2_toggle_button", false, IS_TOGGLE_BUTTON);
            break;
            case 6:
                wid = add_combobox(w, "Combobox", xbutton->x, xbutton->y, 120, 30);
                set_controller_callbacks(designer, wid);
                add_to_list(designer, wid, "add_lv2_combobox", true, IS_COMBOBOX);
            break;
            case 7:
                wid = add_valuedisplay(w, "", xbutton->x, xbutton->y, 40, 30);
                set_controller_callbacks(designer, wid);
                add_to_list(designer, wid, "add_lv2_valuedisplay", false, IS_VALUE_DISPLAY);
            break;
            case 8:
                wid = add_label(w, "Label", xbutton->x, xbutton->y, 60, 30);
                set_controller_callbacks(designer, wid);
                add_to_list(designer, wid, "add_lv2_label", false, IS_LABEL);
            break;
            case 9:
                wid = add_vmeter(w, "VMeter", true, xbutton->x, xbutton->y, 10, 120);
                set_controller_callbacks(designer, wid);
                add_to_list(designer, wid, "add_lv2_vmeter", false, IS_VMETER);
            break;
            case 10:
                wid = add_hmeter(w, "hMeter", true, xbutton->x, xbutton->y, 120, 10);
                set_controller_callbacks(designer, wid);
                add_to_list(designer, wid, "add_lv2_hmeter", false, IS_HMETER);
            break;
            default:
            break;
        }
        if (designer->controls[designer->active_widget_num].have_adjustment) {
            widget_show_all(designer->controller_settings);
            box_entry_set_text(designer->controller_entry[0], active_widget->adj->min_value);
            box_entry_set_text(designer->controller_entry[1], active_widget->adj->max_value);
            box_entry_set_text(designer->controller_entry[2], active_widget->adj->std_value);
            box_entry_set_text(designer->controller_entry[3], active_widget->adj->step);

        } else {
            widget_hide(designer->controller_settings);
        }
        if (designer->controls[designer->active_widget_num].is_type == IS_COMBOBOX) {
            widget_show_all(designer->combobox_settings);
        } else {
            widget_hide(designer->combobox_settings);
        }
    }
}

static void reset_plugin_ui(XUiDesigner *designer) {
    widget_set_title(designer->ui, "");
    designer->ui->width = 600;
    designer->ui->height = 400;
    XResizeWindow(designer->ui->app->dpy, designer->ui->widget, designer->ui->width, designer->ui->height);
    int ch = childlist_has_child(designer->ui->childlist);
    if (ch) {
        for(;ch>0;ch--) {
            remove_from_list(designer, designer->ui->childlist->childs[ch-1]);
            destroy_widget(designer->ui->childlist->childs[ch-1],designer->ui->app);
        }
    }
    int i = 0;
    for (;i<designer->wid_counter-1; i++) {
        free(designer->new_label[i]);
    }
    free(designer->new_label);
    designer->new_label = NULL;
    designer->new_label = (char **)realloc(designer->new_label, (MAX_CONTROLS) * sizeof(char *));

    designer->modify_mod = XUI_NONE;
    active_widget = NULL;
    entry_set_text(designer, "");
    adj_set_value(x_axis->adj, 0.0);
    adj_set_value(y_axis->adj, 0.0);
    adj_set_value(w_axis->adj, 10.0);
    adj_set_value(h_axis->adj, 10.0);
    widget_hide(designer->combobox_settings);
    widget_hide(designer->controller_settings);
    XResizeWindow(designer->ui->app->dpy, designer->ui->widget, designer->ui->width, designer->ui->height-1);

    adj_set_value(designer->index->adj,0.0);
    designer->wid_counter = 0;
    designer->active_widget_num = 0;
}

/*---------------------------------------------------------------------
-----------------------------------------------------------------------    
                lv2 ttl handling
-----------------------------------------------------------------------
----------------------------------------------------------------------*/

int sort_enums(int elem, int array[], int size) {
    int i = 0;
    for(;i<size;i++) {
        if(array[i] == elem) {
            return i; 
        }
    }
    return -1; 
}

void load_plugin_ui(void* w_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    Widget_t * wid = NULL;
    XUiDesigner *designer = (XUiDesigner*)w->parent_struct;
    reset_plugin_ui(designer);
    designer->lv2c.is_enum_port = false;
    designer->lv2c.is_toggle_port = false;
    designer->lv2c.have_adjustment = false;    
    designer->lv2c.is_trigger_port = false;
    designer->lv2c.is_log_port = false;    
    int x = 40;
    int y = 40;
    int x1 = 40;
    int y1 = 40;
    int v = (int)adj_get_value(w->adj);
    if (v) {
        LilvNode* lv2_AudioPort = (lilv_new_uri(designer->world, LV2_CORE__AudioPort));
        LilvNode* lv2_ControlPort = (lilv_new_uri(designer->world, LV2_CORE__ControlPort));
        LilvNode* lv2_InputPort = (lilv_new_uri(designer->world, LV2_CORE__InputPort));
        LilvNode* lv2_OutputPort = (lilv_new_uri(designer->world, LV2_CORE__OutputPort));
        LilvNode* lv2_AtomPort = (lilv_new_uri(designer->world, LV2_ATOM__AtomPort));
        LilvNode* lv2_CVPort = (lilv_new_uri(designer->world, LV2_CORE__CVPort));
        LilvNode* is_int = lilv_new_uri(designer->world, LV2_CORE__integer);
        LilvNode* is_tog = lilv_new_uri(designer->world, LV2_CORE__toggled);
        LilvNode* is_enum = lilv_new_uri(designer->world, LV2_CORE__enumeration);

        LilvNode* notOnGui = lilv_new_uri(designer->world, LV2_PORT_PROPS__notOnGUI);
        LilvNode* is_log = lilv_new_uri(designer->world, LV2_PORT_PROPS__logarithmic);
        LilvNode* has_step = lilv_new_uri(designer->world, LV2_PORT_PROPS__rangeSteps);
        LilvNode* is_trigger = lilv_new_uri(designer->world, LV2_PORT_PROPS__trigger);

        const LilvNode* uri = lilv_new_uri(designer->world, w->label);
        const LilvPlugin* plugin = lilv_plugins_get_by_uri(designer->lv2_plugins, uri);
        if (plugin) {
            LilvNode* nd = NULL;
            //const LilvNode* uri = lilv_plugin_get_uri(plugin);
            nd = lilv_plugin_get_name(plugin);
            if (nd) {
                widget_set_title(designer->ui, lilv_node_as_string(nd));
            }
            int n_in = 0;
            int n_out = 0;
            int n_atoms = 0;
            int n_cv = 0;
            int n_gui = 0;
            lilv_node_free(nd);
            unsigned int num_ports = lilv_plugin_get_num_ports(plugin);
            for (unsigned int n = 0; n < num_ports; n++) {
                if (designer->wid_counter >= MAX_CONTROLS) {
                    Widget_t *dia = open_message_dialog(designer->ui, INFO_BOX, _("INFO"),
                                                    _("MAX CONTROL COUNTER OVERFLOW"),NULL);
                    XSetTransientForHint(w->app->dpy, dia->widget, designer->ui->widget);
                    break;
                }

                const LilvPort* port = lilv_plugin_get_port_by_index(plugin, n);
                if (lilv_port_is_a(plugin, port, lv2_AudioPort)) {
                    if (lilv_port_is_a(plugin, port, lv2_InputPort)) {
                        n_in++;
                    } else {
                        n_out++;
                    }
                    continue;
                } else if (lilv_port_is_a(plugin, port, lv2_CVPort)) {
                    n_cv++;
                    continue;
                } else if (lilv_port_is_a(plugin, port, lv2_AtomPort)) {
                    n_atoms++;
                    continue;
                } else if (lilv_port_has_property(plugin, port, notOnGui)) {
                    n_gui++;
                    continue;
                } else if (lilv_port_is_a(plugin, port, lv2_ControlPort)) {
                    LilvNode* nm = lilv_port_get_name(plugin, port);
                    designer->lv2c.Port_Index = n;
                    designer->lv2c.name = lilv_node_as_string(nm);
                    asprintf (&designer->new_label[designer->active_widget_num], "%s",lilv_node_as_string(nm));
                    lilv_node_free(nm);
                    if (lilv_port_is_a(plugin, port, lv2_InputPort)) {
                        designer->lv2c.is_input_port = true;
                        designer->lv2c.is_output_port = false;
                    } else if (lilv_port_is_a(plugin, port, lv2_OutputPort)) {
                        designer->lv2c.is_input_port = false;
                        designer->lv2c.is_output_port = true;
                    }

                    LilvNode *pdflt, *pmin, *pmax;
                    lilv_port_get_range(plugin, port, &pdflt, &pmin, &pmax);
                    if (pmin) {
                        designer->lv2c.min = lilv_node_as_float(pmin);
                        lilv_node_free(pmin);
                    }
                    if (pmax) {
                        designer->lv2c.max = lilv_node_as_float(pmax);
                        lilv_node_free(pmax);
                    }
                    if (pdflt) {
                        designer->lv2c.def = lilv_node_as_float(pdflt);
                        lilv_node_free(pdflt);
                    }

                    if (lilv_port_has_property(plugin, port, is_int)) {
                        designer->lv2c.is_int_port = true;
                    } else {
                        designer->lv2c.is_int_port = false;
                    }

                    if (lilv_port_has_property(plugin, port, is_tog)) {
                        designer->lv2c.is_toggle_port = true;
                    } else {
                        designer->lv2c.is_toggle_port = false;
                    }

                    if (lilv_port_has_property(plugin, port, is_enum)) {
                        LilvScalePoints* sp = lilv_port_get_scale_points(plugin, port);
                        int num_sp = lilv_scale_points_size(sp);
                        if (num_sp > 0) {
                            designer->lv2c.is_enum_port = true;
                        } else {
                            designer->lv2c.is_enum_port = false;
                        }
                    } else {
                        designer->lv2c.is_enum_port = false;
                    }

                    if (lilv_port_has_property(plugin, port, is_trigger)) {
                        designer->lv2c.is_trigger_port = true;
                    } else {
                        designer->lv2c.is_trigger_port = false;
                    }

                    if (lilv_port_has_property(plugin, port, is_log)) {
                        designer->lv2c.is_log_port = true;
                    } else {
                        designer->lv2c.is_log_port = false;
                    }
                }

                if (designer->lv2c.is_input_port) {
                    if (designer->lv2c.is_toggle_port) {
                        if (x+70 >= 1200) {
                            y += 130;
                            y1 += 130;
                            x = 40;
                        } else {
                            x1 += 80;
                        }
                        wid = add_toggle_button(designer->ui, designer->new_label[designer->active_widget_num], x, y, 60, 60);
                        set_controller_callbacks(designer, wid);
                        add_to_list(designer, wid, "add_lv2_toggle_button", false, IS_TOGGLE_BUTTON);
                        designer->controls[designer->active_widget_num].port_index = designer->lv2c.Port_Index;
                        x += 80;
                    } else if (designer->lv2c.is_trigger_port) {
                        if (x+70 >= 1200) {
                            y += 130;
                            y1 += 130;
                            x = 40;
                        } else {
                            x1 += 80;
                        }
                        wid = add_button(designer->ui, designer->new_label[designer->active_widget_num], x, y, 60, 60);
                        set_controller_callbacks(designer, wid);
                        add_to_list(designer, wid, "add_lv2_button", false, IS_BUTTON);
                        designer->controls[designer->active_widget_num].port_index = designer->lv2c.Port_Index;
                        x += 80;
                    } else if (designer->lv2c.is_enum_port) {
                        if (x+130 >= 1200) {
                            y += 130;
                            y1 += 130;
                            x = 40;
                        } else {
                            x1 += 140;
                        }
                        wid = add_combobox(designer->ui, designer->new_label[designer->active_widget_num], x, y, 120, 30);
                        set_controller_callbacks(designer, wid);
                        
                        LilvScalePoints* sp = lilv_port_get_scale_points(plugin, port);
                        int num_sp = lilv_scale_points_size(sp);
                        int sp_count = 0;
                        int sppos[num_sp];
                        char splabes[num_sp][32];
                        if (num_sp > 0) {
                            for (LilvIter* it = lilv_scale_points_begin(sp);
                                    !lilv_scale_points_is_end(sp, it);
                                    it = lilv_scale_points_next(sp, it)) {
                                const LilvScalePoint* p = lilv_scale_points_get(sp, it);
                                utf8ncpy(&splabes[sp_count][0], lilv_node_as_string(lilv_scale_point_get_label(p)), 31);
                                sppos[sp_count] = lilv_node_as_float(lilv_scale_point_get_value(p));
                                sp_count++;
                            }
                            int i = designer->lv2c.min;
                            char s[32];
                            for (;i<designer->lv2c.max+1;i++) {
                                int j = sort_enums(i,sppos,num_sp);
                                if (j>-1) {
                                    combobox_add_entry(wid,&splabes[j][0]);
                                } else {
                                    snprintf(s, 31,"%d",  i);
                                    combobox_add_entry(wid,s);
                                }
                            }
                            lilv_scale_points_free(sp);
                        }

                        set_adjustment(wid->adj, designer->lv2c.def, designer->lv2c.def, designer->lv2c.min,
                                                designer->lv2c.max, designer->lv2c.is_int_port? 1.0:0.01, CL_ENUM);
                        add_to_list(designer, wid, "add_lv2_combobox", true, IS_COMBOBOX);
                        designer->controls[designer->active_widget_num].port_index = designer->lv2c.Port_Index;
                        x += 140;
                    } else {
                        if (x+70 >= 1200) {
                            y += 130;
                            y1 += 130;
                            x = 40;
                        } else {
                            x1 += 80;
                        }
                        wid = add_knob(designer->ui, designer->new_label[designer->active_widget_num], x, y, 60, 80);
                        set_adjustment(wid->adj, designer->lv2c.def, designer->lv2c.def, designer->lv2c.min,
                            designer->lv2c.max, designer->lv2c.is_int_port? 1:0.01,designer->lv2c.is_log_port? CL_LOGARITHMIC:CL_CONTINUOS);
                        set_controller_callbacks(designer, wid);
                        add_to_list(designer, wid, "add_lv2_knob", true, IS_KNOB);
                        designer->controls[designer->active_widget_num].port_index = designer->lv2c.Port_Index;
                        x += 80;
                    }
                } else if (designer->lv2c.is_output_port) {
                    if (x+20 >= 1200) {
                        y += 130;
                        y1 += 130;
                        x = 40;
                    } else {
                        x1 += 30;
                    }
                    wid = add_vmeter(designer->ui, designer->new_label[designer->active_widget_num], false, x, y, 10, 120);
                    set_adjustment(wid->adj, designer->lv2c.def, designer->lv2c.def, designer->lv2c.min, designer->lv2c.max,
                        designer->lv2c.is_int_port? 1:0.01, designer->lv2c.is_log_port? CL_LOGARITHMIC:CL_METER);
                    set_controller_callbacks(designer, wid);
                    add_to_list(designer, wid, "add_lv2_vmeter", true, IS_VMETER);
                    designer->controls[designer->active_widget_num].port_index = designer->lv2c.Port_Index;
                    x += 30;
                }
            }
            designer->ui->width = min(1200,x1);
            designer->ui->height = min(600,y1+130);
            XResizeWindow(designer->ui->app->dpy, designer->ui->widget, designer->ui->width, designer->ui->height);
        }
        lilv_node_free(lv2_AudioPort);
        lilv_node_free(lv2_ControlPort);
        lilv_node_free(lv2_InputPort);
        lilv_node_free(lv2_OutputPort);
        lilv_node_free(lv2_AtomPort);
        lilv_node_free(lv2_CVPort);
        lilv_node_free(is_int);
        lilv_node_free(is_tog);
        lilv_node_free(is_enum);

        lilv_node_free(notOnGui);
        lilv_node_free(is_log);
        lilv_node_free(has_step);
        lilv_node_free(is_trigger);
    }
    widget_show_all(designer->ui);
}

void load_uris(Widget_t *lv2_uris, const LilvPlugins* lv2_plugins) {
    for (LilvIter* it = lilv_plugins_begin(lv2_plugins);
      !lilv_plugins_is_end(lv2_plugins, it);
      it = lilv_plugins_next(lv2_plugins, it)) {
        const LilvPlugin* plugin = lilv_plugins_get(lv2_plugins, it);
        if (plugin) {
            const LilvNode* uri = lilv_plugin_get_uri(plugin);
            combobox_add_entry(lv2_uris,lilv_node_as_string(uri));
        }
    }
}

void set_path(LilvWorld* world, const char* workdir) {
    char lwd[PATH_MAX];
    memset(lwd,0,PATH_MAX*sizeof(lwd[0]));
    strcat(lwd,"file://");
    strcat(lwd,workdir);
    LilvNode* path = lilv_new_string(world, lwd);
    lilv_world_set_option(world, LILV_OPTION_LV2_PATH, path);
    lilv_node_free(path);
}

/*---------------------------------------------------------------------
-----------------------------------------------------------------------    
                main
-----------------------------------------------------------------------
----------------------------------------------------------------------*/

int main (int argc, char ** argv) {

    extern char *optarg;
    char *path = NULL;
    int a = 0;
    static char usage[] = "usage: %s \n"
    "[-p path] optional set a path to open a ttl file from\n";

    while ((a = getopt(argc, argv, "p:h?")) != -1) {
        switch (a) {
            break;
            case 'p': path = optarg;
            break;
            case 'h':
            case '?': fprintf(stderr, usage, argv[0]);
                exit(1);
            break;
        }
    }

    XUiDesigner *designer = (XUiDesigner*)malloc(sizeof(XUiDesigner));
    designer->modify_mod = XUI_NONE;
    designer->select_widget_num = 0;
    designer->active_widget_num = 0;
    active_widget = NULL;
    designer->wid_counter = 0;

    Xputty app;
    main_init(&app);
    designer->new_label = NULL;
    designer->new_label = (char **)realloc(designer->new_label, (MAX_CONTROLS) * sizeof(char *));
    int m = 0;
    for (;m<MAX_CONTROLS;m++) designer->controls[m].wid = NULL;
    //set_light_theme(&app);
    designer->w = create_window(&app, DefaultRootWindow(app.dpy), 0, 0, 1200, 800);
    widget_set_title(designer->w, "XUiDesigner");
    designer->w->func.expose_callback = draw_window;

    designer->world = lilv_world_new();
    if (path !=NULL) set_path(designer->world, path);
    lilv_world_load_all(designer->world);
    designer->lv2_plugins = lilv_world_get_all_plugins(designer->world);
    
    designer->lv2_uris = add_combobox(designer->w, "", 300, 25, 600, 30);
    designer->lv2_uris->parent_struct = designer;
    combobox_add_entry(designer->lv2_uris,_("--"));
    load_uris(designer->lv2_uris, designer->lv2_plugins);
    combobox_set_active_entry(designer->lv2_uris, 0);
    designer->lv2_uris->func.value_changed_callback = load_plugin_ui;
    

    designer->ui = create_window(&app, DefaultRootWindow(app.dpy), 0, 0, 600, 400);
    Atom wmStateAbove = XInternAtom(app.dpy, "_NET_WM_STATE_ABOVE", 1 );
    Atom wmNetWmState = XInternAtom(app.dpy, "_NET_WM_STATE", 1 );
    XChangeProperty(app.dpy, designer->ui->widget, wmNetWmState, XA_ATOM, 32, 
        PropModeReplace, (unsigned char *) &wmStateAbove, 1); 
    XSetTransientForHint(app.dpy, designer->ui->widget, designer->w->widget);
    designer->ui->parent_struct = designer;
    designer->ui->func.expose_callback = draw_window;
    designer->ui->func.button_release_callback = button_released_callback;

    designer->widgets = add_combobox(designer->w, "", 20, 25, 120, 30);
    designer->widgets->scale.gravity = SOUTHWEST;
    designer->widgets->flags |= NO_PROPAGATE;
    designer->widgets->parent_struct = designer;
    combobox_add_entry(designer->widgets,_("--"));
    combobox_add_entry(designer->widgets,_("Knob"));
    combobox_add_entry(designer->widgets,_("HSlider"));
    combobox_add_entry(designer->widgets,_("VSlider"));
    combobox_add_entry(designer->widgets,_("Button"));
    combobox_add_entry(designer->widgets,_("Toggle Button"));
    combobox_add_entry(designer->widgets,_("Combo Box"));
    combobox_add_entry(designer->widgets,_("Value Display"));
    combobox_add_entry(designer->widgets,_("Label"));
    combobox_add_entry(designer->widgets,_("VMeter"));
    combobox_add_entry(designer->widgets,_("HMeter"));
    combobox_set_active_entry(designer->widgets, 0);
    designer->widgets->func.value_changed_callback = set_widget_callback;

    add_label(designer->w, "Label", 1000, 10, 180, 30);

    designer->controller_label = create_widget(&app, designer->w, 1000, 50, 180, 30);
    memset(designer->controller_label->input_label, 0, 32 * (sizeof designer->controller_label->input_label[0]) );
    designer->controller_label->func.expose_callback = entry_add_text;
    designer->controller_label->func.key_press_callback = entry_get_text;
    designer->controller_label->flags &= ~USE_TRANSPARENCY;
    //designer->controller_label->scale.gravity = EASTWEST;
    designer->controller_label->parent_struct = designer;
    Cursor c = XCreateFontCursor(app.dpy, XC_xterm);
    XDefineCursor (app.dpy, designer->controller_label->widget, c);
    XFreeCursor(app.dpy, c);

    add_label(designer->w, "Port Index", 1000, 80, 180, 30);
    designer->index = add_combobox(designer->w, "", 1000, 120, 70, 30);
    designer->index->parent_struct = designer;
    combobox_add_numeric_entrys(designer->index, 0, MAX_CONTROLS);
    combobox_set_active_entry(designer->index, 0);
    designer->set_index = add_button(designer->w, "Set", 1090, 120, 60, 30);
    designer->set_index->parent_struct = designer;
    designer->set_index->func.value_changed_callback = set_port_index;

    add_label(designer->w, "Position/Size", 1000, 150, 180, 30);

    x_axis = add_hslider(designer->w, "X", 1000, 200, 180, 30);
    x_axis->parent_struct = designer;
    set_adjustment(x_axis->adj,0.0, 0.0, 0.0, 1200.0, 1.0, CL_CONTINUOS);
    x_axis->func.value_changed_callback = set_x_axis_callback;

    y_axis = add_hslider(designer->w, "Y", 1000, 240, 180, 30);
    y_axis->parent_struct = designer;
    set_adjustment(y_axis->adj,0.0, 0.0, 0.0, 600.0, 1.0, CL_CONTINUOS);
    y_axis->func.value_changed_callback = set_y_axis_callback;

    w_axis = add_hslider(designer->w, "Width", 1000, 280, 180, 30);
    w_axis->parent_struct = designer;
    set_adjustment(w_axis->adj,10.0, 10.0, 10.0, 300.0, 1.0, CL_CONTINUOS);
    w_axis->func.value_changed_callback = set_w_axis_callback;

    h_axis = add_hslider(designer->w, "Height", 1000, 320, 180, 30);
    h_axis->parent_struct = designer;
    set_adjustment(h_axis->adj,10.0, 10.0, 10.0, 300.0, 1.0, CL_CONTINUOS);
    h_axis->func.value_changed_callback = set_h_axis_callback;

    widget_show_all(designer->w);

    designer->combobox_settings = create_widget(&app, designer->w, 1000, 360, 180, 200);
    add_label(designer->combobox_settings, "Add Combobox Entry", 0, 0, 180, 30);
    designer->combobox_entry = add_input_box(designer->combobox_settings, 0, 0, 40, 140, 30);
    designer->combobox_entry->parent_struct = designer;
    designer->combobox_entry->func.user_callback = set_combobox_entry;

    designer->add_entry = add_button(designer->combobox_settings, "Add", 140, 40, 40, 30);
    designer->add_entry->parent_struct = designer;
    designer->add_entry->func.value_changed_callback = add_combobox_entry;

    designer->controller_settings = create_widget(&app, designer->w, 1000, 360, 180, 250);
    add_label(designer->controller_settings, "Controller Settings", 0, 0, 180, 30);
    const char* labels[4] = { "Min","Max","Default", "Step Size"};
    int k = 0;
    for (;k<4;k++) {
        add_label(designer->controller_settings, labels[k], 0, (k+1)*40, 90, 30);
        designer->controller_entry[k] = add_input_box(designer->controller_settings, 1, 100, (k+1)*40, 60, 30);
        designer->controller_entry[k]->parent_struct = designer;
    }

    designer->set_adjust = add_button(designer->controller_settings, "Set", 100, 200, 60, 30);
    designer->set_adjust->parent_struct = designer;
    designer->set_adjust->func.value_changed_callback = set_controller_adjustment;

    widget_show_all(designer->ui);
    main_run(&app);

    lilv_world_free(designer->world);
    print_list(designer);
    main_quit(&app);
    int i = 0;
    for (;i<designer->wid_counter-1; i++) {
        free(designer->new_label[i]);
    }
    free(designer->new_label);
    free(designer);

    return 0;
}
    
