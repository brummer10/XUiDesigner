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

#include "XUiControllerType.h"
#include "XUiGenerator.h"
#include "XUiImageLoader.h"
#include "XUiDraw.h"


/*---------------------------------------------------------------------
-----------------------------------------------------------------------    
                designer controller dummy callback
-----------------------------------------------------------------------
----------------------------------------------------------------------*/


void null_callback(void* UNUSED(w_), void* UNUSED(user_data)) {
    
}


/*---------------------------------------------------------------------
-----------------------------------------------------------------------    
                change controller type
-----------------------------------------------------------------------
----------------------------------------------------------------------*/

void copy_widget_settings(XUiDesigner *designer, Widget_t *wid, Widget_t *new_wid) {
    if (wid->adj != NULL) {
        if (designer->controls[wid->data].is_type == IS_COMBOBOX ) {
            Widget_t *menu = wid->childlist->childs[1];
            Widget_t* view_port =  menu->childlist->childs[0];
            ComboBox_t *comboboxlist = (ComboBox_t*)view_port->parent_struct;
            unsigned int i = 0;
            for (;i<comboboxlist->list_size;i++) {
                combobox_add_entry(new_wid, comboboxlist->list_names[i]);
            }
        } else if (wid->adj->type == CL_LOGARITHMIC) {
             set_adjustment(new_wid->adj, powf(10,wid->adj->std_value), powf(10,wid->adj->std_value),
                powf(10,wid->adj->min_value),powf(10,wid->adj->max_value), wid->adj->step, wid->adj->type);
        } else if (wid->adj->type == CL_LOGSCALE) {
            set_adjustment(new_wid->adj, log10(wid->adj->std_value)*wid->adj->log_scale,
                                        log10(wid->adj->std_value)*wid->adj->log_scale,
                                        log10(wid->adj->min_value)*wid->adj->log_scale,
                                        log10(wid->adj->max_value)*wid->adj->log_scale,
                                        wid->adj->step, wid->adj->type);
        } else {
            set_adjustment(new_wid->adj, wid->adj->std_value, wid->adj->std_value,
                wid->adj->min_value,wid->adj->max_value, wid->adj->step, wid->adj->type);
        }
    }
    memcpy(new_wid->color_scheme, wid->color_scheme, sizeof (struct XColor_t));
    set_controller_callbacks(designer, new_wid, true);
    new_wid->data = wid->data;
    designer->wid_counter--;
    adj_set_value(designer->index->adj, adj_get_value(designer->index->adj)-1.0);
}

