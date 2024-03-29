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

#include "XUiColorChooser.h"


/*---------------------------------------------------------------------
-----------------------------------------------------------------------    
                color handling
-----------------------------------------------------------------------
----------------------------------------------------------------------*/

static void draw_lum_slider(void *w_, void* UNUSED(user_data)) {
    Widget_t *w = (Widget_t*)w_;
    XWindowAttributes attrs;
    XGetWindowAttributes(w->app->dpy, (Window)w->widget, &attrs);
    int width = attrs.width-2;
    int height = attrs.height-2;
    float center = (float)height/2;
    float upcenter = (float)height;
    
    if (attrs.map_state != IsViewable) return;

    float sliderstate = adj_get_state(w->adj_x);

    cairo_pattern_t* pat = cairo_pattern_create_linear ( 10, 0, width-10, 0);
    cairo_pattern_add_color_stop_rgba (pat, 0, 0,0,0,1);
    cairo_pattern_add_color_stop_rgba (pat, 1, 1,1,1,1);
    cairo_set_source (w->crb, pat);

    cairo_move_to (w->crb, center, center);
    cairo_line_to(w->crb,width-center-10,center);
    cairo_set_line_cap (w->crb,CAIRO_LINE_CAP_ROUND);
    cairo_set_line_width(w->crb,center);
    cairo_stroke(w->crb);

    use_shadow_color_scheme(w, get_color_state(w));
    cairo_move_to (w->crb, center, center);
    cairo_line_to(w->crb,width-center-10,center);
    cairo_set_line_width(w->crb,center/10);
    cairo_stroke(w->crb);

    use_shadow_color_scheme(w, get_color_state(w));
    cairo_arc(w->crb, (center) +
        ((width-10-upcenter) * sliderstate),center, center/2, 0, 2 * M_PI );
    cairo_fill_preserve(w->crb);
    cairo_set_line_width(w->crb,1);
    cairo_stroke(w->crb);

    use_bg_color_scheme(w, get_color_state(w));
    cairo_arc(w->crb, (center) +
        ((width-10-upcenter) * sliderstate),center, center/3, 0, 2 * M_PI );
    cairo_fill_preserve(w->crb);
    use_fg_color_scheme(w, NORMAL_);
    cairo_set_line_width(w->crb,center/15);
    cairo_stroke(w->crb);
    cairo_new_path (w->crb);

    cairo_text_extents_t extents;


    use_text_color_scheme(w, get_color_state(w));
    cairo_set_font_size (w->crb, w->app->normal_font/w->scale.ascale);
    cairo_text_extents(w->crb,w->label , &extents);

    cairo_move_to (w->crb, width/2-extents.width/2, height );
    cairo_show_text(w->crb, w->label);
    cairo_new_path (w->crb);

    cairo_set_font_size (w->crb, w->app->small_font/w->scale.ascale);
    char s[64];
    const char* format[] = {"%.1f", "%.2f", "%.3f"};
    float value = adj_get_value(w->adj);
        if (fabs(w->adj->step)>0.99) {
            snprintf(s, 63,"%d",  (int) value);
        } else if (fabs(w->adj->step)>0.09) {
            snprintf(s, 63, format[1-1], value);
        } else {
            snprintf(s, 63, format[2-1], value);
        }
    cairo_text_extents(w->crb,s , &extents);
    cairo_move_to (w->crb, width/2-extents.width/2, extents.height );
    cairo_show_text(w->crb, s);
    cairo_new_path (w->crb);
    cairo_pattern_destroy (pat);
}

