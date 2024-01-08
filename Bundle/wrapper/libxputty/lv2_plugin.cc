/*
 *                           0BSD 
 * 
 *                    BSD Zero Clause License
 * 
 *  Copyright (c) 2019 Hermann Meyer
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


#include "lv2_plugin.h"

/*---------------------------------------------------------------------
-----------------------------------------------------------------------    
                the main LV2 handle->XWindow
-----------------------------------------------------------------------
----------------------------------------------------------------------*/

#ifdef USE_MIDI
void messenger_init(MidiMessenger *mm) {
    int i = 0;
    for (; i < 25; i++) {
        mm->send_cc[i] &= ~_FULL;
        mm->send_cc[i] |= _EMPTY;
    }
}

int next(MidiMessenger *mm, int i) {
    while (++i < 25) {
        if (mm->send_cc[i] & _FULL) {
            return i;
        }
    }
    return -1;
}

void fill(MidiMessenger *mm, uint8_t *midi_send, int i) {
    if (mm->me_num[i] == 3) {
        midi_send[2] =  mm->bg_num[i];
    }
    midi_send[1] = mm->pg_num[i];    // program value
    midi_send[0] = mm->cc_num[i];    // controller+ channel
    mm->send_cc[i] &= ~_FULL;
    mm->send_cc[i] |= _EMPTY;
}

bool send_midi_cc(MidiMessenger *mm, uint8_t _cc, const uint8_t _pg,
                            const uint8_t _bgn, const uint8_t _num) {

    for(int i = 0; i < 25; i++) {
        if (mm->send_cc[i] & _FULL) {
            if (mm->cc_num[i] == _cc && mm->pg_num[i] == _pg &&
                mm->bg_num[i] == _bgn && mm->me_num[i] == _num)
                return true;
        } else if (mm->send_cc[i] & _EMPTY) {
            mm->cc_num[i] = _cc;
            mm->pg_num[i] = _pg;
            mm->bg_num[i] = _bgn;
            mm->me_num[i] = _num;
            mm->send_cc[i] &= ~_EMPTY;
            mm->send_cc[i] |=_FULL;
            return true;
        }
    }
    return false;
}
#endif

// draw the window
static void draw_window(void *w_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    set_pattern(w,&w->color_scheme->selected,&w->color_scheme->normal,BACKGROUND_);
    cairo_paint (w->crb);
    set_pattern(w,&w->color_scheme->normal,&w->color_scheme->selected,BACKGROUND_);
    cairo_rectangle (w->crb,4,4,w->width-8,w->height-8);
    cairo_set_line_width(w->crb,4);
    cairo_stroke(w->crb);

#ifndef HIDE_NAME
    cairo_text_extents_t extents;
    use_text_color_scheme(w, get_color_state(w));
    cairo_set_font_size (w->crb, w->app->big_font/w->scale.ascale);
    cairo_text_extents(w->crb,w->label , &extents);
    double tw = extents.width/2.0;
#endif

    widget_set_scale(w);
    if (w->image) {
        cairo_set_source_surface (w->crb, w->image, 0, 0);
        cairo_paint (w->crb);
    }
    use_text_color_scheme(w, get_color_state(w));
#ifdef USE_ATOM
    X11_UI* ui = (X11_UI*)w->parent_struct;
    X11_UI_Private_t *ps = (X11_UI_Private_t*)ui->private_ptr;
    if (strlen(ps->filename)) {
        cairo_text_extents_t extents_f;
        cairo_text_extents(w->crb, basename(ps->filename), &extents_f);
        double twf = extents_f.width/2.0;
        cairo_move_to (w->crb, max(5,(w->scale.init_width*0.5)-twf), w->scale.init_y+20 );
        cairo_show_text(w->crb, basename(ps->filename));       
    }
#endif
#ifndef HIDE_NAME
    cairo_move_to (w->crb, (w->scale.init_width*0.5)-tw, w->scale.init_height-10 );
    cairo_show_text(w->crb, w->label);
#endif
    widget_reset_scale(w);
    cairo_new_path (w->crb);
}

