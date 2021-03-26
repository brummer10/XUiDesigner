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


#include "XUiSettings.h"
#include "XUiGenerator.h"
#include "XUiTextInput.h"


static void set_project_title(void *w_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    XUiDesigner *designer = (XUiDesigner*)w->parent_struct;
    TextBox_t *text_box = (TextBox_t*)w->private_struct;
    if (strlen(text_box->input_label)>1) {
        widget_set_title(designer->ui,text_box->input_label);
        expose_widget(designer->ui);
    }
}

static void set_project_author(void *w_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    XUiDesigner *designer = (XUiDesigner*)w->parent_struct;
    TextBox_t *text_box = (TextBox_t*)w->private_struct;
    if (strlen(text_box->input_label)>1) {
        free(designer->lv2c.author);
        designer->lv2c.author = NULL;
        asprintf(&designer->lv2c.author, "%s", text_box->input_label);
    }
}

static void set_project_uri(void *w_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    XUiDesigner *designer = (XUiDesigner*)w->parent_struct;
    TextBox_t *text_box = (TextBox_t*)w->private_struct;
    if (strlen(text_box->input_label)>1) {
        free(designer->lv2c.uri);
        designer->lv2c.uri = NULL;
        asprintf(&designer->lv2c.uri, "%s", text_box->input_label);
    }
}

static void set_project_ui_uri(void *w_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    XUiDesigner *designer = (XUiDesigner*)w->parent_struct;
    TextBox_t *text_box = (TextBox_t*)w->private_struct;
    if (strlen(text_box->input_label)>1) {
        free(designer->lv2c.ui_uri);
        designer->lv2c.ui_uri = NULL;
        asprintf(&designer->lv2c.ui_uri, "%s", text_box->input_label);
    }
}

static void set_project_type(void *w_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    XUiDesigner *designer = (XUiDesigner*)w->parent_struct;
    Widget_t* menu =  w->childlist->childs[1];
    Widget_t* view_port =  menu->childlist->childs[0];
    ComboBox_t *comboboxlist = (ComboBox_t*)view_port->parent_struct;
    free(designer->lv2c.plugintype);
    designer->lv2c.plugintype = NULL;
    asprintf(&designer->lv2c.plugintype, "%s", comboboxlist->list_names[(int)adj_get_value(w->adj)]);
}

static void set_project_audio_input(void *w_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    XUiDesigner *designer = (XUiDesigner*)w->parent_struct;
    designer->lv2c.audio_input = (int)adj_get_value(w->adj);
}

static void set_project_audio_output(void *w_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    XUiDesigner *designer = (XUiDesigner*)w->parent_struct;
    designer->lv2c.audio_output = (int)adj_get_value(w->adj);
}

static void set_project_bypass_switch(void *w_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    XUiDesigner *designer = (XUiDesigner*)w->parent_struct;
    if (w->flags & HAS_POINTER) {
        designer->lv2c.bypass = (int)adj_get_value(w->adj);
        if (designer->lv2c.bypass) {
            Widget_t *wid = add_toggle_button(designer->ui, "Bypass", 60, 60, 60, 60);
            set_controller_callbacks(designer, wid, true);
            designer->controls[designer->active_widget_num].destignation_enabled = true;
            add_to_list(designer, wid, "add_lv2_toggle_button", false, IS_TOGGLE_BUTTON);
            designer->prev_active_widget = wid;
        } else {
            int m = 0;
            for (;m<MAX_CONTROLS;m++) {
                if (designer->controls[m].wid != NULL && designer->controls[m].destignation_enabled) {
                    destroy_widget(designer->controls[m].wid, w->app);
                    remove_from_list(designer, designer->controls[m].wid);
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
                }
            }

        }
    }
}


