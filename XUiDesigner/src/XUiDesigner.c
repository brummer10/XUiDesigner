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
#include "XUiTurtleView.h"
#include "XUiWritePlugin.h"
#include "XUiSystray.h"
#include "XUiConfig.h"
#include "XUiFileParser.h"
#include "XUiDraw.h"
#include "XUiMultiSelect.h"

#include "xtabbox_private.h"


/*---------------------------------------------------------------------
-----------------------------------------------------------------------    
                designer functions
-----------------------------------------------------------------------
----------------------------------------------------------------------*/


static void set_designer_callbacks(XUiDesigner *designer, Widget_t* wid) {
    if (designer->controls[wid->data].is_type == IS_TABBOX) {
        int v = (int)adj_get_value(wid->adj);
        Widget_t *wi = designer->active_widget->childlist->childs[v];
        box_entry_set_text(designer->controller_label, wi->label);
    } else {
        box_entry_set_text(designer->controller_label, wid->label);
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

void hide_show_as_needed(XUiDesigner *designer) {
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
    if (designer->controls[designer->active_widget_num].is_type == IS_KNOB) {
        widget_show(designer->global_knob_image);
    } else {
        widget_hide(designer->global_knob_image);
    }
    if (designer->controls[designer->active_widget_num].is_type == IS_BUTTON) {
        widget_show(designer->global_button_image);
    } else {
        widget_hide(designer->global_button_image);
    }
    if (designer->controls[designer->active_widget_num].is_type == IS_TOGGLE_BUTTON ||
        designer->controls[designer->active_widget_num].is_type == IS_IMAGE_TOGGLE) {
        widget_show(designer->global_switch_image);
    } else {
        widget_hide(designer->global_switch_image);
    }
    if (designer->grid_view) {
        widget_show(designer->grid_size_x);
        widget_show(designer->grid_size_y);
    } else {
        widget_hide(designer->grid_size_x);
        widget_hide(designer->grid_size_y);
    }

}

void set_controller_callbacks(XUiDesigner *designer, Widget_t *wid, bool set_designer) {
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
    designer->active_widget = wid;
    adj_set_value(designer->index->adj, adj_get_value(designer->index->adj)+1.0);
    designer->controls[designer->wid_counter].port_index = adj_get_value(designer->index->adj);
    designer->controls[designer->wid_counter].in_frame = 0;
    free(designer->controls[designer->active_widget_num].symbol);
    designer->controls[designer->active_widget_num].symbol = NULL;
    asprintf (&designer->controls[designer->active_widget_num].symbol, "%s",wid->label);
    asprintf (&designer->new_label[designer->active_widget_num], "%s",wid->label);
    if (set_designer) {
        set_designer_callbacks(designer, wid);
        widget_show_all(designer->ui);
    }
    designer->wid_counter++;
    Cursor c = XCreateFontCursor(wid->app->dpy, XC_hand2);
    XDefineCursor (wid->app->dpy, wid->widget, c);
    XFreeCursor(wid->app->dpy, c);
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
                designer function callbacks
-----------------------------------------------------------------------
----------------------------------------------------------------------*/


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
            tab->func.expose_callback = draw_tab;
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
    switch(designer->select_widget_num) {
        case 1:
            designer->drag_icon.w = 60;
            designer->drag_icon.h = 80;
        break;
        case 2:
            designer->drag_icon.w = 120;
            designer->drag_icon.h = 30;
        break;
        case 3:
            designer->drag_icon.w = 30;
            designer->drag_icon.h = 120;
        break;
        case 4:
            designer->drag_icon.w = 60;
            designer->drag_icon.h = 60;
        break;
        case 5:
            designer->drag_icon.w = 60;
            designer->drag_icon.h = 60;
        break;
        case 6:
            designer->drag_icon.w = 120;
            designer->drag_icon.h = 30;
        break;
        case 7:
            designer->drag_icon.w = 40;
            designer->drag_icon.h = 30;
        break;
        case 8:
            designer->drag_icon.w = 60;
            designer->drag_icon.h = 30;
        break;
        case 9:
            designer->drag_icon.w = 10;
            designer->drag_icon.h = 120;
        break;
        case 10:
            designer->drag_icon.w = 120;
            designer->drag_icon.h = 10;
        break;
        case 11:
        case 12:
        case 13:
        case 14:
            designer->drag_icon.w = 120;
            designer->drag_icon.h = 120;
        break;
        default:
            designer->drag_icon.w = 1;
            designer->drag_icon.h = 1;
        break;
    }
 }

void x_axis_release_callback(void *w_, void* button_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    XUiDesigner *designer = (XUiDesigner*)w->parent_struct;
    expose_widget(w);
    if (adj_get_value(designer->move_all->adj)) {
        fix_pos_for_all(designer, designer->controls[w->data].is_type);
    }
}

void y_axis_release_callback(void *w_, void* button_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    XUiDesigner *designer = (XUiDesigner*)w->parent_struct;
    expose_widget(w);
    if (adj_get_value(designer->move_all->adj)) {
        fix_pos_for_all(designer, designer->controls[w->data].is_type);
    }
}

static void set_x_axis_callback(void *w_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    XUiDesigner *designer = (XUiDesigner*)w->parent_struct;
    int v = (int)adj_get_value(w->adj);
    if (designer->active_widget != NULL) {
        if (adj_get_value(designer->move_all->adj)) {
            designer->ui->flags |= DONT_PROPAGATE;
            move_all_for_type(designer, designer->controls[designer->active_widget_num].is_type,
                v - designer->active_widget->x, 0);
        } else {
            XMoveWindow(designer->active_widget->app->dpy,
                designer->active_widget->widget,v, (int)adj_get_value(designer->y_axis->adj));
        }
    }
}

static void set_y_axis_callback(void *w_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    XUiDesigner *designer = (XUiDesigner*)w->parent_struct;
    int v = (int)adj_get_value(w->adj);
    if (designer->active_widget != NULL) {
        if (adj_get_value(designer->move_all->adj)) {
            designer->ui->flags |= DONT_PROPAGATE;
            move_all_for_type(designer, designer->controls[designer->active_widget_num].is_type,
                0, v - designer->active_widget->y);
        } else {
            XMoveWindow(designer->active_widget->app->dpy,
                designer->active_widget->widget, (int)adj_get_value(designer->x_axis->adj), v);
        }
    }
}

static void set_w_axis_callback(void *w_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    XUiDesigner *designer = (XUiDesigner*)w->parent_struct;
    if (adj_get_value(designer->aspect_ratio->adj)) {
        set_ratio(w);
        return;
    }
    int v = (int)adj_get_value(w->adj);
    if (designer->active_widget != NULL) {
        if (adj_get_value(designer->resize_all->adj)) {
            resize_all_for_type(designer, designer->active_widget,
                designer->controls[designer->active_widget_num].is_type,
                v - designer->active_widget->width, 0);
        } else {
            XResizeWindow(designer->active_widget->app->dpy,
                designer->active_widget->widget, v, (int)adj_get_value(designer->h_axis->adj));
        }
    }
}

static void set_h_axis_callback(void *w_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    XUiDesigner *designer = (XUiDesigner*)w->parent_struct;
    if (adj_get_value(designer->aspect_ratio->adj)) {
        set_ratio(w);
        return;
    }
    int v = (int)adj_get_value(w->adj);
    if (designer->active_widget != NULL) {
        if (adj_get_value(designer->resize_all->adj)) {
            resize_all_for_type(designer, designer->active_widget,
                designer->controls[designer->active_widget_num].is_type,
                0, v - designer->active_widget->height);
        } else {
            XResizeWindow(designer->active_widget->app->dpy,
                designer->active_widget->widget, (int)adj_get_value(designer->w_axis->adj), v);
        }
    }
}

static void set_drag_icon(void *w_, void *xmotion_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    XUiDesigner *designer = (XUiDesigner*)w->parent_struct;
    XMotionEvent *xmotion = (XMotionEvent*)xmotion_;
    if (adj_get_value(designer->widgets->adj)) {
        designer->drag_icon.x = w->x + xmotion->x - designer->drag_icon.w/2;
        designer->drag_icon.y = w->y + xmotion->y - designer->drag_icon.h/2;
        designer->drag_icon.is_active = true;
        reset_selection(designer);
        widget_draw(designer->ui, NULL);
    } else {
        if (designer->drag_icon.is_active) {
            designer->drag_icon.is_active = false;
            reset_selection(designer);
            widget_draw(designer->ui, NULL);
        }
        if (((xmotion->state & Button1Mask) != 0)) {
            if (designer->multi_selected < 2) {
                designer->select_width = xmotion->x;
                designer->select_height = xmotion->y;
                designer->multi_selected = 1;
            } else if (designer->multi_selected == 2) {
                move_selection(designer, xmotion);
            }
            widget_draw(designer->ui, NULL);
        } else {
            designer->ui->flags &= ~DONT_PROPAGATE;
        }
    }
    //designer->ui->flags &= ~DONT_PROPAGATE;
}

void move_wid(void *w_, void *xmotion_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    Widget_t *p = (Widget_t*)w->parent;
    XUiDesigner *designer = (XUiDesigner*)p->parent_struct;
    XMotionEvent *xmotion = (XMotionEvent*)xmotion_;
    static int is_curser = 2;
    designer->ui->flags |= DONT_PROPAGATE;
    switch(designer->modify_mod) {
        case XUI_POSITION:
        {
            if ((xmotion->state & Button1Mask) == 0) break;
            int x = xmotion->x_root - designer->pos_x;
            int y = xmotion->y_root-designer->pos_y;
            int pos_x = w->x + x;
            int pos_y = w->y + y;
            if (adj_get_value(designer->move_all->adj)) {
                move_all_for_type(designer, designer->controls[w->data].is_type, x, y);
            } else {
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
                pos_x = max(1, min(designer->ui->width - w->width, pos_x));
                pos_y = max(1, min(designer->ui->height - w->height, pos_y));
                XMoveWindow(w->app->dpy,w->widget,pos_x, pos_y);
            }
            xevfunc store = designer->x_axis->func.value_changed_callback;
            designer->x_axis->func.value_changed_callback = null_callback;
            adj_set_value(designer->x_axis->adj, pos_x);
            designer->x_axis->func.value_changed_callback = store;
            store = designer->y_axis->func.value_changed_callback;
            designer->y_axis->func.value_changed_callback = null_callback;
            adj_set_value(designer->y_axis->adj, pos_y);
            designer->y_axis->func.value_changed_callback = store;
        }
        break;
        case XUI_SIZE:
        {
            int v = fabs(xmotion->x_root-designer->pos_x) > fabs(xmotion->y_root-designer->pos_y) ? 
                xmotion->x_root-designer->pos_x : xmotion->y_root-designer->pos_y;
            if (adj_get_value(designer->resize_all->adj)) {
                resize_all_for_type(designer, w, designer->controls[w->data].is_type, v, v);
            } else {
                XResizeWindow(w->app->dpy, w->widget, max(10,w->width + v), max(10,w->height + v));
                if (designer->controls[w->data].is_type == IS_TABBOX) {
                    int elem = w->childlist->elem;
                    int i = 0;
                    for(;i<elem;i++) {
                        Widget_t *wi = w->childlist->childs[i];
                        XResizeWindow(w->app->dpy, wi->widget, max(10,wi->width + v),
                                                               max(10,wi->height + v));
                    }
                }
            }
            xevfunc store = designer->w_axis->func.value_changed_callback;
            designer->w_axis->func.value_changed_callback = null_callback;
            adj_set_value(designer->w_axis->adj, w->width + v);
            designer->w_axis->func.value_changed_callback = store;
            store = designer->h_axis->func.value_changed_callback;
            designer->h_axis->func.value_changed_callback = null_callback;
            adj_set_value(designer->h_axis->adj, w->height + v);
            designer->h_axis->func.value_changed_callback = store;
            w->scale.ascale = 1.0;
            designer->pos_x = xmotion->x_root;
            designer->pos_y = xmotion->y_root;
            if (designer->controls[w->data].is_type == IS_TABBOX ||
                    designer->controls[w->data].is_type == IS_FRAME) {
                expose_widget(designer->ui);
            }
        }
        break;
        case XUI_WIDTH:
        {
            if (adj_get_value(designer->resize_all->adj)) {
                resize_all_for_type(designer, w, designer->controls[w->data].is_type,
                            xmotion->x_root-designer->pos_x, 0);
            } else {
                XResizeWindow(w->app->dpy, w->widget, max(10,w->width + (xmotion->x_root-designer->pos_x)), w->height);
                if (designer->controls[w->data].is_type == IS_TABBOX) {
                    int elem = w->childlist->elem;
                    int i = 0;
                    for(;i<elem;i++) {
                        Widget_t *wi = w->childlist->childs[i];
                        XResizeWindow(w->app->dpy, wi->widget,
                            max(10,wi->width + (xmotion->x_root-designer->pos_x)), wi->height);
                    }
                }
            }
            xevfunc store = designer->w_axis->func.value_changed_callback;
            designer->w_axis->func.value_changed_callback = null_callback;
            adj_set_value(designer->w_axis->adj, w->width + ((xmotion->x_root-designer->pos_x)));
            designer->w_axis->func.value_changed_callback = store;
            w->scale.ascale = 1.0;
            designer->pos_x = xmotion->x_root;
            if (designer->controls[w->data].is_type == IS_TABBOX ||
                    designer->controls[w->data].is_type == IS_FRAME) {
                expose_widget(designer->ui);
            }
        }
        break;
        case XUI_HEIGHT:
        {
            if (adj_get_value(designer->resize_all->adj)) {
                resize_all_for_type(designer, w, designer->controls[w->data].is_type,
                            0, xmotion->y_root-designer->pos_y);
            } else {
                XResizeWindow(w->app->dpy, w->widget, w->width, max(10,w->height + (xmotion->y_root-designer->pos_y)));
                if (designer->controls[w->data].is_type == IS_TABBOX) {
                    int elem = w->childlist->elem;
                    int i = 0;
                    for(;i<elem;i++) {
                        Widget_t *wi = w->childlist->childs[i];
                        XResizeWindow(w->app->dpy, wi->widget, wi->width,
                            max(10,wi->height+ (xmotion->x_root-designer->pos_x)));
                    }
                }
            }
            xevfunc store = designer->h_axis->func.value_changed_callback;
            designer->h_axis->func.value_changed_callback = null_callback;
            adj_set_value(designer->h_axis->adj, w->height + (xmotion->y_root-designer->pos_y));
            designer->h_axis->func.value_changed_callback = store;
            w->scale.ascale = 1.0;
            designer->pos_y = xmotion->y_root;
            if (designer->controls[w->data].is_type == IS_TABBOX ||
                    designer->controls[w->data].is_type == IS_FRAME) {
                expose_widget(designer->ui);
            }
        }
        break;
        case XUI_NONE:
        {
            if (xmotion->x > w->width-10 && xmotion->y > w->height-10 && is_curser != 1) {
                is_curser = 1;
                Cursor c = XCreateFontCursor(w->app->dpy, XC_bottom_right_corner);
                XDefineCursor (w->app->dpy, w->widget, c);
                XFreeCursor(w->app->dpy, c);
            } else if (xmotion->x < w->width-10 &&  xmotion->y > w->height-10 && is_curser != 4) {
                is_curser = 4;
                Cursor c = XCreateFontCursor(w->app->dpy, XC_bottom_side);
                XDefineCursor (w->app->dpy, w->widget, c);
                XFreeCursor(w->app->dpy, c);
            } else if (xmotion->x > w->width-10 && xmotion->y < w->height-10 && is_curser != 3) {
                is_curser = 3;
                Cursor c = XCreateFontCursor(w->app->dpy, XC_right_side);
                XDefineCursor (w->app->dpy, w->widget, c);
                XFreeCursor(w->app->dpy, c);
            } else if ((xmotion->x <= w->width-10 && xmotion->y <= w->height-10) && is_curser != 2) {
                is_curser = 2;
                Cursor c = XCreateFontCursor(w->app->dpy, XC_hand2);
                XDefineCursor (w->app->dpy, w->widget, c);
                XFreeCursor(w->app->dpy, c);
            } else {
                is_curser = 0;
            }
        }
        break;
        default:
        break;
    }
}

static void set_cursor(void *w_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    XUiDesigner *designer = (XUiDesigner*)w->parent_struct;
    if (adj_get_value(designer->widgets->adj)) {
        designer->cursor = XCreateFontCursor(w->app->dpy, XC_diamond_cross);
        XDefineCursor (w->app->dpy, w->widget, designer->cursor);
    }
}

static void unset_cursor(void *w_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    XUiDesigner *designer = (XUiDesigner*)w->parent_struct;
    if (designer->cursor) {
        XUndefineCursor (w->app->dpy, w->widget);
        XFreeCursor(designer->ui->app->dpy, designer->cursor);
        designer->cursor = 0;
    }
    if (designer->drag_icon.is_active) {
        designer->drag_icon.is_active = false;
        expose_widget(designer->ui);
    }
}

void set_pos_wid(void *w_, void *button_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    Widget_t *p = (Widget_t*)w->parent;
    XWindowAttributes attrs;
    XGetWindowAttributes(w->app->dpy, (Window)w->widget, &attrs);
    int width = attrs.width;
    int height = attrs.height;
    XUiDesigner *designer = (XUiDesigner*)p->parent_struct;
    designer->active_widget_num = -1;
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

        hide_show_as_needed(designer);
    }
    if (xbutton->x > width-10 && xbutton->y > height-10) {
        designer->modify_mod = XUI_SIZE;
    } else if (xbutton->y > height-10) {
        if (adj_get_value(designer->aspect_ratio->adj)) {
            designer->modify_mod = XUI_SIZE;
        } else {
            designer->modify_mod = XUI_HEIGHT;
        }
    } else if (xbutton->x > width-10) {
        if (adj_get_value(designer->aspect_ratio->adj)) {
            designer->modify_mod = XUI_SIZE;
        } else {
            designer->modify_mod = XUI_WIDTH;
        }
    } else {
        designer->modify_mod = XUI_POSITION;
    }
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
        if (adj_get_value(designer->move_all->adj)) {
            fix_pos_for_all(designer, designer->controls[w->data].is_type);
            return;
        }
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
            box_entry_set_text(designer->controller_label, wi->label);
        } else if (designer->controls[w->data].is_type == IS_COMBOBOX) {
            Widget_t *wi = w->childlist->childs[0];
            wi->x = width-wi->width;
            wi->scale.init_x = wi->x;
            wi->scale.init_width = wi->width;
            wi->scale.init_height = wi->height;
            wi->scale.ascale = 1.0;
        }
        if (designer->controls[designer->active_widget_num].is_type != IS_FRAME &&
            designer->controls[designer->active_widget_num].is_type != IS_IMAGE &&
            designer->controls[designer->active_widget_num].is_type != IS_TABBOX) {
                
                check_reparent(designer, xbutton, w);
                
            }
        expose_widget(p);
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
        if (designer->controls[designer->active_widget_num].is_type == IS_FRAME ||
            designer->controls[designer->active_widget_num].is_type == IS_IMAGE ||
            designer->controls[designer->active_widget_num].is_type == IS_TABBOX ||
            designer->controls[designer->active_widget_num].is_type == IS_WAVEVIEW ) {
            designer->ctype_switch->state = 4;
        } else {
            designer->ctype_switch->state = 0;
            store = designer->ctype_switch->func.value_changed_callback;
            designer->ctype_switch->func.value_changed_callback = null_callback;
            set_active_radio_entry_num(designer->ctype_switch, sel);
            designer->ctype_switch->func.value_changed_callback = store;
        }
        pop_menu_show(w,designer->context_menu,5,true);
    }
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