// if controller value changed send notify to host
static void value_changed(void *w_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    X11_UI* ui = (X11_UI*)w->parent_struct;
    float v = adj_get_value(w->adj);
    ui->write_function(ui->controller,w->data,sizeof(float),0,&v);
    plugin_value_changed(ui, w, (PortIndex)w->data);
}


Widget_t* add_lv2_knob(Widget_t *w, Widget_t *p, PortIndex index, const char * label,
                                X11_UI* ui, int x, int y, int width, int height) {
    w = add_knob(p, label, x, y, width, height);
    w->parent_struct = ui;
    w->data = index;
    w->func.value_changed_callback = value_changed;
    return w;
}


Widget_t* add_lv2_combobox(Widget_t *w, Widget_t *p, PortIndex index, const char * label,
                                X11_UI* ui, int x, int y, int width, int height) {
    w = add_combobox(p, label, x, y, width, height);
    w->parent_struct = ui;
    w->data = index;
    w->func.value_changed_callback = value_changed;
    return w;
}


Widget_t* add_lv2_vmeter(Widget_t *w, Widget_t *p, PortIndex index, const char * label,
                                X11_UI* ui, int x, int y, int width, int height) {
    w = add_vmeter(p, label, false, x, y, width, height);
    w->parent_struct = ui;
    w->data = index;
    tooltip_set_text(w, label);
    w->func.value_changed_callback = value_changed;
    return w;
}

Widget_t* add_lv2_hmeter(Widget_t *w, Widget_t *p, PortIndex index, const char * label,
                                X11_UI* ui, int x, int y, int width, int height) {
    w = add_hmeter(p, label, false, x, y, width, height);
    w->parent_struct = ui;
    w->data = index;
    tooltip_set_text(w, label);
    w->func.value_changed_callback = value_changed;
    return w;
}

Widget_t* add_lv2_vslider(Widget_t *w, Widget_t *p, PortIndex index, const char * label,
                                X11_UI* ui, int x, int y, int width, int height) {
    w = add_vslider(p, label, x, y, width, height);
    w->parent_struct = ui;
    w->data = index;
    w->func.value_changed_callback = value_changed;
    return w;
}

Widget_t* add_lv2_hslider(Widget_t *w, Widget_t *p, PortIndex index, const char * label,
                                X11_UI* ui, int x, int y, int width, int height) {
    w = add_hslider(p, label, x, y, width, height);
    w->parent_struct = ui;
    w->data = index;
    w->func.value_changed_callback = value_changed;
    return w;
}

Widget_t* add_lv2_toggle_button(Widget_t *w, Widget_t *p, PortIndex index, const char * label,
                                X11_UI* ui, int x, int y, int width, int height) {
    w = add_toggle_button(p, label, x, y, width, height);
    w->parent_struct = ui;
    w->data = index;
    w->func.value_changed_callback = value_changed;
    return w;
}

Widget_t* add_lv2_image_toggle(Widget_t *w, Widget_t *p, PortIndex index, const char * label,
                                X11_UI* ui, int x, int y, int width, int height) {
    w = add_switch_image_button(p, label, x, y, width, height);
    w->parent_struct = ui;
    w->data = index;
    w->func.value_changed_callback = value_changed;
    return w;
}

Widget_t* add_lv2_button(Widget_t *w, Widget_t *p, PortIndex index, const char * label,
                                X11_UI* ui, int x, int y, int width, int height) {
    w = add_button(p, label, x, y, width, height);
    w->parent_struct = ui;
    w->data = index;
    w->func.value_changed_callback = value_changed;
    return w;
}

Widget_t* add_lv2_image_button(Widget_t *w, Widget_t *p, PortIndex index, const char * label,
                                X11_UI* ui, int x, int y, int width, int height) {
    w = add_image_button(p, label, x, y, width, height);
    w->parent_struct = ui;
    w->data = index;
    w->func.value_changed_callback = value_changed;
    return w;
}

