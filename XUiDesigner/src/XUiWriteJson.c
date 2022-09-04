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

#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <libgen.h>
#include <ctype.h>

#include "XUiWriteJson.h"
#include "XUiGenerator.h"
#include "XUiWriteUI.h"

bool need_comma = false;
bool need_tab = false;

static const char* parse_type(WidgetType is_type) {
    switch(is_type) {
        case IS_KNOB:
        return "IS_KNOB";
        break;
        case IS_HSLIDER:
        return "IS_HSLIDER";
        break;
        case IS_VSLIDER:
        return "IS_VSLIDER";
        break;
        case IS_BUTTON :
        return "IS_BUTTON";
        break;
        case IS_TOGGLE_BUTTON:
        return "IS_TOGGLE_BUTTON";
        break;
        case IS_COMBOBOX:
        return "IS_COMBOBOX";
        break;
        case IS_VALUE_DISPLAY:
        return "IS_VALUE_DISPLAY";
        break;
        case IS_LABEL:
        return "IS_LABEL";
        break;
        case IS_VMETER:
        return "IS_VMETER";
        break;
        case IS_HMETER:
        return "IS_HMETER";
        break;
        case IS_WAVEVIEW:
        return "IS_WAVEVIEW";
        break;
        case IS_FRAME:
        return "IS_FRAME";
        break;
        case IS_TABBOX :
        return "IS_TABBOX";
        break;
        case IS_IMAGE :
        return "IS_IMAGE";
        break;
        // keep those below
        case IS_FILE_BUTTON:
        return "IS_FILE_BUTTON";
        break;
        case IS_IMAGE_TOGGLE:
        return "IS_IMAGE_TOGGLE";
        break;
        default:
        return "";
    } 
}

static void json_start_object (char *s) {
    printf ("{\n  \"%s\" :", s);
    need_comma = false;
}

static void json_start_array(void) {
    printf (" [");
    need_comma = false;
}

static void json_close_array(void) {
    printf ("] ");
    need_comma = true;
}

static void json_start_value_pair(void) {
    printf ("\n    {\n");
    need_tab = true;
}

static void json_close_value_pair(void) {
    printf ("\n    }\n  ");
    need_tab = false;
}

static void json_add_int(int i) {
    printf ("%s %i ", need_comma ? "," : "", i);
    need_comma = true;
}

static void json_add_float(float f) {
    printf ("%s %f ", need_comma ? "," : "", f);
    need_comma = true;
}

static void json_add_string(const char* s) {
    printf ("%s \"%s\"", need_comma ? "," : "", s);
    need_comma = true;
}

static void json_add_key(const char* s) {
    printf ("%s%s  \"%s\" :", need_comma ? ",\n" : "", need_tab ? "    " : "", s);
    need_comma = false;
}

static void json_close_object(void) {
    printf ("\n}\n");
    need_comma = false;
}

