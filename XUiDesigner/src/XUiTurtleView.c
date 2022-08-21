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

#include "XUiTurtleView.h"
#include "XUiWriteTurtle.h"


/*---------------------------------------------------------------------
-----------------------------------------------------------------------    
                show generated ttl file for loaded plugin
-----------------------------------------------------------------------
----------------------------------------------------------------------*/

static void adjust_viewport(void *w_, void* UNUSED(user_data)) {
    Widget_t *parent = (Widget_t*)w_;
    Widget_t *w = parent->childlist->childs[1];
    XWindowAttributes attrs;
    XGetWindowAttributes(parent->app->dpy, (Window)parent->widget, &attrs);
    int height_t = attrs.height;
    XGetWindowAttributes(parent->app->dpy, (Window)w->widget, &attrs);
    int height = attrs.height;
    float d = (float)height/(float)height_t;
    float max_value = (float)((float)height/((float)(height_t/(float)(height-height_t))*(d*10.0)));
    float value = adj_get_value(w->adj);
    w->adj_y->max_value = max_value;
    if (max_value < value) adj_set_value(w->adj,value);
}

static void draw_ttlview(void *w_, void* UNUSED(user_data)) {
    Widget_t *w = (Widget_t*)w_;
    Widget_t *p = (Widget_t*)w->parent;
    FILE *fp;
    if ((fp=fopen("/tmp/xui.ttl", "r"))==NULL) {
        printf("open failed\n");
        return;
    }
    int i = 1;
    char buf[200];
    while(fgets(buf,sizeof(buf),fp) != NULL) {
        i++;
    }
    fseek(fp, 0, SEEK_SET);
    XResizeWindow(w->app->dpy, w->widget, w->width, (w->app->normal_font/w->scale.ascale) * i + 6 * i + 10);
    adjust_viewport(p, NULL);

    cairo_set_source_rgba(w->crb,  0.13, 0.13, 0.13, 1.0);
    cairo_paint (w->crb);

    i = 1;
    use_text_color_scheme(w, get_color_state(w));
    cairo_set_font_size (w->crb, w->app->normal_font/w->scale.ascale);

    while(fgets(buf,sizeof(buf),fp) != NULL) {
        strdecode(buf, "\n", "");
        cairo_move_to (w->crb, 10, (w->app->normal_font/w->scale.ascale) * i + 6 * i);
        cairo_show_text(w->crb, buf);
        i++;
    }
    fclose(fp);
    fp = NULL;
    
}

static void draw_viewslider(void *w_, void* UNUSED(user_data)) {
    Widget_t *w = (Widget_t*)w_;
    int v = (int)w->adj->max_value;
    if (!v) return;
    XWindowAttributes attrs;
    XGetWindowAttributes(w->app->dpy, (Window)w->widget, &attrs);
    if (attrs.map_state != IsViewable) return;
    int width = attrs.width;
    int height = attrs.height;
    float sliderstate = adj_get_state(w->adj);
    use_bg_color_scheme(w, get_color_state(w));
    cairo_rectangle(w->crb, 0,0,width,height);
    cairo_fill_preserve(w->crb);
    use_shadow_color_scheme(w, NORMAL_);
    cairo_fill(w->crb);
    use_bg_color_scheme(w, NORMAL_);
    cairo_rectangle(w->crb, 0,(height-10)*sliderstate,width,10);
    cairo_fill(w->crb);
}

static void set_viewpoint(void *w_, void* UNUSED(user_data)) {
    Widget_t *w = (Widget_t*)w_;
    Widget_t *p = (Widget_t*)w->parent;
    Widget_t *slider = p->childlist->childs[0];
    adj_set_state(slider->adj, adj_get_state(w->adj));
    int v = (int)max(0,adj_get_value(w->adj));
    XMoveWindow(w->app->dpy,w->widget,0, -10*v);
}

static void set_viewport(void *w_, void* UNUSED(user_data)) {
    Widget_t *w = (Widget_t*)w_;
    Widget_t *p = (Widget_t*)w->parent;
    Widget_t *viewport = p->childlist->childs[1];
    adj_set_state(viewport->adj, adj_get_state(w->adj));
}

