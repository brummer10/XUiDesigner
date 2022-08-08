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


#include "XUiDraw.h"

/*---------------------------------------------------------------------
-----------------------------------------------------------------------    
                designer drawing calls
-----------------------------------------------------------------------
----------------------------------------------------------------------*/


void draw_window(void *w_, void* UNUSED(user_data)) {
    Widget_t *w = (Widget_t*)w_;
    //use_bg_color_scheme(w, NORMAL_);
    cairo_set_source_rgba(w->crb,  0.13, 0.13, 0.13, 1.0);
    cairo_paint (w->crb);
}

void draw_ui(void *w_, void* UNUSED(user_data)) {
    Widget_t *w = (Widget_t*)w_;
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
        cairo_set_source_surface (w->crb, designer->grid_image, 0, 0);
        cairo_paint (w->crb);
    }
    if (designer->drag_icon.is_active) {
        use_shadow_color_scheme(w, SELECTED_);
        cairo_rectangle(w->crb, designer->drag_icon.x, designer->drag_icon.y,
                                designer->drag_icon.w, designer->drag_icon.h);
        cairo_fill(w->crb);
    }
    if (designer->active_widget && designer->active_widget->parent == w) {
        XWindowAttributes attrs;
        XGetWindowAttributes(w->app->dpy, (Window)designer->active_widget->widget, &attrs);
        int x = attrs.x -1;
        int y = attrs.y -1;
        int width = attrs.width +2;
        int height = attrs.height +2;
        cairo_set_line_width(w->crb, 1.0);
        use_frame_color_scheme(w, ACTIVE_);
        cairo_rectangle(w->crb, x, y, width, height);
        cairo_stroke(w->crb);        
    }
    if (!designer->drag_icon.is_active && designer->multi_selected) {
        use_frame_color_scheme(w, ACTIVE_);
        static const double dashed3[] = {2.0};
        cairo_set_dash(w->crb, dashed3, 1, 0);
        cairo_rectangle(w->crb, designer->select_x, designer->select_y,
                                designer->select_width-designer->select_x, designer->select_height-designer->select_y);
        cairo_stroke(w->crb);
    }
}

static void rounded_frame(cairo_t *cr,float x, float y, float w, float h, float lsize) {
    cairo_new_path (cr);
    float r = 20.0;
    cairo_move_to(cr, x+lsize+r,y);
    cairo_line_to(cr, x+w-r,y);
    cairo_curve_to(cr, x+w,y,x+w,y,x+w,y+r);
    cairo_line_to(cr, x+w,y+h-r);
    cairo_curve_to(cr, x+w,y+h,x+w,y+h,x+w-r,y+h);
    cairo_line_to(cr, x+r,y+h);
    cairo_curve_to(cr, x,y+h,x,y+h,x,y+h-r);
    cairo_line_to(cr, x,y+r);
    cairo_curve_to(cr, x,y,x,y,x+r,y);
}

void draw_frame(void *w_, void* UNUSED(user_data)) {
    Widget_t *w = (Widget_t*)w_;
    Widget_t *p = (Widget_t*)w->parent;
    XUiDesigner *designer = (XUiDesigner*)p->parent_struct;
    XWindowAttributes attrs;
    XGetWindowAttributes(w->app->dpy, (Window)w->widget, &attrs);
    int width_t = attrs.width;
    int height_t = attrs.height;

    if (w->image) {
        int width = cairo_xlib_surface_get_width(w->image);
        int height = cairo_xlib_surface_get_height(w->image);
        double x = (double)width_t/(double)(width);
        double y = (double)height_t/(double)height;
        double x1 = (double)(width)/(double)width_t;
        double y1 = (double)height/(double)height_t;
        cairo_scale(w->crb, x,y);
        cairo_set_source_surface (w->crb, w->image, 0, 0);
        rounded_frame(w->crb, 5/x, 5/y, (width_t-10)/x, (height_t-10)/y, 0);
        cairo_close_path (w->crb);
        cairo_fill (w->crb);
        cairo_scale(w->crb, x1,y1);
    }

    cairo_text_extents_t extents;
    use_text_color_scheme(w, get_color_state(w));
    cairo_set_font_size (w->crb, w->app->normal_font/w->scale.ascale);
    cairo_text_extents(w->crb,w->label , &extents);
    cairo_move_to (w->crb, 30, extents.height);
    cairo_show_text(w->crb, w->label);
    cairo_new_path (w->crb);

    cairo_set_line_width(w->crb,3);
    use_frame_color_scheme(w, INSENSITIVE_);
    rounded_frame(w->crb, 5, 5, width_t-10, height_t-10, extents.width+10);
    cairo_stroke(w->crb);

    if (designer->active_widget && designer->active_widget->parent == w) {
        XWindowAttributes attr;
        XGetWindowAttributes(w->app->dpy, (Window)designer->active_widget->widget, &attr);
        int x = attr.x -1;
        int y = attr.y -1;
        int width = attr.width +2;
        int height = attr.height +2;
        cairo_set_line_width(w->crb, 1.0);
        use_frame_color_scheme(w, ACTIVE_);
        cairo_rectangle(w->crb, x, y, width, height);
        cairo_stroke(w->crb);        
    }
}