static void draw_color_widget(void *w_, void* UNUSED(user_data)) {
    Widget_t *w = (Widget_t*)w_;
    ColorChooser_t *color_chooser = (ColorChooser_t*)w->private_struct;
    XUiDesigner *designer = (XUiDesigner*)w->parent_struct;
    Colors *c = designer->selected_scheme;
    cairo_set_source_rgba(w->crb,  0.13, 0.13, 0.13, 1.0);
    cairo_paint (w->crb);

    int width = w->width-60;
    int height = w->height-160;

    int grow = (width > height) ? height:width;
    int cwheel_x = grow-1;
    int cwheel_y = grow-1;

    int cwheelx = (width - cwheel_x) * 0.5;
    int cwheely = (height - cwheel_y) * 0.5;

    double cwheelstate = 0;

    double pointer_off =cwheel_x/5.5;
    color_chooser->radius = min(cwheel_x-pointer_off, cwheel_y-pointer_off) / 2;

    double r = 1.0;
    double g = 0.0;
    double b = 0.0;
    double l = color_chooser->lum;
    double a = color_chooser->alpha;
    int red = 1;
    int green = 0;
    int blue = 0;
    for (int i = 0; i<300;i++) {
        double angle = cwheelstate * 2 * M_PI;
        double lengh_x = (cwheelx+color_chooser->radius+pointer_off/2) -
            color_chooser->radius * sin(angle);
        double lengh_y = (cwheely+color_chooser->radius+pointer_off/2) +
            color_chooser->radius * cos(angle);
        color_chooser->center_x = (cwheelx+color_chooser->radius+pointer_off/2);
        color_chooser->center_y = (cwheely+color_chooser->radius+pointer_off/2);
        cairo_pattern_t* pat = cairo_pattern_create_linear ( color_chooser->center_x, color_chooser->center_y, lengh_x,lengh_y);
        cairo_pattern_add_color_stop_rgba (pat, 0, l,l,l,a);
        cairo_pattern_add_color_stop_rgba (pat, 1, min(1.0,r+l),min(1.0,g+l),min(1.0,b+l),a);
        cairo_set_source (w->crb, pat);
        cairo_set_line_join(w->crb, CAIRO_LINE_JOIN_BEVEL);
        cairo_move_to(w->crb, color_chooser->center_x, color_chooser->center_y);
        cairo_line_to(w->crb,lengh_x,lengh_y);
        cairo_set_line_width(w->crb,6);
        cairo_stroke(w->crb);
        cwheelstate +=0.00333;

        if (r<1.0 && (!red || blue)) {
             r += 0.02;
        } else if (b>0.0 && blue){
            red = 1;
            b -= 0.02;
        }

        if (g<1.0 && !blue && green !=1) {
            g += 0.02;
            green = -1;
        } else if (g>0.0 && r>0.0 && red) {
            r -= 0.02;
            green = 1;
        } else {
            green = 0;
        }
        
        if (b<1.0 && !blue && !green) {
            b +=0.02;
        } else if (b>0.0 && g>0.0) {
            g -= 0.02;
            blue = 1;
        } 
        cairo_pattern_destroy (pat);
    }
    cairo_new_path (w->crb);
    cairo_set_line_width(w->crb,3);
    cairo_arc(w->crb,color_chooser->focus_x, color_chooser->focus_y, 6, 0, 2 * M_PI );
    use_fg_color_scheme(w, get_color_state(w));
    cairo_stroke(w->crb);
    cairo_set_line_width(w->crb,1);
    cairo_arc(w->crb,color_chooser->focus_x, color_chooser->focus_y, 6, 0, 2 * M_PI );
    use_bg_color_scheme(w, get_color_state(w));
    cairo_stroke(w->crb);

    cairo_set_font_size(w->crb, w->app->small_font);
    cairo_move_to (w->crb, 10,  w->height-25);
    use_text_color_scheme(w, NORMAL_);
    cairo_show_text(w->crb, "bg");
    cairo_rectangle(w->crb, 10,w->height-20,  20, 20);
    cairo_set_source_rgba(w->crb, c->bg[0],  c->bg[1], c->bg[2],  c->bg[3]);
    cairo_fill(w->crb);

    cairo_move_to (w->crb, 45,  w->height-25);
    use_text_color_scheme(w, NORMAL_);
    cairo_show_text(w->crb, "fg");
    cairo_rectangle(w->crb, 45, w->height-20, 20, 20);
    cairo_set_source_rgba(w->crb, c->fg[0],  c->fg[1], c->fg[2],  c->fg[3]);
    cairo_fill(w->crb);

    cairo_move_to (w->crb, 80,  w->height-25);
    use_text_color_scheme(w, NORMAL_);
    cairo_show_text(w->crb, "base");
    cairo_rectangle(w->crb, 80, w->height-20, 20, 20);
    cairo_set_source_rgba(w->crb, c->base[0],  c->base[1], c->base[2],  c->base[3]);
    cairo_fill(w->crb);

    cairo_move_to (w->crb, 115,  w->height-25);
    use_text_color_scheme(w, NORMAL_);
    cairo_show_text(w->crb, "text");
    cairo_rectangle(w->crb, 115, w->height-20, 20, 20);
    cairo_set_source_rgba(w->crb, c->text[0],  c->text[1], c->text[2],  c->text[3]);
    cairo_fill(w->crb);

    cairo_move_to (w->crb, 140,  w->height-25);
    use_text_color_scheme(w, NORMAL_);
    cairo_show_text(w->crb, "shadow");
    cairo_rectangle(w->crb, 150, w->height-20, 20, 20);
    cairo_set_source_rgba(w->crb, c->shadow[0],  c->shadow[1], c->shadow[2],  c->shadow[3]);
    cairo_fill(w->crb);

    cairo_move_to (w->crb, 185,  w->height-25);
    use_text_color_scheme(w, NORMAL_);
    cairo_show_text(w->crb, "frame");
    cairo_rectangle(w->crb, 185, w->height-20, 20, 20);
    cairo_set_source_rgba(w->crb, c->frame[0],  c->frame[1], c->frame[2],  c->frame[3]);
    cairo_fill(w->crb);

    cairo_move_to (w->crb, 220,  w->height-25);
    use_text_color_scheme(w, NORMAL_);
    cairo_show_text(w->crb, "light");
    cairo_rectangle(w->crb, 220, w->height-20, 20, 20);
    cairo_set_source_rgba(w->crb, c->light[0],  c->light[1], c->light[2],  c->light[3]);
    cairo_fill(w->crb);
}