static void button_press_callback(void *w_, void *button_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    XUiDesigner *designer = (XUiDesigner*)w->parent_struct;
    XButtonEvent *xbutton = (XButtonEvent*)button_;
    if (!is_in_selection(designer, xbutton->x, xbutton->y)) {
        designer->select_x = xbutton->x;
        designer->select_y = xbutton->y;
        designer->select_sx = xbutton->x;
        designer->select_sy = xbutton->y;
        designer->multi_selected = 0;
    } else if (designer->multi_selected == 2) {
        designer->select_x2 = xbutton->x;
        designer->select_y2 = xbutton->y;
    }
}

static void button_release_callback(void *w_, void *button_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    XUiDesigner *designer = (XUiDesigner*)w->parent_struct;
    if (designer->wid_counter >= MAX_CONTROLS) {
        Widget_t *dia = open_message_dialog(w, INFO_BOX, _("INFO"),
                            _("MAX CONTROL COUNTER OVERFLOW"),NULL);
        XSetTransientForHint(w->app->dpy, dia->widget, w->widget);
        return;
    }
    //fprintf(stderr, "%i\n", designer->select_widget_num);
    XButtonEvent *xbutton = (XButtonEvent*)button_;
    if(xbutton->button == Button1) {
        Widget_t *wid = NULL;
        if ((wid = add_controller(designer, xbutton, wid)) == NULL) {
            if (!is_in_selection(designer, xbutton->x, xbutton->y) &&
                                        designer->multi_selected < 2) {
                designer->select_width = xbutton->x;
                designer->select_height = xbutton->y;
                designer->multi_selected = 2;
                widget_draw(designer->ui, NULL);
            } else if (designer->multi_selected == 2) {
                fix_pos_for_selection(designer);
                designer->select_sx = designer->select_x;
                designer->select_sy = designer->select_y;
            }
        }
        hide_show_as_needed(designer);
        designer->prev_active_widget = wid;
        widget_hide(designer->set_project);
    } else if(xbutton->button == Button3) {
        reset_selection(designer);
        adj_set_value(designer->widgets->adj, 0);
        widget_draw(designer->ui, NULL);
        if (designer->cursor) {
            XUndefineCursor (w->app->dpy, w->widget);
            XFreeCursor(designer->ui->app->dpy, designer->cursor);
            designer->cursor = 0;
        }
        if (designer->drag_icon.is_active) {
            designer->drag_icon.is_active = false;
            expose_widget(designer->ui);
        }
    }
}

