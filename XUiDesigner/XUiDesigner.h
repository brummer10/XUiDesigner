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


#include <stdint.h>
#include <stdlib.h>
#include <pwd.h>
#include <stdio.h>
#include <unistd.h>
#include <limits.h>

#include <lilv/lilv.h>
#include "lv2/lv2plug.in/ns/ext/presets/presets.h"
#include "lv2/lv2plug.in/ns/ext/state/state.h"
#include "lv2/lv2plug.in/ns/ext/urid/urid.h"
#include <lv2/lv2plug.in/ns/ext/atom/atom.h>
#include <lv2/lv2plug.in/ns/ext/buf-size/buf-size.h>
#include <lv2/lv2plug.in/ns/ext/options/options.h>
#include <lv2/lv2plug.in/ns/ext/uri-map/uri-map.h>
#include <lv2/lv2plug.in/ns/ext/port-props/port-props.h>
#include "lv2/lv2plug.in/ns/ext/patch/patch.h"
#include "lv2/lv2plug.in/ns/extensions/ui/ui.h"

#include "xwidgets.h"
#include "xmessage-dialog.h"
#include "xfile-dialog.h"


#pragma once

#ifndef XUIDESIGNER_H_
#define XUIDESIGNER_H_

#ifdef __cplusplus
extern "C" {
#endif


#ifndef LV2_CORE__enabled
#define LV2_CORE__enabled LV2_CORE_PREFIX "enabled"
#endif


/*---------------------------------------------------------------------
-----------------------------------------------------------------------    
                define maximum allowed controller numbers
-----------------------------------------------------------------------
----------------------------------------------------------------------*/

#define MAX_CONTROLS 225

/*---------------------------------------------------------------------
-----------------------------------------------------------------------    
                enums
-----------------------------------------------------------------------
----------------------------------------------------------------------*/

enum {
    XUI_NONE       = 0,
    XUI_POSITION   = 1,
    XUI_SIZE       = 2,
    XUI_WIDTH      = 3,
    XUI_HEIGHT     = 4,
};
 
typedef enum {
    IS_KNOB           ,
    IS_HSLIDER        ,
    IS_VSLIDER        ,
    IS_BUTTON         ,
    IS_TOGGLE_BUTTON  ,
    IS_COMBOBOX       ,
    IS_VALUE_DISPLAY  ,
    IS_LABEL          ,
    IS_VMETER         ,
    IS_HMETER         ,
    IS_WAVEVIEW       ,
    IS_FRAME          ,
    IS_TABBOX         ,
    IS_IMAGE          ,
    // keep those below
    IS_FILE_BUTTON    ,
    IS_IMAGE_TOGGLE   ,
} WidgetType;

/*---------------------------------------------------------------------
-----------------------------------------------------------------------    
                struct to hold the info for printout
-----------------------------------------------------------------------
----------------------------------------------------------------------*/

typedef struct {
    Widget_t * wid;
    char* image;
    WidgetType is_type;
    uint32_t port_index;
    bool destignation_enabled;
    bool is_atom_patch;
    bool is_audio_output;
    bool is_audio_input;
    bool is_atom_output;
    bool is_atom_input;
    bool have_adjustment;
    int grid_snap_option;
    const char* type;
    int in_frame;
    int in_tab;
    int tab_box;
    char* name;
    char* symbol;
} Controller;

/*---------------------------------------------------------------------
-----------------------------------------------------------------------    
                struct to hold the info for the designer
-----------------------------------------------------------------------
----------------------------------------------------------------------*/

typedef struct {
    char* ui_uri;
    char* uri;
    char* author;
    char* name;
    char* plugintype;
    char* symbol;
    int audio_input;
    int audio_output;
    int midi_input;
    int midi_output;
    int atom_output_port;
    int atom_input_port;
    int bypass;
    bool is_audio_port;
    bool is_input_port;
    bool is_output_port;
    bool is_atom_port;
    bool is_atom_patch;
    bool is_patch_path;
    bool is_toggle_port;
    bool is_enum_port;
    bool is_int_port;
    bool is_trigger_port;
    bool is_log_port;
    bool have_adjustment;
    int Port_Index;
    float min;
    float max;
    float def;
    float step;
} LV2_CONTROLLER;

typedef struct {
    int x;
    int y;
    int w;
    int h;
    bool is_active;
} DragIcon;

typedef struct {
    LilvWorld* world;
    const LilvPlugins* lv2_plugins;        

    Pixmap *icon;
    Widget_t *x_axis;
    Widget_t *y_axis;
    Widget_t *w_axis;
    Widget_t *h_axis;
    Widget_t *active_widget;
    Widget_t *prev_active_widget;
    Widget_t *w;
    Widget_t *ui;
    Widget_t *systray;
    Widget_t *systray_menu;
    Widget_t *settings;
    Widget_t *set_project;
    Widget_t *project_title;
    Widget_t *project_author;
    Widget_t *project_type;
    Widget_t *project_uri;
    Widget_t *project_ui_uri;
    Widget_t *project_audio_input;
    Widget_t *project_audio_output;
    Widget_t *project_midi_input;
    Widget_t *project_midi_output;
    Widget_t *project_bypass;
    Widget_t *controller_label;
    Widget_t *widgets;
    Widget_t *index;
    Widget_t *set_index;
    Widget_t *combobox_settings;
    Widget_t *add_entry;
    Widget_t *combobox_entry;
    Widget_t *controller_settings;
    Widget_t *controller_entry[4];
    Widget_t *tabbox_settings;
    Widget_t *tabbox_entry[2];
    Widget_t *set_adjust;
    Widget_t *lv2_uris;
    Widget_t *image_loader;
    Widget_t *unload_image;
    Widget_t *context_menu;
    Widget_t *menu_item_load;
    Widget_t *menu_item_unload;
    Widget_t *color_chooser;
    Widget_t *color_widget;
    Widget_t *color_sel;
    Widget_t *color_scheme_select;
    Widget_t *grid;
    Widget_t *grid_size_x;
    Widget_t *grid_size_y;
    Widget_t *grid_snap_select;
    Widget_t *grid_snap_left;
    Widget_t *grid_snap_center;
    Widget_t *grid_snap_right;
    Widget_t *ctype_switch;
    Widget_t *test;
    Widget_t *save;
    Widget_t *exit;
    Cursor cursor;
    Colors *selected_scheme;
    DragIcon drag_icon;
    bool run_test;
    bool grid_view;
    bool is_project;
    bool generate_ui_only;
    int active_widget_num;
    int pos_x;
    int pos_y;
    int width;
    int height;
    int grid_width;
    int grid_height;
    int modify_mod;
    bool run;
    int wid_counter;
    int select_widget_num;
    char** new_label;
    char** tab_label;
    char* image_path;
    char* image;
    LV2_CONTROLLER lv2c;
    Controller controls[MAX_CONTROLS];
} XUiDesigner;

void draw_window(void *w_, void* user_data);

void set_controller_callbacks(XUiDesigner *designer, Widget_t *wid, bool set_designer);

void fix_pos_wid(void *w_, void *button_, void* user_data);

void fix_pos_tab(void *w_, void *button_, void* user_data);

void set_pos_tab(void *w_, void *button_, void* user_data);

void move_tab(void *w_, void *xmotion_, void* user_data);

char *getUserName();

#ifdef __cplusplus
}
#endif

#endif //XUIDESIGNER_H_
