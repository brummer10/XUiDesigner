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
#include "XUiTextInput.h"
#include "XUiControllerType.h"
#include "XUiGridControl.h"
#include "XUiReparent.h"
#include "XUiSettings.h"
#include "XUiWriteTurtle.h"
#include "xtabbox_private.h"

/*---------------------------------------------------------------------
-----------------------------------------------------------------------    
                designer function calls
-----------------------------------------------------------------------
----------------------------------------------------------------------*/

void draw_window(void *w_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    use_bg_color_scheme(w, NORMAL_);
    cairo_paint (w->crb);
}

static void draw_systray(void *w_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    use_bg_color_scheme(w, NORMAL_);
    cairo_paint (w->crb);
    if (w->image) {
        widget_set_scale(w);
        cairo_set_source_surface (w->crb, w->image, 0, 0);
        cairo_mask_surface (w->crb, w->image, 0, 0);
        widget_reset_scale(w);
    }
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
    TextBox_t *text_box = (TextBox_t*)w->private_struct;
    if (designer->controls[designer->active_widget_num].is_type == IS_COMBOBOX ) {
        if (strlen(text_box->input_label)>1) {
            combobox_add_entry(designer->controls[designer->active_widget_num].wid,
                                            text_box->input_label);
            memset(text_box->input_label, 0, 256 *
                (sizeof text_box->input_label[0]));
            expose_widget(designer->combobox_entry);
        }
    }
}