void draw_image(void *w_, void* UNUSED(user_data)) {
    Widget_t *w = (Widget_t*)w_;
    Widget_t *p = (Widget_t*)w->parent;
    XUiDesigner *designer = (XUiDesigner*)p->parent_struct;
    XWindowAttributes attrs;
    XGetWindowAttributes(w->app->dpy, (Window)w->widget, &attrs);
    int width_t = attrs.width;
    int height_t = attrs.height;

    if (!w->image) {
        cairo_text_extents_t extents;
        use_base_color_scheme(w, get_color_state(w));
        cairo_set_font_size (w->crb, w->app->big_font/w->scale.ascale);
        cairo_text_extents(w->crb,"Missing Image" , &extents);
        cairo_move_to (w->crb, (w->width -extents.width)*0.5, (w->height - extents.height)*0.5);
        cairo_show_text(w->crb, "Missing Image");
        cairo_new_path (w->crb);

        cairo_set_line_width(w->crb,3);
        rounded_frame(w->crb, 5, 5, width_t-10, height_t-10, 0);
        cairo_stroke(w->crb);
    }

    if (w->image) {
        int width = cairo_xlib_surface_get_width(w->image);
        int height = cairo_xlib_surface_get_height(w->image);
        double x = (double)width_t/(double)(width);
        double y = (double)height_t/(double)height;
        double x1 = (double)(width)/(double)width_t;
        double y1 = (double)height/(double)height_t;
        cairo_scale(w->crb, x,y);
        cairo_set_source_surface (w->crb, w->image, 0, 0);
        cairo_paint (w->crb);
        cairo_scale(w->crb, x1,y1);
    } 

    if (designer->active_widget && designer->active_widget->parent == w) {
        XWindowAttributes attr;
        XGetWindowAttributes(w->app->dpy, (Window)designer->active_widget->widget, &attr);
        int x = attr.x -1;
        int y = attr.y -1;
        int width = attr.width +2;
        int height = attr.height +2;
        cairo_set_line_width(w->crb, 1.0);
        use_frame_color_scheme(w, ACTIVE_);
        cairo_rectangle(w->crb, x, y, width, height);
        cairo_stroke(w->crb);        
    }
}

static void rounded_box(cairo_t *cr,float x, float y, float w, float h, float lsize) {
    cairo_new_path (cr);
    float r = 10.0;
    cairo_move_to(cr, x+lsize,y);
    cairo_line_to(cr, x+w,y);
    cairo_curve_to(cr, x+w,y,x+w,y,x+w,y);
    cairo_line_to(cr, x+w,y+h-r);
    cairo_curve_to(cr, x+w,y+h,x+w,y+h,x+w-r,y+h);
    cairo_line_to(cr, x+r,y+h);
    cairo_curve_to(cr, x,y+h,x,y+h,x,y+h-r);
    cairo_line_to(cr, x,y+r);
    cairo_curve_to(cr, x,y,x,y,x,y);
}

void draw_tabbox(void *w_, void* UNUSED(user_data)) {
    Widget_t *w = (Widget_t*)w_;
    Widget_t *p = (Widget_t*)w->parent;
    XUiDesigner *designer = (XUiDesigner*)p->parent_struct;
    XWindowAttributes attrs;
    XGetWindowAttributes(w->app->dpy, (Window)w->widget, &attrs);
    int width_t = attrs.width;
    int height_t = attrs.height;

    cairo_set_source_surface (w->crb, p->buffer, -attrs.x, -attrs.y);
    cairo_paint (w->crb);

    int tabsize = 1;
    int elem = w->childlist->elem;
    if (elem) tabsize = width_t/elem;
    int v = (int)adj_get_value(w->adj);

    cairo_new_path (w->crb);
    cairo_set_line_width(w->crb,1);
    use_frame_color_scheme(w, NORMAL_);
    rounded_box(w->crb, 1, 21, width_t-2, height_t-22, (v+1)*tabsize);
    cairo_stroke(w->crb);

    cairo_text_extents_t extents;
    use_text_color_scheme(w, get_color_state(w));
    cairo_set_font_size (w->crb, w->app->normal_font/w->scale.ascale);
    int i = 0;
    int t = 0;
    for(;i<elem;i++) {
        Widget_t *wi = w->childlist->childs[i];
        if(v == i) {
            cairo_move_to (w->crb, t+1, 21);
            cairo_line_to(w->crb, t+1, 1);
            cairo_line_to(w->crb, t+tabsize-1, 1);
            cairo_line_to(w->crb, t+tabsize-1, 21);
            use_frame_color_scheme(w, NORMAL_);
            cairo_stroke(w->crb);
            if (designer->active_widget && designer->active_widget->parent == wi) {
                XWindowAttributes attr;
                XGetWindowAttributes(w->app->dpy, (Window)designer->active_widget->widget, &attr);
                int x = attr.x +1;
                int y = attr.y +20;
                int width = attr.width +2;
                int height = attr.height +2;
                cairo_set_line_width(w->crb, 1.0);
                use_frame_color_scheme(w, ACTIVE_);
                cairo_rectangle(w->crb, x, y, width, height);
                cairo_stroke(w->crb);        
            }
            use_text_color_scheme(w, ACTIVE_);
            widget_show_all(wi);
        } else {
            use_bg_color_scheme(w, ACTIVE_);
            cairo_rectangle(w->crb, t+2, 1, tabsize-4, 20);
            cairo_fill_preserve(w->crb);
            use_frame_color_scheme(w, NORMAL_);
            cairo_stroke(w->crb);
            use_text_color_scheme(w, INSENSITIVE_);
            widget_hide(wi);
        }

        cairo_text_extents(w->crb,"Äy" , &extents);
        cairo_move_to (w->crb, 5+t, 2+extents.height);
        cairo_show_text(w->crb, wi->label);
        cairo_new_path (w->crb);
        t += tabsize;
    }
}

void draw_tab(void *w_, void* UNUSED(user_data)) {
    Widget_t *w = (Widget_t*)w_;
    Widget_t *p = (Widget_t*)w->parent;
    draw_tabbox(p, NULL);
}

