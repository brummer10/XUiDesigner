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


/*---------------------------------------------------------------------
-----------------------------------------------------------------------    
                change controller type
-----------------------------------------------------------------------
----------------------------------------------------------------------*/

void copy_widget_settings(XUiDesigner *designer, Widget_t *wid, Widget_t *new_wid) {
    if (wid->adj->type == CL_LOGARITHMIC) {
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
    set_controller_callbacks(designer, new_wid, true);
    new_wid->data = wid->data;
    designer->wid_counter--;
    adj_set_value(designer->index->adj, adj_get_value(designer->index->adj)-1.0);
}

void switch_controller_type(void *w_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    XUiDesigner *designer = (XUiDesigner*)w->parent_struct;
    Widget_t *wid = designer->active_widget;
    designer->prev_active_widget = NULL;
    Widget_t *new_wid = NULL;
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
        break;
        case 5:
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