static void set_project(void *w_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    XUiDesigner *designer = (XUiDesigner*)w->parent_struct;
    if (w->flags & HAS_POINTER && !adj_get_value(w->adj_y)) {
        set_project_audio_output(designer->project_audio_output, NULL);
        set_project_audio_input(designer->project_audio_input, NULL);
        set_project_type(designer->project_type, NULL);
        set_project_ui_uri(designer->project_ui_uri, NULL);
        set_project_uri(designer->project_uri, NULL);
        set_project_author(designer->project_author, NULL);
        set_project_title(designer->project_title, NULL);
        designer->lv2c.midi_input = adj_get_value(designer->project_midi_input->adj);
        designer->lv2c.midi_output = adj_get_value(designer->project_midi_output->adj);
        widget_hide(designer->set_project);
        designer->is_project = true;
    }
}

void create_project_settings_window(XUiDesigner *designer) {
    Atom wmStateAbove = XInternAtom(designer->w->app->dpy, "_NET_WM_STATE_ABOVE", 1 );
    Atom wmNetWmState = XInternAtom(designer->w->app->dpy, "_NET_WM_STATE", 1 );

    designer->set_project = create_window(designer->w->app, DefaultRootWindow(designer->w->app->dpy), 0, 0, 320, 520);
    XChangeProperty(designer->w->app->dpy, designer->set_project->widget, wmNetWmState, XA_ATOM, 32, 
        PropModeReplace, (unsigned char *) &wmStateAbove, 1); 
    XSetTransientForHint(designer->w->app->dpy, designer->set_project->widget, designer->ui->widget);
    designer->set_project->parent_struct = designer;
    designer->set_project->func.expose_callback = draw_window;
    widget_set_title(designer->set_project, _("Project"));

    add_label(designer->set_project, _("Projet Name"), 0, 0, 260, 30);
    designer->project_title = add_input_box(designer->set_project, 0, 10, 30, 300, 30);
    designer->project_title->parent_struct = designer;
    designer->project_title->func.user_callback = set_project_title;

    add_label(designer->set_project, _("Author"), 0, 60, 240, 30);
    designer->project_author = add_input_box(designer->set_project, 0, 10, 90, 300, 30);
    designer->project_author->parent_struct = designer;
    designer->project_author->func.user_callback = set_project_author;

    add_label(designer->set_project, _("URI"), 0, 120, 240, 30);
    designer->project_uri = add_input_box(designer->set_project, 0, 10, 150, 300, 30);
    designer->project_uri->parent_struct = designer;
    designer->project_uri->func.user_callback = set_project_uri;

    add_label(designer->set_project, _("UI URI"), 0, 180, 240, 30);
    designer->project_ui_uri = add_input_box(designer->set_project, 0, 10, 210, 300, 30);
    designer->project_ui_uri->parent_struct = designer;
    designer->project_ui_uri->func.user_callback = set_project_ui_uri;

    add_label(designer->set_project, _("Plugin Type"), 0, 240, 180, 30);
    designer->project_type = add_combobox(designer->set_project, "Type", 10, 270, 300, 30);
    designer->project_type->parent_struct = designer;
    combobox_add_entry(designer->project_type,"DelayPlugin");
    combobox_add_entry(designer->project_type,"ReverbPlugin");
    combobox_add_entry(designer->project_type,"DistortionPlugin");
    combobox_add_entry(designer->project_type,"WaveshaperPlugin");
    combobox_add_entry(designer->project_type,"DynamicsPlugin");
    combobox_add_entry(designer->project_type,"AmplifierPlugin");
    combobox_add_entry(designer->project_type,"CompressorPlugin");
    combobox_add_entry(designer->project_type,"EnvelopePlugin");
    combobox_add_entry(designer->project_type,"ExpanderPlugin");
    combobox_add_entry(designer->project_type,"GatePlugin");
    combobox_add_entry(designer->project_type,"LimiterPlugin");
    combobox_add_entry(designer->project_type,"FilterPlugin");
    combobox_add_entry(designer->project_type,"AllpassPlugin");
    combobox_add_entry(designer->project_type,"BandpassPlugin");
    combobox_add_entry(designer->project_type,"CombPlugin");
    combobox_add_entry(designer->project_type,"EQPlugin");
    combobox_add_entry(designer->project_type,"MultiEQPlugin");
    combobox_add_entry(designer->project_type,"ParaEQPlugin");
    combobox_add_entry(designer->project_type,"HighpassPlugin");
    combobox_add_entry(designer->project_type,"LowpassPlugin");
    combobox_add_entry(designer->project_type,"GeneratorPlugin");
    combobox_add_entry(designer->project_type,"ConstantPlugin");
    combobox_add_entry(designer->project_type,"InstrumentPlugin");
    combobox_add_entry(designer->project_type,"OscillatorPlugin");
    combobox_add_entry(designer->project_type,"ModulatorPlugin");
    combobox_add_entry(designer->project_type,"ChorusPlugin");
    combobox_add_entry(designer->project_type,"FlangerPlugin");
    combobox_add_entry(designer->project_type,"PhaserPlugin");
    combobox_add_entry(designer->project_type,"ReverbPlugin");
    combobox_add_entry(designer->project_type,"SimulatorPlugin");
    combobox_add_entry(designer->project_type,"ReverbPlugin");
    combobox_add_entry(designer->project_type,"SpatialPlugin");
    combobox_add_entry(designer->project_type,"SpectralPlugin");
    combobox_add_entry(designer->project_type,"PitchPlugin");
    combobox_add_entry(designer->project_type,"UtilityPlugin");
    combobox_add_entry(designer->project_type,"AnalyserPlugin");
    combobox_add_entry(designer->project_type,"ConverterPlugin");
    combobox_add_entry(designer->project_type,"FunctionPlugin");
    combobox_add_entry(designer->project_type,"MixerPlugin");
    designer->project_type->func.value_changed_callback = set_project_type;

    add_label(designer->set_project, _("Audio Input"), 10, 300, 80, 30);
    designer->project_audio_input = add_combobox(designer->set_project, "", 10, 330, 80, 30);
    designer->project_audio_input->parent_struct = designer;
    combobox_add_numeric_entrys(designer->project_audio_input, 0, 16);
    combobox_set_active_entry(designer->project_audio_input, 1);
    designer->project_audio_input->func.value_changed_callback = set_project_audio_input;

    add_label(designer->set_project, _("Audio Output"), 130, 300, 80, 30);
    designer->project_audio_output = add_combobox(designer->set_project, "", 130, 330, 80, 30);
    designer->project_audio_output->parent_struct = designer;
    combobox_add_numeric_entrys(designer->project_audio_output, 0, 16);
    combobox_set_active_entry(designer->project_audio_output, 1);
    designer->project_audio_output->func.value_changed_callback = set_project_audio_output;

    add_label(designer->set_project, _("MIDI"), 0, 360, 180, 30);
    designer->project_midi_input = add_check_button(designer->set_project, "Input", 10, 395, 20, 20);
    add_label(designer->set_project, _("Input"), 30, 395, 60, 20);
    designer->project_midi_output = add_check_button(designer->set_project, "Output", 130, 395, 20, 20);
    add_label(designer->set_project, _("Output"), 150, 395, 60, 20);

    add_label(designer->set_project, _("Bypass Switch"), 0, 420, 180, 30);
    designer->project_bypass = add_check_button(designer->set_project, "Bypass", 10, 455, 20, 20);
    add_label(designer->set_project, _("lv2:enabled"), 30, 455, 80, 20);
    designer->project_bypass->parent_struct = designer;
    designer->project_bypass->func.value_changed_callback = set_project_bypass_switch;

    Widget_t* tmp = add_button(designer->set_project, _("Set"), 170, 480, 40, 30);
    tmp->parent_struct = designer;
    tmp->func.value_changed_callback = set_project;
}