Widget_t* add_lv2_valuedisplay(Widget_t *w, Widget_t *p, PortIndex index, const char * label,
                                X11_UI* ui, int x, int y, int width, int height) {
    w = add_valuedisplay(p, label, x, y, width, height);
    w->parent_struct = ui;
    w->data = index;
    w->func.value_changed_callback = value_changed;
    return w;
}

Widget_t* add_lv2_label(Widget_t *w, Widget_t *p, PortIndex index, const char * label,
                                X11_UI* ui, int x, int y, int width, int height) {
    w = add_label(p, label, x, y, width, height);
    w->parent_struct = ui;
    w->data = index;
    w->func.value_changed_callback = value_changed;
    return w;
}

Widget_t* add_lv2_frame(Widget_t *w, Widget_t *p, PortIndex index, const char * label,
                                X11_UI* ui, int x, int y, int width, int height) {
    w = add_frame(p, label, x, y, width, height);
    w->parent_struct = ui;
    w->data = index;
    w->func.value_changed_callback = value_changed;
    return w;
}

Widget_t* add_lv2_image(Widget_t *w, Widget_t *p, PortIndex index, const char * label,
                                X11_UI* ui, int x, int y, int width, int height) {
    w = add_image(p, label, x, y, width, height);
    w->parent_struct = ui;
    w->data = index;
    return w;
}

Widget_t* add_lv2_waveview(Widget_t *w, Widget_t *p, PortIndex index, const char * label,
                                X11_UI* ui, int x, int y, int width, int height) {
    w = add_waveview(p, label, x, y, width, height);
    w->parent_struct = ui;
    w->data = index;
    w->func.value_changed_callback = value_changed;
    return w;
}

Widget_t* add_lv2_tabbox(Widget_t *w, Widget_t *p, PortIndex index, const char * label,
                                X11_UI* ui, int x, int y, int width, int height) {
    w = add_tabbox(p, label, x, y, width, height);
    w->parent_struct = ui;
    w->data = index;
    return w;
}

Widget_t* add_lv2_tab(Widget_t *w, Widget_t *p, PortIndex index, const char * label, X11_UI* ui) {
    w = tabbox_add_tab(p, label);
    w->parent_struct = ui;
    w->data = index;
    return w;
}

Widget_t* add_lv2_file_button(Widget_t *w, Widget_t *p, PortIndex index, const char * label,
                                X11_UI* ui, int x, int y, int width, int height) {
    w = add_file_button(p, x, y, width, height, "", "");
    w->data = index;
    return w;
}

Widget_t* add_lv2_midikeyboard(Widget_t *w, Widget_t *p, PortIndex index, const char * label,
                                X11_UI* ui, int x, int y, int width, int height) {
    w = add_midi_keyboard(p, label, x, y, width, height);
    w->parent_struct = ui;
    w->data = index;
    return w;
}

void load_bg_image(X11_UI* ui, const char* image) {
    cairo_surface_t *getpng = cairo_image_surface_create_from_png (image);
    int width = cairo_image_surface_get_width(getpng);
    int height = cairo_image_surface_get_height(getpng);
    int width_t = ui->win->scale.init_width;
    int height_t = ui->win->scale.init_height;
    double x = (double)width_t/(double)width;
    double y = (double)height_t/(double)height;
    cairo_surface_destroy(ui->win->image);
    ui->win->image = NULL;

    ui->win->image = cairo_surface_create_similar (ui->win->surface, 
                        CAIRO_CONTENT_COLOR_ALPHA, width_t, height_t);
    cairo_t *cri = cairo_create (ui->win->image);
    cairo_scale(cri, x,y);    
    cairo_set_source_surface (cri, getpng,0,0);
    cairo_paint (cri);
    cairo_surface_destroy(getpng);
    cairo_destroy(cri);
}

void load_controller_image(Widget_t* w, const char* image) {
    cairo_surface_t *getpng = cairo_image_surface_create_from_png (image);
    int width = cairo_image_surface_get_width(getpng);
    int height = cairo_image_surface_get_height(getpng);
    cairo_surface_destroy(w->image);
    w->image = NULL;

    w->image = cairo_surface_create_similar (w->surface, 
                        CAIRO_CONTENT_COLOR_ALPHA, width, height);
    cairo_t *cri = cairo_create (w->image);
    cairo_set_source_surface (cri, getpng,0,0);
    cairo_paint (cri);
    cairo_surface_destroy(getpng);
    cairo_destroy(cri);
}

