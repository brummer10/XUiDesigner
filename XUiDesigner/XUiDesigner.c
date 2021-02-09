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

#include "XUiDesigner.h"
#include "XUiGenerator.h"
#include "XUiColorChooser.h"
#include "XUiLv2Parser.h"
#include "XUiImageLoader.h"


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

void entry_set_text(XUiDesigner *designer, const char* label) {
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
            if (designer->active_widget != NULL) {
                if (strlen(w->input_label)) {
                    asprintf (&designer->new_label[designer->active_widget_num], "%s", w->input_label);
                    designer->new_label[designer->active_widget_num][strlen( w->input_label)-1] = 0;
                    designer->active_widget->label = (const char*)designer->new_label[designer->active_widget_num];
                    expose_widget(designer->active_widget);
                }
            }
            return;
        }
        if(status == XLookupChars || status == XLookupBoth){
            entry_add_text(w, buf);
            if (designer->active_widget != NULL) {
                if (strlen(w->input_label)) {
                    asprintf (&designer->new_label[designer->active_widget_num], "%s", w->input_label);
                    designer->new_label[designer->active_widget_num][strlen( w->input_label)-1] = 0;
                    designer->active_widget->label = (const char*)designer->new_label[designer->active_widget_num];
                    expose_widget(designer->active_widget);
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

static void draw_ui(void *w_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    XWindowAttributes attrs;
    XGetWindowAttributes(w->app->dpy, (Window)w->widget, &attrs);
    int width = attrs.width;
    int height = attrs.height;
    XUiDesigner *designer = (XUiDesigner*)w->parent_struct;
    use_bg_color_scheme(w, NORMAL_);
    cairo_paint (w->crb);
    cairo_paint (w->cr);
    if (w->image) {
        widget_set_scale(w);
        cairo_set_source_surface (w->crb, w->image, 0, 0);
        cairo_paint (w->crb);
        widget_reset_scale(w);
    }
    if (designer->grid_view) {
        cairo_set_line_width(w->crb, 1.0);
        use_frame_color_scheme(w, INSENSITIVE_);
        int i = 0;
        cairo_move_to(w->crb, 0, 0);
        for (;i<width;i +=designer->grid_width) {
            cairo_move_to(w->crb, i, 0);
            cairo_line_to(w->crb,i, height);
            cairo_stroke_preserve(w->crb);
        }
        i = 0;
        cairo_move_to(w->crb, 0, 0);
        for (;i<height;i +=designer->grid_height) {
            cairo_move_to(w->crb, 0, i);
            cairo_line_to(w->crb,width, i);
            cairo_stroke_preserve(w->crb);
        }
        cairo_stroke(w->crb);
    }
}

static void set_port_index(void *w_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    XUiDesigner *designer = (XUiDesigner*)w->parent_struct;
    if (w->flags & HAS_POINTER && !adj_get_value(w->adj_y)) {
        if (designer->active_widget != NULL) {
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
                designer->active_widget->adj->min_value = atof(designer->controller_entry[0]->input_label);
                strcat(designer->controller_entry[0]->input_label, "|");
            }
            if (strlen(designer->controller_entry[1]->input_label)>1) {
                designer->controller_entry[1]->input_label[strlen(designer->controller_entry[1]->input_label)-1] = 0;
                designer->active_widget->adj->max_value = atof(designer->controller_entry[1]->input_label);
                strcat(designer->controller_entry[1]->input_label, "|");
            }
            if (strlen(designer->controller_entry[2]->input_label)>1) {
                designer->controller_entry[2]->input_label[strlen(designer->controller_entry[2]->input_label)-1] = 0;
                designer->active_widget->adj->value = atof(designer->controller_entry[2]->input_label);
                designer->active_widget->adj->std_value = atof(designer->controller_entry[2]->input_label);
                strcat(designer->controller_entry[2]->input_label, "|");
            }
            if (strlen(designer->controller_entry[3]->input_label)>1) {
                designer->controller_entry[3]->input_label[strlen(designer->controller_entry[3]->input_label)-1] = 0;
                designer->active_widget->adj->step = atof(designer->controller_entry[3]->input_label);
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
    XUiDesigner *designer = (XUiDesigner*)w->parent_struct;
    int v = (int)adj_get_value(w->adj);
    if (designer->active_widget != NULL)
        XMoveWindow(designer->active_widget->app->dpy,designer->active_widget->widget,v, (int)adj_get_value(designer->y_axis->adj));
}

static void set_y_axis_callback(void *w_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    XUiDesigner *designer = (XUiDesigner*)w->parent_struct;
    int v = (int)adj_get_value(w->adj);
    if (designer->active_widget != NULL)
        XMoveWindow(designer->active_widget->app->dpy,designer->active_widget->widget, (int)adj_get_value(designer->x_axis->adj), v);
}

static void set_w_axis_callback(void *w_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    XUiDesigner *designer = (XUiDesigner*)w->parent_struct;
    int v = (int)adj_get_value(w->adj);
    if (designer->active_widget != NULL)
        XResizeWindow(designer->active_widget->app->dpy,designer->active_widget->widget, v, (int)adj_get_value(designer->h_axis->adj));
}

static void set_h_axis_callback(void *w_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    XUiDesigner *designer = (XUiDesigner*)w->parent_struct;
    int v = (int)adj_get_value(w->adj);
    if (designer->active_widget != NULL)
        XResizeWindow(designer->active_widget->app->dpy,designer->active_widget->widget, (int)adj_get_value(designer->w_axis->adj), v);
}

static void move_wid(void *w_, void *xmotion_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    Widget_t *p = (Widget_t*)w->parent;
    XUiDesigner *designer = (XUiDesigner*)p->parent_struct;
    XMotionEvent *xmotion = (XMotionEvent*)xmotion_;
    static int is_curser = 2;
    switch(designer->modify_mod) {
        case XUI_POSITION:
        {
            if ((xmotion->state & Button1Mask) == 0) break;
            int pos_x = w->x + (xmotion->x_root-designer->pos_x);
            int pos_y = w->y + (xmotion->y_root-designer->pos_y);
            int pos_width = w->width;
            int snap_grid_x = pos_x/designer->grid_width;
            int snap_grid_y = pos_y/designer->grid_height;
            if (designer->grid_view) {
                pos_x = snap_grid_x * designer->grid_width;
                pos_y = snap_grid_y * designer->grid_height;
                if (designer->controls[w->data].grid_snap_option == 1) {
                    for (;pos_width > designer->grid_width; pos_width -=designer->grid_width);
                    if (w->width > designer->grid_width) {
                        pos_x += designer->grid_width - pos_width/2;
                    } else {
                        pos_x += designer->grid_width - pos_width * 2;
                    }
                } else if (designer->controls[w->data].grid_snap_option == 2) {
                    for (;pos_width > designer->grid_width; pos_width -=designer->grid_width);
                    pos_x += designer->grid_width - pos_width;
                }
            }
            XMoveWindow(w->app->dpy,w->widget,pos_x, pos_y);
            adj_set_value(designer->x_axis->adj, pos_x);
            adj_set_value(designer->y_axis->adj, pos_y);
        }
        break;
        case XUI_SIZE:
            XResizeWindow(w->app->dpy, w->widget, max(10,w->width + (xmotion->x_root-designer->pos_x)), max(10,w->height+ (xmotion->x_root-designer->pos_x)));
            adj_set_value(designer->w_axis->adj, w->width + ((xmotion->x_root-designer->pos_x)));
            adj_set_value(designer->h_axis->adj, w->height+ ((xmotion->x_root-designer->pos_x)));
            w->scale.ascale = 1.0;
            designer->pos_x = xmotion->x_root;
        break;
        case XUI_WIDTH:
            XResizeWindow(w->app->dpy, w->widget, max(10,w->width + (xmotion->x_root-designer->pos_x)), w->height);
            adj_set_value(designer->w_axis->adj, w->width + ((xmotion->x_root-designer->pos_x)));
            w->scale.ascale = 1.0;
            designer->pos_x = xmotion->x_root;
        break;
        case XUI_HEIGHT:
            XResizeWindow(w->app->dpy, w->widget, w->width, max(10,w->height + (xmotion->y_root-designer->pos_y)));
            adj_set_value(designer->h_axis->adj, w->height + ((xmotion->y_root-designer->pos_y)));
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

static void draw_frame(void *w_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    Widget_t *p = (Widget_t*)w->parent;
    XUiDesigner *designer = (XUiDesigner*)p->parent_struct;
    if (w->data == designer->active_widget_num) {
        XWindowAttributes attrs;
        XGetWindowAttributes(w->app->dpy, (Window)w->widget, &attrs);
        int width = attrs.width;
        int height = attrs.height;
        use_frame_color_scheme(w, ACTIVE_);
        cairo_rectangle(w->cr, 0, 0, width, height);
        cairo_stroke(w->cr);
    }
}

static void draw_trans(void *w_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    transparent_draw(w, NULL);
    draw_frame(w, NULL);
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
        designer->active_widget_num = -1;
        if (designer->prev_active_widget != NULL)
            draw_trans(designer->prev_active_widget,NULL);
        designer->active_widget = (Widget_t*)w_;
        designer->active_widget_num = w->data;
        designer->pos_x = xbutton->x_root;
        designer->pos_y = xbutton->y_root;
        entry_set_text(designer, w->label);
        adj_set_value(designer->x_axis->adj, w->x);
        adj_set_value(designer->y_axis->adj, w->y);
        adj_set_value(designer->w_axis->adj, w->width);
        adj_set_value(designer->h_axis->adj, w->height);
        adj_set_value(designer->index->adj, 
            (float)designer->controls[designer->active_widget_num].port_index);

        if (designer->controls[designer->active_widget_num].have_adjustment  &&
                designer->controls[designer->active_widget_num].is_type != IS_COMBOBOX) {
            widget_show_all(designer->controller_settings);
            box_entry_set_text(designer->controller_entry[0], designer->active_widget->adj->min_value);
            box_entry_set_text(designer->controller_entry[1], designer->active_widget->adj->max_value);
            box_entry_set_text(designer->controller_entry[2], designer->active_widget->adj->std_value);
            box_entry_set_text(designer->controller_entry[3], designer->active_widget->adj->step);

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
    draw_frame(designer->active_widget, NULL);
    designer->prev_active_widget = w;
}

static void null_callback(void *w_, void* user_data) {
    
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
        designer->active_widget = (Widget_t*)w_;
        designer->active_widget_num = w->data;
    } else if(xbutton->button == Button3) {
        designer->modify_mod = XUI_NONE;
        designer->active_widget = (Widget_t*)w_;
        designer->active_widget_num = w->data;
        if (designer->controls[designer->active_widget_num].is_type == IS_COMBOBOX ||
            designer->controls[designer->active_widget_num].is_type == IS_VALUE_DISPLAY ||
            designer->controls[designer->active_widget_num].is_type == IS_VMETER ||
            designer->controls[designer->active_widget_num].is_type == IS_HMETER ||
            designer->controls[designer->active_widget_num].is_type == IS_VSLIDER ||
            designer->controls[designer->active_widget_num].is_type == IS_HSLIDER) {
            designer->menu_item_load->state = 4;
            designer->menu_item_unload->state = 4;
        } else {
            designer->menu_item_load->state = 0;
            designer->menu_item_unload->state = 0;
        }
        xevfunc store = designer->grid_snap_select->func.value_changed_callback;
        designer->grid_snap_select->func.value_changed_callback = null_callback;
        radio_item_set_active(designer->controls[w->data].grid_snap_option == 0 ?
            designer->grid_snap_left : designer->controls[w->data].grid_snap_option == 1 ?
            designer->grid_snap_center : designer->grid_snap_right);
        designer->grid_snap_select->func.value_changed_callback = store;
        int sel = designer->controls[designer->active_widget_num].is_type;
        store = designer->ctype_switch->func.value_changed_callback;
        designer->ctype_switch->func.value_changed_callback = null_callback;
        set_active_radio_entry_num(designer->ctype_switch, sel);
        designer->ctype_switch->func.value_changed_callback = store;
        pop_menu_show(w,designer->context_menu,5,true);
    }
    draw_frame(designer->active_widget, NULL);
}

static void set_designer_callbacks(XUiDesigner *designer, Widget_t* wid) {
    entry_set_text(designer, wid->label);
    xevfunc store = designer->x_axis->func.value_changed_callback;
    designer->x_axis->func.value_changed_callback = null_callback;
    adj_set_value(designer->x_axis->adj, (float)wid->x);
    designer->x_axis->func.value_changed_callback = store;
    store = designer->y_axis->func.value_changed_callback;
    designer->y_axis->func.value_changed_callback = null_callback;
    adj_set_value(designer->y_axis->adj, (float)wid->y);
    designer->y_axis->func.value_changed_callback = store;
    store = designer->w_axis->func.value_changed_callback;
    designer->w_axis->func.value_changed_callback = null_callback;
    adj_set_value(designer->w_axis->adj, (float)wid->width);
    designer->w_axis->func.value_changed_callback = store;
    store = designer->h_axis->func.value_changed_callback;
    designer->h_axis->func.value_changed_callback = null_callback;
    adj_set_value(designer->h_axis->adj, (float)wid->height);
    designer->h_axis->func.value_changed_callback = store;
}

void set_controller_callbacks(XUiDesigner *designer, Widget_t *wid, bool set_designer) {
    XSelectInput(wid->app->dpy, wid->widget,StructureNotifyMask|ExposureMask|KeyPressMask 
                    |EnterWindowMask|LeaveWindowMask|ButtonReleaseMask
                    |ButtonPressMask|Button1MotionMask|PointerMotionMask);
    wid->data = designer->wid_counter;
    wid->scale.gravity = NONE;
    //wid->parent_struct = designer;
    wid->func.enter_callback = draw_trans;
    wid->func.leave_callback = draw_trans;
    wid->func.button_press_callback = set_pos_wid;
    wid->func.button_release_callback = fix_pos_wid;
    wid->func.motion_callback = move_wid;
    designer->active_widget_num = designer->wid_counter;
    designer->active_widget = wid;
    adj_set_value(designer->index->adj, adj_get_value(designer->index->adj)+1.0);
    designer->controls[designer->wid_counter].port_index = adj_get_value(designer->index->adj);
    if (set_designer)
        set_designer_callbacks(designer, wid);
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
                set_controller_callbacks(designer, wid, true);
                add_to_list(designer, wid, "add_lv2_knob", true, IS_KNOB);
            break;
            case 2:
                wid = add_hslider(w, "HSlider", xbutton->x, xbutton->y, 120, 30);
                set_controller_callbacks(designer, wid, true);
                add_to_list(designer, wid, "add_lv2_hslider", true, IS_HSLIDER);
            break;
            case 3:
                wid = add_vslider(w, "VSlider", xbutton->x, xbutton->y, 30, 120);
                set_controller_callbacks(designer, wid, true);
                add_to_list(designer, wid, "add_lv2_vslider", true, IS_VSLIDER);
            break;
            case 4:
                wid = add_button(w, "Button", xbutton->x, xbutton->y, 60, 60);
                set_controller_callbacks(designer, wid, true);
                add_to_list(designer, wid, "add_lv2_button", false, IS_BUTTON);
            break;
            case 5:
                wid = add_toggle_button(w, "Switch", xbutton->x, xbutton->y, 60, 60);
                set_controller_callbacks(designer, wid, true);
                add_to_list(designer, wid, "add_lv2_toggle_button", false, IS_TOGGLE_BUTTON);
            break;
            case 6:
                wid = add_combobox(w, "Combobox", xbutton->x, xbutton->y, 120, 30);
                set_controller_callbacks(designer, wid, true);
                add_to_list(designer, wid, "add_lv2_combobox", true, IS_COMBOBOX);
            break;
            case 7:
                wid = add_valuedisplay(w, "", xbutton->x, xbutton->y, 40, 30);
                set_controller_callbacks(designer, wid, true);
                add_to_list(designer, wid, "add_lv2_valuedisplay", true, IS_VALUE_DISPLAY);
            break;
            case 8:
                wid = add_label(w, "Label", xbutton->x, xbutton->y, 60, 30);
                set_controller_callbacks(designer, wid, true);
                add_to_list(designer, wid, "add_lv2_label", false, IS_LABEL);
            break;
            case 9:
                wid = add_vmeter(w, "VMeter", false, xbutton->x, xbutton->y, 10, 120);
                set_controller_callbacks(designer, wid, true);
                add_to_list(designer, wid, "add_lv2_vmeter", true, IS_VMETER);
            break;
            case 10:
                wid = add_hmeter(w, "hMeter", false, xbutton->x, xbutton->y, 120, 10);
                set_controller_callbacks(designer, wid, true);
                add_to_list(designer, wid, "add_lv2_hmeter", true, IS_HMETER);
            break;
            default:
            break;
        }
        if (designer->controls[designer->active_widget_num].have_adjustment &&
                designer->controls[designer->active_widget_num].is_type != IS_COMBOBOX) {
            widget_show_all(designer->controller_settings);
            box_entry_set_text(designer->controller_entry[0], designer->active_widget->adj->min_value);
            box_entry_set_text(designer->controller_entry[1], designer->active_widget->adj->max_value);
            box_entry_set_text(designer->controller_entry[2], designer->active_widget->adj->std_value);
            box_entry_set_text(designer->controller_entry[3], designer->active_widget->adj->step);

        } else {
            widget_hide(designer->controller_settings);
        }
        if (designer->controls[designer->active_widget_num].is_type == IS_COMBOBOX) {
            widget_show_all(designer->combobox_settings);
        } else {
            widget_hide(designer->combobox_settings);
        }
        if (designer->prev_active_widget != NULL)
            draw_trans(designer->prev_active_widget,NULL);
        designer->prev_active_widget = wid;
    }
}

/*---------------------------------------------------------------------
-----------------------------------------------------------------------    
                Grid control
-----------------------------------------------------------------------
----------------------------------------------------------------------*/

void snap_to_grid(XUiDesigner *designer) {
    int ch = childlist_has_child(designer->ui->childlist);
    if (ch) {
        for(;ch>0;ch--) {
            Widget_t *w = designer->ui->childlist->childs[ch-1];
            int pos_x = w->x ;
            int pos_y = w->y ;
            int snap_grid_x = pos_x/designer->grid_width;
            int snap_grid_y = pos_y/designer->grid_height;
            int pos_width = w->width;
            if (designer->grid_view) {
                pos_x = snap_grid_x * designer->grid_width;
                pos_y = snap_grid_y * designer->grid_height;
            }
            if (designer->controls[w->data].grid_snap_option == 1) {
                for (;pos_width > designer->grid_width; pos_width -=designer->grid_width);
                if (w->width > designer->grid_width) {
                    pos_x += designer->grid_width - pos_width/2;
                } else {
                    pos_x += designer->grid_width - pos_width * 2;
                }
            } else if (designer->controls[w->data].grid_snap_option == 2) {
                for (;pos_width > designer->grid_width; pos_width -= designer->grid_width);
                pos_x += designer->grid_width - pos_width;
            }
            XMoveWindow(w->app->dpy,w->widget,pos_x, pos_y);
            w->x = pos_x;
            w->y = pos_y;
            w->scale.init_x   = pos_x;
            w->scale.init_y   = pos_y;
        }
    }
}

static void set_grid_width(void *w_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    XUiDesigner *designer = (XUiDesigner*)w->parent_struct;
    designer->grid_width = (int)adj_get_value(w->adj);
    snap_to_grid(designer);
}

static void set_grid_height(void *w_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    XUiDesigner *designer = (XUiDesigner*)w->parent_struct;
    designer->grid_height = (int)adj_get_value(w->adj);
    snap_to_grid(designer);
}

static void use_grid(void *w_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    XUiDesigner *designer = (XUiDesigner*)w->parent_struct;
    designer->grid_view = (bool)adj_get_value(w->adj);
    if (designer->grid_view) {
        snap_to_grid(designer);
        widget_show(designer->grid_size_x);
        widget_show(designer->grid_size_y);
    } else {
        widget_hide(designer->grid_size_x);
        widget_hide(designer->grid_size_y);
    }
    expose_widget(designer->ui);
}

static void select_grid_mode(void *w_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    XUiDesigner *designer = (XUiDesigner*)w->parent_struct;
    int v = (int) adj_get_value(w->adj);
    switch (v) {
        case 0:
            designer->controls[designer->active_widget_num].grid_snap_option = 0;
            snap_to_grid(designer);
        break;
        case 1:
            designer->controls[designer->active_widget_num].grid_snap_option = 1;
            snap_to_grid(designer);
        break;
        case 2:
            designer->controls[designer->active_widget_num].grid_snap_option = 2;
            snap_to_grid(designer);
        break;
        default:
        break;
    }
}

/*---------------------------------------------------------------------
-----------------------------------------------------------------------    
                change controller type
-----------------------------------------------------------------------
----------------------------------------------------------------------*/

static void copy_widget_settings(XUiDesigner *designer, Widget_t *wid, Widget_t *new_wid) {
    if (wid->adj->type == CL_LOGARITHMIC) {
         set_adjustment(new_wid->adj, powf(10,wid->adj->std_value), powf(10,wid->adj->std_value),
            powf(10,wid->adj->min_value),powf(10,wid->adj->max_value), wid->adj->step, wid->adj->type);
    } else if (wid->adj->type == CL_LOGSCALE) {
        set_adjustment(new_wid->adj, log10(wid->adj->std_value)*wid->adj->log_scale,
                                    log10(wid->adj->std_value)*wid->adj->log_scale,
                                    log10(wid->adj->min_value)*wid->adj->log_scale,
                                    log10(wid->adj->max_value)*wid->adj->log_scale,
                                    wid->adj->step, wid->adj->type);
    } else {
        set_adjustment(new_wid->adj, wid->adj->std_value, wid->adj->std_value,
            wid->adj->min_value,wid->adj->max_value, wid->adj->step, wid->adj->type);
    }
    set_controller_callbacks(designer, new_wid, true);
    new_wid->data = wid->data;
    designer->wid_counter--;
}

static void switch_controller_type(void *w_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    XUiDesigner *designer = (XUiDesigner*)w->parent_struct;
    Widget_t *wid = designer->active_widget;
    Widget_t *new_wid = NULL;
    int v = (int) adj_get_value(w->adj);
    switch (v) {
        case 0:
            asprintf (&designer->new_label[designer->active_widget_num], "%s",wid->label);
            new_wid = add_knob(designer->ui, designer->new_label[designer->active_widget_num],
                                                                        wid->x, wid->y, 60, 80);
            copy_widget_settings(designer, wid, new_wid);
            new_wid->adj->type = new_wid->adj->type == CL_LOGARITHMIC ? CL_LOGARITHMIC :
                new_wid->adj->type == CL_LOGSCALE ? CL_LOGSCALE : CL_CONTINUOS;
            add_to_list(designer, new_wid, "add_lv2_knob", true, IS_KNOB);
            destroy_widget(wid, designer->w->app);
            designer->controls[new_wid->data].image = NULL;
            widget_show(new_wid);
            designer->active_widget = new_wid;
        break;
        case 1:
            asprintf (&designer->new_label[designer->active_widget_num], "%s",wid->label);
            new_wid = add_hslider(designer->ui, designer->new_label[designer->active_widget_num],
                                                                        wid->x, wid->y, 120, 30);
            copy_widget_settings(designer, wid, new_wid);
            new_wid->adj->type = new_wid->adj->type == CL_LOGARITHMIC ? CL_LOGARITHMIC :
                new_wid->adj->type == CL_LOGSCALE ? CL_LOGSCALE : CL_CONTINUOS;
            add_to_list(designer, new_wid, "add_lv2_hslider", true, IS_HSLIDER);
            destroy_widget(wid, designer->w->app);
            designer->controls[new_wid->data].image = NULL;
            widget_show(new_wid);
            designer->active_widget = new_wid;
        break;
        case 2:
            asprintf (&designer->new_label[designer->active_widget_num], "%s",wid->label);
            new_wid = add_vslider(designer->ui, designer->new_label[designer->active_widget_num],
                                                                        wid->x, wid->y, 30, 120);
            copy_widget_settings(designer, wid, new_wid);
            new_wid->adj->type = new_wid->adj->type == CL_LOGARITHMIC ? CL_LOGARITHMIC :
                new_wid->adj->type == CL_LOGSCALE ? CL_LOGSCALE : CL_CONTINUOS;
            add_to_list(designer, new_wid, "add_lv2_vslider", true, IS_VSLIDER);
            destroy_widget(wid, designer->w->app);
            designer->controls[new_wid->data].image = NULL;
            widget_show(new_wid);
            designer->active_widget = new_wid;
        break;
        case 3:
            asprintf (&designer->new_label[designer->active_widget_num], "%s",wid->label);
            new_wid = add_button(designer->ui, designer->new_label[designer->active_widget_num],
                                                                        wid->x, wid->y, 60, 60);
            set_controller_callbacks(designer, new_wid, true);
            new_wid->data = wid->data;
            designer->wid_counter--;
            add_to_list(designer, new_wid, "add_lv2_button", false, IS_BUTTON);
            destroy_widget(wid, designer->w->app);
            designer->controls[new_wid->data].image = NULL;
            widget_show(new_wid);
            designer->active_widget = new_wid;
        break;
        case 4:
            asprintf (&designer->new_label[designer->active_widget_num], "%s",wid->label);
            new_wid = add_toggle_button(designer->ui, designer->new_label[designer->active_widget_num],
                                                                        wid->x, wid->y, 60, 60);
            set_controller_callbacks(designer, new_wid, true);
            new_wid->data = wid->data;
            designer->wid_counter--;
            add_to_list(designer, new_wid, "add_lv2_toggle_button", false, IS_TOGGLE_BUTTON);
            destroy_widget(wid, designer->w->app);
            designer->controls[new_wid->data].image = NULL;
            widget_show(new_wid);
            designer->active_widget = new_wid;
        break;
        case 5:
            asprintf (&designer->new_label[designer->active_widget_num], "%s",wid->label);
            new_wid = add_combobox(designer->ui, designer->new_label[designer->active_widget_num],
                                                                        wid->x, wid->y, 120, 30);
            set_controller_callbacks(designer, new_wid, true);
            new_wid->data = wid->data;
            designer->wid_counter--;
            add_to_list(designer, new_wid, "add_lv2_combobox", true, IS_COMBOBOX);
            destroy_widget(wid, designer->w->app);
            designer->controls[new_wid->data].image = NULL;
            widget_show(new_wid);
            designer->active_widget = new_wid;
        break;
        case 6:
            asprintf (&designer->new_label[designer->active_widget_num], "%s",wid->label);
            new_wid = add_valuedisplay(designer->ui, designer->new_label[designer->active_widget_num],
                                                                        wid->x, wid->y, 30, 120);
            copy_widget_settings(designer, wid, new_wid);
            new_wid->adj->type = new_wid->adj->type == CL_LOGARITHMIC ? CL_LOGARITHMIC :
                new_wid->adj->type == CL_LOGSCALE ? CL_LOGSCALE : CL_CONTINUOS;
            add_to_list(designer, new_wid, "add_lv2_valuedisplay", true, IS_VALUE_DISPLAY);
            destroy_widget(wid, designer->w->app);
            designer->controls[new_wid->data].image = NULL;
            widget_show(new_wid);
            designer->active_widget = new_wid;
        break;
        case 7:
            asprintf (&designer->new_label[designer->active_widget_num], "%s",wid->label);
            new_wid = add_label(designer->ui, designer->new_label[designer->active_widget_num],
                                                                        wid->x, wid->y, 120, 30);
            set_controller_callbacks(designer, new_wid, true);
            new_wid->data = wid->data;
            designer->wid_counter--;
            add_to_list(designer, new_wid, "add_lv2_label", false, IS_LABEL);
            destroy_widget(wid, designer->w->app);
            designer->controls[new_wid->data].image = NULL;
            widget_show(new_wid);
            designer->active_widget = new_wid;
        break;
        case 8:
            asprintf (&designer->new_label[designer->active_widget_num], "%s",wid->label);
            new_wid = add_vmeter(designer->ui, designer->new_label[designer->active_widget_num],
                                                                false, wid->x, wid->y, 30, 120);
            copy_widget_settings(designer, wid, new_wid);
            new_wid->adj->type = new_wid->adj->type == CL_LOGARITHMIC ? CL_LOGARITHMIC :
                new_wid->adj->type == CL_LOGSCALE ? CL_LOGSCALE : CL_METER;
            add_to_list(designer, new_wid, "add_lv2_vmeter", true, IS_VMETER);
            destroy_widget(wid, designer->w->app);
            designer->controls[new_wid->data].image = NULL;
            widget_show(new_wid);
            designer->active_widget = new_wid;
        break;
        case 9:
            asprintf (&designer->new_label[designer->active_widget_num], "%s",wid->label);
            new_wid = add_hmeter(designer->ui, designer->new_label[designer->active_widget_num],
                                                                false, wid->x, wid->y, 30, 120);
            copy_widget_settings(designer, wid, new_wid);
            new_wid->adj->type = new_wid->adj->type == CL_LOGARITHMIC ? CL_LOGARITHMIC :
                new_wid->adj->type == CL_LOGSCALE ? CL_LOGSCALE : CL_METER;
            add_to_list(designer, new_wid, "add_lv2_hmeter", true, IS_HMETER);
            destroy_widget(wid, designer->w->app);
            designer->controls[new_wid->data].image = NULL;
            widget_show(new_wid);
            designer->active_widget = new_wid;
        break;
        default:
        break;
    }
    
}

static void run_exit(void *w_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    if (w->flags & HAS_POINTER && !adj_get_value(w->adj_y)) {
        XUiDesigner *designer = (XUiDesigner*)w->parent_struct;
        quit(designer->w);
    }
}

/*---------------------------------------------------------------------
-----------------------------------------------------------------------    
                main
-----------------------------------------------------------------------
----------------------------------------------------------------------*/

int main (int argc, char ** argv) {

#ifdef ENABLE_NLS
    // set Message type to locale to fetch localisation support
    setlocale (LC_MESSAGES, "");
    // set Ctype to C to avoid symbol clashes from different locales
    setlocale (LC_CTYPE, "C");
    bindtextdomain(GETTEXT_PACKAGE, LOCAL_DIR);
    bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
    textdomain(GETTEXT_PACKAGE);
#endif

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
    designer->active_widget = NULL;
    designer->prev_active_widget = NULL;
    designer->wid_counter = 0;
    designer->image_path = NULL;
    designer->image = NULL;
    designer->icon = NULL;
    designer->run_test = false;
    designer->lv2c.ui_uri = NULL;
    designer->grid_width = 30;
    designer->grid_height = 15;

    Xputty app;
    main_init(&app);
    designer->new_label = NULL;
    designer->new_label = (char **)realloc(designer->new_label, (MAX_CONTROLS) * sizeof(char *));
    int m = 0;
    for (;m<MAX_CONTROLS;m++) {
        designer->controls[m].wid = NULL;
        designer->controls[m].image = NULL;
        designer->controls[m].grid_snap_option = 0;
    }
    //set_light_theme(&app);
    designer->w = create_window(&app, DefaultRootWindow(app.dpy), 0, 0, 1200, 800);
    designer->w->parent_struct = designer;
    widget_set_title(designer->w, _("XUiDesigner"));
    widget_set_icon_from_png(designer->w, designer->icon, LDVAR(gear_png));
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
    designer->ui->func.expose_callback = draw_ui;
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

    designer->image_loader = add_image_button(designer->w,20,75,40,40, "", ".png");
    tooltip_set_text(designer->image_loader,_("Load Background Image (*.png)"));
    designer->image_loader->func.user_callback = image_load_response;
    
    designer->unload_image = add_button(designer->w, "", 80, 75, 40, 40);
    widget_get_png(designer->unload_image, LDVAR(cancel_png));
    tooltip_set_text(designer->unload_image,_("Unload Background Image"));
    designer->unload_image->parent_struct = designer;
    designer->unload_image->func.value_changed_callback = unload_background_image;

    designer->color_chooser = add_image_toggle_button(designer->w, "", 20, 135, 40, 40);
    widget_get_png(designer->color_chooser, LDVAR(colors_png));
    tooltip_set_text(designer->color_chooser,_("Show/Hide Color Chooser"));
    designer->color_chooser->parent_struct = designer;
    create_color_chooser (designer);
    designer->color_chooser->func.value_changed_callback = show_color_chooser;

    designer->grid = add_toggle_button(designer->w, "", 80, 135, 40, 40);
    widget_get_png(designer->grid, LDVAR(grid_png));
    tooltip_set_text(designer->grid,_("Snap to grid"));
    designer->grid->parent_struct = designer;
    designer->grid->func.value_changed_callback = use_grid;

    designer->test = add_button(designer->w, "", 1020, 740, 40, 40);
    widget_get_png(designer->test, LDVAR(gear_png));
    tooltip_set_text(designer->test,_("Run test build"));
    designer->test->parent_struct = designer;
    designer->test->func.value_changed_callback = run_test;

    designer->save = add_button(designer->w, "", 1080, 740, 40, 40);
    widget_get_png(designer->save, LDVAR(save_png));
    tooltip_set_text(designer->save,_("Save"));
    designer->save->parent_struct = designer;
    designer->save->func.value_changed_callback = run_save;

    designer->exit = add_button(designer->w, "", 1140, 740, 40, 40);
    widget_get_png(designer->exit, LDVAR(exit_png));
    tooltip_set_text(designer->exit,_("Exit"));
    designer->exit->parent_struct = designer;
    designer->exit->func.value_changed_callback = run_exit;

    add_label(designer->w, _("Label"), 1000, 10, 180, 30);

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

    add_label(designer->w, _("Port Index"), 1000, 80, 180, 30);
    designer->index = add_combobox(designer->w, "", 1000, 120, 70, 30);
    designer->index->parent_struct = designer;
    combobox_add_numeric_entrys(designer->index, 0, MAX_CONTROLS);
    combobox_set_active_entry(designer->index, 0);
    designer->set_index = add_button(designer->w, _("Set"), 1090, 120, 60, 30);
    designer->set_index->parent_struct = designer;
    designer->set_index->func.value_changed_callback = set_port_index;

    add_label(designer->w, _("Position/Size"), 1000, 150, 180, 30);

    designer->x_axis = add_hslider(designer->w, _("X"), 1000, 200, 180, 30);
    designer->x_axis->parent_struct = designer;
    set_adjustment(designer->x_axis->adj,0.0, 0.0, 0.0, 1200.0, 1.0, CL_CONTINUOS);
    designer->x_axis->func.value_changed_callback = set_x_axis_callback;

    designer->y_axis = add_hslider(designer->w, _("Y"), 1000, 240, 180, 30);
    designer->y_axis->parent_struct = designer;
    set_adjustment(designer->y_axis->adj,0.0, 0.0, 0.0, 600.0, 1.0, CL_CONTINUOS);
    designer->y_axis->func.value_changed_callback = set_y_axis_callback;

    designer->w_axis = add_hslider(designer->w, _("Width"), 1000, 280, 180, 30);
    designer->w_axis->parent_struct = designer;
    set_adjustment(designer->w_axis->adj,10.0, 10.0, 10.0, 300.0, 1.0, CL_CONTINUOS);
    designer->w_axis->func.value_changed_callback = set_w_axis_callback;

    designer->h_axis = add_hslider(designer->w, _("Height"), 1000, 320, 180, 30);
    designer->h_axis->parent_struct = designer;
    set_adjustment(designer->h_axis->adj,10.0, 10.0, 10.0, 300.0, 1.0, CL_CONTINUOS);
    designer->h_axis->func.value_changed_callback = set_h_axis_callback;


    widget_show_all(designer->w);

    designer->grid_size_x = add_valuedisplay(designer->w, _("Grid X"), 125, 135, 40, 20);
    designer->grid_size_x->parent_struct = designer;
    set_adjustment(designer->grid_size_x->adj,(float)designer->grid_width,
        (float)designer->grid_width, 10.0, 300.0, 1.0, CL_CONTINUOS);
    tooltip_set_text(designer->grid_size_x,_("Grid width"));
    designer->grid_size_x->func.value_changed_callback = set_grid_width;

    designer->grid_size_y = add_valuedisplay(designer->w, _("Grid Y"), 125, 155, 40, 20);
    designer->grid_size_y->parent_struct = designer;
    set_adjustment(designer->grid_size_y->adj,(float)designer->grid_height,
        (float)designer->grid_height, 10.0, 300.0, 1.0, CL_CONTINUOS);
    tooltip_set_text(designer->grid_size_y,_("Grid height"));
    designer->grid_size_y->func.value_changed_callback = set_grid_height;

    designer->combobox_settings = create_widget(&app, designer->w, 1000, 360, 180, 200);
    add_label(designer->combobox_settings, _("Add Combobox Entry"), 0, 0, 180, 30);
    designer->combobox_entry = add_input_box(designer->combobox_settings, 0, 0, 40, 140, 30);
    designer->combobox_entry->parent_struct = designer;
    designer->combobox_entry->func.user_callback = set_combobox_entry;

    designer->add_entry = add_button(designer->combobox_settings, _("Add"), 140, 40, 40, 30);
    designer->add_entry->parent_struct = designer;
    designer->add_entry->func.value_changed_callback = add_combobox_entry;

    designer->controller_settings = create_widget(&app, designer->w, 1000, 360, 180, 250);
    add_label(designer->controller_settings, _("Controller Settings"), 0, 0, 180, 30);
    const char* labels[4] = { "Min","Max","Default", "Step Size"};
    int k = 0;
    for (;k<4;k++) {
        add_label(designer->controller_settings, labels[k], 0, (k+1)*40, 90, 30);
        designer->controller_entry[k] = add_input_box(designer->controller_settings, 1, 100, (k+1)*40, 60, 30);
        designer->controller_entry[k]->parent_struct = designer;
    }

    designer->set_adjust = add_button(designer->controller_settings, _("Set"), 100, 200, 60, 30);
    designer->set_adjust->parent_struct = designer;
    designer->set_adjust->func.value_changed_callback = set_controller_adjustment;

    designer->context_menu = create_menu(designer->w,25);
    designer->context_menu->parent_struct = designer;
    designer->menu_item_load = menu_add_item(designer->context_menu,_("Load Controller Image"));
    designer->menu_item_unload = menu_add_item(designer->context_menu,_("Unload Controller Image"));

    designer->grid_snap_select = cmenu_add_submenu(designer->context_menu, _("Grid snap"));
    designer->grid_snap_select->parent_struct = designer;
    designer->grid_snap_left = menu_add_radio_entry(designer->grid_snap_select, _("Left"));
    designer->grid_snap_center = menu_add_radio_entry(designer->grid_snap_select, _("Center"));
    designer->grid_snap_right = menu_add_radio_entry(designer->grid_snap_select, _("Right"));
    radio_item_set_active(designer->grid_snap_left);
    designer->grid_snap_select->func.value_changed_callback = select_grid_mode;

    designer->ctype_switch = cmenu_add_submenu(designer->context_menu, _("Switch Controller type"));
    designer->ctype_switch->parent_struct = designer;
    menu_add_radio_entry(designer->ctype_switch,_("Knob"));
    menu_add_radio_entry(designer->ctype_switch,_("HSlider"));
    menu_add_radio_entry(designer->ctype_switch,_("VSlider"));
    menu_add_radio_entry(designer->ctype_switch,_("Button"));
    menu_add_radio_entry(designer->ctype_switch,_("Toggle Button"));
    menu_add_radio_entry(designer->ctype_switch,_("Combo Box"));
    menu_add_radio_entry(designer->ctype_switch,_("Value Display"));
    menu_add_radio_entry(designer->ctype_switch,_("Label"));
    menu_add_radio_entry(designer->ctype_switch,_("VMeter"));
    menu_add_radio_entry(designer->ctype_switch,_("HMeter"));
    designer->ctype_switch->func.value_changed_callback = switch_controller_type;

    menu_add_item(designer->context_menu,_("Delete Controller"));
    designer->context_menu->func.button_release_callback = pop_menu_response;

    widget_show_all(designer->ui);
    main_run(&app);

    //print_list(designer);
    lilv_world_free(designer->world);
    fprintf(stderr, "bye, bye\n");
    main_quit(&app);
    int i = 0;
    for (;i<designer->wid_counter-1; i++) {
        free(designer->new_label[i]);
    }
    free(designer->new_label);
    free(designer->image_path);
    free(designer->image);
    free(designer->lv2c.ui_uri);
    m = 0;
    for (;m<MAX_CONTROLS;m++) {
        free(designer->controls[m].image);
    }
    if (designer->icon) {
        XFreePixmap(designer->w->app->dpy, (*designer->icon));
    }
    free(designer);

    return 0;
}
    