/*---------------------------------------------------------------------
-----------------------------------------------------------------------    
                trap xerror (XGetImage may fail)
-----------------------------------------------------------------------
----------------------------------------------------------------------*/


static int (*default_error_handler) (Display *UNUSED(dpy), XErrorEvent *UNUSED(e));

static int dummy_error_handler(Display *UNUSED(dpy), XErrorEvent *UNUSED(e)) {
    return 0;
}

/*---------------------------------------------------------------------
-----------------------------------------------------------------------    
                Color Chooser functions
-----------------------------------------------------------------------
----------------------------------------------------------------------*/

static void get_selected_scheme(XUiDesigner *designer, ColorChooser_t *color_chooser) {
    int s = (int)adj_get_value(color_chooser->color_scheme_select->adj);
    switch (s) {
        case 0: designer->selected_scheme = &get_active_widget(designer)->color_scheme->normal;
        break;
        case 1: designer->selected_scheme = &get_active_widget(designer)->color_scheme->prelight;
        break;
        case 2: designer->selected_scheme = &get_active_widget(designer)->color_scheme->selected;
        break;
        case 3: designer->selected_scheme = &get_active_widget(designer)->color_scheme->active;
        break;
        case 4: designer->selected_scheme = &get_active_widget(designer)->color_scheme->insensitive;
        break;
        default:
        break;
    }
}

static double *get_selected_color(XUiDesigner *designer, ColorChooser_t *color_chooser) {
    int s = (int)adj_get_value(color_chooser->color_sel->adj);
    double *use = NULL;
    switch (s) {
        case 0: use = designer->selected_scheme->bg;
        break;
        case 1: use = designer->selected_scheme->fg;
        break;
        case 2: use = designer->selected_scheme->base;
        break;
        case 3: use = designer->selected_scheme->text;
        break;
        case 4: use = designer->selected_scheme->shadow;
        break;
        case 5: use = designer->selected_scheme->frame;
        break;
        case 6: use = designer->selected_scheme->light;
        break;
        default:
        break;
    }
    return use;
}

static void set_costum_color(XUiDesigner *designer, ColorChooser_t *color_chooser, int a, float v) {
    get_selected_scheme(designer, color_chooser);
    int s = (int)adj_get_value(color_chooser->color_sel->adj);
    switch (s) {
        case 0:
            designer->selected_scheme->bg[a] = v;
        break;
        case 1:
            designer->selected_scheme->fg[a] = v;
        break;
        case 2:
            designer->selected_scheme->base[a] = v;
        break;
        case 3:
            designer->selected_scheme->text[a] = v;
        break;
        case 4:
            designer->selected_scheme->shadow[a] = v;
        break;
        case 5:
            designer->selected_scheme->frame[a] = v;
        break;
        case 6:
            designer->selected_scheme->light[a] = v;
        break;
        default:
        break;
    }
}

static void set_rgba_color(XUiDesigner *designer, ColorChooser_t *color_chooser,
                                    float r, float g, float b, float a) {
    get_selected_scheme(designer, color_chooser);
    widget_set_color(get_selected_color(designer, color_chooser), r, g, b, a);
    //set_costum_color(designer, color_chooser, 0, r);
    //set_costum_color(designer, color_chooser, 1, g);
    //set_costum_color(designer, color_chooser, 2, b);
    //set_costum_color(designer, color_chooser, 3, a);
    if (adj_get_value(designer->global_color->adj) ||
        designer->controls[designer->active_widget_num].is_type == IS_COMBOBOX)
        color_scheme_to_childs(get_active_widget(designer));
}

static void a_callback(void *w_, void* UNUSED(user_data)) {
    Widget_t *w = (Widget_t*)w_;
    Widget_t *p = (Widget_t*)w->parent;
    ColorChooser_t *color_chooser = (ColorChooser_t*)p->private_struct;
    color_chooser->alpha = adj_get_value(w->adj);
    XUiDesigner *designer = (XUiDesigner*)w->parent_struct;
    set_costum_color(designer, color_chooser, 3, color_chooser->alpha);
    color_scheme_to_childs(get_active_widget(designer));
    expose_widget(color_chooser->color_widget);
    expose_widget(get_active_widget(designer));
}

static void set_selected_color(void *w_, void* UNUSED(user_data)) {
    Widget_t *w = (Widget_t*)w_;
    Widget_t *p = (Widget_t*)w->parent;
    XUiDesigner *designer = (XUiDesigner*)w->parent_struct;
    ColorChooser_t *color_chooser = (ColorChooser_t*)p->private_struct;
    int s = (int)adj_get_value(color_chooser->color_sel->adj);
    double *use = NULL;
    switch (s) {
        case 0: use = designer->selected_scheme->bg;
        break;
        case 1: use = designer->selected_scheme->fg;
        break;
        case 2: use = designer->selected_scheme->base;
        break;
        case 3: use = designer->selected_scheme->text;
        break;
        case 4: use = designer->selected_scheme->shadow;
        break;
        case 5: use = designer->selected_scheme->frame;
        break;
        case 6: use = designer->selected_scheme->light;
        break;
        default:
        break;
    }
    if (use) {
        color_chooser->alpha = use[3];
        adj_set_value(color_chooser->al->adj, color_chooser->alpha);
        expose_widget(color_chooser->color_widget);
        set_focus_by_color(color_chooser->color_widget, use[0], use[1], use[2]);
    }
}

