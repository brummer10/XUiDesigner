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

static void draw_color_widget(void *w_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    XUiDesigner *designer = (XUiDesigner*)w->parent_struct;
    ColorChooser_t *color_chooser = (ColorChooser_t*)w->private_struct;
    cairo_set_source_rgba(w->crb,  0.13, 0.13, 0.13, 1.0);
    cairo_paint (w->crb);

    int width = w->width-60;
    int height = w->height-120;

    int grow = (width > height) ? height:width;
    int cwheel_x = grow-1;
    int cwheel_y = grow-1;

    int cwheelx = (width - cwheel_x) * 0.5;
    int cwheely = (height - cwheel_y) * 0.5;

    double cwheelstate = 0;

    double pointer_off =cwheel_x/5.5;
    double radius = min(cwheel_x-pointer_off, cwheel_y-pointer_off) / 2;

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
        double lengh_x = (cwheelx+radius+pointer_off/2) - radius * sin(angle);
        double lengh_y = (cwheely+radius+pointer_off/2) + radius * cos(angle);
        double radius_x = (cwheelx+radius+pointer_off/2);
        double radius_y = (cwheely+radius+pointer_off/2);
        cairo_pattern_t* pat = cairo_pattern_create_linear ( radius_x, radius_y, lengh_x,lengh_y);
        cairo_pattern_add_color_stop_rgba (pat, 0, l,l,l,a);
        cairo_pattern_add_color_stop_rgba (pat, 1, r,g,b,a);
        cairo_set_source (w->crb, pat);
        cairo_set_line_join(w->crb, CAIRO_LINE_JOIN_BEVEL);
        cairo_move_to(w->crb, radius_x, radius_y);
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
    use_bg_color_scheme(w, (int)adj_get_value(designer->color_scheme_select->adj));
    cairo_fill(w->crb);

    cairo_move_to (w->crb, 45,  w->height-25);
    use_text_color_scheme(w, NORMAL_);
    cairo_show_text(w->crb, "fg");
    cairo_rectangle(w->crb, 45, w->height-20, 20, 20);
    use_fg_color_scheme(w, (int)adj_get_value(designer->color_scheme_select->adj));
    cairo_fill(w->crb);

    cairo_move_to (w->crb, 80,  w->height-25);
    use_text_color_scheme(w, NORMAL_);
    cairo_show_text(w->crb, "base");
    cairo_rectangle(w->crb, 80, w->height-20, 20, 20);
    use_base_color_scheme(w, (int)adj_get_value(designer->color_scheme_select->adj));
    cairo_fill(w->crb);

    cairo_move_to (w->crb, 115,  w->height-25);
    use_text_color_scheme(w, NORMAL_);
    cairo_show_text(w->crb, "text");
    cairo_rectangle(w->crb, 115, w->height-20, 20, 20);
    use_text_color_scheme(w, (int)adj_get_value(designer->color_scheme_select->adj));
    cairo_fill(w->crb);

    cairo_move_to (w->crb, 140,  w->height-25);
    use_text_color_scheme(w, NORMAL_);
    cairo_show_text(w->crb, "shadow");
    cairo_rectangle(w->crb, 150, w->height-20, 20, 20);
    use_shadow_color_scheme(w, (int)adj_get_value(designer->color_scheme_select->adj));
    cairo_fill(w->crb);

    cairo_move_to (w->crb, 185,  w->height-25);
    use_text_color_scheme(w, NORMAL_);
    cairo_show_text(w->crb, "frame");
    cairo_rectangle(w->crb, 185, w->height-20, 20, 20);
    use_frame_color_scheme(w, (int)adj_get_value(designer->color_scheme_select->adj));
    cairo_fill(w->crb);

    cairo_move_to (w->crb, 220,  w->height-25);
    use_text_color_scheme(w, NORMAL_);
    cairo_show_text(w->crb, "light");
    cairo_rectangle(w->crb, 220, w->height-20, 20, 20);
    use_light_color_scheme(w, (int)adj_get_value(designer->color_scheme_select->adj));
    cairo_fill(w->crb);
}

