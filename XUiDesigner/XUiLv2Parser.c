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

#include "XUiLv2Parser.h"
#include "XUiGenerator.h"
#include "XUiTextInput.h"
#include "XUiWriteTurtle.h"
#include "XUiWriteJson.h"
#include "XUiSettings.h"


/*---------------------------------------------------------------------
-----------------------------------------------------------------------    
                lv2 ttl handling
-----------------------------------------------------------------------
----------------------------------------------------------------------*/

static void reset_plugin_ui(XUiDesigner *designer) {
    widget_set_title(designer->ui, "NoName");
    designer->ui->width = 600;
    designer->ui->height = 400;
    widget_hide(designer->ui);
    XFlush(designer->w->app->dpy);
    int ch = childlist_has_child(designer->ui->childlist);
    if (ch) {
        for(;ch>0;ch--) {
            remove_from_list(designer, designer->ui->childlist->childs[ch-1]);
            destroy_widget(designer->ui->childlist->childs[ch-1],designer->ui->app);
        }
    }
    int i = 0;
    for (;i<MAX_CONTROLS; i++) {
        free(designer->new_label[i]);
    }
    free(designer->new_label);
    designer->new_label = NULL;
    designer->new_label = (char **)malloc(MAX_CONTROLS * sizeof(char *));
    i = 0;
    for (;i<MAX_CONTROLS; i++) {
        designer->new_label[i] = NULL;
    }

    designer->modify_mod = XUI_NONE;
    designer->active_widget = NULL;
    designer->is_project = false;
    designer->lv2c.audio_input = 0;
    designer->lv2c.audio_output = 0;
    adj_set_value(designer->project_audio_input->adj, (float)designer->lv2c.audio_input);
    adj_set_value(designer->project_audio_output->adj, (float)designer->lv2c.audio_output);
    free(designer->lv2c.uri);
    designer->lv2c.uri = NULL;
    asprintf(&designer->lv2c.uri, "%s", "urn:test_ui");
    free(designer->lv2c.plugintype);
    designer->lv2c.plugintype = NULL;
    asprintf(&designer->lv2c.plugintype, "%s", "MixerPlugin");
    adj_set_value(designer->project_type->adj, designer->project_type->adj->max_value);
    free(designer->lv2c.author);
    designer->lv2c.author = NULL;
    asprintf(&designer->lv2c.author, "%s", getUserName());
    free(designer->lv2c.ui_uri);
    designer->lv2c.ui_uri = NULL;
    asprintf(&designer->lv2c.ui_uri, "%s", "urn:test_ui");    

    box_entry_set_text(designer->controller_label, "");
    adj_set_value(designer->x_axis->adj, 0.0);
    adj_set_value(designer->y_axis->adj, 0.0);
    adj_set_value(designer->w_axis->adj, 10.0);
    adj_set_value(designer->h_axis->adj, 10.0);
    widget_hide(designer->combobox_settings);
    widget_hide(designer->controller_settings);
    XResizeWindow(designer->ui->app->dpy, designer->ui->widget, designer->ui->width, designer->ui->height);

    adj_set_value(designer->index->adj,0.0);
    designer->wid_counter = 0;
    designer->active_widget_num = 0;
    designer->prev_active_widget = NULL;
}

static int sort_enums(int elem, int array[], int size) {
    int i = 0;
    for(;i<size;i++) {
        if(array[i] == elem) {
            return i; 
        }
    }
    return -1; 
}