void set_selected_color_on_map(void *w_, void* UNUSED(user_data)) {
    Widget_t *w = (Widget_t*)w_;
    XWindowAttributes attrs;
    XGetWindowAttributes(w->app->dpy, (Window)w->widget, &attrs);
    if (attrs.map_state != IsViewable) return;
    XUiDesigner *designer = (XUiDesigner*)w->parent_struct;
    ColorChooser_t *color_chooser = (ColorChooser_t*)w->private_struct;
    get_selected_scheme(designer, color_chooser);
    int s = (int)adj_get_value(color_chooser->color_sel->adj);
    double *use = NULL;
    switch (s) {
        case 0: use = designer->selected_scheme->bg;
        break;
        case 1: use = designer->selected_scheme->fg;
        break;
        case 2: use = designer->selected_scheme->base;
        break;
        case 3: use = designer->selected_scheme->text;
        break;
        case 4: use = designer->selected_scheme->shadow;
        break;
        case 5: use = designer->selected_scheme->frame;
        break;
        case 6: use = designer->selected_scheme->light;
        break;
        default:
        break;
    }
    if (use) {
        color_chooser->alpha = use[3];
        adj_set_value(color_chooser->al->adj, color_chooser->alpha);
        set_focus_by_color(color_chooser->color_widget, use[0], use[1], use[2]);
        expose_widget(color_chooser->color_widget);
    }
}

static void reset_plugin_colors(void *w_, void* UNUSED(user_data)) {
    Widget_t *w = (Widget_t*)w_;
    XUiDesigner *designer = (XUiDesigner*)w->parent_struct;
    if (w->flags & HAS_POINTER && !adj_get_value(w->adj_y)) {
        memcpy(designer->ui->color_scheme, designer->w->color_scheme, sizeof (struct XColor_t));
        color_scheme_to_childs(designer->ui);
        expose_widget(designer->ui);
        set_selected_color_on_map(w, NULL);
    }
}

static void set_selected_scheme(void *w_, void* UNUSED(user_data)) {
    Widget_t *w = (Widget_t*)w_;
    XUiDesigner *designer = (XUiDesigner*)w->parent_struct;
    ColorChooser_t *color_chooser = (ColorChooser_t*)w->private_struct;    
    //Xputty *main = designer->w->app;
    int s = (int)adj_get_value(w->adj);
    switch (s) {
        case 0: designer->selected_scheme = &get_active_widget(designer)->color_scheme->normal;
        break;
        case 1: designer->selected_scheme = &get_active_widget(designer)->color_scheme->prelight;
        break;
        case 2: designer->selected_scheme = &get_active_widget(designer)->color_scheme->selected;
        break;
        case 3: designer->selected_scheme = &get_active_widget(designer)->color_scheme->active;
        break;
        case 4: designer->selected_scheme = &get_active_widget(designer)->color_scheme->insensitive;
        break;
        default:
        break;
    }
    set_selected_color(color_chooser->color_sel,NULL);
}

static bool is_in_circle(ColorChooser_t *color_chooser, int x, int y) {
    int a = (x - color_chooser->center_x);
    int b = (y - color_chooser->center_y);
    int c = (color_chooser->radius - 3);
    return (((a*a) + (b*b)) < (c * c));
}

static void get_pixel_fallback(ColorChooser_t *color_chooser, int x, int y, XColor *color) {
    cairo_surface_t *pixel = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 1, 1);
    cairo_t *cri = cairo_create (pixel);
    cairo_set_source_surface (cri, color_chooser->color_widget->buffer, -x, -y);
    cairo_paint (cri);

    const unsigned char *data = cairo_image_surface_get_data(pixel);
    color->red = (data[2]/255.0) * 65535.0;
    color->green = (data[1]/255.0) * 65535.0;
    color->blue = (data[0]/255.0) * 65535.0;
    cairo_surface_destroy(pixel);
    cairo_destroy(cri);
}

