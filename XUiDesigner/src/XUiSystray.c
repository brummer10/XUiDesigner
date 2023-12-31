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


#include "XUiSystray.h"
#include "XUiTextInput.h"
#include "XUiGenerator.h"
#include "XUiSettings.h"


typedef struct {
    Widget_t *systray;
    Widget_t *systray_menu;
} Systray_t;

/*---------------------------------------------------------------------
-----------------------------------------------------------------------    
                systray responses
-----------------------------------------------------------------------
----------------------------------------------------------------------*/


static void draw_systray(void *w_, void* UNUSED(user_data)) {
    Widget_t *w = (Widget_t*)w_;
    use_systray_color(w);
    cairo_paint (w->crb);
    if (w->image) {
        widget_set_scale(w);
        cairo_set_source_surface (w->crb, w->image, 0, 0);
        cairo_mask_surface (w->crb, w->image, 0, 0);
        widget_reset_scale(w);
    }
}

static void systray_menu_response(void *w_, void* item_, void* UNUSED(user_data)) {
    Widget_t *w = (Widget_t*)w_;
    XUiDesigner *designer = (XUiDesigner*)w->parent_struct;
    switch (*(int*)item_) {
        case 0:
        {
            const char* home = getenv("HOME");
            Widget_t* tmp = open_directory_dialog(designer->ui, home, NULL);
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
            if (!designer->set_project) create_project_settings_window(designer);
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

static void systray_released(void *w_, void* button_, void* UNUSED(user_data)) {
    Widget_t *w = (Widget_t*)w_;
    XUiDesigner *designer = (XUiDesigner*)w->parent_struct;
    Systray_t *systray = (Systray_t*)w->private_struct;
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
                if (designer->set_project)
                    widget_hide(designer->set_project);
                XFlush(designer->w->app->dpy);
            } else {
                widget_show_all(designer->w);
                widget_show_all(designer->ui);
                hide_show_as_needed(designer);
            }
        } else if (xbutton->button == Button3) {
            pop_menu_show(w,systray->systray_menu,3,true);
        }
    }
}

static void create_systray_menu(XUiDesigner *designer, Systray_t *systray) {
    systray->systray_menu = create_menu(designer->w,25);
    systray->systray_menu->private_struct = systray;
    systray->systray_menu->parent_struct = designer;
    menu_add_item(systray->systray_menu,_("Save as:"));
    menu_add_item(systray->systray_menu,_("Setup Project"));
    menu_add_item(systray->systray_menu,_("Quit"));
    systray->systray_menu->func.button_release_callback = systray_menu_response;
}

static void systray_mem_free(void *w_, void* UNUSED(user_data)) {
    Widget_t *w = (Widget_t*)w_;
    Systray_t *systray = (Systray_t*)w->private_struct;
    free(systray);
}

void create_systray_widget(XUiDesigner *designer, int x, int y, int w, int h) {
    Systray_t *systray = (Systray_t*)malloc(sizeof(Systray_t));
    systray->systray = create_window(designer->w->app, DefaultRootWindow(designer->w->app->dpy), x, y, w, h);
    systray->systray->private_struct = systray;
    systray->systray->parent_struct = designer;
    systray->systray->flags |= HAS_MEM;
    systray->systray->func.mem_free_callback = systray_mem_free;
    systray->systray->func.expose_callback = draw_systray;
    widget_get_png(systray->systray, LDVAR(gear_png));
    systray->systray->func.button_release_callback = systray_released;
    send_systray_message(systray->systray);
    create_systray_menu(designer, systray);   
}