Widget_t* create_controller(XUiDesigner *designer, const LilvPlugin* plugin, const LilvPort* port,
                                                                int *xa, int *ya, int *x1a, int *y1a) {
    int x = (*xa);
    int y = (*ya);
    int x1 = (*x1a);
    int y1 = (*y1a);
    Widget_t * wid = NULL;
    if (designer->lv2c.is_input_port) {
        if (designer->lv2c.is_audio_port) {
            
            wid = create_widget(designer->ui->app, designer->ui,  0, 0, 1, 1);
            wid->label = designer->new_label[designer->active_widget_num];
            set_controller_callbacks(designer, wid, false);
            add_to_list(designer, wid, "add_audio_port", false, IS_BUTTON);
            designer->controls[designer->active_widget_num].port_index = designer->lv2c.Port_Index;
            designer->controls[designer->active_widget_num].is_audio_input = true;
            if (designer->lv2c.symbol != NULL) {
                free(designer->controls[designer->active_widget_num].symbol);
                designer->controls[designer->active_widget_num].symbol = NULL;
                asprintf (&designer->controls[designer->active_widget_num].symbol, "%s",designer->lv2c.symbol);
            }
        } else if (designer->lv2c.is_atom_port) {
            
            wid = create_widget(designer->ui->app, designer->ui,  0, 0, 1, 1);
            wid->label = designer->new_label[designer->active_widget_num];
            set_controller_callbacks(designer, wid, false);
            add_to_list(designer, wid, "add_atom_port", false, IS_BUTTON);
            designer->controls[designer->active_widget_num].port_index = designer->lv2c.Port_Index;
            designer->controls[designer->active_widget_num].is_atom_input = true;
            if (designer->lv2c.symbol != NULL) {
                free(designer->controls[designer->active_widget_num].symbol);
                designer->controls[designer->active_widget_num].symbol = NULL;
                asprintf (&designer->controls[designer->active_widget_num].symbol, "%s",designer->lv2c.symbol);
            }
        } else if (designer->lv2c.is_toggle_port) {
            if (x+70 >= 1200) {
                y += 130;
                y1 += 130;
                x = 40;
            } else {
                x1 += 80;
            }
            wid = add_toggle_button(designer->ui, designer->new_label[designer->active_widget_num], x, y, 60, 60);
            set_controller_callbacks(designer, wid, false);
            tooltip_set_text(wid, wid->label);
            add_to_list(designer, wid, "add_lv2_toggle_button", false, IS_TOGGLE_BUTTON);
            if (designer->lv2c.bypass) {
                designer->controls[designer->active_widget_num].destignation_enabled = true;
            }
            designer->controls[designer->active_widget_num].port_index = designer->lv2c.Port_Index;
            designer->controls[designer->active_widget_num].is_atom_patch = designer->lv2c.is_atom_patch? true : false;
            if (designer->lv2c.symbol != NULL) {
                free(designer->controls[designer->active_widget_num].symbol);
                designer->controls[designer->active_widget_num].symbol = NULL;
                asprintf (&designer->controls[designer->active_widget_num].symbol, "%s",designer->lv2c.symbol);
            }
            x += 80;
        } else if (designer->lv2c.is_trigger_port) {
            if (x+70 >= 1200) {
                y += 130;
                y1 += 130;
                x = 40;
            } else {
                x1 += 80;
            }
            wid = add_button(designer->ui, designer->new_label[designer->active_widget_num], x, y, 60, 60);
            set_controller_callbacks(designer, wid, false);
            tooltip_set_text(wid, wid->label);
            add_to_list(designer, wid, "add_lv2_button", false, IS_BUTTON);
            designer->controls[designer->active_widget_num].port_index = designer->lv2c.Port_Index;
            designer->controls[designer->active_widget_num].is_atom_patch = designer->lv2c.is_atom_patch? true : false;
            if (designer->lv2c.symbol != NULL) {
                free(designer->controls[designer->active_widget_num].symbol);
                designer->controls[designer->active_widget_num].symbol = NULL;
                asprintf (&designer->controls[designer->active_widget_num].symbol, "%s",designer->lv2c.symbol);
            }
            x += 80;
        } else if (designer->lv2c.is_enum_port) {
            if (x+130 >= 1200) {
                y += 130;
                y1 += 130;
                x = 40;
            } else {
                x1 += 140;
            }
            wid = add_combobox(designer->ui, designer->new_label[designer->active_widget_num], x, y, 120, 30);
            set_controller_callbacks(designer, wid, false);
            tooltip_set_text(wid, wid->label);

            LilvScalePoints* sp = lilv_port_get_scale_points(plugin, port);
            int num_sp = lilv_scale_points_size(sp);
            int sp_count = 0;
            int sppos[num_sp];
            char splabes[num_sp][32];
            if (num_sp > 0) {
                for (LilvIter* it = lilv_scale_points_begin(sp);
                        !lilv_scale_points_is_end(sp, it);
                        it = lilv_scale_points_next(sp, it)) {
                    const LilvScalePoint* p = lilv_scale_points_get(sp, it);
                    utf8ncpy(&splabes[sp_count][0], lilv_node_as_string(lilv_scale_point_get_label(p)), 31);
                    sppos[sp_count] = lilv_node_as_float(lilv_scale_point_get_value(p));
                    sp_count++;
                }
                int i = designer->lv2c.min;
                char s[32];
                for (;i<designer->lv2c.max+1;i++) {
                    int j = sort_enums(i,sppos,num_sp);
                    if (j>-1) {
                        combobox_add_entry(wid,&splabes[j][0]);
                    } else {
                        snprintf(s, 31,"%d",  i);
                        combobox_add_entry(wid,s);
                    }
                }
                lilv_scale_points_free(sp);
            }

            set_adjustment(wid->adj, designer->lv2c.def, designer->lv2c.def, designer->lv2c.min,
                                    designer->lv2c.max, designer->lv2c.is_int_port? 1.0:0.01, CL_ENUM);
            add_to_list(designer, wid, "add_lv2_combobox", true, IS_COMBOBOX);
            designer->controls[designer->active_widget_num].port_index = designer->lv2c.Port_Index;
            designer->controls[designer->active_widget_num].is_atom_patch = designer->lv2c.is_atom_patch? true : false;
            if (designer->lv2c.symbol != NULL) {
                free(designer->controls[designer->active_widget_num].symbol);
                designer->controls[designer->active_widget_num].symbol = NULL;
                asprintf (&designer->controls[designer->active_widget_num].symbol, "%s",designer->lv2c.symbol);
            }
            x += 140;
        } else if (designer->lv2c.is_patch_path) {
            if (x+70 >= 1200) {
                y += 130;
                y1 += 130;
                x = 40;
            } else {
                x1 += 80;
            }
            wid = add_file_button(designer->ui, x, y, 60, 60, "", "");
            wid->label = designer->new_label[designer->active_widget_num];
            set_controller_callbacks(designer, wid, false);
            tooltip_set_text(wid, wid->label);
            add_to_list(designer, wid, "add_lv2_file_button", false, IS_FILE_BUTTON);
            designer->controls[designer->active_widget_num].port_index = designer->lv2c.Port_Index;
            designer->controls[designer->active_widget_num].is_atom_patch = designer->lv2c.is_atom_patch? true : false;
            if (designer->lv2c.symbol != NULL) {
                free(designer->controls[designer->active_widget_num].symbol);
                designer->controls[designer->active_widget_num].symbol = NULL;
                asprintf (&designer->controls[designer->active_widget_num].symbol, "%s",designer->lv2c.symbol);
            }
            x += 80;

        } else {
            if (x+70 >= 1200) {
                y += 130;
                y1 += 130;
                x = 40;
            } else {
                x1 += 80;
            }
            designer->lv2c.step = designer->lv2c.is_log_port? 0.01 : designer->lv2c.min<0? 
            (fabs(designer->lv2c.min)+fabs(designer->lv2c.max))*0.01:fabs(designer->lv2c.max)* 0.01;
            wid = add_knob(designer->ui, designer->new_label[designer->active_widget_num], x, y, 60, 80);
            set_adjustment(wid->adj, designer->lv2c.def, designer->lv2c.def, designer->lv2c.min,
                designer->lv2c.max, designer->lv2c.is_int_port? 1:designer->lv2c.step, designer->lv2c.is_log_port?
                designer->lv2c.min>0 ? CL_LOGARITHMIC : CL_LOGSCALE :CL_CONTINUOS);
            set_controller_callbacks(designer, wid, false);
            tooltip_set_text(wid, wid->label);
            add_to_list(designer, wid, "add_lv2_knob", true, IS_KNOB);
            designer->controls[designer->active_widget_num].port_index = designer->lv2c.Port_Index;
            designer->controls[designer->active_widget_num].is_atom_patch = designer->lv2c.is_atom_patch? true : false;
            if (designer->lv2c.symbol != NULL) {
                free(designer->controls[designer->active_widget_num].symbol);
                designer->controls[designer->active_widget_num].symbol = NULL;
                asprintf (&designer->controls[designer->active_widget_num].symbol, "%s",designer->lv2c.symbol);
            }
            x += 80;
        }
    } else if (designer->lv2c.is_output_port) {
        if (designer->lv2c.is_audio_port) {
            
            wid = create_widget(designer->ui->app, designer->ui,  0, 0, 1, 1);
            wid->label = designer->new_label[designer->active_widget_num];
            set_controller_callbacks(designer, wid, false);
            add_to_list(designer, wid, "add_audio_port", false, IS_BUTTON);
            designer->controls[designer->active_widget_num].port_index = designer->lv2c.Port_Index;
            designer->controls[designer->active_widget_num].is_audio_output = true;
            free(designer->controls[designer->active_widget_num].symbol);
            designer->controls[designer->active_widget_num].symbol = NULL;
            asprintf (&designer->controls[designer->active_widget_num].symbol, "%s",designer->lv2c.symbol);
        } else if (designer->lv2c.is_atom_port) {
            
            wid = create_widget(designer->ui->app, designer->ui,  0, 0, 1, 1);
            wid->label = designer->new_label[designer->active_widget_num];
            set_controller_callbacks(designer, wid, false);
            add_to_list(designer, wid, "add_atom_port", false, IS_BUTTON);
            designer->controls[designer->active_widget_num].port_index = designer->lv2c.Port_Index;
            designer->controls[designer->active_widget_num].is_atom_output = true;
            if (designer->lv2c.symbol != NULL) {
                free(designer->controls[designer->active_widget_num].symbol);
                designer->controls[designer->active_widget_num].symbol = NULL;
                asprintf (&designer->controls[designer->active_widget_num].symbol, "%s",designer->lv2c.symbol);
            }
        } else {
            if (x+20 >= 1200) {
                y += 130;
                y1 += 130;
                x = 40;
            } else {
                x1 += 30;
            }
            wid = add_vmeter(designer->ui, designer->new_label[designer->active_widget_num], false, x, y, 10, 120);
            set_adjustment(wid->adj, designer->lv2c.def, designer->lv2c.def, designer->lv2c.min, designer->lv2c.max,
                designer->lv2c.is_int_port? 1:0.01, designer->lv2c.is_log_port?
                designer->lv2c.min>0 ? CL_LOGARITHMIC : CL_LOGSCALE : CL_METER);
            set_controller_callbacks(designer, wid, false);
            tooltip_set_text(wid, wid->label);
            add_to_list(designer, wid, "add_lv2_vmeter", true, IS_VMETER);
            designer->controls[designer->active_widget_num].port_index = designer->lv2c.Port_Index;
            designer->controls[designer->active_widget_num].is_atom_patch = designer->lv2c.is_atom_patch? true : false;
            if (designer->lv2c.symbol != NULL) {
                free(designer->controls[designer->active_widget_num].symbol);
                designer->controls[designer->active_widget_num].symbol = NULL;
                asprintf (&designer->controls[designer->active_widget_num].symbol, "%s",designer->lv2c.symbol);
            }
            x += 30;
        }
    }
    (*xa) = x;
    (*ya) = y;
    (*x1a) = x1;
    (*y1a) = y1;
    return wid;
}