static void set_costum_color(XUiDesigner *designer, int a, float v) {
    int s = (int)adj_get_value(designer->color_sel->adj);
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

static void a_callback(void *w_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    Widget_t *p = (Widget_t*)w->parent;
    ColorChooser_t *color_chooser = (ColorChooser_t*)p->private_struct;
    color_chooser->alpha = adj_get_value(w->adj);
    XUiDesigner *designer = (XUiDesigner*)w->parent_struct;
    set_costum_color(designer, 3, color_chooser->alpha);
    expose_widget(designer->color_widget);
    expose_widget(designer->ui);
}

static void set_selected_color(void *w_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    Widget_t *p = (Widget_t*)w->parent;
    XUiDesigner *designer = (XUiDesigner*)w->parent_struct;
    ColorChooser_t *color_chooser = (ColorChooser_t*)p->private_struct;
    int s = (int)adj_get_value(designer->color_sel->adj);
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
    expose_widget(designer->color_widget);
    if (use) {
        adj_set_value(color_chooser->al->adj, use[3]);
    }
}

static void set_selected_scheme(void *w_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    XUiDesigner *designer = (XUiDesigner*)w->parent_struct;
    Xputty *main = designer->w->app;
    int s = (int)adj_get_value(w->adj);
    switch (s) {
        case 0: designer->selected_scheme = &main->color_scheme->normal;
        break;
        case 1: designer->selected_scheme = &main->color_scheme->prelight;
        break;
        case 2: designer->selected_scheme = &main->color_scheme->selected;
        break;
        case 3: designer->selected_scheme = &main->color_scheme->active;
        break;
        case 4: designer->selected_scheme = &main->color_scheme->insensitive;
        break;
        default:
        break;
    }
    set_selected_color(designer->color_sel,NULL);
}

static void get_pixel(Widget_t *w, int x, int y, XColor *color) {
  XImage *image;
  image = XGetImage (w->app->dpy, w->widget, x, y, 1, 1, AllPlanes, XYPixmap);
  color->pixel = XGetPixel(image, 0, 0);
  XFree (image);
  XQueryColor (w->app->dpy, DefaultColormap(w->app->dpy, DefaultScreen (w->app->dpy)), color);
}

static void get_color(void *w_, void* button_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    XUiDesigner *designer = (XUiDesigner*)w->parent_struct;
    ColorChooser_t *color_chooser = (ColorChooser_t*)w->private_struct;
    XButtonEvent *xbutton = (XButtonEvent*)button_;
    if (w->flags & HAS_POINTER) {
        if(xbutton->button == Button1 && xbutton->x > 10 && xbutton->y > 10
                && xbutton->x < w->width-10 && xbutton->y < w->height-10 ) {
            XColor c;
            get_pixel(w, xbutton->x, xbutton->y, &c);
            double r = (double)c.red/65535.0;
            double g = (double)c.green/65535.0;
            double b = (double)c.blue/65535.0;
            //fprintf(stderr, "%f %f %f %f\n", r, g, b, color_chooser->alpha);
            set_costum_color(designer, 0, r);
            set_costum_color(designer, 1, g);
            set_costum_color(designer, 2, b);
            set_costum_color(designer, 3, color_chooser->alpha);
            expose_widget(designer->color_widget);
            expose_widget(designer->ui);
        }
        
    }
}

static void lum_callback(void *w_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    Widget_t *p = (Widget_t*)w->parent;
    XColor c;
    ColorChooser_t *color_chooser = (ColorChooser_t*)p->private_struct;
    color_chooser->lum = adj_get_value(w->adj);
    XUiDesigner *designer = (XUiDesigner*)w->parent_struct;
    get_pixel(p, color_chooser->focus_x, color_chooser->focus_y, &c);
    double r = (double)c.red/65535.0;
    double g = (double)c.green/65535.0;
    double b = (double)c.blue/65535.0;
    set_costum_color(designer, 0, r);
    set_costum_color(designer, 1, g);
    set_costum_color(designer, 2, b);
    set_costum_color(designer, 3, color_chooser->alpha);
    expose_widget(designer->color_widget);
    expose_widget(designer->ui);
}

static void set_focus_motion(void *w_, void *xmotion_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    XUiDesigner *designer = (XUiDesigner*)w->parent_struct;
    ColorChooser_t *color_chooser = (ColorChooser_t*)w->private_struct;
    XMotionEvent *xmotion = (XMotionEvent*)xmotion_;
    if(xmotion->x > 10 && xmotion->y > 10 && xmotion->x < w->width-10
                                    && xmotion->y < w->height-10 ) {
        color_chooser->focus_x = xmotion->x;
        color_chooser->focus_y = xmotion->y;
        XColor c;
        get_pixel(w, xmotion->x, xmotion->y, &c);
        double r = (double)c.red/65535.0;
        double g = (double)c.green/65535.0;
        double b = (double)c.blue/65535.0;
        set_costum_color(designer, 0, r);
        set_costum_color(designer, 1, g);
        set_costum_color(designer, 2, b);
        set_costum_color(designer, 3, color_chooser->alpha);
        expose_widget(designer->color_widget);
        expose_widget(designer->ui);
    }
}

// set the curser to the mouse pointer
static void set_focus(void *w_, void* button_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    XUiDesigner *designer = (XUiDesigner*)w->parent_struct;
    ColorChooser_t *color_chooser = (ColorChooser_t*)w->private_struct;
    XButtonEvent *xbutton = (XButtonEvent*)button_;
    if(xbutton->button == Button1) {
        color_chooser->focus_x = xbutton->x;
        color_chooser->focus_y = xbutton->y;
        expose_widget(designer->color_widget);
    }
}

static void set_focus_on_key(void *w_, void *key_, void *user_data) {
    Widget_t *w = (Widget_t*)w_;
    XUiDesigner *designer = (XUiDesigner*)w->parent_struct;
    ColorChooser_t *color_chooser = (ColorChooser_t*)w->private_struct;
    if(color_chooser->focus_x >= 10 && color_chooser->focus_y >= 10 &&
      color_chooser->focus_x <= w->width-10 && color_chooser->focus_y <= w->height-10 ) {
        XKeyEvent *key = (XKeyEvent*)key_;
        if (!key) return;
        int nk = key_mapping(w->app->dpy, key);
        if (nk) {
            switch (nk) {
                case 3: color_chooser->focus_y -=1.0;
                break;
                case 4: color_chooser->focus_x +=1.0;
                break;
                case 5: color_chooser->focus_y +=1.0;
                break;
                case 6: color_chooser->focus_x -=1.0;
                break;
                default:
                break;
            }
        }
        XColor c;
        get_pixel(w, color_chooser->focus_x, color_chooser->focus_y, &c);
        double r = (double)c.red/65535.0;
        double g = (double)c.green/65535.0;
        double b = (double)c.blue/65535.0;
        set_costum_color(designer, 0, r);
        set_costum_color(designer, 1, g);
        set_costum_color(designer, 2, b);
        set_costum_color(designer, 3, color_chooser->alpha);
        expose_widget(designer->color_widget);
        expose_widget(designer->ui);
    }
}

void color_chooser_mem_free(void *w_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    ColorChooser_t *color_chooser = (ColorChooser_t*)w->private_struct;
    free(color_chooser);
}

void create_color_chooser (XUiDesigner *designer) {
    Xputty *main = designer->w->app;
    designer->selected_scheme = &main->color_scheme->normal;
    ColorChooser_t *color_chooser = (ColorChooser_t*)malloc(sizeof(ColorChooser_t));
    color_chooser->alpha = 1.0;
    color_chooser->lum = 1.0;
    color_chooser->focus_x = 10.0;
    color_chooser->focus_y = 10.0;

    designer->color_widget = create_window(designer->w->app, DefaultRootWindow(designer->w->app->dpy), 0, 0, 260, 340);
    Atom wmStateAbove = XInternAtom(designer->w->app->dpy, "_NET_WM_STATE_ABOVE", 1 );
    Atom wmNetWmState = XInternAtom(designer->w->app->dpy, "_NET_WM_STATE", 1 );
    XChangeProperty(designer->w->app->dpy, designer->color_widget->widget, wmNetWmState, XA_ATOM, 32, 
        PropModeReplace, (unsigned char *) &wmStateAbove, 1); 
    XSetTransientForHint(designer->w->app->dpy, designer->color_widget->widget, designer->w->widget);
    widget_set_title(designer->color_widget, _("cairo-color-mixer"));
    designer->color_widget->func.expose_callback = draw_color_widget;
    designer->color_widget->func.button_press_callback = set_focus;
    designer->color_widget->func.button_release_callback = get_color;
    designer->color_widget->func.motion_callback = set_focus_motion;
    designer->color_widget->func.key_press_callback = set_focus_on_key;
    designer->color_widget->func.mem_free_callback = color_chooser_mem_free;
    designer->color_widget->parent_struct = designer;
    designer->color_widget->private_struct = color_chooser;

    color_chooser->al = add_vslider(designer->color_widget, _("A"), 200, 40, 40, 200);
    set_adjustment(color_chooser->al->adj, 1.0, 1.0, 0.0, 1.0, 0.005, CL_CONTINUOS);
    adj_set_value(color_chooser->al->adj,designer->selected_scheme->bg[3]);
    color_chooser->al->data = 4;
    color_chooser->al->scale.gravity = WESTNORTH;
    color_chooser->al->parent_struct = designer;
    color_chooser->al->func.value_changed_callback = a_callback;

    color_chooser->lu = add_hslider(designer->color_widget, _("luminescent"), 10, 210, 200, 40);
    set_adjustment(color_chooser->lu->adj, 1.0, 1.0, 0.0, 1.0, 0.005, CL_CONTINUOS);
    adj_set_value(color_chooser->lu->adj,1.0);
    color_chooser->lu->scale.gravity = SOUTHEAST;
    color_chooser->lu->parent_struct = designer;
    color_chooser->lu->func.value_changed_callback = lum_callback;

    designer->color_scheme_select = add_combobox(designer->color_widget, _("Scheme"), 20, 260, 120,30);
    combobox_add_entry(designer->color_scheme_select,_("normal"));
    combobox_add_entry(designer->color_scheme_select,_("prelight"));
    combobox_add_entry(designer->color_scheme_select,_("selected"));
    combobox_add_entry(designer->color_scheme_select,_("active"));
    combobox_add_entry(designer->color_scheme_select,_("insensitive"));
    combobox_set_active_entry(designer->color_scheme_select, 0);
    designer->color_scheme_select->scale.gravity = SOUTHEAST;
    designer->color_scheme_select->parent_struct = designer;
    designer->color_scheme_select->func.value_changed_callback = set_selected_scheme;

    designer->color_sel = add_combobox(designer->color_widget, _("Colors"), 160, 260, 80,30);
    combobox_add_entry(designer->color_sel,_("bg"));
    combobox_add_entry(designer->color_sel,_("fg"));
    combobox_add_entry(designer->color_sel,_("base"));
    combobox_add_entry(designer->color_sel,_("text"));
    combobox_add_entry(designer->color_sel,_("shadow"));
    combobox_add_entry(designer->color_sel,_("frame"));
    combobox_add_entry(designer->color_sel,_("light"));
    combobox_set_active_entry(designer->color_sel, 0);
    designer->color_sel->scale.gravity = SOUTHEAST;
    designer->color_sel->parent_struct = designer;
    designer->color_sel->func.value_changed_callback = set_selected_color;
   
}

void show_color_chooser(void *w_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    XUiDesigner *designer = (XUiDesigner*)w->parent_struct;
    if (w->flags & HAS_POINTER && adj_get_value(w->adj_y)) {
        widget_show_all(designer->color_widget);
        int x1, y1;
        Window child;
        XTranslateCoordinates( designer->w->app->dpy, designer->color_chooser->widget, DefaultRootWindow(
                        designer->w->app->dpy), 0, 60, &x1, &y1, &child );
        XMoveWindow(designer->w->app->dpy, designer->color_widget->widget,x1,y1);
    } else if (w->flags & HAS_POINTER && !adj_get_value(w->adj_y)) {
        widget_hide(designer->color_widget);
    }
}