// init the xwindow and return the LV2UI handle
static LV2UI_Handle instantiate(const LV2UI_Descriptor * descriptor,
            const char * plugin_uri, const char * bundle_path,
            LV2UI_Write_Function write_function,
            LV2UI_Controller controller, LV2UI_Widget * widget,
            const LV2_Feature * const * features) {

    X11_UI* ui = (X11_UI*)malloc(sizeof(X11_UI));

    if (!ui) {
        fprintf(stderr,"ERROR: failed to instantiate plugin with URI %s\n", plugin_uri);
        return NULL;
    }

    ui->parentXwindow = 0;
    ui->private_ptr = NULL;
    ui->need_resize = 1;

    int i = 0;
    for(;i<CONTROLS;i++)
        ui->widget[i] = NULL;
    i = 0;
    for(;i<GUI_ELEMENTS;i++)
        ui->elem[i] = NULL;

    i = 0;
    for (; features[i]; ++i) {
        if (!strcmp(features[i]->URI, LV2_UI__parent)) {
            ui->parentXwindow = features[i]->data;
        } else if (!strcmp(features[i]->URI, LV2_UI__resize)) {
            ui->resize = (LV2UI_Resize*)features[i]->data;
        } else if (!strcmp(features[i]->URI, LV2_URID_URI "#map")) {
            ui->map = (LV2_URID_Map*)features[i]->data;
        }
    }

    if (ui->parentXwindow == NULL)  {
        fprintf(stderr, "ERROR: Failed to open parentXwindow for %s\n", plugin_uri);
        free(ui);
        return NULL;
    }

#ifdef USE_MIDI
    lv2_atom_forge_init(&ui->forge,ui->map);
    ui->atom_eventTransfer  = ui->map->map(ui->map->handle, LV2_ATOM__eventTransfer);
    ui->midi_MidiEvent = ui->map->map(ui->map->handle, LV2_MIDI__MidiEvent);
    ui->midiatom.type = ui->midi_MidiEvent;
    ui->midiatom.size = 3;
    messenger_init(&ui->mm);
#endif

    // init Xputty
    main_init(&ui->main);
    int w = 1;
    int h = 1;
    plugin_set_window_size(&w,&h,plugin_uri);
    // create the toplevel Window on the parentXwindow provided by the host
    ui->win = create_window(&ui->main, (Window)ui->parentXwindow, 0, 0, w, h);
    ui->win->parent_struct = ui;
#ifdef __linux__
    ui->win->flags |= DONT_PROPAGATE;
#endif
    ui->win->label = plugin_set_name();
    // connect the expose func
    ui->win->func.expose_callback = draw_window;
    // create controller widgets
    plugin_create_controller_widgets(ui,plugin_uri);
    // map all widgets into the toplevel Widget_t
    widget_show_all(ui->win);
    // set the widget pointer to the X11 Window from the toplevel Widget_t
    *widget = (void*)ui->win->widget;
    // request to resize the parentXwindow to the size of the toplevel Widget_t
    if (ui->resize){
        ui->resize->ui_resize(ui->resize->handle, w, h);
    }
    // store pointer to the host controller
    ui->controller = controller;
    // store pointer to the host write function
    ui->write_function = write_function;
    
    return (LV2UI_Handle)ui;
}

// cleanup after usage
static void cleanup(LV2UI_Handle handle) {
    X11_UI* ui = (X11_UI*)handle;
    plugin_cleanup(ui);
    // Xputty free all memory used
    main_quit(&ui->main);
    free(ui->private_ptr);
    free(ui);
}

static void null_callback(void *w_, void* user_data) {
    
}

/*---------------------------------------------------------------------
-----------------------------------------------------------------------    
                        LV2 interface
-----------------------------------------------------------------------
----------------------------------------------------------------------*/