void switch_controller_type(void *w_, void* UNUSED(user_data)) {
    Widget_t *w = (Widget_t*)w_;
    XUiDesigner *designer = (XUiDesigner*)w->parent_struct;
    Widget_t *wid = designer->active_widget;
    designer->prev_active_widget = NULL;
    Widget_t *new_wid = NULL;
    if (designer->controls[designer->active_widget_num].is_type == IS_COMBOBOX ) {
        designer->controls[designer->active_widget_num].is_type = -1;
    }
    int v = (int) adj_get_value(w->adj);
    switch (v) {
        case 0:
            asprintf (&designer->new_label[designer->active_widget_num], "%s",wid->label);
            new_wid = add_knob(designer->ui, designer->new_label[designer->active_widget_num],
                                                                        wid->x, wid->y, 60, 80);
            copy_widget_settings(designer, wid, new_wid);
            new_wid->adj->type = new_wid->adj->type == CL_LOGARITHMIC ? CL_LOGARITHMIC :
                new_wid->adj->type == CL_LOGSCALE ? CL_LOGSCALE : CL_CONTINUOS;
            add_to_list(designer, new_wid, "add_lv2_knob", true, IS_KNOB);
            destroy_widget(wid, designer->w->app);
            designer->controls[new_wid->data].image = NULL;
            widget_show(new_wid);
            designer->active_widget = new_wid;
            designer->active_widget_num = new_wid->data;
            if (designer->global_knob_image_file != NULL && adj_get_value(designer->global_knob_image->adj)) 
                load_single_controller_image(designer, designer->global_knob_image_file);
        break;
        case 1:
            asprintf (&designer->new_label[designer->active_widget_num], "%s",wid->label);
            new_wid = add_hslider(designer->ui, designer->new_label[designer->active_widget_num],
                                                                        wid->x, wid->y, 120, 30);
            copy_widget_settings(designer, wid, new_wid);
            new_wid->adj->type = new_wid->adj->type == CL_LOGARITHMIC ? CL_LOGARITHMIC :
                new_wid->adj->type == CL_LOGSCALE ? CL_LOGSCALE : CL_CONTINUOS;
            add_to_list(designer, new_wid, "add_lv2_hslider", true, IS_HSLIDER);
            destroy_widget(wid, designer->w->app);
            designer->controls[new_wid->data].image = NULL;
            widget_show(new_wid);
            designer->active_widget = new_wid;
            designer->active_widget_num = new_wid->data;
            if (designer->global_hslider_image_file != NULL && adj_get_value(designer->global_hslider_image->adj)) 
                load_single_controller_image(designer, designer->global_hslider_image_file);
        break;
        case 2:
            asprintf (&designer->new_label[designer->active_widget_num], "%s",wid->label);
            new_wid = add_vslider(designer->ui, designer->new_label[designer->active_widget_num],
                                                                        wid->x, wid->y, 30, 120);
            copy_widget_settings(designer, wid, new_wid);
            new_wid->adj->type = new_wid->adj->type == CL_LOGARITHMIC ? CL_LOGARITHMIC :
                new_wid->adj->type == CL_LOGSCALE ? CL_LOGSCALE : CL_CONTINUOS;
            add_to_list(designer, new_wid, "add_lv2_vslider", true, IS_VSLIDER);
            destroy_widget(wid, designer->w->app);
            designer->controls[new_wid->data].image = NULL;
            widget_show(new_wid);
            designer->active_widget = new_wid;
            designer->active_widget_num = new_wid->data;
            if (designer->global_vslider_image_file != NULL && adj_get_value(designer->global_vslider_image->adj)) 
                load_single_controller_image(designer, designer->global_vslider_image_file);
        break;
        case 3:
            asprintf (&designer->new_label[designer->active_widget_num], "%s",wid->label);
            new_wid = add_button(designer->ui, designer->new_label[designer->active_widget_num],
                                                                        wid->x, wid->y, 60, 60);
            set_controller_callbacks(designer, new_wid, true);
            new_wid->data = wid->data;
            designer->wid_counter--;
            add_to_list(designer, new_wid, "add_lv2_button", false, IS_BUTTON);
            destroy_widget(wid, designer->w->app);
            designer->controls[new_wid->data].image = NULL;
            widget_show(new_wid);
            designer->active_widget = new_wid;
            designer->active_widget_num = new_wid->data;
            if (designer->global_button_image_file != NULL && adj_get_value(designer->global_button_image->adj))
                load_single_controller_image(designer, designer->global_button_image_file);
        break;
        case 4:
            asprintf (&designer->new_label[designer->active_widget_num], "%s",wid->label);
            new_wid = add_toggle_button(designer->ui, designer->new_label[designer->active_widget_num],
                                                                        wid->x, wid->y, 60, 60);
            set_controller_callbacks(designer, new_wid, true);
            new_wid->data = wid->data;
            designer->wid_counter--;
            add_to_list(designer, new_wid, "add_lv2_toggle_button", false, IS_TOGGLE_BUTTON);
            destroy_widget(wid, designer->w->app);
            designer->controls[new_wid->data].image = NULL;
            widget_show(new_wid);
            designer->active_widget = new_wid;
            designer->active_widget_num = new_wid->data;
            if (designer->global_switch_image_file != NULL && adj_get_value(designer->global_switch_image->adj))
                load_single_controller_image(designer, designer->global_switch_image_file);
        break;
        case 5:
            if (designer->controls[designer->active_widget_num].is_type == -1 ) {
                designer->controls[designer->active_widget_num].is_type = IS_COMBOBOX;
            }
            asprintf (&designer->new_label[designer->active_widget_num], "%s",wid->label);
            new_wid = add_combobox(designer->ui, designer->new_label[designer->active_widget_num],
                                                                        wid->x, wid->y, 120, 30);
            set_controller_callbacks(designer, new_wid, true);
            new_wid->data = wid->data;
            designer->wid_counter--;
            add_to_list(designer, new_wid, "add_lv2_combobox", true, IS_COMBOBOX);
            destroy_widget(wid, designer->w->app);
            designer->controls[new_wid->data].image = NULL;
            widget_show(new_wid);
            designer->active_widget = new_wid;
        break;
        case 6:
            asprintf (&designer->new_label[designer->active_widget_num], "%s",wid->label);
            new_wid = add_valuedisplay(designer->ui, designer->new_label[designer->active_widget_num],
                                                                        wid->x, wid->y, 30, 120);
            copy_widget_settings(designer, wid, new_wid);
            new_wid->adj->type = new_wid->adj->type == CL_LOGARITHMIC ? CL_LOGARITHMIC :
                new_wid->adj->type == CL_LOGSCALE ? CL_LOGSCALE : CL_CONTINUOS;
            add_to_list(designer, new_wid, "add_lv2_valuedisplay", true, IS_VALUE_DISPLAY);
            destroy_widget(wid, designer->w->app);
            designer->controls[new_wid->data].image = NULL;
            widget_show(new_wid);
            designer->active_widget = new_wid;
        break;
        case 7:
            asprintf (&designer->new_label[designer->active_widget_num], "%s",wid->label);
            new_wid = add_label(designer->ui, designer->new_label[designer->active_widget_num],
                                                                        wid->x, wid->y, 120, 30);
            set_controller_callbacks(designer, new_wid, true);
            new_wid->data = wid->data;
            designer->wid_counter--;
            add_to_list(designer, new_wid, "add_lv2_label", false, IS_LABEL);
            destroy_widget(wid, designer->w->app);
            designer->controls[new_wid->data].image = NULL;
            widget_show(new_wid);
            designer->active_widget = new_wid;
        break;
        case 8:
            asprintf (&designer->new_label[designer->active_widget_num], "%s",wid->label);
            new_wid = add_vmeter(designer->ui, designer->new_label[designer->active_widget_num],
                                                                false, wid->x, wid->y, 30, 120);
            copy_widget_settings(designer, wid, new_wid);
            new_wid->adj->type = new_wid->adj->type == CL_LOGARITHMIC ? CL_LOGARITHMIC :
                new_wid->adj->type == CL_LOGSCALE ? CL_LOGSCALE : CL_METER;
            add_to_list(designer, new_wid, "add_lv2_vmeter", true, IS_VMETER);
            destroy_widget(wid, designer->w->app);
            designer->controls[new_wid->data].image = NULL;
            widget_show(new_wid);
            designer->active_widget = new_wid;
        break;
        case 9:
            asprintf (&designer->new_label[designer->active_widget_num], "%s",wid->label);
            new_wid = add_hmeter(designer->ui, designer->new_label[designer->active_widget_num],
                                                                false, wid->x, wid->y, 30, 120);
            copy_widget_settings(designer, wid, new_wid);
            new_wid->adj->type = new_wid->adj->type == CL_LOGARITHMIC ? CL_LOGARITHMIC :
                new_wid->adj->type == CL_LOGSCALE ? CL_LOGSCALE : CL_METER;
            add_to_list(designer, new_wid, "add_lv2_hmeter", true, IS_HMETER);
            destroy_widget(wid, designer->w->app);
            designer->controls[new_wid->data].image = NULL;
            widget_show(new_wid);
            designer->active_widget = new_wid;
        break;
        default:
        break;
    }
}

