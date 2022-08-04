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

#include "XUiReparent.h"
#include "XUiControllerType.h"
#include "XUiImageLoader.h"
#include "XUiGenerator.h"


/*---------------------------------------------------------------------
-----------------------------------------------------------------------    
                reparent widgets
-----------------------------------------------------------------------
----------------------------------------------------------------------*/

static void reparent_widget(XUiDesigner *designer, Widget_t* parent, Widget_t *wid, int j, int v,
                                                int x, int y, int width, int height) {
    designer->prev_active_widget = NULL;
    Widget_t *new_wid = NULL;
    WidgetType tp = designer->controls[wid->data].is_type;
        switch (tp) {
        case IS_KNOB:
            asprintf (&designer->new_label[wid->data], "%s",wid->label);
            new_wid = add_knob(parent, designer->new_label[wid->data],
                                                                        x, y, width, height);
            copy_widget_settings(designer, wid, new_wid);
            new_wid->adj->type = new_wid->adj->type == CL_LOGARITHMIC ? CL_LOGARITHMIC :
                new_wid->adj->type == CL_LOGSCALE ? CL_LOGSCALE : CL_CONTINUOS;
            add_to_list(designer, new_wid, "add_lv2_knob", true, IS_KNOB);
            destroy_widget(wid, designer->w->app);
            widget_show(new_wid);
            designer->active_widget = new_wid;
            if (designer->controls[new_wid->data].image != NULL) {
                load_single_controller_image(designer, designer->controls[new_wid->data].image);
            }
            designer->active_widget_num = new_wid->data;
            designer->controls[new_wid->data].in_frame = j;
            designer->controls[new_wid->data].in_tab = v;
        break;
        case IS_HSLIDER:
            asprintf (&designer->new_label[wid->data], "%s",wid->label);
            new_wid = add_hslider(parent, designer->new_label[wid->data],
                                                                        x, y, width, height);
            copy_widget_settings(designer, wid, new_wid);
            new_wid->adj->type = new_wid->adj->type == CL_LOGARITHMIC ? CL_LOGARITHMIC :
                new_wid->adj->type == CL_LOGSCALE ? CL_LOGSCALE : CL_CONTINUOS;
            add_to_list(designer, new_wid, "add_lv2_hslider", true, IS_HSLIDER);
            destroy_widget(wid, designer->w->app);
            designer->controls[new_wid->data].image = NULL;
            widget_show(new_wid);
            designer->active_widget = new_wid;
            designer->active_widget_num = new_wid->data;
            designer->controls[new_wid->data].in_frame = j;
            designer->controls[new_wid->data].in_tab = v;
        break;
        case IS_VSLIDER:
            asprintf (&designer->new_label[wid->data], "%s",wid->label);
            new_wid = add_vslider(parent, designer->new_label[wid->data],
                                                                        x, y, width, height);
            copy_widget_settings(designer, wid, new_wid);
            new_wid->adj->type = new_wid->adj->type == CL_LOGARITHMIC ? CL_LOGARITHMIC :
                new_wid->adj->type == CL_LOGSCALE ? CL_LOGSCALE : CL_CONTINUOS;
            add_to_list(designer, new_wid, "add_lv2_vslider", true, IS_VSLIDER);
            destroy_widget(wid, designer->w->app);
            designer->controls[new_wid->data].image = NULL;
            widget_show(new_wid);
            designer->active_widget = new_wid;
            designer->active_widget_num = new_wid->data;
            designer->controls[new_wid->data].in_frame = j;
            designer->controls[new_wid->data].in_tab = v;
        break;
        case IS_BUTTON:
            asprintf (&designer->new_label[wid->data], "%s",wid->label);
            new_wid = add_button(parent, designer->new_label[wid->data],
                                                                        x, y, width, height);
            copy_widget_settings(designer, wid, new_wid);
            add_to_list(designer, new_wid, "add_lv2_button", false, IS_BUTTON);
            destroy_widget(wid, designer->w->app);
            widget_show(new_wid);
            designer->active_widget = new_wid;
            if (designer->controls[new_wid->data].image != NULL) {
                load_single_controller_image(designer, designer->controls[new_wid->data].image);
            }
            designer->active_widget_num = new_wid->data;
            designer->controls[new_wid->data].in_frame = j;
            designer->controls[new_wid->data].in_tab = v;
        break;
        case IS_TOGGLE_BUTTON:
            asprintf (&designer->new_label[wid->data], "%s",wid->label);
            new_wid = add_toggle_button(parent, designer->new_label[wid->data],
                                                                        x, y, width, height);
            copy_widget_settings(designer, wid, new_wid);
            add_to_list(designer, new_wid, "add_lv2_toggle_button", false, IS_TOGGLE_BUTTON);
            destroy_widget(wid, designer->w->app);
            widget_show(new_wid);
            designer->active_widget = new_wid;
            if (designer->controls[new_wid->data].image != NULL) {
                load_single_controller_image(designer, designer->controls[new_wid->data].image);
            }
            designer->active_widget_num = new_wid->data;
            designer->controls[new_wid->data].in_frame = j;
            designer->controls[new_wid->data].in_tab = v;
        break;
        case IS_COMBOBOX:
            asprintf (&designer->new_label[wid->data], "%s",wid->label);
            new_wid = add_combobox(parent, designer->new_label[wid->data],
                                                                        x, y, width, height);
            copy_widget_settings(designer, wid, new_wid);
            add_to_list(designer, new_wid, "add_lv2_combobox", true, IS_COMBOBOX);
            destroy_widget(wid, designer->w->app);
            designer->controls[new_wid->data].image = NULL;
            widget_show(new_wid);
            designer->active_widget = new_wid;
            designer->active_widget_num = new_wid->data;
            designer->controls[new_wid->data].in_frame = j;
            designer->controls[new_wid->data].in_tab = v;
        break;
        case IS_VALUE_DISPLAY:
            asprintf (&designer->new_label[wid->data], "%s",wid->label);
            new_wid = add_valuedisplay(parent, designer->new_label[wid->data],
                                                                        x, y, width, height);
            copy_widget_settings(designer, wid, new_wid);
            new_wid->adj->type = new_wid->adj->type == CL_LOGARITHMIC ? CL_LOGARITHMIC :
                new_wid->adj->type == CL_LOGSCALE ? CL_LOGSCALE : CL_CONTINUOS;
            add_to_list(designer, new_wid, "add_lv2_valuedisplay", true, IS_VALUE_DISPLAY);
            destroy_widget(wid, designer->w->app);
            designer->controls[new_wid->data].image = NULL;
            widget_show(new_wid);
            designer->active_widget = new_wid;
            designer->active_widget_num = new_wid->data;
            designer->controls[new_wid->data].in_frame = j;
            designer->controls[new_wid->data].in_tab = v;
        break;
        case IS_LABEL:
            asprintf (&designer->new_label[wid->data], "%s",wid->label);
            new_wid = add_label(parent, designer->new_label[wid->data],
                                                                        x, y, width, height);
            set_controller_callbacks(designer, new_wid, true);
            new_wid->data = wid->data;
            designer->wid_counter--;
            add_to_list(designer, new_wid, "add_lv2_label", false, IS_LABEL);
            destroy_widget(wid, designer->w->app);
            designer->controls[new_wid->data].image = NULL;
            widget_show(new_wid);
            designer->active_widget = new_wid;
            designer->active_widget_num = new_wid->data;
            designer->controls[new_wid->data].in_frame = j;
            designer->controls[new_wid->data].in_tab = v;
        break;
        case IS_VMETER:
            asprintf (&designer->new_label[wid->data], "%s",wid->label);
            new_wid = add_vmeter(parent, designer->new_label[wid->data],
                                                                false, x, y, width, height);
            copy_widget_settings(designer, wid, new_wid);
            new_wid->adj->type = new_wid->adj->type == CL_LOGARITHMIC ? CL_LOGARITHMIC :
                new_wid->adj->type == CL_LOGSCALE ? CL_LOGSCALE : CL_METER;
            add_to_list(designer, new_wid, "add_lv2_vmeter", true, IS_VMETER);
            destroy_widget(wid, designer->w->app);
            designer->controls[new_wid->data].image = NULL;
            widget_show(new_wid);
            designer->active_widget = new_wid;
            designer->active_widget_num = new_wid->data;
            designer->controls[new_wid->data].in_frame = j;
            designer->controls[new_wid->data].in_tab = v;
        break;
        case IS_HMETER:
            asprintf (&designer->new_label[wid->data], "%s",wid->label);
            new_wid = add_hmeter(parent, designer->new_label[wid->data],
                                                                false, x, y, width, height);
            copy_widget_settings(designer, wid, new_wid);
            new_wid->adj->type = new_wid->adj->type == CL_LOGARITHMIC ? CL_LOGARITHMIC :
                new_wid->adj->type == CL_LOGSCALE ? CL_LOGSCALE : CL_METER;
            add_to_list(designer, new_wid, "add_lv2_hmeter", true, IS_HMETER);
            destroy_widget(wid, designer->w->app);
            designer->controls[new_wid->data].image = NULL;
            widget_show(new_wid);
            designer->active_widget = new_wid;
            designer->active_widget_num = new_wid->data;
            designer->controls[new_wid->data].in_frame = j;
            designer->controls[new_wid->data].in_tab = v;
        break;
        case IS_IMAGE_TOGGLE:
            asprintf (&designer->new_label[wid->data], "%s",wid->label);
            new_wid = add_switch_image_button(parent, designer->new_label[wid->data],
                                                                        x, y, width, height);
            copy_widget_settings(designer, wid, new_wid);
            add_to_list(designer, new_wid, "add_lv2_image_toggle", false, IS_IMAGE_TOGGLE);
            destroy_widget(wid, designer->w->app);
            widget_show(new_wid);
            designer->active_widget = new_wid;
            if (designer->controls[new_wid->data].image != NULL) {
                load_single_controller_image(designer, designer->controls[new_wid->data].image);
            }
            designer->active_widget_num = new_wid->data;
            designer->controls[new_wid->data].in_frame = j;
            designer->controls[new_wid->data].in_tab = v;
        default:
        break;
    }

}