// port value change message from host
static void port_event(LV2UI_Handle handle, uint32_t port_index,
                        uint32_t buffer_size, uint32_t format,
                        const void * buffer) {
    X11_UI* ui = (X11_UI*)handle;
    float value = *(float*)buffer;
    int i=0;
    for (;i<CONTROLS;i++) {
        if (ui->widget[i] && port_index == (uint32_t)ui->widget[i]->data) {
            // prevent event loop between host and plugin
            xevfunc store = ui->widget[i]->func.value_changed_callback;
            ui->widget[i]->func.value_changed_callback = null_callback;
            // Xputty check if the new value differs from the old one
            // and set new one, when needed
            adj_set_value(ui->widget[i]->adj, value);
            // activate value_change_callback back
            ui->widget[i]->func.value_changed_callback = store;
        }
   }
   plugin_port_event(handle, port_index, buffer_size, format, buffer);
}

#ifdef USE_MIDI
// send midi data to the midi output port 
void send_midi_data(X11_UI* ui) {

    int i = next(&ui->mm, -1);
    if (i < 0) return;
    uint8_t obj_buf[OBJ_BUF_SIZE];
    uint8_t vec[3];
    lv2_atom_forge_set_buffer(&ui->forge, obj_buf, OBJ_BUF_SIZE);

    while (i >= 0) {
        memset(vec, 0, 3 * sizeof(uint8_t));
        fill(&ui->mm, vec, i);

        lv2_atom_forge_frame_time(&ui->forge,0);
        LV2_Atom* msg = (LV2_Atom*)lv2_atom_forge_raw(&ui->forge,&ui->midiatom,sizeof(LV2_Atom));
        lv2_atom_forge_raw(&ui->forge,vec, sizeof(vec));
        lv2_atom_forge_pad(&ui->forge,sizeof(vec)+sizeof(LV2_Atom));
        ui->write_function(ui->controller, ui->midi_port, lv2_atom_total_size(msg),
                                                        ui->atom_eventTransfer, msg);
        i = next(&ui->mm, i);
    }
}
#endif

// LV2 idle interface to host
static int ui_idle(LV2UI_Handle handle) {
    X11_UI* ui = (X11_UI*)handle;
    if (ui->need_resize == 1) {
        ui->need_resize = 2;
    } else if (ui->need_resize == 2) {
        int i=0;
        for (;i<CONTROLS;i++) {
            os_move_window(ui->main.dpy, ui->widget[i], ui->widget[i]->x, ui->widget[i]->y);
        }
        ui->need_resize = 0;
    }
    // Xputty event loop setup to run one cycle when called
    run_embedded(&ui->main);
#ifdef USE_MIDI
    send_midi_data(ui);
#endif
    return 0;
}

// LV2 resize interface to host
static int ui_resize(LV2UI_Feature_Handle handle, int w, int h) {
    X11_UI* ui = (X11_UI*)handle;
    // Xputty sends configure event to the toplevel widget to resize itself
    if (ui) send_configure_event(ui->win,0, 0, w, h);
    return 0;
}

// connect idle and resize functions to host
static const void* extension_data(const char* uri) {
    static const LV2UI_Idle_Interface idle = { ui_idle };
    static const LV2UI_Resize resize = { 0 ,ui_resize };
    if (!strcmp(uri, LV2_UI__idleInterface)) {
        return &idle;
    }
    if (!strcmp(uri, LV2_UI__resize)) {
        return &resize;
    }
    return NULL;
}

static const LV2UI_Descriptor descriptors[] = {
    {PLUGIN_UI_URI,instantiate,cleanup,port_event,extension_data},
#ifdef PLUGIN_UI_URI2
    {PLUGIN_UI_URI2,instantiate,cleanup,port_event,extension_data},
#endif
};

LV2_SYMBOL_EXPORT
const LV2UI_Descriptor* lv2ui_descriptor(uint32_t index) {
    if (index >= sizeof(descriptors) / sizeof(descriptors[0])) {
        return NULL;
    }
    return descriptors + index;
}