static void update_clabel(void *w_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    XUiDesigner *designer = (XUiDesigner*)w->parent_struct;
    TextBox_t *text_box = (TextBox_t*)w->private_struct;
    if (designer->active_widget == NULL) return;
    if (designer->controls[designer->active_widget_num].is_type == IS_TABBOX) {
        int v = (int)adj_get_value(designer->active_widget->adj);
        if (designer->active_widget_num+v > MAX_CONTROLS) {
            Widget_t *dia = open_message_dialog(w, INFO_BOX, _("INFO"),
                _("MAX CONTROL COUNTER OVERFLOW | Sorry, cant edit the label anymore"),NULL);
            XSetTransientForHint(w->app->dpy, dia->widget, w->widget);
            return;
        }
        free(designer->tab_label[designer->active_widget_num+v]);
        designer->tab_label[designer->active_widget_num+v] = NULL;
        asprintf (&designer->tab_label[designer->active_widget_num+v], "%s", text_box->input_label);
        //designer->tab_label[designer->active_widget_num+v][strlen( text_box->input_label)-1] = 0;
        Widget_t *wi = designer->active_widget->childlist->childs[v];
        wi->label = (const char*)designer->tab_label[designer->active_widget_num+v];
    } else {
        free(designer->new_label[designer->active_widget_num]);
        designer->new_label[designer->active_widget_num] = NULL;
        asprintf (&designer->new_label[designer->active_widget_num], "%s", text_box->input_label);
        //designer->new_label[designer->active_widget_num][strlen( text_box->input_label)-1] = 0;
        designer->active_widget->label = (const char*)designer->new_label[designer->active_widget_num];
    }
    expose_widget(designer->active_widget);
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
                save as LV2 bundle
-----------------------------------------------------------------------
----------------------------------------------------------------------*/


static void run_save_as(void *w_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    XUiDesigner *designer = (XUiDesigner*)w->parent_struct;
    const char* home = getenv("HOME");
    Widget_t *dia = open_directory_dialog(designer->ui, home);
    XSetTransientForHint(w->app->dpy, dia->widget, designer->ui->widget);
    designer->ui->func.dialog_callback = run_save;
}

static void save_response(void *w_, void* user_data) {
    if(user_data !=NULL) {
        Widget_t *w = (Widget_t*)w_;
        XUiDesigner *designer = (XUiDesigner*)w->parent_struct;
        int response = *(int*)user_data;
        if(response == 1)
            designer->generate_ui_only = true;
        else
            designer->generate_ui_only = false;
        run_save_as(w_, user_data);
    }
}

static void ask_save_as(void *w_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    if (w->flags & HAS_POINTER && !adj_get_value(w->adj_y)) {
        XUiDesigner *designer = (XUiDesigner*)w->parent_struct;
        Widget_t *dia = open_message_dialog(designer->ui, SELECTION_BOX,
            "Save as:",  "Save as:", "Only UI-Bundle       |Full Plugin-Bundle       ");
        XSetTransientForHint(w->app->dpy, dia->widget, designer->ui->widget);
        designer->ui->func.dialog_callback = save_response;
    }
}

/*---------------------------------------------------------------------
-----------------------------------------------------------------------    
                filter LV2 plugins, show only UI-less
-----------------------------------------------------------------------
----------------------------------------------------------------------*/


static void filter_plugin_ui(void *w_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    if (w->flags & HAS_POINTER && adj_get_value(w->adj_y)) {
        XUiDesigner *designer = (XUiDesigner*)w->parent_struct;
        filter_uris(designer->lv2_uris, designer->lv2_names, designer->lv2_plugins);
    } else if (w->flags & HAS_POINTER && !adj_get_value(w->adj_y)) {
        XUiDesigner *designer = (XUiDesigner*)w->parent_struct;
        combobox_delete_entrys(designer->lv2_uris);
        combobox_delete_entrys(designer->lv2_names);
        combobox_add_entry(designer->lv2_uris,_("--"));
        combobox_add_entry(designer->lv2_names,_("--"));
        load_uris(designer->lv2_uris, designer->lv2_names, designer->lv2_plugins);
        combobox_set_active_entry(designer->lv2_uris, 0);
        combobox_set_active_entry(designer->lv2_names, 0);
    }
}

/*---------------------------------------------------------------------
-----------------------------------------------------------------------    
                keep editor window centered on main window
-----------------------------------------------------------------------
----------------------------------------------------------------------*/

static void win_configure_callback(void *w_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    XUiDesigner *designer = (XUiDesigner*)w->parent_struct;
    XWindowAttributes attrs;
    XGetWindowAttributes(w->app->dpy, (Window)w->widget, &attrs);
    if (attrs.map_state != IsViewable) return;

    int width = attrs.width;
    int height = attrs.height;
    int x1, y1;
    Window child;
    XTranslateCoordinates( w->app->dpy, w->widget, DefaultRootWindow(
                    w->app->dpy), 0, 0, &x1, &y1, &child );

    XGetWindowAttributes(w->app->dpy, (Window)designer->ui->widget, &attrs);
    if (attrs.map_state != IsViewable) return;

    XMoveWindow(w->app->dpy,designer->ui->widget, x1 +
        ((width - attrs.width)/2), y1 + ((height - attrs.height)/2));
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
    bindtextdomain(GETTEXT_PACKAGE, LOCAL_DIR);
    bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
    textdomain(GETTEXT_PACKAGE);
#endif

    // set Ctype to C to avoid symbol clashes from different locales
    setlocale (LC_CTYPE, "C");

    extern char *optarg;
    char *path = NULL;
    char *ffile = NULL;
    int a = 0;
    static char usage[] = "usage: %s \n"
    "[-p path] optional set a path to open a ttl file from\n"
    "[-f faust] optional set a faust *.dsp file to parse from\n";

    while ((a = getopt(argc, argv, "p:f:h?")) != -1) {
        switch (a) {
            break;
            case 'p': path = optarg;
            break;
            case 'f': ffile = optarg;
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
    designer->faust_file = NULL;
    designer->global_knob_image_file = NULL;
    designer->global_button_image_file = NULL;
    designer->global_switch_image_file = NULL;
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
    designer->lv2c.symbol = NULL;
    designer->lv2c.name = NULL;
    designer->lv2c.audio_input = 1;
    designer->lv2c.audio_output = 1;
    designer->lv2c.midi_input = 0;
    designer->lv2c.midi_output = 0;
    designer->lv2c.atom_output_port = -1;
    designer->lv2c.atom_input_port = -1;
    designer->lv2c.bypass = 0;
    designer->grid_width = 15;
    designer->grid_height = 15;
    designer->is_project = true;
    designer->is_faust_file = false;
    designer->generate_ui_only = false;
    designer->drag_icon.x = 0;
    designer->drag_icon.w = 0;
    designer->drag_icon.y = 0;
    designer->drag_icon.h = 0;
    designer->drag_icon.is_active = false;
    reset_selection(designer);

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
        designer->controls[m].symbol = NULL;
    }

    Xputty app;
    main_init(&app);
    //set_light_theme(&app);
    designer->w = create_window(&app, DefaultRootWindow(app.dpy), 0, 0, 1200, 800);
    designer->w->parent_struct = designer;
    designer->w->flags |= DONT_PROPAGATE;
    widget_set_title(designer->w, _("XUiDesigner"));
    widget_set_icon_from_png(designer->w, designer->icon, LDVAR(gear_png));
    designer->w->func.expose_callback = draw_window;
    widget_set_dnd_aware(designer->w);
    designer->w->func.dnd_notify_callback = dnd_load_response;
    designer->w->func.configure_notify_callback = win_configure_callback;

    create_systray_widget(designer, 0, 0, 240, 240);

    designer->world = lilv_world_new();
    if (path !=NULL) set_path(designer->world, path);
    lilv_world_load_all(designer->world);
    designer->lv2_plugins = lilv_world_get_all_plugins(designer->world);

    designer->lv2_uris = add_combobox(designer->w, "", 300, 25, 600, 30);
    designer->lv2_uris->parent_struct = designer;
    designer->lv2_uris->flags |= IS_TOOLTIP;
    combobox_add_entry(designer->lv2_uris,_("--"));

    designer->lv2_names = add_combobox(designer->w, "", 300, 25, 600, 30);
    designer->lv2_names->parent_struct = designer;
    tooltip_set_text(designer->lv2_names->childlist->childs[0], _("Select LV2 Plugin"));
    combobox_add_entry(designer->lv2_names,_("--"));
    load_uris(designer->lv2_uris, designer->lv2_names, designer->lv2_plugins);
    combobox_set_active_entry(designer->lv2_names, 0);
    combobox_set_active_entry(designer->lv2_uris, 0);
    designer->lv2_names->func.value_changed_callback = load_plugin_ui;
    combobox_set_menu_size(designer->lv2_names, 24);

    designer->filter_lv2_uris = add_toggle_button(designer->w, "Filter", 250, 25, 40, 30);
    tooltip_set_text(designer->filter_lv2_uris,_("Show only UI-less plugins"));
    designer->filter_lv2_uris->parent_struct = designer;
    designer->filter_lv2_uris->func.value_changed_callback = filter_plugin_ui;

    designer->ui = create_window(&app, DefaultRootWindow(app.dpy), 0, 0, 600, 400);
    XSelectInput(designer->ui->app->dpy, designer->ui->widget,StructureNotifyMask|ExposureMask|KeyPressMask 
                    |EnterWindowMask|LeaveWindowMask|ButtonReleaseMask
                    |ButtonPressMask|Button1MotionMask|PointerMotionMask);
   // Atom wmStateAbove = XInternAtom(app.dpy, "_NET_WM_STATE_ABOVE", 1 );
  //  Atom wmNetWmState = XInternAtom(app.dpy, "_NET_WM_STATE", 1 );
  //  XChangeProperty(app.dpy, designer->ui->widget, wmNetWmState, XA_ATOM, 32, 
   //     PropModeReplace, (unsigned char *) &wmStateAbove, 1); 
    XSetTransientForHint(app.dpy, designer->ui->widget, designer->w->widget);
    widget_set_title(designer->ui, _("NoName"));
    designer->ui->parent_struct = designer;
    designer->ui->flags |= HIDE_ON_DELETE | NO_PROPAGATE;
    designer->ui->func.expose_callback = draw_ui;
    designer->ui->func.button_press_callback = button_press_callback;
    designer->ui->func.button_release_callback = button_release_callback;
    designer->ui->func.enter_callback = set_cursor;
    designer->ui->func.leave_callback = unset_cursor;
    designer->ui->func.motion_callback = set_drag_icon;
    designer->ui->func.map_notify_callback = transparent_draw;
    designer->grid_image = cairo_image_surface_create ( CAIRO_FORMAT_ARGB32,
        DisplayWidth(app.dpy, DefaultScreen(app.dpy)), DisplayHeight(app.dpy, DefaultScreen(app.dpy)));
    draw_grid(designer->ui, designer->grid_image);

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
    combobox_add_entry(designer->widgets,_("Image"));
    combobox_set_active_entry(designer->widgets, 0);
    tooltip_set_text(designer->widgets->childlist->childs[0], _("Select Controller Type"));
    combobox_set_menu_size(designer->widgets, 15);
    designer->widgets->func.value_changed_callback = set_widget_callback;

    designer->image_loader = add_image_button(designer->w,20,75,40,40, "", "image");
    tooltip_set_text(designer->image_loader,_("Load Background Image (*.png | *.svg)"));
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

    designer->settings = add_button(designer->w, "", 900, 740, 40, 40);
    widget_get_png(designer->settings, LDVAR(settings_png));
    tooltip_set_text(designer->settings,_("Project Settings"));
    designer->settings->parent_struct = designer;
    designer->settings->func.value_changed_callback = run_settings;

    designer->ttlfile = add_button(designer->w, "", 960, 740, 40, 40);
    widget_get_png(designer->ttlfile, LDVAR(file_png));
    tooltip_set_text(designer->ttlfile,_("Show ttl file"));
    designer->ttlfile->parent_struct = designer;
    designer->ttlfile->func.value_changed_callback = run_generate_ttl;

    designer->test = add_button(designer->w, "", 1020, 740, 40, 40);
    widget_get_png(designer->test, LDVAR(gear_png));
    tooltip_set_text(designer->test,_("Run test build"));
    designer->test->parent_struct = designer;
    designer->test->func.value_changed_callback = run_test;

    designer->save = add_button(designer->w, "", 1080, 740, 40, 40);
    widget_get_png(designer->save, LDVAR(save_png));
    tooltip_set_text(designer->save,_("Save"));
    designer->save->parent_struct = designer;
    designer->save->func.value_changed_callback = ask_save_as;

    designer->exit = add_button(designer->w, "", 1140, 740, 40, 40);
    widget_get_png(designer->exit, LDVAR(exit_png));
    tooltip_set_text(designer->exit,_("Exit"));
    designer->exit->parent_struct = designer;
    designer->exit->func.value_changed_callback = run_exit;

    add_label(designer->w, _("Label"), 1000, 10, 180, 30);
    designer->controller_label = add_input_box(designer->w, 0, 1000, 50, 180, 30);
    designer->controller_label->func.user_callback = update_clabel;
    designer->controller_label->func.value_changed_callback = update_clabel;
    designer->controller_label->parent_struct = designer;

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
    designer->x_axis->func.button_release_callback = x_axis_release_callback;

    designer->y_axis = add_hslider(designer->w, _("Y"), 1000, 240, 180, 30);
    designer->y_axis->parent_struct = designer;
    set_adjustment(designer->y_axis->adj,0.0, 0.0, 0.0, 600.0, 1.0, CL_CONTINUOS);
    designer->y_axis->func.value_changed_callback = set_y_axis_callback;
    designer->y_axis->func.button_release_callback = y_axis_release_callback;

    designer->w_axis = add_hslider(designer->w, _("Width"), 1000, 280, 180, 30);
    designer->w_axis->parent_struct = designer;
    set_adjustment(designer->w_axis->adj,10.0, 10.0, 10.0, 600.0, 1.0, CL_CONTINUOS);
    designer->w_axis->func.value_changed_callback = set_w_axis_callback;

    designer->h_axis = add_hslider(designer->w, _("Height"), 1000, 320, 180, 30);
    designer->h_axis->parent_struct = designer;
    set_adjustment(designer->h_axis->adj,10.0, 10.0, 10.0, 600.0, 1.0, CL_CONTINUOS);
    designer->h_axis->func.value_changed_callback = set_h_axis_callback;

    designer->resize_all = add_check_box(designer->w, _("  Global size"), 1020, 360, 180, 20);
    tooltip_set_text(designer->resize_all,_("Resize all Controller of the same type"));
    designer->resize_all->parent_struct = designer;

    designer->aspect_ratio = add_check_box(designer->w, _("  Aspect Ratio"), 1020, 390, 180, 20);
    tooltip_set_text(designer->aspect_ratio,_("Keep Aspect Ratio when resize a Controller"));
    designer->aspect_ratio->parent_struct = designer;

    designer->move_all = add_check_box(designer->w, _("  Move All"), 1020, 420, 180, 20);
    tooltip_set_text(designer->move_all,_("Move all Controller of the same type"));
    designer->move_all->parent_struct = designer;


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

    designer->combobox_settings = create_widget(&app, designer->w, 1000, 440, 180, 200);
    add_label(designer->combobox_settings, _("Add Combobox Entry"), 0, 0, 180, 30);
    designer->combobox_entry = add_input_box(designer->combobox_settings, 0, 0, 40, 140, 30);
    designer->combobox_entry->parent_struct = designer;
    designer->combobox_entry->func.user_callback = set_combobox_entry;

    designer->global_button_image = add_check_box(designer->w, _("Use Global Button Image"), 1000, 450, 180, 20);
    tooltip_set_text(designer->global_button_image,_("Use the Image loaded on one Button for all Buttons"));
    designer->global_button_image->parent_struct = designer;

    designer->global_switch_image = add_check_box(designer->w, _("Use Global Switch Image"), 1000, 450, 180, 20);
    tooltip_set_text(designer->global_switch_image,_("Use the Image loaded on one Switch for all Switchs"));
    designer->global_switch_image->parent_struct = designer;

    designer->add_entry = add_button(designer->combobox_settings, _("Add"), 140, 40, 40, 30);
    designer->add_entry->parent_struct = designer;
    designer->add_entry->func.value_changed_callback = add_combobox_entry;

    designer->controller_settings = create_widget(&app, designer->w, 1000, 440, 180, 270);
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

    designer->global_knob_image = add_check_box(designer->controller_settings, _("Use Global Knob Image"), 1, 240, 160, 20);
    tooltip_set_text(designer->global_knob_image,_("Use the Image loaded on one Knob for all Knobs"));
    designer->global_knob_image->parent_struct = designer;

    designer->tabbox_settings = create_widget(&app, designer->w, 1000, 440, 180, 250);
    add_label(designer->tabbox_settings, _("Tab Box Settings"), 0, 0, 180, 30);
    designer->tabbox_entry[0] = add_button(designer->tabbox_settings, _("Add Tab"), 40, 40, 100, 30);
    designer->tabbox_entry[0]->parent_struct = designer;
    designer->tabbox_entry[0]->func.value_changed_callback = add_tabbox_entry;
    designer->tabbox_entry[1] = add_button(designer->tabbox_settings, _("Remove Tab"), 40, 80, 100, 30);
    designer->tabbox_entry[1]->parent_struct = designer;
    designer->tabbox_entry[1]->func.value_changed_callback = remove_tabbox_entry;

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
    //menu_add_radio_entry(designer->ctype_switch,_("WaveView"));
    //menu_add_radio_entry(designer->ctype_switch,_("Frame"));
    //menu_add_radio_entry(designer->ctype_switch,_("Tab Box"));
    //menu_add_radio_entry(designer->ctype_switch,_("Image"));
    designer->ctype_switch->func.value_changed_callback = switch_controller_type;

    menu_add_item(designer->context_menu,_("Delete Controller"));
    designer->context_menu->func.button_release_callback = pop_menu_response;

    create_project_settings_window(designer);
    create_text_view_window(designer);

    widget_show_all(designer->w);
    widget_show_all(designer->ui);
    hide_show_as_needed(designer);
    read_config(designer);
    if (ffile != NULL) parse_faust_file(designer, ffile);
    main_run(&app);

    save_config(designer);
    //print_ttl(designer);
    lilv_world_free(designer->world);
    fprintf(stderr, "bye, bye\n");
    if (designer->icon) {
        XFreePixmap(designer->w->app->dpy, (*designer->icon));
    }
    main_quit(&app);
    cairo_surface_destroy(designer->grid_image);
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
        free(designer->controls[m].symbol);
    }
    free(designer->tab_label);
    free(designer->image_path);
    free(designer->image);
    free(designer->faust_file);
    free(designer->global_knob_image_file);
    free(designer->global_button_image_file);
    free(designer->global_switch_image_file);
    free(designer->lv2c.ui_uri);
    free(designer->lv2c.uri);
    free(designer->lv2c.author);
    free(designer->lv2c.name);
    free(designer->lv2c.plugintype);
    free(designer->lv2c.symbol);
    m = 0;
    for (;m<MAX_CONTROLS;m++) {
        free(designer->controls[m].image);
    }
    free(designer);

    return 0;
}
    