static Widget_t* add_viewport(Widget_t *parent, int width, int height) {
    Widget_t *slider = add_vslider(parent, "", width, 0, 10, height);
    slider->func.expose_callback = draw_viewslider;
    slider->adj_y = add_adjustment(slider,0.0, 0.0, 0.0, 1.0,0.0085, CL_VIEWPORTSLIDER);
    slider->adj = slider->adj_y;
    slider->func.value_changed_callback = set_viewport;
    slider->scale.gravity = NORTHWEST;
    slider->flags &= ~USE_TRANSPARENCY;
    slider->flags |= NO_AUTOREPEAT | NO_PROPAGATE;

    Widget_t *wid = create_widget(parent->app, parent, 0, 0, width, height);
    XSelectInput(wid->app->dpy, wid->widget,StructureNotifyMask|ExposureMask|KeyPressMask 
                    |EnterWindowMask|LeaveWindowMask|ButtonReleaseMask|KeyReleaseMask
                    |ButtonPressMask|Button1MotionMask|PointerMotionMask);
    Widget_t *p = (Widget_t*)parent->parent;
    wid->parent_struct = p->parent_struct;
    wid->scale.gravity = NONE;
    wid->flags &= ~USE_TRANSPARENCY;
    wid->flags |= NO_AUTOREPEAT | NO_PROPAGATE;
    XWindowAttributes attrs;
    XGetWindowAttributes(parent->app->dpy, (Window)parent->widget, &attrs);
    int height_t = attrs.height;
    float d = (float)height/(float)height_t;
    float max_value = (float)((float)height/((float)(height_t/(float)(height-height_t))*(d*10.0)));
    wid->adj_y = add_adjustment(wid,0.0, 0.0, 0.0,max_value ,3.0, CL_VIEWPORT);
    wid->adj = wid->adj_y;
    wid->func.adj_callback = set_viewpoint;
    wid->func.expose_callback = draw_ttlview;
    adj_set_value(wid->adj,0.0);

    parent->func.configure_notify_callback = adjust_viewport;

    return wid;
}

void create_text_view_window(XUiDesigner *designer) {
    Atom wmStateAbove = XInternAtom(designer->w->app->dpy, "_NET_WM_STATE_ABOVE", 1 );
    Atom wmNetWmState = XInternAtom(designer->w->app->dpy, "_NET_WM_STATE", 1 );

    designer->ttlfile_view = create_window(designer->w->app, DefaultRootWindow(designer->w->app->dpy), 0, 0, 620, 800);
    XChangeProperty(designer->w->app->dpy, designer->ttlfile_view->widget, wmNetWmState, XA_ATOM, 32, 
        PropModeReplace, (unsigned char *) &wmStateAbove, 1); 
    //XSetTransientForHint(designer->w->app->dpy, w->widget, designer->ui->widget);
    //designer->set_project->func.expose_callback = draw_text_window;
    designer->ttlfile_view->flags |= HIDE_ON_DELETE;
    widget_set_title(designer->ttlfile_view, _("ttl"));
    widget_set_icon_from_png(designer->ttlfile_view, LDVAR(file_png));

    add_viewport(designer->ttlfile_view, 610, 800);
}

void run_generate_ttl(void *w_, void* UNUSED(user_data)) {
    Widget_t *w = (Widget_t*)w_;
    if (w->flags & HAS_POINTER && !adj_get_value(w->adj_y)) {
        XUiDesigner *designer = (XUiDesigner*)w->parent_struct;

        FILE *fp;
        if((fp=freopen("/tmp/xui.ttl", "w" ,stdout))==NULL) {
            printf("open failed\n");
            return;
        }

        print_ttl(designer);

        fclose(fp);
        fp = NULL;
        XWindowAttributes attrs;
        XGetWindowAttributes(w->app->dpy, (Window)designer->ttlfile_view->widget, &attrs);
        if (attrs.map_state != IsViewable) {
            widget_show_all(designer->ttlfile_view);
        } else {
            Widget_t *viewport = designer->ttlfile_view->childlist->childs[1];
            expose_widget(viewport);
        }

    }
}