void print_json(XUiDesigner *designer) {
    char *name = NULL;
    XFetchName(designer->ui->app->dpy, designer->ui->widget, &name);
    if (name == NULL) asprintf(&name, "%s", "noname");

    int i = 0;
    int j = 0;
    for (;i<MAX_CONTROLS;i++) {
        if (designer->controls[i].wid != NULL && (designer->controls[i].is_type != IS_FRAME &&
                                                designer->controls[i].is_type != IS_TABBOX &&
                                                !designer->controls[i].is_audio_input &&
                                                !designer->controls[i].is_audio_output &&
                                                !designer->controls[i].is_atom_input &&
                                                !designer->controls[i].is_atom_output)) {
            j++;
        }
    }
    json_start_object ("Project");
    json_add_string(designer->lv2c.ui_uri);

    json_add_key ("Name");
    json_add_string(name);

    json_add_key ("Author");
    json_add_string(designer->lv2c.author);

    json_add_key ("Window size");
    json_start_array();
    json_add_int(designer->ui->width);
    json_add_int(designer->ui->height);
    json_close_array();
    
    json_add_key ("Contolls");
    json_add_int(j);

    i = 0;
    for (;i<MAX_CONTROLS;i++) {
        if (designer->controls[i].wid != NULL) {
            if (designer->controls[i].is_type == IS_FRAME) {
                json_add_key ("Frame Box");
                json_start_array();
                json_start_value_pair();
                json_add_key ("Type");
                json_add_string(designer->controls[i].type);
                json_add_key ("Label");
                json_add_string(designer->controls[i].wid->label);
                json_add_key ("Size");
                json_start_array();
                json_add_int(designer->controls[i].wid->x);
                json_add_int(designer->controls[i].wid->y);
                json_add_int(designer->controls[i].wid->width);
                json_add_int(designer->controls[i].wid->height);
                json_close_array();
                json_add_key ("Image");
                if (designer->controls[i].image != NULL ) {
                    json_add_string(designer->controls[i].image);
                } else {
                    json_add_string("");
                }
                json_close_value_pair();
                json_close_array();
            } else if (designer->controls[i].is_type == IS_TABBOX) {
                json_add_key ("TAB Box");
                json_start_array();
                json_start_value_pair();
                json_add_key ("Type");
                json_add_string(designer->controls[i].type);
                json_add_key ("Label");
                json_add_string(designer->controls[i].wid->label);
                json_add_key ("Size");
                json_start_array();
                json_add_int(designer->controls[i].wid->x);
                json_add_int(designer->controls[i].wid->y);
                json_add_int(designer->controls[i].wid->width);
                json_add_int(designer->controls[i].wid->height);
                json_close_array();
                json_add_key ("Image");
                if (designer->controls[i].image != NULL ) {
                    json_add_string(designer->controls[i].image);
                } else {
                    json_add_string("None");
                }
                int elem = designer->controls[i].wid->childlist->elem;
                if (elem) {
                    json_add_key ("TAB Box item");
                }
                int t = 0;
                for(;t<elem;t++) {
                    Widget_t *wi = designer->controls[i].wid->childlist->childs[t];
                    json_start_array();
                    json_start_value_pair();
                    json_add_key ("Type");
                    json_add_string(designer->controls[i].type);
                    json_add_key ("Label");
                    json_add_string(designer->controls[i].wid->label);
                    json_add_key ("Size");
                    json_start_array();
                    json_add_int(wi->x);
                    json_add_int(wi->y);
                    json_add_int(wi->width);
                    json_add_int(wi->height);
                    json_close_array();
                    json_close_value_pair();
                    json_close_array();
                }
                json_close_value_pair();
                json_close_array();
            } else if (!designer->controls[i].is_audio_output && !designer->controls[i].is_audio_input &&
                !designer->controls[i].is_atom_output && !designer->controls[i].is_atom_input) {
                json_add_key (parse_type(designer->controls[i].is_type));
                json_start_array();
                json_start_value_pair();
                json_add_key ("Type");
                json_add_string(designer->controls[i].type);
                json_add_key ("Label");
                json_add_string(designer->controls[i].wid->label);
                json_add_key ("Port");
                json_add_int(designer->controls[i].port_index);
                json_add_key ("Symbol");
                json_add_string(designer->controls[i].symbol);
                json_add_key ("Size");
                json_start_array();
                json_add_int(designer->controls[i].wid->x);
                json_add_int(designer->controls[i].wid->y);
                json_add_int(designer->controls[i].wid->width);
                json_add_int(designer->controls[i].wid->height);
                json_close_array();
                json_add_key ("Image");
                if (designer->controls[i].image != NULL ) {
                    json_add_string(designer->controls[i].image);
                } else {
                    json_add_string("None");
                }
                if (designer->controls[i].have_adjustment) {
                    json_add_key ("Adjustment");
                    json_add_string(parse_adjusment_type(designer->controls[i].wid->adj->type));
                    json_add_key ("Default Value");
                    json_add_float(adj_get_std_value(designer->controls[i].wid->adj));
                    json_add_key ("Min Value");
                    json_add_float(adj_get_min_value(designer->controls[i].wid->adj));
                    json_add_key ("Max Value");
                    json_add_float(adj_get_max_value(designer->controls[i].wid->adj));
                    json_add_key ("Step Size");
                    json_add_float(designer->controls[i].wid->adj->step);
                }
                if (designer->controls[i].is_type == IS_COMBOBOX) {
                    Widget_t *menu = designer->controls[i].wid->childlist->childs[1];
                    Widget_t* view_port =  menu->childlist->childs[0];
                    ComboBox_t *comboboxlist = (ComboBox_t*)view_port->parent_struct;
                    json_add_key ("Enums");
                    json_start_array();
                    unsigned int k = 0;
                    for(; k<comboboxlist->list_size;k++) {
                        json_add_string(comboboxlist->list_names[k]);
                    }
                    json_close_array();
                }
                json_close_value_pair();
                json_close_array();
            }
        }
    }

    json_close_object();
}