void load_plugin_ui(void* w_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    XUiDesigner *designer = (XUiDesigner*)w->parent_struct;
    reset_plugin_ui(designer);
    designer->lv2c.is_atom_patch = false;
    designer->lv2c.is_patch_path = false;
    designer->lv2c.is_enum_port = false;
    designer->lv2c.is_toggle_port = false;
    designer->lv2c.have_adjustment = false;    
    designer->lv2c.is_trigger_port = false;
    designer->lv2c.is_log_port = false;    
    int x = 40;
    int y = 40;
    int x1 = 40;
    int y1 = 40;
    int v = (int)adj_get_value(w->adj);
    if (v) {
        LilvNode* lv2_AudioPort = (lilv_new_uri(designer->world, LV2_CORE__AudioPort));
        LilvNode* lv2_ControlPort = (lilv_new_uri(designer->world, LV2_CORE__ControlPort));
        LilvNode* lv2_InputPort = (lilv_new_uri(designer->world, LV2_CORE__InputPort));
        LilvNode* lv2_OutputPort = (lilv_new_uri(designer->world, LV2_CORE__OutputPort));
        LilvNode* lv2_AtomPort = (lilv_new_uri(designer->world, LV2_ATOM__AtomPort));
        LilvNode* lv2_CVPort = (lilv_new_uri(designer->world, LV2_CORE__CVPort));
        LilvNode* is_int = lilv_new_uri(designer->world, LV2_CORE__integer);
        LilvNode* is_tog = lilv_new_uri(designer->world, LV2_CORE__toggled);
        LilvNode* is_enum = lilv_new_uri(designer->world, LV2_CORE__enumeration);
        LilvNode* is_min = lilv_new_uri(designer->world, LV2_CORE__minimum);
        LilvNode* is_max = lilv_new_uri(designer->world, LV2_CORE__maximum);
        LilvNode* is_def = lilv_new_uri(designer->world, LV2_CORE__default);
        
        LilvNode* is_label = lilv_new_uri(designer->world, LILV_NS_RDFS "label");
        LilvNode* is_range = lilv_new_uri(designer->world, LILV_NS_RDFS "range");
        LilvNode* is_float = lilv_new_uri(designer->world, LV2_ATOM__Float);
        LilvNode* is_path = lilv_new_uri(designer->world, LV2_ATOM__Path);
        LilvNode* is_bool = lilv_new_uri(designer->world, LV2_ATOM__Bool);
        LilvNode* is_a_int = lilv_new_uri(designer->world, LV2_ATOM__Int);
        

        LilvNode* notOnGui = lilv_new_uri(designer->world, LV2_PORT_PROPS__notOnGUI);
        LilvNode* is_log = lilv_new_uri(designer->world, LV2_PORT_PROPS__logarithmic);
        LilvNode* has_step = lilv_new_uri(designer->world, LV2_PORT_PROPS__rangeSteps);
        LilvNode* is_trigger = lilv_new_uri(designer->world, LV2_PORT_PROPS__trigger);
        LilvNode* is_ena = lilv_new_uri(designer->world, LV2_CORE__enabled);
        LilvNode* ui_X11UI = lilv_new_uri(designer->world, LV2_UI__X11UI);

        LilvNode* patch_writable = lilv_new_uri(designer->world, LV2_PATCH__writable);
        LilvNode* patch_readable = lilv_new_uri(designer->world, LV2_PATCH__readable);

        const LilvNode* uri = lilv_new_uri(designer->world, w->label);
        const LilvPlugin* plugin = lilv_plugins_get_by_uri(designer->lv2_plugins, uri);
 
        if (plugin) {
            free(designer->lv2c.symbol);
            designer->lv2c.symbol = NULL;
            const LilvNode* uri = lilv_plugin_get_uri(plugin);
            free(designer->lv2c.uri);
            designer->lv2c.uri = NULL;
            asprintf(&designer->lv2c.uri, "%s", lilv_node_as_string(uri));
            const LilvPluginClass* cls = lilv_plugin_get_class(plugin);
            free(designer->lv2c.plugintype);
            designer->lv2c.plugintype = NULL;
            asprintf(&designer->lv2c.plugintype, "%s", lilv_node_as_string(lilv_plugin_class_get_label(cls)));
            strdecode(designer->lv2c.plugintype, " ", "");
            set_project_type_by_name (designer->project_type, designer->lv2c.plugintype);
            const LilvNode* author = lilv_plugin_get_author_name(plugin);
            free(designer->lv2c.author);
            designer->lv2c.author = NULL;
            asprintf(&designer->lv2c.author, "%s", lilv_node_as_string(author));

            LilvNode* nd = NULL;
            //const LilvNode* uri = lilv_plugin_get_uri(plugin);
            LilvUIs* uis = lilv_plugin_get_uis(plugin);
            for (LilvIter* it = lilv_uis_begin(uis);
                                    !lilv_uis_is_end(uis, it);
                                    it = lilv_uis_next(uis, it)) {
                const LilvUI* ui = lilv_uis_get(uis, it);
                const LilvNode* ui_uri = lilv_ui_get_uri(ui);
                if (ui_uri) {
                    free(designer->lv2c.ui_uri);
                    designer->lv2c.ui_uri = NULL;
                    if (lilv_ui_is_a(ui, ui_X11UI)) {
                        asprintf(&designer->lv2c.ui_uri, "%s", lilv_node_as_string(ui_uri));
                    } else {
                        asprintf(&designer->lv2c.ui_uri, "%s-x", lilv_node_as_string(ui_uri));
                    }
                    break;
                }
                
            }
            lilv_uis_free(uis);
            nd = lilv_plugin_get_name(plugin);
            if (nd) {
                free(designer->lv2c.name);
                asprintf(&designer->lv2c.name, "%s", lilv_node_as_string(nd));
                strdecode(designer->lv2c.name, "(", "_");
                strdecode(designer->lv2c.name, ")", "_");
                widget_set_title(designer->ui, designer->lv2c.name);
            }
            int n_in = 0;
            int n_out = 0;
            int n_atoms = 0;
            int n_cv = 0;
            int n_gui = 0;
            lilv_node_free(nd);

            LilvNodes* writables = lilv_world_find_nodes(designer->world,
                        lilv_plugin_get_uri(plugin), patch_writable, NULL);
            LilvNodes* readables = lilv_world_find_nodes(designer->world,
                        lilv_plugin_get_uri(plugin), patch_readable, NULL);
            bool is_w = false;
            LILV_FOREACH(nodes, p, readables) {
                const LilvNode* rproperty = lilv_nodes_get(readables, p);
                LILV_FOREACH(nodes, p, writables) {
                    is_w = false;
                    const LilvNode* wproperty = lilv_nodes_get(writables, p);
                    if (lilv_node_equals(wproperty,rproperty)) {
                        is_w = true;
                        break;
                    }
                }
                if (!is_w) {
                    designer->lv2c.is_output_port = true;
                    designer->lv2c.is_atom_patch = true;
                    designer->lv2c.Port_Index = -5;
                    designer->lv2c.min = lilv_node_as_float(lilv_world_get(designer->world, rproperty, is_min, NULL));
                    designer->lv2c.max = lilv_node_as_float(lilv_world_get(designer->world, rproperty, is_max, NULL));
                    designer->lv2c.def = lilv_node_as_float(lilv_world_get(designer->world, rproperty, is_def, NULL));
                    asprintf (&designer->new_label[designer->active_widget_num], "%s",
                        lilv_node_as_string(lilv_world_get(designer->world, rproperty, is_label, NULL)));
                    Widget_t * wid = create_controller(designer, plugin, NULL, &x, &y, &x1, &y1);
                    wid->parent_struct = (void*)lilv_node_as_uri(rproperty);
                }
            }
            LILV_FOREACH(nodes, p, writables) {
                designer->lv2c.is_int_port = false;
                designer->lv2c.is_toggle_port = false;
                designer->lv2c.is_patch_path = false;
                designer->lv2c.is_output_port = false;
                designer->lv2c.is_audio_port = false;
                designer->lv2c.is_atom_port = false;
                const LilvNode* property = lilv_nodes_get(writables, p);
                //const char* uri = lilv_node_as_uri(property);
                //fprintf(stderr, "%s\n", uri);
                const char* label = lilv_node_as_string(lilv_world_get(designer->world, property, is_label, NULL));
                const LilvNode* range = lilv_world_get(designer->world, property, is_range, NULL);
                if (lilv_node_equals(range, is_float)) {
                    designer->lv2c.is_input_port = true;
                    designer->lv2c.is_atom_patch = true;
                    designer->lv2c.Port_Index = -1;
                    designer->lv2c.min = lilv_node_as_float(lilv_world_get(designer->world, property, is_min, NULL));
                    designer->lv2c.max = lilv_node_as_float(lilv_world_get(designer->world, property, is_max, NULL));
                    designer->lv2c.def = lilv_node_as_float(lilv_world_get(designer->world, property, is_def, NULL));
                    asprintf (&designer->new_label[designer->active_widget_num], "%s",label);
                    Widget_t * wid = create_controller(designer, plugin, NULL, &x, &y, &x1, &y1);
                    wid->parent_struct = (void*)lilv_node_as_uri(property);
                    //fprintf(stderr, "label  %s min %f max %f def %f \n", label, minimum, maximum, def);
                } else if (lilv_node_equals(range, is_bool)) {
                    designer->lv2c.is_input_port = true;
                    designer->lv2c.is_atom_patch = true;
                    designer->lv2c.is_toggle_port = true;
                    designer->lv2c.Port_Index = -3;
                    asprintf (&designer->new_label[designer->active_widget_num], "%s",label);
                    Widget_t * wid = create_controller(designer, plugin, NULL, &x, &y, &x1, &y1);
                    wid->parent_struct = (void*)lilv_node_as_uri(property);
                    //fprintf(stderr, "%s is toggle\n", label);
                } else if (lilv_node_equals(range, is_path)) {
                    designer->lv2c.is_input_port = true;
                    designer->lv2c.is_atom_patch = true;
                    designer->lv2c.is_patch_path = true;
                    designer->lv2c.Port_Index = -4;
                    asprintf (&designer->new_label[designer->active_widget_num], "%s",label);
                    Widget_t * wid = create_controller(designer, plugin, NULL, &x, &y, &x1, &y1);
                    wid->parent_struct = (void*)lilv_node_as_uri(property);
                    //fprintf(stderr, "%s is path\n", label);
                } else if (lilv_node_equals(range, is_a_int)) {
                    designer->lv2c.is_input_port = true;
                    designer->lv2c.is_atom_patch = true;
                    designer->lv2c.is_int_port = true;
                    designer->lv2c.Port_Index = -2;
                    designer->lv2c.min = lilv_node_as_float(lilv_world_get(designer->world, property, is_min, NULL));
                    designer->lv2c.max = lilv_node_as_float(lilv_world_get(designer->world, property, is_max, NULL));
                    designer->lv2c.def = lilv_node_as_float(lilv_world_get(designer->world, property, is_def, NULL));
                    asprintf (&designer->new_label[designer->active_widget_num], "%s",label);
                    Widget_t * wid = create_controller(designer, plugin, NULL, &x, &y, &x1, &y1);
                    wid->parent_struct = (void*)lilv_node_as_uri(property);
                }
            }
            lilv_nodes_free(writables);
            lilv_nodes_free(readables);

            designer->lv2c.is_atom_patch = false;
            designer->lv2c.is_patch_path = false;

            unsigned int num_ports = lilv_plugin_get_num_ports(plugin);

            int ena_port = -1;
            const LilvPort* enabled_port = lilv_plugin_get_port_by_designation(plugin, lv2_ControlPort, is_ena);
            if (enabled_port) {
                ena_port = lilv_port_get_index(plugin, enabled_port);
            }

            for (unsigned int n = 0; n < num_ports; n++) {
                if (designer->active_widget_num >= MAX_CONTROLS) {
                    Widget_t *dia = open_message_dialog(designer->ui, INFO_BOX, _("INFO"),
                                                    _("MAX CONTROL COUNTER OVERFLOW"),NULL);
                    XSetTransientForHint(w->app->dpy, dia->widget, designer->ui->widget);
                    break;
                }

                designer->lv2c.is_atom_patch = false;
                designer->lv2c.is_audio_port = false;
                designer->lv2c.is_atom_port = false;
                designer->lv2c.bypass = false;
                if (n == ena_port) designer->lv2c.bypass = true;
                const LilvPort* port = lilv_plugin_get_port_by_index(plugin, n);
                if (lilv_port_is_a(plugin, port, lv2_AudioPort)) {
                    if (lilv_port_is_a(plugin, port, lv2_InputPort)) {
                        n_in++;
                        LilvNode* nm = lilv_port_get_name(plugin, port);
                        designer->lv2c.Port_Index = n;
                        designer->lv2c.is_audio_port = true;
                        designer->lv2c.is_input_port = true;
                        designer->lv2c.is_output_port = false;
                        free(designer->lv2c.symbol);
                        designer->lv2c.symbol = NULL;
                        asprintf (&designer->lv2c.symbol,  "%s",lilv_node_as_string(lilv_port_get_symbol(plugin, port)));
                        asprintf (&designer->new_label[designer->active_widget_num], "%s",lilv_node_as_string(nm));
                        lilv_node_free(nm);
                        designer->lv2c.audio_input++;
                        adj_set_value(designer->project_audio_input->adj, (float)designer->lv2c.audio_input);
                    } else {
                        n_out++;
                        designer->lv2c.audio_output++;
                        LilvNode* nm = lilv_port_get_name(plugin, port);
                        designer->lv2c.Port_Index = n;
                        designer->lv2c.is_audio_port = true;
                        designer->lv2c.is_input_port = false;
                        designer->lv2c.is_output_port = true;
                        free(designer->lv2c.symbol);
                        designer->lv2c.symbol = NULL;
                        asprintf (&designer->lv2c.symbol,  "%s",lilv_node_as_string(lilv_port_get_symbol(plugin, port)));
                        asprintf (&designer->new_label[designer->active_widget_num], "%s",lilv_node_as_string(nm));
                        lilv_node_free(nm);
                        adj_set_value(designer->project_audio_output->adj, (float)designer->lv2c.audio_output);
                    }
                    //continue;
                } else if (lilv_port_is_a(plugin, port, lv2_CVPort)) {
                    n_cv++;
                    continue;
                } else if (lilv_port_is_a(plugin, port, lv2_AtomPort)) {
                    if (lilv_port_is_a(plugin, port, lv2_InputPort)) {
                        designer->lv2c.atom_input_port = n;
                        LilvNode* nm = lilv_port_get_name(plugin, port);
                        designer->lv2c.Port_Index = n;
                        designer->lv2c.is_audio_port = false;
                        designer->lv2c.is_atom_port = true;
                        designer->lv2c.is_input_port = true;
                        designer->lv2c.is_output_port = false;
                        free(designer->lv2c.symbol);
                        designer->lv2c.symbol = NULL;
                        asprintf (&designer->lv2c.symbol,  "%s",lilv_node_as_string(lilv_port_get_symbol(plugin, port)));
                        asprintf (&designer->new_label[designer->active_widget_num], "%s",lilv_node_as_string(nm));
                        lilv_node_free(nm);
                        
                    } else if (lilv_port_is_a(plugin, port, lv2_OutputPort)) {
                        designer->lv2c.atom_output_port = n;
                        LilvNode* nm = lilv_port_get_name(plugin, port);
                        designer->lv2c.Port_Index = n;
                        designer->lv2c.is_audio_port = false;
                        designer->lv2c.is_atom_port = true;
                        designer->lv2c.is_input_port = false;
                        designer->lv2c.is_output_port = true;
                        free(designer->lv2c.symbol);
                        designer->lv2c.symbol = NULL;
                        asprintf (&designer->lv2c.symbol,  "%s",lilv_node_as_string(lilv_port_get_symbol(plugin, port)));
                        asprintf (&designer->new_label[designer->active_widget_num], "%s",lilv_node_as_string(nm));
                        lilv_node_free(nm);
                    }
                    n_atoms++;
                    //continue;
                } else if (lilv_port_has_property(plugin, port, notOnGui)) {
                    n_gui++;
                    continue;
                } else if (lilv_port_is_a(plugin, port, lv2_ControlPort)) {
                    LilvNode* nm = lilv_port_get_name(plugin, port);
                    designer->lv2c.Port_Index = n;
                    asprintf (&designer->new_label[designer->active_widget_num], "%s",lilv_node_as_string(nm));
                    lilv_node_free(nm);
                    free(designer->lv2c.symbol);
                    designer->lv2c.symbol = NULL;
                    asprintf (&designer->lv2c.symbol,  "%s",lilv_node_as_string(lilv_port_get_symbol(plugin, port)));
                    if (lilv_port_is_a(plugin, port, lv2_InputPort)) {
                        designer->lv2c.is_input_port = true;
                        designer->lv2c.is_output_port = false;
                    } else if (lilv_port_is_a(plugin, port, lv2_OutputPort)) {
                        designer->lv2c.is_input_port = false;
                        designer->lv2c.is_output_port = true;
                    }

                    LilvNode *pdflt, *pmin, *pmax;
                    lilv_port_get_range(plugin, port, &pdflt, &pmin, &pmax);
                    if (pmin) {
                        designer->lv2c.min = lilv_node_as_float(pmin);
                        lilv_node_free(pmin);
                    }
                    if (pmax) {
                        designer->lv2c.max = lilv_node_as_float(pmax);
                        lilv_node_free(pmax);
                    }
                    if (pdflt) {
                        designer->lv2c.def = lilv_node_as_float(pdflt);
                        lilv_node_free(pdflt);
                    }

                    if (lilv_port_has_property(plugin, port, is_int)) {
                        designer->lv2c.is_int_port = true;
                    } else {
                        designer->lv2c.is_int_port = false;
                    }

                    if (lilv_port_has_property(plugin, port, is_tog)) {
                        designer->lv2c.is_toggle_port = true;
                    } else {
                        designer->lv2c.is_toggle_port = false;
                    }

                    if (lilv_port_has_property(plugin, port, is_enum)) {
                        LilvScalePoints* sp = lilv_port_get_scale_points(plugin, port);
                        int num_sp = lilv_scale_points_size(sp);
                        if (num_sp > 0) {
                            designer->lv2c.is_enum_port = true;
                            lilv_scale_points_free(sp);
                        } else {
                            designer->lv2c.is_enum_port = false;
                        }
                    } else {
                        designer->lv2c.is_enum_port = false;
                    }

                    if (lilv_port_has_property(plugin, port, is_trigger)) {
                        designer->lv2c.is_trigger_port = true;
                    } else {
                        designer->lv2c.is_trigger_port = false;
                    }

                    if (lilv_port_has_property(plugin, port, is_log)) {
                        designer->lv2c.is_log_port = true;
                    } else {
                        designer->lv2c.is_log_port = false;
                    }
                }
                create_controller(designer, plugin, port, &x, &y, &x1, &y1);
            }
            designer->ui->width = min(1200,x1);
            designer->ui->height = min(600,y1+130);
            XResizeWindow(designer->ui->app->dpy, designer->ui->widget, designer->ui->width, designer->ui->height);
        }
        lilv_node_free(lv2_AudioPort);
        lilv_node_free(lv2_ControlPort);
        lilv_node_free(lv2_InputPort);
        lilv_node_free(lv2_OutputPort);
        lilv_node_free(lv2_AtomPort);
        lilv_node_free(lv2_CVPort);
        lilv_node_free(is_int);
        lilv_node_free(is_tog);
        lilv_node_free(is_enum);
        lilv_node_free(is_min);
        lilv_node_free(is_max);
        lilv_node_free(is_def);

        lilv_node_free(is_label);
        lilv_node_free(is_range);
        lilv_node_free(is_float);
        lilv_node_free(is_path);
        lilv_node_free(is_bool);
        lilv_node_free(is_a_int);

        lilv_node_free(notOnGui);
        lilv_node_free(is_log);
        lilv_node_free(has_step);
        lilv_node_free(is_trigger);
        lilv_node_free(is_ena);
        lilv_node_free(ui_X11UI);
        lilv_node_free(patch_readable);
        lilv_node_free(patch_writable);
    } else {
        designer->is_project = true;
    }
    //print_list(designer);
    //print_ttl(designer);
    //print_json(designer);
    //XResizeWindow(designer->ui->app->dpy, designer->ui->widget, designer->ui->width, designer->ui->height-1);
    if (designer->active_widget != NULL)
        box_entry_set_text(designer->controller_label, designer->active_widget->label);
    widget_show_all(designer->ui);
}

void load_uris(Widget_t *lv2_uris, const LilvPlugins* lv2_plugins) {
    for (LilvIter* it = lilv_plugins_begin(lv2_plugins);
      !lilv_plugins_is_end(lv2_plugins, it);
      it = lilv_plugins_next(lv2_plugins, it)) {
        const LilvPlugin* plugin = lilv_plugins_get(lv2_plugins, it);
        if (plugin) {
            const LilvNode* uri = lilv_plugin_get_uri(plugin);
            combobox_add_entry(lv2_uris,lilv_node_as_string(uri));
        }
    }
}

void set_path(LilvWorld* world, const char* workdir) {
    char lwd[PATH_MAX];
    memset(lwd,0,PATH_MAX*sizeof(lwd[0]));
    strcat(lwd,"file://");
    strcat(lwd,workdir);
    LilvNode* path = lilv_new_string(world, lwd);
    lilv_world_set_option(world, LILV_OPTION_LV2_PATH, path);
    lilv_node_free(path);
}