void check_reparent(XUiDesigner *designer, XButtonEvent *xbutton, Widget_t *w) {
    Widget_t *p = (Widget_t*)w->parent;
    Widget_t *pp = (Widget_t*)p->parent;
    XWindowAttributes attrs;
    XGetWindowAttributes(w->app->dpy, (Window)w->widget, &attrs);
    int x = attrs.x;
    int y = attrs.y;
    int width = attrs.width;
    int height = attrs.height;
    int i = 0;
    int j = 0;
    Widget_t *frame = NULL;
    for (;i<MAX_CONTROLS;i++) {
        if (designer->controls[i].wid != NULL && (designer->controls[i].is_type == IS_FRAME ||
            designer->controls[i].is_type == IS_TABBOX || designer->controls[i].is_type == IS_IMAGE )) {
            j++;
            frame = designer->controls[i].wid;
            XGetWindowAttributes(w->app->dpy, (Window)frame->widget, &attrs);
            int fx = attrs.x;
            int fy = attrs.y;
            int fwidth = attrs.width;
            int fheight = attrs.height;
            int v = 0;
            if (x>fx && y>fy && x+width<fx+fwidth && y+height<fy+fheight && p != frame && pp != frame) {
                if (designer->controls[i].is_type == IS_TABBOX) {
                    v = (int)adj_get_value(designer->controls[i].wid->adj);
                    frame = designer->controls[i].wid->childlist->childs[v];
                    if (frame == NULL) break;
                    v +=1;
                }
                if (p == frame || w == frame) break;
                int x1, y1;
                Window child;
                XTranslateCoordinates( w->app->dpy, designer->ui->widget, frame->widget, x, y, &x1, &y1, &child );
                reparent_widget(designer, frame, w, j, v, x1, y1, width, height);
                break;
            } else if ((p == frame || pp == frame) && (x<0 || y<0 || x>fwidth || y>fheight)) {
                int x1, y1;
                Window child;
                XTranslateCoordinates( w->app->dpy, frame->widget, designer->ui->widget, x, y, &x1, &y1, &child );
                reparent_widget(designer, designer->ui, w, 0, 0, x1, y1, width, height);
                break;
            }
        }
    }
    
}