static bool get_pixel(Widget_t *w, int x, int y, XColor *color) {
    bool ret = true;
    XImage *image = NULL;
    default_error_handler = XSetErrorHandler(dummy_error_handler);
    image = XGetImage (w->app->dpy, DefaultRootWindow(w->app->dpy), x, y, 1, 1, AllPlanes, ZPixmap);
    if (!image) {
        ColorChooser_t *color_chooser = (ColorChooser_t*)w->private_struct;
        int x1, y1;
        Window child;
        XTranslateCoordinates( w->app->dpy, DefaultRootWindow(
                    w->app->dpy), color_chooser->color_widget->widget, x, y, &x1, &y1, &child );
        if (is_in_circle(color_chooser, x1, y1)) {
            get_pixel_fallback(color_chooser, x1, y1, color);
        } else {
            ret = false;
        }
    }
    XSetErrorHandler(default_error_handler);
    if (image) {
        color->pixel = XGetPixel(image, 0, 0);
        XDestroyImage (image);
        XQueryColor (w->app->dpy, DefaultColormap(w->app->dpy, DefaultScreen (w->app->dpy)), color);
    }
    return ret;
}

static void get_color(void *w_, void* button_, void* UNUSED(user_data)) {
    Widget_t *w = (Widget_t*)w_;
    XUiDesigner *designer = (XUiDesigner*)w->parent_struct;
    ColorChooser_t *color_chooser = (ColorChooser_t*)w->private_struct;
    XButtonEvent *xbutton = (XButtonEvent*)button_;
    XColor c;
    if (w->app->hold_grab == color_chooser->color_widget) {
        int x1, y1;
        Window child;
        XTranslateCoordinates( w->app->dpy, xbutton->window, DefaultRootWindow(
                            w->app->dpy), xbutton->x, xbutton->y, &x1, &y1, &child );
        if (!get_pixel(w, x1, y1, &c)) return;
        double r = (double)c.red/65535.0;
        double g = (double)c.green/65535.0;
        double b = (double)c.blue/65535.0;
        //fprintf(stderr, "%f %f %f %f\n", r, g, b, color_chooser->alpha);
        set_rgba_color(designer, color_chooser, r, g, b, color_chooser->alpha);
        set_focus_by_color(w, r, g, b);
        expose_widget(color_chooser->color_widget);
        expose_widget(get_active_widget(designer));
    } else if (w->flags & HAS_POINTER) {
        if (xbutton->button == Button1 &&  is_in_circle(color_chooser, xbutton->x, xbutton->y)) {
            int x1, y1;
            Window child;
            XTranslateCoordinates( w->app->dpy, w->widget, DefaultRootWindow(
                            w->app->dpy), xbutton->x, xbutton->y, &x1, &y1, &child );
            if (!get_pixel(w, x1, y1, &c)) return;
            double r = (double)c.red/65535.0;
            double g = (double)c.green/65535.0;
            double b = (double)c.blue/65535.0;
            //fprintf(stderr, "%f %f %f %f\n", r, g, b, color_chooser->alpha);
            set_rgba_color(designer, color_chooser, r, g, b, color_chooser->alpha);
            expose_widget(color_chooser->color_widget);
            expose_widget(get_active_widget(designer));
        }
    }
}

static void lum_callback(void *w_, void* UNUSED(user_data)) {
    Widget_t *w = (Widget_t*)w_;
    Widget_t *p = (Widget_t*)w->parent;
    XColor c;
    ColorChooser_t *color_chooser = (ColorChooser_t*)p->private_struct;
    color_chooser->lum = adj_get_value(w->adj);
    XUiDesigner *designer = (XUiDesigner*)w->parent_struct;
    int x1, y1;
    Window child;
    XTranslateCoordinates( w->app->dpy, color_chooser->color_widget->widget, DefaultRootWindow(
            w->app->dpy), color_chooser->focus_x, color_chooser->focus_y, &x1, &y1, &child );
    if (!get_pixel(p, x1, y1, &c)) return;
    double r = (double)c.red/65535.0;
    double g = (double)c.green/65535.0;
    double b = (double)c.blue/65535.0;
    set_rgba_color(designer, color_chooser, r, g, b, color_chooser->alpha);
    expose_widget(color_chooser->color_widget);
    expose_widget(get_active_widget(designer));
}

static void null_callback(void* UNUSED(w_), void* UNUSED(user_data)) {
    
}

