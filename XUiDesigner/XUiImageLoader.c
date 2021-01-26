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

#include "XUiImageLoader.h"
#include "XUiGenerator.h"


/*---------------------------------------------------------------------
-----------------------------------------------------------------------    
                image handling
-----------------------------------------------------------------------
----------------------------------------------------------------------*/

void image_load_response(void *w_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    Widget_t *p = (Widget_t*)w->parent;
    XUiDesigner *designer = (XUiDesigner*)p->parent_struct;
    if(user_data !=NULL) {

        if( access(*(const char**)user_data, F_OK ) == -1 ) {
            Widget_t *dia = open_message_dialog(w, ERROR_BOX, *(const char**)user_data,
                                                _("Couldn't access file, sorry"),NULL);
            XSetTransientForHint(w->app->dpy, dia->widget, w->widget);
            return;
        }
        cairo_surface_t *getpng = cairo_image_surface_create_from_png (*(const char**)user_data);
        int width = cairo_image_surface_get_width(getpng);
        int height = cairo_image_surface_get_height(getpng);
        int width_t = designer->ui->scale.init_width;
        int height_t = designer->ui->scale.init_height;
        double x = (double)width_t/(double)width;
        double y = (double)height_t/(double)height;
        cairo_surface_destroy(designer->ui->image);
        designer->ui->image = NULL;

        designer->ui->image = cairo_surface_create_similar (designer->ui->surface, 
                            CAIRO_CONTENT_COLOR_ALPHA, width_t, height_t);
        cairo_t *cri = cairo_create (designer->ui->image);
        cairo_scale(cri, x,y);    
        cairo_set_source_surface (cri, getpng,0,0);
        cairo_paint (cri);
        cairo_surface_destroy(getpng);
        cairo_destroy(cri);
        expose_widget(designer->ui);
        free(designer->image);
        designer->image = NULL;
        designer->image = strdup(*(const char**)user_data);
    }
}

void unload_background_image(void *w_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    if (w->flags & HAS_POINTER && !adj_get_value(w->adj_y)) {
        XUiDesigner *designer = (XUiDesigner*)w->parent_struct;
        cairo_surface_destroy(designer->ui->image);
        designer->ui->image = NULL;
        expose_widget(designer->ui);
        free(designer->image);
        designer->image = NULL;
    }
}

static void controller_image_load_response(void *w_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    XUiDesigner *designer = (XUiDesigner*)w->parent_struct;
    if (!designer->active_widget) return;
    if(user_data !=NULL) {

        if( access(*(const char**)user_data, F_OK ) == -1 ) {
            Widget_t *dia = open_message_dialog(w, ERROR_BOX, *(const char**)user_data,
                                                _("Couldn't access file, sorry"),NULL);
            XSetTransientForHint(w->app->dpy, dia->widget, designer->ui->widget);
            return;
        }
        cairo_surface_t *getpng = cairo_image_surface_create_from_png (*(const char**)user_data);
        int width = cairo_image_surface_get_width(getpng);
        int height = cairo_image_surface_get_height(getpng);
        cairo_surface_destroy(designer->active_widget->image);
        designer->active_widget->image = NULL;

        designer->active_widget->image = cairo_surface_create_similar (designer->active_widget->surface, 
                            CAIRO_CONTENT_COLOR_ALPHA, width, height);
        cairo_t *cri = cairo_create (designer->active_widget->image);
        cairo_set_source_surface (cri, getpng,0,0);
        cairo_paint (cri);
        cairo_surface_destroy(getpng);
        cairo_destroy(cri);
        expose_widget(designer->active_widget);
        free(designer->controls[designer->active_widget_num].image);
        designer->controls[designer->active_widget_num].image = NULL;
        designer->controls[designer->active_widget_num].image = strdup(*(const char**)user_data);
        char *tmp = strdup(*(const char**)user_data);
        free(designer->image_path);
        designer->image_path = NULL;
        designer->image_path = strdup(dirname(tmp));
        free(tmp);
    }
}

static void unload_controller_image(void *w_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    if (!w) return;
    Widget_t *p = (Widget_t*)w->parent;
    XUiDesigner *designer = (XUiDesigner*)p->parent_struct;
    if (designer->controls[designer->active_widget_num].is_type == IS_COMBOBOX ||
        designer->controls[designer->active_widget_num].is_type == IS_VALUE_DISPLAY ||
        designer->controls[designer->active_widget_num].is_type == IS_VMETER ||
        designer->controls[designer->active_widget_num].is_type == IS_HMETER ||
        designer->controls[designer->active_widget_num].is_type == IS_VSLIDER ||
        designer->controls[designer->active_widget_num].is_type == IS_HSLIDER) return;
    cairo_surface_destroy(w->image);
    w->image = NULL;
    expose_widget(w);
    free(designer->controls[designer->active_widget_num].image);
    designer->controls[designer->active_widget_num].image = NULL;
}

void pop_menu_response(void *w_, void* item_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    XUiDesigner *designer = (XUiDesigner*)w->parent_struct;
    switch (*(int*)item_) {
        case 0: 
        {
        if (designer->controls[designer->active_widget_num].is_type == IS_COMBOBOX ||
            designer->controls[designer->active_widget_num].is_type == IS_VALUE_DISPLAY ||
            designer->controls[designer->active_widget_num].is_type == IS_VMETER ||
            designer->controls[designer->active_widget_num].is_type == IS_HMETER ||
            designer->controls[designer->active_widget_num].is_type == IS_VSLIDER ||
            designer->controls[designer->active_widget_num].is_type == IS_HSLIDER) break;
            Widget_t *dia = open_file_dialog(designer->ui, designer->image_path, ".png");
            XSetTransientForHint(designer->ui->app->dpy, dia->widget, designer->ui->widget);
            designer->ui->func.dialog_callback = controller_image_load_response;
        }
        break;
        case 1:
            unload_controller_image(designer->active_widget,NULL);
        break;
        case 2:
        break;
        case 3:
        break;
        case 4:
            remove_from_list(designer, designer->active_widget);
            destroy_widget(designer->active_widget, w->app);
            designer->active_widget = NULL;
            entry_set_text(designer, "");
            adj_set_value(designer->x_axis->adj, 0.0);
            adj_set_value(designer->y_axis->adj, 0.0);
            adj_set_value(designer->w_axis->adj, 10.0);
            adj_set_value(designer->h_axis->adj, 10.0);
            widget_hide(designer->combobox_settings);
            widget_hide(designer->controller_settings);
        break;
        default:
        break;
    }
}
