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

#include "XUiGridControl.h"


/*---------------------------------------------------------------------
-----------------------------------------------------------------------    
                Grid control
-----------------------------------------------------------------------
----------------------------------------------------------------------*/

void draw_grid(Widget_t *w, cairo_surface_t *image) {
    XUiDesigner *designer = (XUiDesigner*)w->parent_struct;    
    int width = cairo_image_surface_get_width(image);
    int height = cairo_image_surface_get_height(image);

    cairo_t *crb = cairo_create (image);
    cairo_set_source_rgba(crb, 0.0, 0.0, 0.0, 0.0);
    cairo_set_operator(crb,CAIRO_OPERATOR_CLEAR);
    cairo_paint(crb);
    cairo_set_operator(crb,CAIRO_OPERATOR_OVER);

    Colors *c = get_color_scheme(w, INSENSITIVE_);
    cairo_set_source_rgba(crb, c->frame[0],  c->frame[1], c->frame[2],  c->frame[3]);
    cairo_set_line_width(crb, 1.0);

    int i = 0;
    cairo_move_to(crb, 0, 0);
    for (;i<width;i +=designer->grid_width) {
        cairo_move_to(crb, i, 0);
        cairo_line_to(crb,i, height);
        cairo_stroke_preserve(crb);
    }
    i = 0;
    cairo_move_to(crb, 0, 0);
    for (;i<height;i +=designer->grid_height) {
        cairo_move_to(crb, 0, i);
        cairo_line_to(crb,width, i);
        cairo_stroke_preserve(crb);
    }
    cairo_stroke(crb);
    cairo_destroy(crb);
}

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

void set_grid_width(void *w_, void* UNUSED(user_data)) {
    Widget_t *w = (Widget_t*)w_;
    XUiDesigner *designer = (XUiDesigner*)w->parent_struct;
    designer->grid_width = (int)adj_get_value(w->adj);
    draw_grid(designer->ui, designer->grid_image);
    snap_to_grid(designer);
}

void set_grid_height(void *w_, void* UNUSED(user_data)) {
    Widget_t *w = (Widget_t*)w_;
    XUiDesigner *designer = (XUiDesigner*)w->parent_struct;
    designer->grid_height = (int)adj_get_value(w->adj);
    draw_grid(designer->ui, designer->grid_image);
    snap_to_grid(designer);
}

void use_grid(void *w_, void* UNUSED(user_data)) {
    Widget_t *w = (Widget_t*)w_;
    XUiDesigner *designer = (XUiDesigner*)w->parent_struct;
    if (!designer->grid_image) {
        designer->grid_image = cairo_image_surface_create ( CAIRO_FORMAT_ARGB32,
                    DisplayWidth(w->app->dpy, DefaultScreen(w->app->dpy)),
                    DisplayHeight(w->app->dpy, DefaultScreen(w->app->dpy)));
        draw_grid(designer->ui, designer->grid_image);
    }
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

void select_grid_mode(void *w_, void* UNUSED(user_data)) {
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