void set_focus_by_color(Widget_t* wid, const double r, const double g, const double b) {
    XWindowAttributes attrs;
    XGetWindowAttributes(wid->app->dpy, (Window)wid->widget, &attrs);    
    if (attrs.map_state != IsViewable) return;

    XUiDesigner *designer = (XUiDesigner*)wid->parent_struct;
    ColorChooser_t *color_chooser = (ColorChooser_t*)wid->private_struct;
    color_chooser->lum =  min(min(r,g),b);
    xevfunc store = color_chooser->lu->func.value_changed_callback;
    color_chooser->lu->func.value_changed_callback = null_callback;
    adj_set_value(color_chooser->lu->adj, color_chooser->lum);
    float alpha = color_chooser->alpha;
    color_chooser->alpha = 1.0;
    color_chooser->focus_x = -10.0;
    color_chooser->focus_y = -10.0;
    transparent_draw(color_chooser->color_widget, NULL);
    color_chooser->lu->func.value_changed_callback = store;
    color_chooser->alpha = alpha;
    // convert cairo color to true color
    unsigned long _R = r*255;
    unsigned long _G = g*255;
    unsigned long _B = b*255;
    //fprintf(stderr, "RGB = %ld %ld %ld \n", _R,_G,_B);
    XImage *image = NULL;
    default_error_handler = XSetErrorHandler(dummy_error_handler);
    image = XGetImage (wid->app->dpy, wid->widget, 0, 0, wid->width-60, wid->height-120, AllPlanes, ZPixmap);
    XSetErrorHandler(default_error_handler);
    if (!image) return;
    int i = 10;
    int j = 10;
    unsigned long pixel = 0;
    
    for (;j<wid->height-141;j++) {
        for (;i<wid->width-61;i++) {
            if (is_in_circle(color_chooser, i, j)) {
                pixel = XGetPixel(image, i, j);
                //fprintf(stderr, "%lu ,",pixel);
                if (((pixel >> 0x10) & 0xFF) - _R < 4 &&
                    (((pixel >> 0x08) & 0xFF) - _G) < 4 &&
                    ((pixel & 0xFF) - _B) < 4 ) {
                        color_chooser->focus_x = (double)i;
                        color_chooser->focus_y = (double)j;
                    }
                if ((((pixel >> 0x10) & 0xFF) - _R) < 2 &&
                    (((pixel >> 0x08) & 0xFF) - _G) < 2 && 
                    ((pixel & 0xFF) - _B) < 2 ) {
                    color_chooser->focus_x = (double)i;
                    color_chooser->focus_y = (double)j;
                    set_rgba_color(designer, color_chooser, r, g, b, color_chooser->alpha);
                    expose_widget(color_chooser->color_widget);
                    expose_widget(get_active_widget(designer));
                    //fprintf(stderr,"found %ld,%ld,%ld ", (pixel >> 0x10) & 0xFF, (pixel >> 0x08) & 0xFF, pixel & 0xFF);
                    j = wid->height-121;
                    break;
                }
            }
        }
        i = 10;
    }
    XDestroyImage (image);
}

static void set_focus_motion(void *w_, void *xmotion_, void* UNUSED(user_data)) {
    Widget_t *w = (Widget_t*)w_;
    XUiDesigner *designer = (XUiDesigner*)w->parent_struct;
    ColorChooser_t *color_chooser = (ColorChooser_t*)w->private_struct;
    XMotionEvent *xmotion = (XMotionEvent*)xmotion_;
    if(is_in_circle (color_chooser, xmotion->x, xmotion->y)) {
        color_chooser->focus_x = xmotion->x;
        color_chooser->focus_y = xmotion->y;
        //fprintf(stderr, "%f %f \n", color_chooser->focus_x, color_chooser->focus_y);
        XColor c;
        int x1, y1;
        Window child;
        XTranslateCoordinates( w->app->dpy, w->widget, DefaultRootWindow(
                        w->app->dpy), xmotion->x, xmotion->y, &x1, &y1, &child );
        if (!get_pixel(w, x1, y1, &c)) return;
        double r = (double)c.red/65535.0;
        double g = (double)c.green/65535.0;
        double b = (double)c.blue/65535.0;
        set_rgba_color(designer, color_chooser, r, g, b, color_chooser->alpha);
        expose_widget(color_chooser->color_widget);
        expose_widget(get_active_widget(designer));
    }
}

// set the curser to the mouse pointer
static void set_focus(void *w_, void* button_, void* UNUSED(user_data)) {
    Widget_t *w = (Widget_t*)w_;
    ColorChooser_t *color_chooser = (ColorChooser_t*)w->private_struct;
    if (w->app->hold_grab != color_chooser->color_widget) {
        XButtonEvent *xbutton = (XButtonEvent*)button_;
        if(xbutton->button == Button1 && is_in_circle (color_chooser, xbutton->x, xbutton->y)) {
            color_chooser->focus_x = xbutton->x;
            color_chooser->focus_y = xbutton->y;
            expose_widget(color_chooser->color_widget);
        }
    }
}

static void reset_grab(void *w_, void *key_, void* UNUSED(user_data)) {
    Widget_t *w = (Widget_t*)w_;
    ColorChooser_t *color_chooser = (ColorChooser_t*)w->private_struct;
    XKeyEvent *key = (XKeyEvent*)key_;
    if (!key) return;
    if ((key->keycode == XKeysymToKeycode(w->app->dpy,XK_Control_L) ||
        key->keycode == XKeysymToKeycode(w->app->dpy,XK_Control_R)) &&
                (w->app->hold_grab == color_chooser->color_widget)) {
        XUngrabPointer(w->app->dpy,CurrentTime);
        w->app->hold_grab = NULL;
    }
}