Widget_t *add_controller(XUiDesigner *designer, XButtonEvent *xbutton, Widget_t *wid) {
    Widget_t *w = designer->ui;
    switch(designer->select_widget_num) {
        case 1:
            asprintf(&designer->controls[designer->wid_counter].name, "Knob%i", designer->wid_counter);
            wid = add_knob(w, designer->controls[designer->wid_counter].name, xbutton->x-30, xbutton->y-40, 60, 80);
            set_controller_callbacks(designer, wid, true);
            add_to_list(designer, wid, "add_lv2_knob", true, IS_KNOB);
            if (designer->global_knob_image_file != NULL && adj_get_value(designer->global_knob_image->adj)) 
                load_single_controller_image(designer, designer->global_knob_image_file);
        break;
        case 2:
            asprintf(&designer->controls[designer->wid_counter].name, "HSlider%i", designer->wid_counter);
            wid = add_hslider(w, designer->controls[designer->wid_counter].name, xbutton->x-60, xbutton->y-15, 120, 30);
            set_controller_callbacks(designer, wid, true);
            add_to_list(designer, wid, "add_lv2_hslider", true, IS_HSLIDER);
            if (designer->global_hslider_image_file != NULL && adj_get_value(designer->global_hslider_image->adj)) 
                load_single_controller_image(designer, designer->global_hslider_image_file);
        break;
        case 3:
            asprintf(&designer->controls[designer->wid_counter].name, "VSlider%i", designer->wid_counter);
            wid = add_vslider(w, designer->controls[designer->wid_counter].name, xbutton->x-15, xbutton->y-60, 30, 120);
            set_controller_callbacks(designer, wid, true);
            add_to_list(designer, wid, "add_lv2_vslider", true, IS_VSLIDER);
            if (designer->global_vslider_image_file != NULL && adj_get_value(designer->global_vslider_image->adj)) 
                load_single_controller_image(designer, designer->global_vslider_image_file);
        break;
        case 4:
            asprintf(&designer->controls[designer->wid_counter].name, "Button%i", designer->wid_counter);
            wid = add_button(w, designer->controls[designer->wid_counter].name, xbutton->x-30, xbutton->y-30, 60, 60);
            set_controller_callbacks(designer, wid, true);
            add_to_list(designer, wid, "add_lv2_button", false, IS_BUTTON);
            if (designer->global_button_image_file != NULL && adj_get_value(designer->global_button_image->adj))
                load_single_controller_image(designer, designer->global_button_image_file);
        break;
        case 5:
            asprintf(&designer->controls[designer->wid_counter].name, "Switch%i", designer->wid_counter);
            wid = add_toggle_button(w, designer->controls[designer->wid_counter].name, xbutton->x-30, xbutton->y-30, 60, 60);
            set_controller_callbacks(designer, wid, true);
            add_to_list(designer, wid, "add_lv2_toggle_button", false, IS_TOGGLE_BUTTON);
            if (designer->global_switch_image_file != NULL && adj_get_value(designer->global_switch_image->adj))
                load_single_controller_image(designer, designer->global_switch_image_file);
        break;
        case 6:
            asprintf(&designer->controls[designer->wid_counter].name, "Combobox%i", designer->wid_counter);
            wid = add_combobox(w, designer->controls[designer->wid_counter].name, xbutton->x-60, xbutton->y-15, 120, 30);
            set_controller_callbacks(designer, wid, true);
            add_to_list(designer, wid, "add_lv2_combobox", true, IS_COMBOBOX);
        break;
        case 7:
            asprintf(&designer->controls[designer->wid_counter].name, "ValueDisply%i", designer->wid_counter);
            wid = add_valuedisplay(w, designer->controls[designer->wid_counter].name, xbutton->x-20, xbutton->y-15, 40, 30);
            set_controller_callbacks(designer, wid, true);
            add_to_list(designer, wid, "add_lv2_valuedisplay", true, IS_VALUE_DISPLAY);
        break;
        case 8:
            asprintf(&designer->controls[designer->wid_counter].name, "Label%i", designer->wid_counter);
            wid = add_label(w, designer->controls[designer->wid_counter].name, xbutton->x-30, xbutton->y-15, 60, 30);
            set_controller_callbacks(designer, wid, true);
            add_to_list(designer, wid, "add_lv2_label", false, IS_LABEL);
        break;
        case 9:
            asprintf(&designer->controls[designer->wid_counter].name, "VMeter%i", designer->wid_counter);
            wid = add_vmeter(w, designer->controls[designer->wid_counter].name, false, xbutton->x-5, xbutton->y-60, 10, 120);
            set_controller_callbacks(designer, wid, true);
            add_to_list(designer, wid, "add_lv2_vmeter", true, IS_VMETER);
        break;
        case 10:
            asprintf(&designer->controls[designer->wid_counter].name, "HMeter%i", designer->wid_counter);
            wid = add_hmeter(w, designer->controls[designer->wid_counter].name, false, xbutton->x-60, xbutton->y-5, 120, 10);
            set_controller_callbacks(designer, wid, true);
            add_to_list(designer, wid, "add_lv2_hmeter", true, IS_HMETER);
        break;
        case 11:
            asprintf(&designer->controls[designer->wid_counter].name, "WaveView%i", designer->wid_counter);
            wid = add_waveview(w, designer->controls[designer->wid_counter].name, xbutton->x-60, xbutton->y-60, 120, 120);
            set_controller_callbacks(designer, wid, true);
            add_to_list(designer, wid, "add_lv2_waveview", false, IS_WAVEVIEW);
            float v[9] = { 0.0,-0.5, 0.0, 0.5, 0.0, -0.5, 0.0, 0.5, 0.0};
            update_waveview(wid, &v[0],9);
        break;
        case 12:
            asprintf(&designer->controls[designer->wid_counter].name, "Frame%i", designer->wid_counter);
            wid = add_frame(w, designer->controls[designer->wid_counter].name, xbutton->x-60, xbutton->y-60, 120, 120);
            set_controller_callbacks(designer, wid, true);
            adj_set_value(designer->index->adj, adj_get_value(designer->index->adj)-1.0);
            designer->controls[designer->wid_counter-1].port_index = -1;
            add_to_list(designer, wid, "add_lv2_frame", false, IS_FRAME);
            wid->parent_struct = designer;
            free(designer->controls[wid->data].image);
            designer->controls[wid->data].image = NULL;
            wid->func.expose_callback = draw_frame;
            wid->func.enter_callback = null_callback;
            wid->func.leave_callback = null_callback;
            XLowerWindow(w->app->dpy, wid->widget);
        break;
        case 13:
            asprintf(&designer->controls[designer->wid_counter].name, "Tab%i", designer->wid_counter);
            wid = add_tabbox(w, designer->controls[designer->wid_counter].name, xbutton->x-60, xbutton->y-60, 120, 120);
            set_controller_callbacks(designer, wid, true);
            adj_set_value(designer->index->adj, adj_get_value(designer->index->adj)-1.0);
            designer->controls[designer->wid_counter-1].port_index = -1;
            add_to_list(designer, wid, "add_lv2_tabbox", false, IS_TABBOX);
            wid->parent_struct = designer;
            free(designer->controls[wid->data].image);
            designer->controls[wid->data].image = NULL;
            wid->func.expose_callback = draw_tabbox;
            wid->func.enter_callback = null_callback;
            wid->func.leave_callback = null_callback;
            XLowerWindow(w->app->dpy, wid->widget);
            Widget_t *tab = tabbox_add_tab(wid, "Tab 1");
            tab->parent_struct = designer;
            tab->func.expose_callback = draw_tab;
            tab->func.button_press_callback = set_pos_tab;
            tab->func.button_release_callback = fix_pos_tab;
            tab->func.motion_callback = move_tab;
        break;
        case 14:
            asprintf(&designer->controls[designer->wid_counter].name, "Image%i", designer->wid_counter);
            wid = add_image(w, designer->controls[designer->wid_counter].name, xbutton->x-60, xbutton->y-60, 120, 120);
            wid->label = designer->controls[designer->wid_counter].name;
            set_controller_callbacks(designer, wid, true);
            adj_set_value(designer->index->adj, adj_get_value(designer->index->adj)-1.0);
            designer->controls[designer->wid_counter-1].port_index = -1;
            add_to_list(designer, wid, "add_lv2_image", false, IS_IMAGE);
            wid->parent_struct = designer;
            free(designer->controls[wid->data].image);
            designer->controls[wid->data].image = NULL;
            wid->func.expose_callback = draw_image;
            wid->func.enter_callback = null_callback;
            wid->func.leave_callback = null_callback;
            XLowerWindow(w->app->dpy, wid->widget);
        break;
        case 15:
            if (designer->is_json_file) {
                Widget_t *dia = open_message_dialog(w, ERROR_BOX, "MIDI Keyboard",
                                                _("Couldn't add MIDI Keyboard when work with a json file, sorry"),NULL);
                XSetTransientForHint(w->app->dpy, dia->widget, designer->ui->widget);
                return NULL;
            }
            asprintf(&designer->controls[designer->wid_counter].name, "Midikeyboard%i", designer->wid_counter);
            wid = add_midi_keyboard(w, designer->controls[designer->wid_counter].name, xbutton->x-73, xbutton->y-30, 147, 60);
            wid->label = designer->controls[designer->wid_counter].name;
            set_controller_callbacks(designer, wid, true);
            designer->controls[wid->data].is_midi_patch = true;
            adj_set_value(designer->index->adj, adj_get_value(designer->index->adj)-1.0);
            designer->controls[wid->data].port_index = -1;
            add_to_list(designer, wid, "add_lv2_midikeyboard", false, IS_MIDIKEYBOARD);
            wid->parent_struct = designer;
            free(designer->controls[wid->data].image);
            designer->controls[wid->data].image = NULL;
            wid->func.enter_callback = null_callback;
            wid->func.leave_callback = null_callback;
        break;
        default:
            return NULL;
        break;
    }
    return wid;
}