static void add_combobox_entry(void *w_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    XUiDesigner *designer = (XUiDesigner*)w->parent_struct;
    TextBox_t *text_box = (TextBox_t*)designer->combobox_entry->private_struct;
    if (w->flags & HAS_POINTER && !adj_get_value(w->adj_y)) {
        if (designer->controls[designer->active_widget_num].is_type == IS_COMBOBOX ) {
            if (strlen(text_box->input_label)>1) {
                combobox_add_entry(designer->controls[designer->active_widget_num].wid,
                                                text_box->input_label);
                memset(text_box->input_label, 0, 256 *
                    (sizeof text_box->input_label[0]));
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
            TextBox_t *text_box = (TextBox_t*)designer->controller_entry[0]->private_struct;
            if (strlen(text_box->input_label)>1) {
                designer->active_widget->adj->min_value = atof(text_box->input_label);
            }
            text_box = (TextBox_t*)designer->controller_entry[1]->private_struct;
            if (strlen(text_box->input_label)>1) {
                designer->active_widget->adj->max_value = atof(text_box->input_label);
            }
            text_box = (TextBox_t*)designer->controller_entry[2]->private_struct;
            if (strlen(text_box->input_label)>1) {
                designer->active_widget->adj->value = atof(text_box->input_label);
                designer->active_widget->adj->std_value = atof(text_box->input_label);
            }
            text_box = (TextBox_t*)designer->controller_entry[3]->private_struct;
            if (strlen(text_box->input_label)>1) {
                designer->active_widget->adj->step = atof(text_box->input_label);
            }
        }
    }
}

static void add_tabbox_entry(void *w_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    XUiDesigner *designer = (XUiDesigner*)w->parent_struct;
    if (w->flags & HAS_POINTER && !adj_get_value(w->adj_y)) {
        if (designer->active_widget != NULL) {
            Widget_t *tab = tabbox_add_tab(designer->active_widget,
                (const char*)designer->new_label[designer->active_widget_num]);
            tab->parent_struct = designer;
            tab->func.button_press_callback = set_pos_tab;
            tab->func.button_release_callback = fix_pos_tab;
            tab->func.motion_callback = move_tab;
            expose_widget(designer->active_widget);
        }
    }
}

static void remove_tabbox_entry(void *w_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    XUiDesigner *designer = (XUiDesigner*)w->parent_struct;
    if (w->flags & HAS_POINTER && !adj_get_value(w->adj_y)) {
        if (designer->active_widget != NULL) {
            int v = (int)adj_get_value(designer->active_widget->adj);
            Widget_t *wi = designer->active_widget->childlist->childs[v];
            int el = wi->childlist->elem;
            int j = el;
            for(;j>0l;j--) {
                Widget_t *w = wi->childlist->childs[j-1];
                remove_from_list(designer, w);
            }
            tabbox_remove_tab(designer->active_widget,v);
            expose_widget(designer->active_widget);
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
            if (designer->controls[w->data].is_type == IS_TABBOX) {
                int elem = w->childlist->elem;
                int i = 0;
                for(;i<elem;i++) {
                    Widget_t *wi = w->childlist->childs[i];
                    XResizeWindow(w->app->dpy, wi->widget, max(10,wi->width + (xmotion->x_root-designer->pos_x)), max(10,wi->height+ (xmotion->x_root-designer->pos_x)));
                }
            }
            w->scale.ascale = 1.0;
            designer->pos_x = xmotion->x_root;
        break;
        case XUI_WIDTH:
            XResizeWindow(w->app->dpy, w->widget, max(10,w->width + (xmotion->x_root-designer->pos_x)), w->height);
            adj_set_value(designer->w_axis->adj, w->width + ((xmotion->x_root-designer->pos_x)));
            if (designer->controls[w->data].is_type == IS_TABBOX) {
                int elem = w->childlist->elem;
                int i = 0;
                for(;i<elem;i++) {
                    Widget_t *wi = w->childlist->childs[i];
                    XResizeWindow(w->app->dpy, wi->widget, max(10,wi->width + (xmotion->x_root-designer->pos_x)), wi->height);
                }
            }
            w->scale.ascale = 1.0;
            designer->pos_x = xmotion->x_root;
        break;
        case XUI_HEIGHT:
            XResizeWindow(w->app->dpy, w->widget, w->width, max(10,w->height + (xmotion->y_root-designer->pos_y)));
            adj_set_value(designer->h_axis->adj, w->height + ((xmotion->y_root-designer->pos_y)));
            if (designer->controls[w->data].is_type == IS_TABBOX) {
                int elem = w->childlist->elem;
                int i = 0;
                for(;i<elem;i++) {
                    Widget_t *wi = w->childlist->childs[i];
                    XResizeWindow(w->app->dpy, wi->widget, wi->width, max(10,wi->height+ (xmotion->x_root-designer->pos_x)));
                }
            }
            w->scale.ascale = 1.0;
            designer->pos_y = xmotion->y_root;
        break;
        case XUI_NONE:
            if (xmotion->x > w->width-5 && xmotion->y > w->height-5 && is_curser != 1) {
                is_curser = 1;
                Cursor c = XCreateFontCursor(w->app->dpy, XC_bottom_right_corner);
                XDefineCursor (w->app->dpy, w->widget, c);
                XFreeCursor(w->app->dpy, c);
            } else if ( xmotion->y > w->height-5 && is_curser != 4) {
                is_curser = 4;
                Cursor c = XCreateFontCursor(w->app->dpy, XC_bottom_side);
                XDefineCursor (w->app->dpy, w->widget, c);
                XFreeCursor(w->app->dpy, c);
            } else if (xmotion->x > w->width-5 && is_curser != 3) {
                is_curser = 3;
                Cursor c = XCreateFontCursor(w->app->dpy, XC_right_side);
                XDefineCursor (w->app->dpy, w->widget, c);
                XFreeCursor(w->app->dpy, c);
            } else if ((xmotion->x <= w->width-5 && xmotion->y <= w->height-5) && is_curser != 2) {
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

static void null_callback(void *w_, void* user_data) {
    
}

static void set_designer_callbacks(XUiDesigner *designer, Widget_t* wid) {
    if (designer->controls[wid->data].is_type == IS_TABBOX) {
        int v = (int)adj_get_value(wid->adj);
        Widget_t *wi = designer->active_widget->childlist->childs[v];
        entry_set_text(designer, wi->label);
    } else {
        entry_set_text(designer, wid->label);
    }
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
    designer->active_widget_num = -1;
    if (designer->prev_active_widget != NULL)
        draw_trans(designer->prev_active_widget,NULL);
    XButtonEvent *xbutton = (XButtonEvent*)button_;
    if(xbutton->button == Button1) {
        designer->active_widget = (Widget_t*)w_;
        designer->active_widget_num = w->data;
        designer->pos_x = xbutton->x_root;
        designer->pos_y = xbutton->y_root;
        set_designer_callbacks(designer,w);
        if (designer->controls[designer->active_widget_num].port_index > -1) {
            adj_set_value(designer->index->adj, 
                (float)designer->controls[designer->active_widget_num].port_index);
        }

        if (designer->controls[designer->active_widget_num].have_adjustment  &&
                designer->controls[designer->active_widget_num].is_type != IS_COMBOBOX) {
            widget_hide(designer->tabbox_settings);
            widget_show_all(designer->controller_settings);
            box_entry_set_value(designer->controller_entry[0], designer->active_widget->adj->min_value);
            box_entry_set_value(designer->controller_entry[1], designer->active_widget->adj->max_value);
            box_entry_set_value(designer->controller_entry[2], designer->active_widget->adj->std_value);
            box_entry_set_value(designer->controller_entry[3], designer->active_widget->adj->step);
        } else if (designer->controls[designer->active_widget_num].is_type == IS_TABBOX) {
            widget_hide(designer->controller_settings);
            widget_show_all(designer->tabbox_settings);
        } else {
            widget_hide(designer->controller_settings);
            widget_hide(designer->tabbox_settings);
        }
        if (designer->controls[designer->active_widget_num].is_type == IS_COMBOBOX) {
            widget_show_all(designer->combobox_settings);
            widget_hide(designer->tabbox_settings);
        } else {
            widget_hide(designer->combobox_settings);
        }
    }
    if (xbutton->x > width-5 && xbutton->y > height-5) {
        designer->modify_mod = XUI_SIZE;
    } else if (xbutton->y > height-5) {
        designer->modify_mod = XUI_HEIGHT;
    } else if (xbutton->x > width-5) {
        designer->modify_mod = XUI_WIDTH;
    } else {
        designer->modify_mod = XUI_POSITION;
    }
    if(designer->active_widget != NULL)
        draw_frame(designer->active_widget, NULL);
    designer->prev_active_widget = w;
}

void fix_pos_wid(void *w_, void *button_, void* user_data) {
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
        if (designer->controls[w->data].is_type == IS_TABBOX) {
            int elem = w->childlist->elem;
            int i = 0;
            for(;i<elem;i++) {
                Widget_t *wi = w->childlist->childs[i];
                wi->scale.init_width = wi->width;
                wi->scale.init_height = wi->height;
                wi->scale.ascale = 1.0;
            }
            _tab_button_released(w, button_, NULL);
            int v = (int)adj_get_value(w->adj);
            Widget_t *wi = designer->active_widget->childlist->childs[v];
            entry_set_text(designer, wi->label);
        }
        if (designer->controls[designer->active_widget_num].is_type != IS_FRAME &&
            designer->controls[designer->active_widget_num].is_type != IS_TABBOX)
            check_reparent(designer, xbutton, w);
    } else if(xbutton->button == Button3) {
        designer->modify_mod = XUI_NONE;
        designer->active_widget = (Widget_t*)w_;
        designer->active_widget_num = w->data;
        if (designer->controls[designer->active_widget_num].is_type == IS_COMBOBOX ||
            designer->controls[designer->active_widget_num].is_type == IS_VALUE_DISPLAY ||
            designer->controls[designer->active_widget_num].is_type == IS_VMETER ||
            designer->controls[designer->active_widget_num].is_type == IS_HMETER ||
            designer->controls[designer->active_widget_num].is_type == IS_VSLIDER ||
            designer->controls[designer->active_widget_num].is_type == IS_LABEL ||
            designer->controls[designer->active_widget_num].is_type == IS_WAVEVIEW ||
            designer->controls[designer->active_widget_num].is_type == IS_TABBOX ||
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
        if (designer->controls[designer->active_widget_num].is_type == IS_IMAGE_TOGGLE)
            sel = IS_TOGGLE_BUTTON;
        store = designer->ctype_switch->func.value_changed_callback;
        designer->ctype_switch->func.value_changed_callback = null_callback;
        set_active_radio_entry_num(designer->ctype_switch, sel);
        designer->ctype_switch->func.value_changed_callback = store;
        pop_menu_show(w,designer->context_menu,5,true);
    }
    if(designer->active_widget != NULL)
        draw_frame(designer->active_widget, NULL);
}

void fix_pos_tab(void *w_, void *button_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    Widget_t *p = (Widget_t*)w->parent;
    fix_pos_wid(p, button_, user_data);
}

void set_pos_tab(void *w_, void *button_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    Widget_t *p = (Widget_t*)w->parent;
    set_pos_wid(p, button_, user_data);
}

void move_tab(void *w_, void *xmotion_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    Widget_t *p = (Widget_t*)w->parent;
    move_wid(p, xmotion_, user_data);
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
    designer->controls[designer->wid_counter].in_frame = 0;
    asprintf (&designer->new_label[designer->active_widget_num], "%s",wid->label);
    if (set_designer) {
        set_designer_callbacks(designer, wid);
    }
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
                asprintf(&designer->controls[designer->wid_counter].name, "Knob%i", designer->wid_counter);
                wid = add_knob(w, designer->controls[designer->wid_counter].name, xbutton->x, xbutton->y, 60, 80);
                set_controller_callbacks(designer, wid, true);
                add_to_list(designer, wid, "add_lv2_knob", true, IS_KNOB);
            break;
            case 2:
                asprintf(&designer->controls[designer->wid_counter].name, "HSlider%i", designer->wid_counter);
                wid = add_hslider(w, designer->controls[designer->wid_counter].name, xbutton->x, xbutton->y, 120, 30);
                set_controller_callbacks(designer, wid, true);
                add_to_list(designer, wid, "add_lv2_hslider", true, IS_HSLIDER);
            break;
            case 3:
                asprintf(&designer->controls[designer->wid_counter].name, "VSlider%i", designer->wid_counter);
                wid = add_vslider(w, designer->controls[designer->wid_counter].name, xbutton->x, xbutton->y, 30, 120);
                set_controller_callbacks(designer, wid, true);
                add_to_list(designer, wid, "add_lv2_vslider", true, IS_VSLIDER);
            break;
            case 4:
                asprintf(&designer->controls[designer->wid_counter].name, "Button%i", designer->wid_counter);
                wid = add_button(w, designer->controls[designer->wid_counter].name, xbutton->x, xbutton->y, 60, 60);
                set_controller_callbacks(designer, wid, true);
                add_to_list(designer, wid, "add_lv2_button", false, IS_BUTTON);
            break;
            case 5:
                asprintf(&designer->controls[designer->wid_counter].name, "Switch%i", designer->wid_counter);
                wid = add_toggle_button(w, designer->controls[designer->wid_counter].name, xbutton->x, xbutton->y, 60, 60);
                set_controller_callbacks(designer, wid, true);
                add_to_list(designer, wid, "add_lv2_toggle_button", false, IS_TOGGLE_BUTTON);
            break;
            case 6:
                asprintf(&designer->controls[designer->wid_counter].name, "Combobox%i", designer->wid_counter);
                wid = add_combobox(w, designer->controls[designer->wid_counter].name, xbutton->x, xbutton->y, 120, 30);
                set_controller_callbacks(designer, wid, true);
                add_to_list(designer, wid, "add_lv2_combobox", true, IS_COMBOBOX);
            break;
            case 7:
                asprintf(&designer->controls[designer->wid_counter].name, "ValueDisply%i", designer->wid_counter);
                wid = add_valuedisplay(w, "designer->controls[designer->wid_counter].name", xbutton->x, xbutton->y, 40, 30);
                set_controller_callbacks(designer, wid, true);
                add_to_list(designer, wid, "add_lv2_valuedisplay", true, IS_VALUE_DISPLAY);
            break;
            case 8:
                asprintf(&designer->controls[designer->wid_counter].name, "Label%i", designer->wid_counter);
                wid = add_label(w, designer->controls[designer->wid_counter].name, xbutton->x, xbutton->y, 60, 30);
                set_controller_callbacks(designer, wid, true);
                add_to_list(designer, wid, "add_lv2_label", false, IS_LABEL);
            break;
            case 9:
                asprintf(&designer->controls[designer->wid_counter].name, "VMeter%i", designer->wid_counter);
                wid = add_vmeter(w, designer->controls[designer->wid_counter].name, false, xbutton->x, xbutton->y, 10, 120);
                set_controller_callbacks(designer, wid, true);
                add_to_list(designer, wid, "add_lv2_vmeter", true, IS_VMETER);
            break;
            case 10:
                asprintf(&designer->controls[designer->wid_counter].name, "HMeter%i", designer->wid_counter);
                wid = add_hmeter(w, designer->controls[designer->wid_counter].name, false, xbutton->x, xbutton->y, 120, 10);
                set_controller_callbacks(designer, wid, true);
                add_to_list(designer, wid, "add_lv2_hmeter", true, IS_HMETER);
            break;
            case 11:
                asprintf(&designer->controls[designer->wid_counter].name, "WaveView%i", designer->wid_counter);
                wid = add_waveview(w, designer->controls[designer->wid_counter].name, xbutton->x, xbutton->y, 120, 120);
                set_controller_callbacks(designer, wid, true);
                add_to_list(designer, wid, "add_lv2_waveview", false, IS_WAVEVIEW);
                float v[9] = { 0.0,-0.5, 0.0, 0.5, 0.0, -0.5, 0.0, 0.5, 0.0};
                update_waveview(wid, &v[0],9);
            break;
            case 12:
                asprintf(&designer->controls[designer->wid_counter].name, "Frame%i", designer->wid_counter);
                wid = add_frame(w, designer->controls[designer->wid_counter].name, xbutton->x, xbutton->y, 120, 120);
                set_controller_callbacks(designer, wid, true);
                adj_set_value(designer->index->adj, adj_get_value(designer->index->adj)-1.0);
                designer->controls[designer->wid_counter-1].port_index = -1;
                add_to_list(designer, wid, "add_lv2_frame", false, IS_FRAME);
                wid->parent_struct = designer;
                wid->func.enter_callback = null_callback;
                wid->func.leave_callback = null_callback;
                XLowerWindow(w->app->dpy, wid->widget);
            break;
            case 13:
                asprintf(&designer->controls[designer->wid_counter].name, "Tab%i", designer->wid_counter);
                wid = add_tabbox(w, designer->controls[designer->wid_counter].name, xbutton->x, xbutton->y, 120, 120);
                set_controller_callbacks(designer, wid, true);
                adj_set_value(designer->index->adj, adj_get_value(designer->index->adj)-1.0);
                designer->controls[designer->wid_counter-1].port_index = -1;
                add_to_list(designer, wid, "add_lv2_tabbox", false, IS_TABBOX);
                wid->parent_struct = designer;
                wid->func.enter_callback = null_callback;
                wid->func.leave_callback = null_callback;
                XLowerWindow(w->app->dpy, wid->widget);
                Widget_t *tab = tabbox_add_tab(wid, "Tab 1");
                tab->parent_struct = designer;
                tab->func.button_press_callback = set_pos_tab;
                tab->func.button_release_callback = fix_pos_tab;
                tab->func.motion_callback = move_tab;
            break;
            default:
            break;
        }
        if (designer->controls[designer->active_widget_num].have_adjustment &&
                designer->controls[designer->active_widget_num].is_type != IS_COMBOBOX) {
            widget_show_all(designer->controller_settings);
            box_entry_set_value(designer->controller_entry[0], designer->active_widget->adj->min_value);
            box_entry_set_value(designer->controller_entry[1], designer->active_widget->adj->max_value);
            box_entry_set_value(designer->controller_entry[2], designer->active_widget->adj->std_value);
            box_entry_set_value(designer->controller_entry[3], designer->active_widget->adj->step);

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
        widget_hide(designer->set_project);
    } else if(xbutton->button == Button3) {
        widget_show_all(designer->set_project);
        char *name = NULL;
        XFetchName(designer->ui->app->dpy, designer->ui->widget, &name);
        if (name != NULL)
            box_entry_set_text(designer->project_title, name);
        if (designer->lv2c.uri != NULL)
            box_entry_set_text(designer->project_uri, designer->lv2c.uri);
        if (designer->lv2c.ui_uri != NULL)
            box_entry_set_text(designer->project_ui_uri, designer->lv2c.ui_uri);
        if (designer->lv2c.author != NULL)
            box_entry_set_text(designer->project_author, designer->lv2c.author);
    }
}

static void systray_menu_response(void *w_, void* item_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    XUiDesigner *designer = (XUiDesigner*)w->parent_struct;
    switch (*(int*)item_) {
        case 0:
        {
            const char* home = getenv("HOME");
            Widget_t* tmp = open_directory_dialog(designer->ui, home);
            Atom wmStateAbove = XInternAtom(w->app->dpy, "_NET_WM_STATE_ABOVE", 1 );
            Atom wmNetWmState = XInternAtom(w->app->dpy, "_NET_WM_STATE", 1 );
            XChangeProperty(w->app->dpy, tmp->widget, wmNetWmState, XA_ATOM, 32, 
                PropModeReplace, (unsigned char *) &wmStateAbove, 1); 
            XSetTransientForHint(w->app->dpy, tmp->widget, designer->ui->widget);
            designer->ui->func.dialog_callback = run_save;
        }
        break;
        case 1:
        {
            widget_show_all(designer->set_project);
            char *name = NULL;
            XFetchName(designer->ui->app->dpy, designer->ui->widget, &name);
            if (name != NULL)
                box_entry_set_text(designer->project_title, name);
            if (designer->lv2c.uri != NULL)
                box_entry_set_text(designer->project_uri, designer->lv2c.uri);
            if (designer->lv2c.ui_uri != NULL)
                box_entry_set_text(designer->project_ui_uri, designer->lv2c.ui_uri);
            if (designer->lv2c.author != NULL)
                box_entry_set_text(designer->project_author, designer->lv2c.author);
        }
        break;
        case 2:
            quit(designer->w);
        break;
        default:
        break;
    }
}

static void systray_released(void *w_, void* button_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    XUiDesigner *designer = (XUiDesigner*)w->parent_struct;
    XButtonEvent *xbutton = (XButtonEvent*)button_;
    if (w->flags & HAS_POINTER) {
        if (xbutton->button == Button1) {
            XWindowAttributes attrs;
            XGetWindowAttributes(w->app->dpy, (Window)designer->w->widget, &attrs);
            if (attrs.map_state == IsViewable) {
                if ((int)adj_get_value(designer->color_chooser->adj)) {
                    adj_set_value(designer->color_chooser->adj, 0.0);
                }
                widget_hide(designer->w);
                widget_hide(designer->ui);
                widget_hide(designer->set_project);
                XFlush(designer->w->app->dpy);
            } else {
                widget_show_all(designer->w);
                widget_show_all(designer->ui);
            }
        } else if (xbutton->button == Button3) {
            pop_menu_show(w,designer->systray_menu,3,true);
        }
    }
}

static void run_settings(void *w_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    if (w->flags & HAS_POINTER && !adj_get_value(w->adj_y)) {
        XUiDesigner *designer = (XUiDesigner*)w->parent_struct;
        XWindowAttributes attrs;
        XGetWindowAttributes(w->app->dpy, (Window)designer->set_project->widget, &attrs);
        if (attrs.map_state != IsViewable) {
            widget_show_all(designer->set_project);
            char *name = NULL;
            XFetchName(designer->ui->app->dpy, designer->ui->widget, &name);
            if (name != NULL)
                box_entry_set_text(designer->project_title, name);
            if (designer->lv2c.uri != NULL)
                box_entry_set_text(designer->project_uri, designer->lv2c.uri);
            if (designer->lv2c.ui_uri != NULL)
                box_entry_set_text(designer->project_ui_uri, designer->lv2c.ui_uri);
            if (designer->lv2c.author != NULL)
                box_entry_set_text(designer->project_author, designer->lv2c.author);
        } else {
            widget_hide(designer->set_project);
        }
    }
}

static void run_exit(void *w_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    if (w->flags & HAS_POINTER && !adj_get_value(w->adj_y)) {
        XUiDesigner *designer = (XUiDesigner*)w->parent_struct;
        quit(designer->w);
    }
}

static void run_save_as(void *w_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    if (w->flags & HAS_POINTER && !adj_get_value(w->adj_y)) {
        XUiDesigner *designer = (XUiDesigner*)w->parent_struct;
        const char* home = getenv("HOME");
        open_directory_dialog(designer->ui, home);
        designer->ui->func.dialog_callback = run_save;
    }
}

char *getUserName() {
    uid_t uid = geteuid();
    struct passwd *pw = getpwuid(uid);
    if (pw) {
        return pw->pw_name;
    }
    return "";
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
    asprintf(&designer->lv2c.ui_uri, "urn:%s:%s", getUserName(), "test_ui");
    designer->lv2c.uri = NULL;
    asprintf(&designer->lv2c.uri, "urn:%s:%s", getUserName(), "test");
    designer->lv2c.author = NULL;
    asprintf(&designer->lv2c.author, "%s", getUserName());
    designer->lv2c.plugintype = NULL;
    asprintf(&designer->lv2c.plugintype, "%s", "MixerPlugin");
    designer->lv2c.audio_input = 1;
    designer->lv2c.audio_output = 1;
    designer->lv2c.midi_input = 0;
    designer->lv2c.midi_output = 0;
    designer->lv2c.bypass = 0;
    designer->grid_width = 30;
    designer->grid_height = 15;
    designer->is_project = false;

    Xputty app;
    main_init(&app);
    designer->new_label = NULL;
    designer->new_label = (char **)malloc(MAX_CONTROLS * sizeof(char *));
    int m = 0;
    for (;m<MAX_CONTROLS; m++) {
        designer->new_label[m] = NULL;
    }
    
    designer->tab_label = NULL;
    designer->tab_label = (char **)malloc(MAX_CONTROLS * sizeof(char *));
    m = 0;
    for (;m<MAX_CONTROLS; m++) {
        designer->tab_label[m] = NULL;
    }

    m = 0;
    for (;m<MAX_CONTROLS;m++) {
        designer->controls[m].wid = NULL;
        designer->controls[m].image = NULL;
        designer->controls[m].grid_snap_option = 0;
        designer->controls[m].destignation_enabled = false;
        designer->controls[m].name = NULL;
    }

    //set_light_theme(&app);
    designer->w = create_window(&app, DefaultRootWindow(app.dpy), 0, 0, 1200, 800);
    designer->w->parent_struct = designer;
    widget_set_title(designer->w, _("XUiDesigner"));
    widget_set_icon_from_png(designer->w, designer->icon, LDVAR(gear_png));
    designer->w->func.expose_callback = draw_window;

    designer->systray = create_window(&app, DefaultRootWindow(app.dpy), 0, 0, 240, 240);
    designer->systray->parent_struct = designer;
    designer->systray->func.expose_callback = draw_systray;
    widget_get_png(designer->systray, LDVAR(gear_png));
    designer->systray->func.button_release_callback = systray_released;
    send_systray_message(designer->systray);

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
    widget_set_title(designer->ui, _("NoName"));
    designer->ui->parent_struct = designer;
    designer->ui->func.expose_callback = draw_ui;
    designer->ui->func.button_release_callback = button_released_callback;

    designer->widgets = add_combobox(designer->w, "", 20, 25, 120, 30);
    designer->widgets->scale.gravity = CENTER;
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
    combobox_add_entry(designer->widgets,_("WaveView"));
    combobox_add_entry(designer->widgets,_("Frame"));
    combobox_add_entry(designer->widgets,_("Tab Box"));
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

    designer->settings = add_button(designer->w, "", 960, 740, 40, 40);
    widget_get_png(designer->settings, LDVAR(settings_png));
    tooltip_set_text(designer->settings,_("Project Settings"));
    designer->settings->parent_struct = designer;
    designer->settings->func.value_changed_callback = run_settings;

    designer->test = add_button(designer->w, "", 1020, 740, 40, 40);
    widget_get_png(designer->test, LDVAR(gear_png));
    tooltip_set_text(designer->test,_("Run test build"));
    designer->test->parent_struct = designer;
    designer->test->func.value_changed_callback = run_test;

    designer->save = add_button(designer->w, "", 1080, 740, 40, 40);
    widget_get_png(designer->save, LDVAR(save_png));
    tooltip_set_text(designer->save,_("Save"));
    designer->save->parent_struct = designer;
    designer->save->func.value_changed_callback = run_save_as;

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
    set_adjustment(designer->w_axis->adj,10.0, 10.0, 10.0, 600.0, 1.0, CL_CONTINUOS);
    designer->w_axis->func.value_changed_callback = set_w_axis_callback;

    designer->h_axis = add_hslider(designer->w, _("Height"), 1000, 320, 180, 30);
    designer->h_axis->parent_struct = designer;
    set_adjustment(designer->h_axis->adj,10.0, 10.0, 10.0, 600.0, 1.0, CL_CONTINUOS);
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

    designer->tabbox_settings = create_widget(&app, designer->w, 1000, 360, 180, 250);
    add_label(designer->tabbox_settings, _("Tab Box Settings"), 0, 0, 180, 30);
    designer->tabbox_entry[0] = add_button(designer->tabbox_settings, _("Add Tab"), 40, 40, 100, 30);
    designer->tabbox_entry[0]->parent_struct = designer;
    designer->tabbox_entry[0]->func.value_changed_callback = add_tabbox_entry;
    designer->tabbox_entry[1] = add_button(designer->tabbox_settings, _("Remove Tab"), 40, 80, 100, 30);
    designer->tabbox_entry[1]->parent_struct = designer;
    designer->tabbox_entry[1]->func.value_changed_callback = remove_tabbox_entry;

    designer->systray_menu = create_menu(designer->w,25);
    designer->systray_menu->parent_struct = designer;
    menu_add_item(designer->systray_menu,_("Save as:"));
    menu_add_item(designer->systray_menu,_("Setup Project"));
    menu_add_item(designer->systray_menu,_("Quit"));
    designer->systray_menu->func.button_release_callback = systray_menu_response;

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
    menu_add_radio_entry(designer->ctype_switch,_("WaveView"));
    menu_add_radio_entry(designer->ctype_switch,_("Frame"));
    menu_add_radio_entry(designer->ctype_switch,_("Tab Box"));
    designer->ctype_switch->func.value_changed_callback = switch_controller_type;

    menu_add_item(designer->context_menu,_("Delete Controller"));
    designer->context_menu->func.button_release_callback = pop_menu_response;

    create_project_settings_window(designer);

    widget_show_all(designer->ui);
    widget_hide(designer->tabbox_settings);
    main_run(&app);

    //print_ttl(designer);
    lilv_world_free(designer->world);
    fprintf(stderr, "bye, bye\n");
    if (designer->icon) {
        XFreePixmap(designer->w->app->dpy, (*designer->icon));
    }
    main_quit(&app);
    int i = 0;
    for (;i<MAX_CONTROLS; i++) {
        free(designer->new_label[i]);
    }
    free(designer->new_label);
    i = 0;
    for (;i<MAX_CONTROLS; i++) {
        free(designer->tab_label[i]);
    }
    m = 0;
    for (;m<MAX_CONTROLS;m++) {
        free(designer->controls[m].name);
    }
    free(designer->tab_label);
    free(designer->image_path);
    free(designer->image);
    free(designer->lv2c.ui_uri);
    free(designer->lv2c.uri);
    m = 0;
    for (;m<MAX_CONTROLS;m++) {
        free(designer->controls[m].image);
    }
    free(designer);

    return 0;
}
    