static void set_focus_on_key(void *w_, void *key_, void* UNUSED(user_data)) {
    Widget_t *w = (Widget_t*)w_;
    XUiDesigner *designer = (XUiDesigner*)w->parent_struct;
    ColorChooser_t *color_chooser = (ColorChooser_t*)w->private_struct;
    XKeyEvent *key = (XKeyEvent*)key_;
    if (!key) return;
    if ((key->keycode == XKeysymToKeycode(w->app->dpy,XK_Control_L) ||
        key->keycode == XKeysymToKeycode(w->app->dpy,XK_Control_R)) && 
                                            w->app->hold_grab == NULL) {
        int err = XGrabPointer(w->app->dpy, DefaultRootWindow(w->app->dpy), True,
                 ButtonPressMask|ButtonReleaseMask|PointerMotionMask,
                 GrabModeAsync, GrabModeAsync, None, None, CurrentTime);
        if (!err) w->app->hold_grab = w;
    }
    int nk = key_mapping(w->app->dpy, key);
    double x = 0.0;
    double y = 0.0;
    if (nk) {
        switch (nk) {
            case 3: y = -1.0;
            break;
            case 4: x = 1.0;
            break;
            case 5: y = 1.0;
            break;
            case 6: x = -1.0;
            break;
            default:
            return;
            break;
        }
    } else {
        return;
    }
    if (is_in_circle (color_chooser, color_chooser->focus_x + x, color_chooser->focus_y + y)) {
        color_chooser->focus_y +=y;
        color_chooser->focus_x +=x;
        XColor c;
        int x1, y1;
        Window child;
        XTranslateCoordinates( w->app->dpy, color_chooser->color_widget->widget, DefaultRootWindow(
                w->app->dpy), color_chooser->focus_x, color_chooser->focus_y, &x1, &y1, &child );
        if (!get_pixel(w, x1, y1, &c)) return;
        double r = (double)c.red/65535.0;
        double g = (double)c.green/65535.0;
        double b = (double)c.blue/65535.0;
        set_rgba_color(designer, color_chooser, r, g, b, color_chooser->alpha);
        expose_widget(color_chooser->color_widget);
        expose_widget(get_active_widget(designer));
    }
}

static void color_chooser_mem_free(void *w_, void* UNUSED(user_data)) {
    Widget_t *w = (Widget_t*)w_;
    ColorChooser_t *color_chooser = (ColorChooser_t*)w->private_struct;
    free(color_chooser);
}

