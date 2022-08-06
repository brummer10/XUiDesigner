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


#include "XUiMultiSelect.h"
#include "XUiReparent.h"
#include "XUiControllerType.h"


void fix_pos_for_all(XUiDesigner *designer, WidgetType is_type) {
    int i = 0;
    for (;i<MAX_CONTROLS;i++) {
        if (designer->controls[i].wid != NULL && designer->controls[i].is_type == is_type) {
            Widget_t *wi = designer->controls[i].wid;
            XWindowAttributes attrs;
            XGetWindowAttributes(wi->app->dpy, (Window)wi->widget, &attrs);
            wi->x = attrs.x;
            wi->y = attrs.y;
            widget_draw(wi, NULL);
            check_reparent(designer, NULL, wi);
        }
    }
}

void move_all_for_type(XUiDesigner *designer, WidgetType is_type, int x, int y) {
    int i = 0;
    for (;i<MAX_CONTROLS;i++) {
        if (designer->controls[i].wid != NULL && designer->controls[i].is_type == is_type) {
            Widget_t *wi = designer->controls[i].wid;
            int pos_x = wi->x + x;
            int pos_y = wi->y + y;
            pos_x = max(1, min(designer->ui->width - wi->width, pos_x));
            pos_y = max(1, min(designer->ui->height - wi->height, pos_y));
            int pos_width = wi->width;
            int snap_grid_x = pos_x/designer->grid_width;
            int snap_grid_y = pos_y/designer->grid_height;
            if (designer->grid_view) {
                pos_x = snap_grid_x * designer->grid_width;
                pos_y = snap_grid_y * designer->grid_height;
                if (designer->controls[wi->data].grid_snap_option == 1) {
                    for (;pos_width > designer->grid_width; pos_width -=designer->grid_width);
                    if (wi->width > designer->grid_width) {
                        pos_x += designer->grid_width - pos_width/2;
                    } else {
                        pos_x += designer->grid_width - pos_width * 2;
                    }
                } else if (designer->controls[wi->data].grid_snap_option == 2) {
                    for (;pos_width > designer->grid_width; pos_width -=designer->grid_width);
                    pos_x += designer->grid_width - pos_width;
                }
            }
            XMoveWindow(wi->app->dpy, wi->widget, pos_x, pos_y);
        }
    }
}

void resize_all_for_type(XUiDesigner *designer, Widget_t *wi, WidgetType is_type, int w, int h) {
    int i = 0;
    for (;i<MAX_CONTROLS;i++) {
        if (designer->controls[i].wid != NULL && designer->controls[i].is_type == is_type) {
            Widget_t *wid = designer->controls[i].wid;
            XResizeWindow(wi->app->dpy, wid->widget, max(10,wi->width + w), max(10,wi->height + h));
            if (is_type == IS_TABBOX) {
                int elem = wid->childlist->elem;
                int i = 0;
                for(;i<elem;i++) {
                    Widget_t *win = wid->childlist->childs[i];
                    XResizeWindow(wi->app->dpy, win->widget, max(10,win->width + w),
                                                           max(10,win->height + h));
                }
            }
            wid->scale.ascale = 1.0;
        }
    }
}

void set_ratio(Widget_t *w) {
    XUiDesigner *designer = (XUiDesigner*)w->parent_struct;
    if (designer->active_widget == NULL) return;
    XWindowAttributes attrs;
    XGetWindowAttributes(w->app->dpy, (Window)designer->active_widget->widget, &attrs);
    if (attrs.map_state != IsViewable) return;
    int width = attrs.width;
    int height = attrs.height;
    int v = 0;
    if (w == designer->w_axis) {
        v = adj_get_value(designer->w_axis->adj) - width;
        xevfunc store = designer->h_axis->func.value_changed_callback;
        designer->h_axis->func.value_changed_callback = null_callback;
        adj_set_value(designer->h_axis->adj, adj_get_value(designer->h_axis->adj) + v);
        designer->h_axis->func.value_changed_callback = store;
    } else if (w == designer->h_axis) {
        v = adj_get_value(designer->h_axis->adj) - height;
        xevfunc store = designer->w_axis->func.value_changed_callback;
        designer->w_axis->func.value_changed_callback = null_callback;
        adj_set_value(designer->w_axis->adj, adj_get_value(designer->w_axis->adj) + v);
        designer->w_axis->func.value_changed_callback = store;
    }
    if (adj_get_value(designer->resize_all->adj)) {
        resize_all_for_type(designer, designer->active_widget,
            designer->controls[designer->active_widget_num].is_type, v , v);
    } else {
        XResizeWindow(designer->active_widget->app->dpy, designer->active_widget->widget,
            (int)adj_get_value(designer->w_axis->adj), (int)adj_get_value(designer->h_axis->adj));
    }
}

