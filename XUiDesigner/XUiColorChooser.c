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
    use_bg_color_scheme(w, NORMAL_);
    cairo_paint (w->crb);
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

static void r_callback(void *w_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    XUiDesigner *designer = (XUiDesigner*)w->parent_struct;
    set_costum_color(designer, 0, adj_get_value(w->adj));
    expose_widget(designer->ui);
}

static void g_callback(void *w_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    XUiDesigner *designer = (XUiDesigner*)w->parent_struct;
    set_costum_color(designer, 1, adj_get_value(w->adj));
    expose_widget(designer->ui);
}

static void b_callback(void *w_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    XUiDesigner *designer = (XUiDesigner*)w->parent_struct;
    set_costum_color(designer, 2, adj_get_value(w->adj));
    expose_widget(designer->ui);
}

static void a_callback(void *w_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    XUiDesigner *designer = (XUiDesigner*)w->parent_struct;
    set_costum_color(designer, 3, adj_get_value(w->adj));
    expose_widget(designer->ui);
}

static void set_selected_color(void *w_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    XUiDesigner *designer = (XUiDesigner*)w->parent_struct;
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
    if (use) {
        adj_set_value(designer->color_widget->childlist->childs[0]->adj, use[0]);
        adj_set_value(designer->color_widget->childlist->childs[1]->adj, use[1]);
        adj_set_value(designer->color_widget->childlist->childs[2]->adj, use[2]);
        adj_set_value(designer->color_widget->childlist->childs[3]->adj, use[3]);
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

void create_color_chooser (XUiDesigner *designer) {
    Widget_t* sl;
    Xputty *main = designer->w->app;
    designer->selected_scheme = &main->color_scheme->normal;
    designer->color_widget = create_window(designer->w->app, DefaultRootWindow(designer->w->app->dpy), 0, 0, 260, 300);
    Atom wmStateAbove = XInternAtom(designer->w->app->dpy, "_NET_WM_STATE_ABOVE", 1 );
    Atom wmNetWmState = XInternAtom(designer->w->app->dpy, "_NET_WM_STATE", 1 );
    XChangeProperty(designer->w->app->dpy, designer->color_widget->widget, wmNetWmState, XA_ATOM, 32, 
        PropModeReplace, (unsigned char *) &wmStateAbove, 1); 
    XSetTransientForHint(designer->w->app->dpy, designer->color_widget->widget, designer->w->widget);
    widget_set_title(designer->color_widget, _("cairo-color-mixer"));
    designer->color_widget->func.expose_callback = draw_color_widget;

    sl = add_vslider(designer->color_widget, _("R"), 20, 40, 40, 200);
    set_adjustment(sl->adj, 0.0, 0.0, 0.0, 1.0, 0.005, CL_CONTINUOS);
    adj_set_value(sl->adj,designer->selected_scheme->bg[0]);
    sl->data = 1;
    sl->parent_struct = designer;
    sl->func.value_changed_callback = r_callback;

    sl = add_vslider(designer->color_widget, _("G"), 80, 40, 40, 200);
    set_adjustment(sl->adj, 0.0, 0.0, 0.0, 1.0, 0.005, CL_CONTINUOS);
    adj_set_value(sl->adj,designer->selected_scheme->bg[1]);
    sl->data = 2;
    sl->parent_struct = designer;
    sl->func.value_changed_callback = g_callback;

    sl = add_vslider(designer->color_widget, _("B"), 140, 40, 40, 200);
    set_adjustment(sl->adj, 0.0, 0.0, 0.0, 1.0, 0.005, CL_CONTINUOS);
    adj_set_value(sl->adj,designer->selected_scheme->bg[2]);
    sl->data = 3;
    sl->parent_struct = designer;
    sl->func.value_changed_callback = b_callback;

    sl = add_vslider(designer->color_widget, _("A"), 200, 40, 40, 200);
    set_adjustment(sl->adj, 1.0, 1.0, 0.0, 1.0, 0.005, CL_CONTINUOS);
    adj_set_value(sl->adj,designer->selected_scheme->bg[3]);
    sl->data = 4;
    sl->parent_struct = designer;
    sl->func.value_changed_callback = a_callback;

    designer->color_scheme_select = add_combobox(designer->color_widget, _("Scheme"), 20, 250, 120,30);
    combobox_add_entry(designer->color_scheme_select,_("normal"));
    combobox_add_entry(designer->color_scheme_select,_("prelight"));
    combobox_add_entry(designer->color_scheme_select,_("selected"));
    combobox_add_entry(designer->color_scheme_select,_("active"));
    combobox_add_entry(designer->color_scheme_select,_("insensitive"));
    combobox_set_active_entry(designer->color_scheme_select, 0);
    designer->color_scheme_select->parent_struct = designer;
    designer->color_scheme_select->func.value_changed_callback = set_selected_scheme;

    designer->color_sel = add_combobox(designer->color_widget, _("Colors"), 160, 250, 80,30);
    combobox_add_entry(designer->color_sel,_("bg"));
    combobox_add_entry(designer->color_sel,_("fg"));
    combobox_add_entry(designer->color_sel,_("base"));
    combobox_add_entry(designer->color_sel,_("text"));
    combobox_add_entry(designer->color_sel,_("shadow"));
    combobox_add_entry(designer->color_sel,_("frame"));
    combobox_add_entry(designer->color_sel,_("light"));
    combobox_set_active_entry(designer->color_sel, 0);
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
