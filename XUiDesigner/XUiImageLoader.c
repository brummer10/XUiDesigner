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
#include "XUiTextInput.h"


/*---------------------------------------------------------------------
-----------------------------------------------------------------------	
                add_image_button
-----------------------------------------------------------------------
----------------------------------------------------------------------*/


typedef struct {
    Widget_t *w;
    char *last_path;
    const char *path;
    const char *filter;
    bool is_active;
} ImageButton;

static void idialog_response(void *w_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    ImageButton *imagebutton = (ImageButton *)w->parent_struct;
    if(user_data !=NULL) {
        char *tmp = strdup(*(const char**)user_data);
        free(imagebutton->last_path);
        imagebutton->last_path = NULL;
        imagebutton->last_path = strdup(dirname(tmp));
        imagebutton->path = imagebutton->last_path;
        free(tmp);
    }
    w->func.user_callback(w,user_data);
    imagebutton->is_active = false;
    adj_set_value(w->adj,0.0);
}

static void ibutton_callback(void *w_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    ImageButton *imagebutton = (ImageButton *)w->parent_struct;
    if (w->flags & HAS_POINTER && adj_get_value(w->adj)){
        imagebutton->w = open_file_dialog(w,imagebutton->path,imagebutton->filter);
        Atom wmStateAbove = XInternAtom(w->app->dpy, "_NET_WM_STATE_ABOVE", 1 );
        Atom wmNetWmState = XInternAtom(w->app->dpy, "_NET_WM_STATE", 1 );
        XChangeProperty(w->app->dpy, imagebutton->w->widget, wmNetWmState, XA_ATOM, 32, 
            PropModeReplace, (unsigned char *) &wmStateAbove, 1); 
        imagebutton->is_active = true;
    } else if (w->flags & HAS_POINTER && !adj_get_value(w->adj)){
        if(imagebutton->is_active)
            destroy_widget(imagebutton->w,w->app);
    }
}

static void ibutton_mem_free(void *w_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    ImageButton *imagebutton = (ImageButton *)w->parent_struct;
    free(imagebutton->last_path);
    imagebutton->last_path = NULL;
    free(imagebutton);
    imagebutton = NULL;
}

Widget_t *add_image_button(Widget_t *parent, int x, int y, int width, int height,
                           const char *path, const char *filter) {
    ImageButton *imagebutton = (ImageButton*)malloc(sizeof(ImageButton));
    imagebutton->path = path;
    imagebutton->filter = filter;
    imagebutton->last_path = NULL;
    imagebutton->w = NULL;
    imagebutton->is_active = false;
    Widget_t *fbutton = add_image_toggle_button(parent, "", x, y, width, height);
    fbutton->parent_struct = imagebutton;
    fbutton->flags |= HAS_MEM;
    widget_get_png(fbutton, LDVAR(image_directory_png));
    fbutton->scale.gravity = CENTER;
    fbutton->func.mem_free_callback = ibutton_mem_free;
    fbutton->func.value_changed_callback = ibutton_callback;
    fbutton->func.dialog_callback = idialog_response;
    return fbutton;
}

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

static void set_image_button(XUiDesigner *designer) {
    Widget_t *wid = designer->active_widget;
    remove_from_list(designer, wid);
    designer->prev_active_widget = NULL;
    Widget_t *new_wid = NULL;
    asprintf (&designer->new_label[designer->active_widget_num], "%s",wid->label);
    new_wid = add_switch_image_button(designer->ui, designer->new_label[designer->active_widget_num],
                                                                wid->x, wid->y, 60, 60);
    set_controller_callbacks(designer, new_wid, true);
    new_wid->data = designer->active_widget_num;
    designer->wid_counter--;
    add_to_list(designer, new_wid, "add_lv2_image_toggle", false, IS_IMAGE_TOGGLE);
    destroy_widget(wid, designer->w->app);
    widget_show(new_wid);
    designer->active_widget = new_wid;
}

static void unset_image_button(XUiDesigner *designer) {
    Widget_t *wid = designer->active_widget;
    remove_from_list(designer, wid);
    designer->prev_active_widget = NULL;
    Widget_t *new_wid = NULL;
    asprintf (&designer->new_label[designer->active_widget_num], "%s",wid->label);
    new_wid = add_toggle_button(designer->ui, designer->new_label[designer->active_widget_num],
                                                                wid->x, wid->y, 60, 60);
    set_controller_callbacks(designer, new_wid, true);
    new_wid->data = designer->active_widget_num;
    designer->wid_counter--;
    add_to_list(designer, new_wid, "add_lv2_toggle_button", false, IS_TOGGLE_BUTTON);
    destroy_widget(wid, designer->w->app);
    designer->controls[new_wid->data].image = NULL;
    widget_show(new_wid);
    designer->active_widget = new_wid;
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
        if (designer->controls[designer->active_widget_num].is_type == IS_TOGGLE_BUTTON) {
            set_image_button(designer);
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
    if (designer->controls[designer->active_widget_num].is_type == IS_IMAGE_TOGGLE) {
        unset_image_button(designer);
    }
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
            designer->prev_active_widget = NULL;
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