Widget_t *create_color_chooser (XUiDesigner *designer) {
    //Xputty *main = designer->w->app;
    ColorChooser_t *color_chooser = (ColorChooser_t*)malloc(sizeof(ColorChooser_t));
    color_chooser->alpha = 1.0;
    color_chooser->lum = 0.0;
    color_chooser->focus_x = 10.0;
    color_chooser->focus_y = 10.0;

    color_chooser->color_widget = create_window(designer->w->app, DefaultRootWindow(designer->w->app->dpy), 0, 0, 260, 380);
    Atom wmStateAbove = XInternAtom(designer->w->app->dpy, "_NET_WM_STATE_ABOVE", 1 );
    Atom wmNetWmState = XInternAtom(designer->w->app->dpy, "_NET_WM_STATE", 1 );
    XChangeProperty(designer->w->app->dpy, color_chooser->color_widget->widget, wmNetWmState, XA_ATOM, 32, 
        PropModeReplace, (unsigned char *) &wmStateAbove, 1); 
    XSetTransientForHint(designer->w->app->dpy, color_chooser->color_widget->widget, designer->w->widget);
    widget_set_title(color_chooser->color_widget, _("cairo-color-mixer"));
    color_chooser->color_widget->flags |= HIDE_ON_DELETE | HAS_MEM;
    color_chooser->color_widget->func.expose_callback = draw_color_widget;
    color_chooser->color_widget->func.button_press_callback = set_focus;
    color_chooser->color_widget->func.button_release_callback = get_color;
    color_chooser->color_widget->func.motion_callback = set_focus_motion;
    color_chooser->color_widget->func.key_press_callback = set_focus_on_key;
    color_chooser->color_widget->func.key_release_callback = reset_grab;
    color_chooser->color_widget->func.mem_free_callback = color_chooser_mem_free;
    color_chooser->color_widget->func.configure_notify_callback = set_selected_color_on_map;
    color_chooser->color_widget->func.visibiliy_change_callback = set_selected_color_on_map;
    color_chooser->color_widget->parent_struct = designer;
    color_chooser->color_widget->private_struct = color_chooser;

    designer->global_color = add_check_box(color_chooser->color_widget, _("  Global Color"), 20, 305, 140, 20);
    tooltip_set_text(designer->global_color,_("Use Color settings for all Widgets"));
    //adj_set_value(designer->global_color->adj, 1.0);
    designer->global_color->parent_struct = designer;

    designer->selected_scheme = &get_active_widget(designer)->color_scheme->normal;

    color_chooser->reset = add_button(color_chooser->color_widget, _("Reset"), 180, 300, 60, 30);
    tooltip_set_text(color_chooser->reset,_("Reset all Colors to default"));
    color_chooser->reset->parent_struct = designer;
    color_chooser->reset->private_struct = color_chooser;
    color_chooser->reset->func.value_changed_callback = reset_plugin_colors;

    color_chooser->al = add_vslider(color_chooser->color_widget, _("A"), 200, 40, 40, 200);
    set_adjustment(color_chooser->al->adj, 1.0, 1.0, 0.0, 1.0, 0.005, CL_CONTINUOS);
    adj_set_value(color_chooser->al->adj,designer->selected_scheme->bg[3]);
    color_chooser->al->data = 4;
    color_chooser->al->scale.gravity = WESTNORTH;
    color_chooser->al->parent_struct = designer;
    color_chooser->al->func.value_changed_callback = a_callback;

    color_chooser->lu = add_hslider(color_chooser->color_widget, _("luminescent"), 10, 210, 200, 40);
    set_adjustment(color_chooser->lu->adj, 0.0, 0.0, 0.0, 1.0, 0.005, CL_CONTINUOS);
    adj_set_value(color_chooser->lu->adj, color_chooser->lum);
    color_chooser->lu->scale.gravity = SOUTHEAST;
    color_chooser->lu->parent_struct = designer;
    color_chooser->lu->func.expose_callback = draw_lum_slider;
    color_chooser->lu->func.value_changed_callback = lum_callback;

    color_chooser->color_scheme_select = add_combobox(color_chooser->color_widget, _("Scheme"), 20, 260, 120,30);
    combobox_add_entry(color_chooser->color_scheme_select,_("normal"));
    combobox_add_entry(color_chooser->color_scheme_select,_("prelight"));
    combobox_add_entry(color_chooser->color_scheme_select,_("selected"));
    combobox_add_entry(color_chooser->color_scheme_select,_("active"));
    combobox_add_entry(color_chooser->color_scheme_select,_("insensitive"));
    combobox_set_active_entry(color_chooser->color_scheme_select, 0);
    color_chooser->color_scheme_select->scale.gravity = SOUTHEAST;
    color_chooser->color_scheme_select->parent_struct = designer;
    color_chooser->color_scheme_select->private_struct = color_chooser;
    color_chooser->color_scheme_select->func.value_changed_callback = set_selected_scheme;

    color_chooser->color_sel = add_combobox(color_chooser->color_widget, _("Colors"), 160, 260, 80,30);
    combobox_add_entry(color_chooser->color_sel,_("bg"));
    combobox_add_entry(color_chooser->color_sel,_("fg"));
    combobox_add_entry(color_chooser->color_sel,_("base"));
    combobox_add_entry(color_chooser->color_sel,_("text"));
    combobox_add_entry(color_chooser->color_sel,_("shadow"));
    combobox_add_entry(color_chooser->color_sel,_("frame"));
    combobox_add_entry(color_chooser->color_sel,_("light"));
    combobox_set_active_entry(color_chooser->color_sel, 0);
    color_chooser->color_sel->scale.gravity = SOUTHEAST;
    color_chooser->color_sel->parent_struct = designer;
    color_chooser->color_sel->func.value_changed_callback = set_selected_color;

    return color_chooser->color_widget;
}

void show_color_chooser(void *w_, void* UNUSED(user_data)) {
    Widget_t *w = (Widget_t*)w_;
    XUiDesigner *designer = (XUiDesigner*)w->parent_struct;
    if (!designer->color_widget) designer->color_widget = create_color_chooser (designer);
    if (w->flags & HAS_POINTER && adj_get_value(w->adj_y)) {
        widget_show_all(designer->color_widget);
        int x1, y1;
        Window child;
        XTranslateCoordinates( designer->w->app->dpy, w->widget, DefaultRootWindow(
                        designer->w->app->dpy), 0, 60, &x1, &y1, &child );
        XMoveWindow(designer->w->app->dpy, designer->color_widget->widget,x1,y1);
        XGrabKey(designer->w->app->dpy, XKeysymToKeycode(designer->w->app->dpy,XK_Control_L),
            AnyModifier, DefaultRootWindow(designer->w->app->dpy), true, GrabModeAsync, GrabModeAsync);
        XGrabKey(designer->w->app->dpy, XKeysymToKeycode(designer->w->app->dpy,XK_Control_R),
            AnyModifier, DefaultRootWindow(designer->w->app->dpy), true, GrabModeAsync, GrabModeAsync);
        designer->w->app->key_snooper = designer->color_widget;
    } else if (w->flags & HAS_POINTER && !adj_get_value(w->adj_y)) {
        widget_hide(designer->color_widget);
        XUngrabKey(designer->w->app->dpy, XKeysymToKeycode(designer->w->app->dpy,XK_Control_L),
                                        AnyModifier, DefaultRootWindow(designer->w->app->dpy));
        XUngrabKey(designer->w->app->dpy, XKeysymToKeycode(designer->w->app->dpy,XK_Control_R),
                                        AnyModifier, DefaultRootWindow(designer->w->app->dpy));
        designer->w->app->key_snooper = NULL;
    }
}
