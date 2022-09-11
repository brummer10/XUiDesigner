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
#include "XUiControllerType.h"


/*---------------------------------------------------------------------
-----------------------------------------------------------------------	
                add_image_load_button
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

static void ibutton_callback(void *w_, void* UNUSED(user_data)) {
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

static void ibutton_mem_free(void *w_, void* UNUSED(user_data)) {
    Widget_t *w = (Widget_t*)w_;
    ImageButton *imagebutton = (ImageButton *)w->parent_struct;
    free(imagebutton->last_path);
    imagebutton->last_path = NULL;
    free(imagebutton);
    imagebutton = NULL;
}

Widget_t *add_image_load_button(Widget_t *parent, int x, int y, int width, int height,
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
        const char* filename = *(const char**)user_data;
        cairo_surface_t *getpng = NULL;
        if (strstr(filename, ".png")) {
            getpng = cairo_image_surface_create_from_png (filename);
        } else if (strstr(filename, ".svg")) {
            getpng = cairo_image_surface_create_from_svg (filename);
        }
        if (!getpng) return;

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
        designer->image = strdup(filename);
    }
}

void unload_background_image(void *w_, void* UNUSED(user_data)) {
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

static void set_image_button(XUiDesigner *designer, WidgetType is_type) {
    Widget_t *wid = designer->active_widget;
    Widget_t *p = (Widget_t*)wid->parent;
    remove_from_list(designer, wid);
    designer->prev_active_widget = NULL;
    Widget_t *new_wid = NULL;
    asprintf (&designer->new_label[designer->active_widget_num], "%s",wid->label);
    if (is_type == IS_TOGGLE_BUTTON) {
        new_wid = add_switch_image_button(p, designer->new_label[designer->active_widget_num],
                                                                wid->x, wid->y, 60, 60);
        copy_widget_settings(designer, wid, new_wid);
        add_to_list(designer, new_wid, "add_lv2_image_toggle", false, IS_IMAGE_TOGGLE);
    } else if (is_type == IS_BUTTON) {
        new_wid = add_image_button(p, designer->new_label[designer->active_widget_num],
                                                                wid->x, wid->y, 60, 60);
        copy_widget_settings(designer, wid, new_wid);
        add_to_list(designer, new_wid, "add_lv2_image_button", false, IS_IMAGE_BUTTON);
    }
    destroy_widget(wid, designer->w->app);
    widget_show(new_wid);
    designer->active_widget = new_wid;
    designer->active_widget_num = new_wid->data;
}

static void set_all_image_button(XUiDesigner *designer, Widget_t *wid, int i, WidgetType is_type) {
    Widget_t *p = (Widget_t*)wid->parent;
    remove_from_list(designer, wid);
    designer->prev_active_widget = NULL;
    Widget_t *new_wid = NULL;
    asprintf (&designer->new_label[i], "%s",wid->label);
    if (is_type == IS_TOGGLE_BUTTON) {
        new_wid = add_switch_image_button(p, designer->new_label[i], wid->x, wid->y, 60, 60);
        copy_widget_settings(designer, wid, new_wid);
        add_to_list(designer, new_wid, "add_lv2_image_toggle", false, IS_IMAGE_TOGGLE);
    } else if (is_type == IS_BUTTON) {
        new_wid = add_image_button(p, designer->new_label[i], wid->x, wid->y, 60, 60);
        copy_widget_settings(designer, wid, new_wid);
        add_to_list(designer, new_wid, "add_lv2_image_button", false, IS_IMAGE_BUTTON);
    }
    destroy_widget(wid, designer->w->app);
    widget_show(new_wid);
}

static void unset_image_button(XUiDesigner *designer, WidgetType is_type) {
    Widget_t *wid = designer->active_widget;
    remove_from_list(designer, wid);
    designer->prev_active_widget = NULL;
    Widget_t *new_wid = NULL;
    asprintf (&designer->new_label[designer->active_widget_num], "%s",wid->label);
    if (is_type == IS_IMAGE_TOGGLE) {
        new_wid = add_toggle_button(designer->ui, designer->new_label[designer->active_widget_num],
                                                                wid->x, wid->y, 60, 60);
        copy_widget_settings(designer, wid, new_wid);
        add_to_list(designer, new_wid, "add_lv2_toggle_button", false, IS_TOGGLE_BUTTON);
    } else if (is_type == IS_IMAGE_BUTTON) {
        new_wid = add_button(designer->ui, designer->new_label[designer->active_widget_num],
                                                                wid->x, wid->y, 60, 60);
        copy_widget_settings(designer, wid, new_wid);
        add_to_list(designer, new_wid, "add_lv2_button", false, IS_BUTTON);
    }
    destroy_widget(wid, designer->w->app);
    designer->controls[new_wid->data].image = NULL;
    widget_show(new_wid);
    designer->active_widget = new_wid;
    designer->active_widget_num = new_wid->data;
}

static void set_slider_frames(void *w_, void* UNUSED(user_data)) {
    Widget_t *w = (Widget_t*)w_;
    XUiDesigner *designer = (XUiDesigner*)w->parent_struct;
    designer->controls[designer->active_widget_num].slider_image_sprites = (int)adj_get_value(w->adj);
    set_slider_image_frame_count(designer->active_widget, adj_get_value(w->adj));
}

static void load_for_all_global(XUiDesigner *designer, WidgetType is_type, cairo_surface_t *getpng,
                                const char* filename, int width, int height) {
    int i = 0;
    for (;i<MAX_CONTROLS;i++) {
        if (designer->controls[i].wid != NULL && designer->controls[i].is_type == is_type ) {
            if (designer->controls[i].is_type == IS_VSLIDER) {
                designer->controls[i].slider_image_sprites =
                    designer->global_vslider_image_sprites;
                
            } else if (designer->controls[i].is_type == IS_HSLIDER) {
                designer->controls[i].slider_image_sprites =
                    designer->global_hslider_image_sprites;
            }
            cairo_surface_destroy(designer->controls[i].wid->image);
            designer->controls[i].wid->image = NULL;

            designer->controls[i].wid->image = cairo_surface_create_similar (designer->controls[i].wid->surface, 
                                CAIRO_CONTENT_COLOR_ALPHA, width, height);
            cairo_t *cri = cairo_create (designer->controls[i].wid->image);
            cairo_set_source_surface (cri, getpng,0,0);
            cairo_paint (cri);
            cairo_destroy(cri);
            expose_widget(designer->controls[i].wid);
            free(designer->controls[i].image);
            designer->controls[i].image = NULL;
            designer->controls[i].image = strdup(filename);
            char *tmp = strdup(filename);
            free(designer->image_path);
            designer->image_path = NULL;
            designer->image_path = strdup(dirname(tmp));
            free(tmp);
        }
    }
}

void load_single_controller_image (XUiDesigner *designer, const char* filename) {
    //if (!designer->active_widget) return;
    char *tmp = strdup(filename);
    if (designer->controls[designer->active_widget_num].is_type == IS_TOGGLE_BUTTON ||
        designer->controls[designer->active_widget_num].is_type == IS_BUTTON) {
        set_image_button(designer, designer->controls[designer->active_widget_num].is_type);
    }
    cairo_surface_t *getpng = NULL;
    if (strstr(filename, ".png")) {
        getpng = cairo_image_surface_create_from_png (filename);
    } else if (strstr(filename, ".svg")) {
        getpng = cairo_image_surface_create_from_svg (filename);
    }
    if (!getpng) return;
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
    designer->controls[designer->active_widget_num].image = strdup(tmp);
    free(designer->image_path);
    designer->image_path = NULL;
    designer->image_path = strdup(dirname(tmp));
    free(tmp);
}

void controller_image_load_response(void *w_, void* user_data) {
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
        if (designer->controls[designer->active_widget_num].is_type == IS_TOGGLE_BUTTON ||
            designer->controls[designer->active_widget_num].is_type == IS_BUTTON) {
            set_image_button(designer, designer->controls[designer->active_widget_num].is_type);
        } else if (designer->controls[designer->active_widget_num].is_type == IS_VSLIDER ||
                    designer->controls[designer->active_widget_num].is_type == IS_HSLIDER) {
            Widget_t *dia = open_message_dialog(w, INFO_BOX, *(const char**)user_data,
                                                _("How many Sprites been in the Image?"),NULL);
            XSetTransientForHint(w->app->dpy, dia->widget, designer->ui->widget);
            Widget_t *counter =  add_valuedisplay(dia, _("SliderSprites"), 125, 65, 60, 40);
            counter->parent_struct = designer;
            set_adjustment(counter->adj, 101, designer->controls[designer->active_widget_num].slider_image_sprites,
                                                                                    1.0, 330.0, 1.0, CL_CONTINUOS);
            counter->func.value_changed_callback = set_slider_frames;
            widget_show(counter);
        }
        const char* filename = *(const char**)user_data;
        cairo_surface_t *getpng = NULL;
        if (strstr(filename, ".png")) {
            getpng = cairo_image_surface_create_from_png (filename);
        } else if (strstr(filename, ".svg")) {
            getpng = cairo_image_surface_create_from_svg (filename);
        }
        if (!getpng) return;

        int width = cairo_image_surface_get_width(getpng);
        int height = cairo_image_surface_get_height(getpng);
        if (designer->controls[designer->active_widget_num].is_type == IS_KNOB &&
                                    adj_get_value(designer->global_knob_image->adj)) {
            load_for_all_global(designer, IS_KNOB, getpng, filename, width, height);
            free(designer->global_knob_image_file);
            designer->global_knob_image_file = NULL;
            asprintf(&designer->global_knob_image_file, "%s", filename);
            cairo_surface_destroy(getpng);
        } else if (designer->controls[designer->active_widget_num].is_type == IS_VSLIDER &&
                                    adj_get_value(designer->global_vslider_image->adj)) {
            designer->global_vslider_image_sprites =
                designer->controls[designer->active_widget_num].slider_image_sprites;
            load_for_all_global(designer, IS_VSLIDER, getpng, filename, width, height);
            free(designer->global_vslider_image_file);
            designer->global_vslider_image_file = NULL;
            asprintf(&designer->global_vslider_image_file, "%s", filename);
            cairo_surface_destroy(getpng);
        } else if (designer->controls[designer->active_widget_num].is_type == IS_HSLIDER &&
                                    adj_get_value(designer->global_hslider_image->adj)) {
            designer->global_hslider_image_sprites =
                designer->controls[designer->active_widget_num].slider_image_sprites;
            load_for_all_global(designer, IS_HSLIDER, getpng, filename, width, height);
            free(designer->global_hslider_image_file);
            designer->global_hslider_image_file = NULL;
            asprintf(&designer->global_hslider_image_file, "%s", filename);
            cairo_surface_destroy(getpng);
        } else if (designer->controls[designer->active_widget_num].is_type == IS_BUTTON &&
                                    adj_get_value(designer->global_button_image->adj)) {
            int i = 0;
            for (;i<MAX_CONTROLS;i++) {
                if (designer->controls[i].wid != NULL && designer->controls[i].is_type == IS_BUTTON) {
                    set_all_image_button(designer, designer->controls[i].wid, i, designer->controls[i].is_type);
                }
            }
            load_for_all_global(designer, IS_BUTTON, getpng, filename, width, height);
            free(designer->global_button_image_file);
            designer->global_button_image_file = NULL;
            asprintf(&designer->global_button_image_file, "%s", filename);
            cairo_surface_destroy(getpng);
        } else if (designer->controls[designer->active_widget_num].is_type == IS_IMAGE_TOGGLE &&
                                    adj_get_value(designer->global_switch_image->adj)) {
            int i = 0;
            for (;i<MAX_CONTROLS;i++) {
                if (designer->controls[i].wid != NULL && designer->controls[i].is_type == IS_TOGGLE_BUTTON) {
                    set_all_image_button(designer, designer->controls[i].wid, i, designer->controls[i].is_type);
                }
            }
            load_for_all_global(designer, IS_IMAGE_TOGGLE, getpng, filename, width, height);
            free(designer->global_switch_image_file);
            designer->global_switch_image_file = NULL;
            asprintf(&designer->global_switch_image_file, "%s", filename);
            cairo_surface_destroy(getpng);
        } else {
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
            designer->controls[designer->active_widget_num].image = strdup(filename);
            char *tmp = strdup(filename);
            free(designer->image_path);
            designer->image_path = NULL;
            designer->image_path = strdup(dirname(tmp));
            free(tmp);
        }
    }
}

static void background_image_load_response(void *w_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    XUiDesigner *designer = (XUiDesigner*)w->parent_struct;
    if (designer->active_widget == NULL) return;
    if(user_data !=NULL) {

        if( access(*(const char**)user_data, F_OK ) == -1 ) {
            Widget_t *dia = open_message_dialog(w, ERROR_BOX, *(const char**)user_data,
                                                _("Couldn't access file, sorry"),NULL);
            XSetTransientForHint(w->app->dpy, dia->widget, w->widget);
            return;
        }
        const char* filename = *(const char**)user_data;
        cairo_surface_t *getpng = NULL;
        if (strstr(filename, ".png")) {
            getpng = cairo_image_surface_create_from_png (filename);
        } else if (strstr(filename, ".svg")) {
            getpng = cairo_image_surface_create_from_svg (filename);
        }
        if (!getpng) return;

        int width = cairo_image_surface_get_width(getpng);
        int height = cairo_image_surface_get_height(getpng);
        int width_t = designer->active_widget->scale.init_width;
        int height_t = designer->active_widget->scale.init_height;
        double x = (double)width_t/(double)width;
        double y = (double)height_t/(double)height;
        cairo_surface_destroy(designer->active_widget->image);
        designer->active_widget->image = NULL;

        designer->active_widget->image = cairo_surface_create_similar (designer->active_widget->surface, 
                            CAIRO_CONTENT_COLOR_ALPHA, width_t, height_t);
        cairo_t *cri = cairo_create (designer->active_widget->image);
        cairo_scale(cri, x,y);    
        cairo_set_source_surface (cri, getpng,0,0);
        cairo_paint (cri);
        cairo_surface_destroy(getpng);
        cairo_destroy(cri);
        expose_widget(designer->active_widget);
        free(designer->controls[designer->active_widget_num].image);
        designer->controls[designer->active_widget_num].image = NULL;
        designer->controls[designer->active_widget_num].image = strdup(filename);
        char *tmp = strdup(filename);
        free(designer->image_path);
        designer->image_path = NULL;
        designer->image_path = strdup(dirname(tmp));
        free(tmp);
    }
}

static void unload_controller_image(void *w_, void* UNUSED(user_data)) {
    Widget_t *w = (Widget_t*)w_;
    if (!w) return;
    Widget_t *p = (Widget_t*)w->parent;
    XUiDesigner *designer = (XUiDesigner*)p->parent_struct;
    if (designer->controls[designer->active_widget_num].is_type == IS_COMBOBOX ||
        designer->controls[designer->active_widget_num].is_type == IS_VALUE_DISPLAY ||
        designer->controls[designer->active_widget_num].is_type == IS_VMETER ||
        designer->controls[designer->active_widget_num].is_type == IS_HMETER ||
        designer->controls[designer->active_widget_num].is_type == IS_LABEL) return;
    cairo_surface_destroy(w->image);
    w->image = NULL;
    expose_widget(w);
    free(designer->controls[designer->active_widget_num].image);
    designer->controls[designer->active_widget_num].image = NULL;
    if (designer->controls[designer->active_widget_num].is_type == IS_IMAGE_TOGGLE ||
        designer->controls[designer->active_widget_num].is_type == IS_IMAGE_BUTTON) {
        unset_image_button(designer, designer->controls[designer->active_widget_num].is_type);
    }
}

void pop_menu_response(void *w_, void* item_, void* UNUSED(user_data)) {
    Widget_t *w = (Widget_t*)w_;
    XUiDesigner *designer = (XUiDesigner*)w->parent_struct;
    switch (*(int*)item_) {
        case 0: 
        {
        if (designer->controls[designer->active_widget_num].is_type == IS_COMBOBOX ||
            designer->controls[designer->active_widget_num].is_type == IS_VALUE_DISPLAY ||
            designer->controls[designer->active_widget_num].is_type == IS_VMETER ||
            designer->controls[designer->active_widget_num].is_type == IS_HMETER ||
            designer->controls[designer->active_widget_num].is_type == IS_LABEL) break;
            Widget_t *dia = open_file_dialog(designer->ui, designer->image_path, "image");
            XSetTransientForHint(designer->ui->app->dpy, dia->widget, designer->ui->widget);
            if (designer->controls[designer->active_widget_num].is_type == IS_FRAME ||
                designer->controls[designer->active_widget_num].is_type == IS_IMAGE) {
                designer->ui->func.dialog_callback = background_image_load_response;
            } else {
                designer->ui->func.dialog_callback = controller_image_load_response;
            }
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
        {
            if (designer->controls[designer->active_widget_num].is_type == IS_FRAME) {
                int elem = designer->active_widget->childlist->elem;
                int i = elem;
                for(;i>0;i--) {
                    Widget_t *wi = designer->active_widget->childlist->childs[i-1];
                    remove_from_list(designer, wi);
                }
            }
            if (designer->controls[designer->active_widget_num].is_type == IS_TABBOX) {
                int elem = designer->active_widget->childlist->elem;
                int i = elem;
                for(;i>0;i--) {
                    Widget_t *wi = designer->active_widget->childlist->childs[i-1];
                    int el = wi->childlist->elem;
                    int j = el;
                    for(;j>0l;j--) {
                        Widget_t *wid = wi->childlist->childs[j-1];
                        remove_from_list(designer, wid);
                    }
                }
            }
            remove_from_list(designer, designer->active_widget);
            destroy_widget(designer->active_widget, w->app);
            designer->active_widget = NULL;
            designer->prev_active_widget = NULL;
            box_entry_set_text(designer->controller_label, "");
            adj_set_value(designer->x_axis->adj, 0.0);
            adj_set_value(designer->y_axis->adj, 0.0);
            adj_set_value(designer->w_axis->adj, 10.0);
            adj_set_value(designer->h_axis->adj, 10.0);
            if (designer->combobox_settings)
                widget_hide(designer->combobox_settings);
            if (designer->controller_settings)
                widget_hide(designer->controller_settings);
        }
        break;
        default:
        break;
    }
}