int is_in_selection(XUiDesigner *designer, int x, int y) {
    if (x > designer->select_x && x < designer->select_width && y >
                designer->select_y && y < designer->select_height) {
        return 1;
    }
    return 0;
}

void fix_pos_for_selection(XUiDesigner *designer) {
    int i = 0;
    for (;i<MAX_CONTROLS;i++) {
        if (designer->controls[i].wid != NULL) {
            Widget_t *wi = designer->controls[i].wid;
            XWindowAttributes attrs;
            XGetWindowAttributes(wi->app->dpy, (Window)wi->widget, &attrs);
            if (is_in_selection(designer, attrs.x, attrs.y)) {
                wi->x = attrs.x;
                wi->y = attrs.y;
                widget_draw(wi, NULL);
                check_reparent(designer, NULL, wi);
            }
        }
    }
}

void move_for_selection(XUiDesigner *designer, int x, int y) {
    int i = 0;
    for (;i<MAX_CONTROLS;i++) {
        if (designer->controls[i].wid != NULL ) {
            Widget_t *wi = designer->controls[i].wid;
            if (is_in_selection(designer, wi->x + x, wi->y + y) &&
                wi->parent == designer->ui) {
                int pos_x = wi->x + x;
                int pos_y = wi->y + y;
                XMoveWindow(wi->app->dpy, wi->widget, pos_x, pos_y);
            }
        }
    }
}

void move_selection(XUiDesigner *designer, XMotionEvent *xmotion) {
    designer->ui->flags |= DONT_PROPAGATE;
    bool moveit = false;
    int pos_x = designer->select_sx + xmotion->x - designer->select_x2;
    int pos_y = designer->select_sy + xmotion->y - designer->select_y2;
    pos_x = max(1, min(designer->ui->width - (designer->select_width -designer->select_x) , pos_x));
    pos_y = max(1, min(designer->ui->height - (designer->select_height - designer->select_y), pos_y));
    int pos_width = designer->select_width;
    int snap_grid_x = pos_x/designer->grid_width;
    int snap_grid_y = pos_y/designer->grid_height;
    if (designer->grid_view) {
        pos_x = snap_grid_x * designer->grid_width;
        pos_y = snap_grid_y * designer->grid_height;
        for (;pos_width > designer->grid_width; pos_width -=designer->grid_width);
        if ((designer->select_width - designer->select_x) > designer->grid_width) {
            pos_x += designer->grid_width - pos_width/2;
        } else {
            pos_x += designer->grid_width - pos_width * 2;
        }
        if (abs(pos_x - designer->select_x) >= designer->grid_width ||
            abs(pos_y - designer->select_y) >= designer->grid_height) {
            moveit = true;
        }
    } else {
        moveit = true;
    }
    if (!moveit) return;
    int move_x = pos_x - designer->select_x;
    int move_y = pos_y - designer->select_y;
    designer->select_x = pos_x;
    designer->select_y = pos_y;
    designer->select_width += move_x;
    designer->select_height += move_y;
    move_for_selection(designer, pos_x - designer->select_sx,
        pos_y - designer->select_sy);
}

void reset_selection(XUiDesigner *designer) {
    designer->select_x = 0;
    designer->select_y = 0;
    designer->select_x2 = 0;
    designer->select_y2 = 0;
    designer->select_sx = 0;
    designer->select_sy = 0;
    designer->select_width = 0;
    designer->select_height = 0;
    designer->multi_selected = 0;
}
