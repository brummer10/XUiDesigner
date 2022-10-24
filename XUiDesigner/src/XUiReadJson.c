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

#include "XUiReadJson.h"
#include "XUiGenerator.h"
#include "XUiWriteUI.h"
#include "XUiFileParser.h"
#include "XUiDraw.h"
#include "XUiImageLoader.h"
#include "XUiControllerType.h"

static char *trim(char *s) {
    while(isspace(*s)) s++;
    return s;
}

static char *get_string(const char *buf, char *s, char *e) {
    char *get = substr(buf, s, e);
    char *c = substr(get, "\"", "\"");
    free(get);
    return c;
}

static char *get_key(const char *buf, char *s, char *e) {
    char *get = substr(buf, s, e);
    return get;
}

static char *get_resource_path(char *path, char *image) {
    char *s = NULL;
    char *b = basename(image);
    asprintf(&s, "%sresources/%s", path, b);
    return s;
}

static void get_color (char *buf, double *c) {
    char *ptr = strtok(buf, "[");
    ptr = strtok(NULL, ",");
    if (ptr != NULL)
        c[0] = strtod(ptr, NULL);
    ptr = strtok(NULL, ",");
    if (ptr != NULL)
        c[1] = strtod(ptr, NULL);
    ptr = strtok(NULL, ",");
    if (ptr != NULL)
        c[2] = strtod(ptr, NULL);
    ptr = strtok(NULL, "]");
    if (ptr != NULL)
        c[3] = strtod(ptr, NULL);
}

static void restore_color(Colors *col, FILE *fp, char *buf) {
    double c[4] = {0.0, 0.0, 0.0, 1.0};
    while (fgets(buf, 128, fp) != NULL) {
        if (strstr(buf, "\".fg\"") != NULL) {
            get_color(buf, c);
            widget_set_color(col->fg, c[0], c[1], c[2], c[3]);
        } else if (strstr(buf, "\".bg\"") != NULL) {
            get_color(buf, c);
            widget_set_color(col->bg, c[0], c[1], c[2], c[3]);
        } else if (strstr(buf, "\".base\"") != NULL) {
            get_color(buf, c);
            widget_set_color(col->base, c[0], c[1], c[2], c[3]);
        } else if (strstr(buf, "\".text\"") != NULL) {
            get_color(buf, c);
            widget_set_color(col->text, c[0], c[1], c[2], c[3]);
        } else if (strstr(buf, "\".shadow\"") != NULL) {
            get_color(buf, c);
            widget_set_color(col->shadow, c[0], c[1], c[2], c[3]);
        } else if (strstr(buf, "\".light\"") != NULL) {
            get_color(buf, c);
            widget_set_color(col->light, c[0], c[1], c[2], c[3]);
            // light is last, so break here
            break;
        }
    }
}

static void parse_colors(XUiDesigner *designer, FILE *fp, char *buf) {
    while (fgets(buf, 128, fp) != NULL) {
        if (strstr(buf, "{") != NULL) {
            //fprintf(stderr, "Start color object\n");
        } else if  (strstr(buf, "\"NORMAL\"") != NULL) {
            Colors *col = &designer->ui->color_scheme->normal;
            restore_color(col, fp, buf);
        } else if  (strstr(buf, "\"PRELIGHT\"") != NULL) {
            Colors *col = &designer->ui->color_scheme->prelight;
            restore_color(col, fp, buf);
        } else if  (strstr(buf, "\"SELECTED\"") != NULL) {
            Colors *col = &designer->ui->color_scheme->selected;
            restore_color(col, fp, buf);
        } else if  (strstr(buf, "\"ACTIVE\"") != NULL) {
            Colors *col = &designer->ui->color_scheme->active;
            restore_color(col, fp, buf);
        } else if  (strstr(buf, "\"INSENSITIVE\"") != NULL) {
            Colors *col = &designer->ui->color_scheme->insensitive;
            restore_color(col, fp, buf);
            // insensitive is last, so break here
            break;
        }
    }
    
}

static CL_type parse_adjusment(const char* cl_type) {
    if (strstr(cl_type, "CL_NONE") != NULL) return CL_NONE;
    else if (strstr(cl_type, "CL_CONTINUOS") != NULL) return CL_CONTINUOS;
    else if (strstr(cl_type, "CL_TOGGLE") != NULL) return CL_TOGGLE;
    else if (strstr(cl_type, "CL_BUTTON") != NULL) return CL_BUTTON;
    else if (strstr(cl_type, "CL_ENUM") != NULL) return CL_ENUM;
    else if (strstr(cl_type, "CL_VIEWPORT") != NULL) return CL_VIEWPORT;
    else if (strstr(cl_type, "CL_METER") != NULL) return CL_METER;
    else if (strstr(cl_type, "CL_LOGARITHMIC") != NULL) return CL_LOGARITHMIC;
    else if (strstr(cl_type, "CL_LOGSCALE") != NULL) return CL_LOGSCALE;
    else if (strstr(cl_type, "CL_VIEWPORTSLIDER") != NULL) return CL_VIEWPORTSLIDER;
    return CL_NONE;
}

static void read_controller_color(Widget_t *wid, char *buf) {
    if (!wid) return;
    int state = 0, mod = 0;
    double c[4] = {0.0, 0.0, 0.0, 1.0};
    char *ptr = strtok(buf, "[");
    ptr = strtok(NULL, ",");
    if (ptr != NULL)
        state = (int)strtod(ptr, NULL);
    ptr = strtok(NULL, ",");
    if (ptr != NULL)
        mod = (int)strtod(ptr, NULL);
    ptr = strtok(NULL, ",");
    if (ptr != NULL)
        c[0] = strtod(ptr, NULL);
    ptr = strtok(NULL, ",");
    if (ptr != NULL)
        c[1] = strtod(ptr, NULL);
    ptr = strtok(NULL, ",");
    if (ptr != NULL)
        c[2] = strtod(ptr, NULL);
    ptr = strtok(NULL, "]");
    if (ptr != NULL)
        c[3] = strtod(ptr, NULL);
    set_widget_color(wid, state, mod, c[0], c[1], c[2], c[3]); 
}

void get_ui_elem(XUiDesigner *designer, Widget_t **wid, Widget_t **tab, FILE *fp, char *buf) {
    int elems = 0;
    int tabs = 0;
    char *type = NULL;
    char *label = NULL;
    int x = 1, y = 1, w = 1, h = 1;
    char *image = NULL;
    Widget_t *tabbox = NULL;
    Widget_t *win = NULL;
    while (fgets(buf, 128, fp) != NULL) {
        if (strstr(buf, "{") != NULL) {
            continue;
        } else if  (strstr(buf, "\"Type\"") != NULL) {
            type =  get_key(buf, ":", ",");
        } else if  (strstr(buf, "\"Label\"") != NULL) {
            label =  get_string(buf, ":", ",");
        } else if (strstr(buf, "\"Size\"") != NULL) {
            char *ptr = strtok(buf, "[");
            ptr = strtok(NULL, ",");
            if (ptr != NULL)
                x = (int)strtod(ptr, NULL);
            ptr = strtok(NULL, ",");
            if (ptr != NULL)
                y = (int)strtod(ptr, NULL);
            ptr = strtok(NULL, ",");
            if (ptr != NULL)
                w = (int)strtod(ptr, NULL);
            ptr = strtok(NULL, "]");
            if (ptr != NULL)
                h = (int)strtod(ptr, NULL);      
        } else if  (strstr(buf, "\"Image\"") != NULL) {
            image =  get_resource_path(designer->resource_path, get_string(buf, ":", ","));
            if (strstr(image, "None") != NULL) {
                free(image);
                image = NULL;
            }
        } else if (strstr(buf, "}") != NULL) {
            //fprintf(stderr, "Stop object\n");
        } else if (strstr(buf, "]") != NULL) {
            if (!type) continue;
            Widget_t *wi = designer->ui;
            asprintf(&designer->controls[designer->wid_counter].name, "%s", label);
            if (strstr(type, "\"add_lv2_frame\"") != NULL) {
                wid[elems] = add_frame(wi, designer->controls[designer->wid_counter].name, x, y, w, h);
                set_controller_callbacks(designer, wid[elems], true);
                adj_set_value(designer->index->adj, adj_get_value(designer->index->adj)-1.0);
                designer->controls[designer->wid_counter-1].port_index = -1;
                add_to_list(designer, wid[elems], "add_lv2_frame", false, IS_FRAME);
                wid[elems]->parent_struct = designer;
                free(designer->controls[wid[elems]->data].image);
                designer->controls[wid[elems]->data].image = NULL;
                wid[elems]->func.expose_callback = draw_frame;
                wid[elems]->func.enter_callback = null_callback;
                wid[elems]->func.leave_callback = null_callback;
                XLowerWindow(wi->app->dpy, wid[elems]->widget);
                win = wid[elems];
                free(type);
                type = NULL;
                elems++;

            } else if (strstr(type, "\"add_lv2_tabbox\"") != NULL) {
                tabbox = add_tabbox(wi, designer->controls[designer->wid_counter].name, x, y, w, h);
                set_controller_callbacks(designer, tabbox, true);
                adj_set_value(designer->index->adj, adj_get_value(designer->index->adj)-1.0);
                designer->controls[designer->wid_counter-1].port_index = -1;
                add_to_list(designer, tabbox, "add_lv2_tabbox", false, IS_TABBOX);
                tabbox->parent_struct = designer;
                free(designer->controls[tabbox->data].image);
                designer->controls[tabbox->data].image = NULL;
                tabbox->func.expose_callback = draw_tabbox;
                tabbox->func.enter_callback = null_callback;
                tabbox->func.leave_callback = null_callback;
                XLowerWindow(wi->app->dpy, tabbox->widget);
                win = tabbox;
                free(type);
                type = NULL;
                elems++;
            } else if (strstr(type, "\"add_lv2_tab\"") != NULL) {
                tab[tabs] = tabbox_add_tab(tabbox, label);
                tab[tabs]->parent_struct = designer;
                tab[tabs]->func.expose_callback = draw_tab;
                tab[tabs]->func.button_press_callback = set_pos_tab;
                tab[tabs]->func.button_release_callback = fix_pos_tab;
                tab[tabs]->func.motion_callback = move_tab;
                free(type);
                type = NULL;
                tabs++;

            } else if (strstr(type, "\"add_lv2_image\"") != NULL) {
                wid[elems] = add_image(wi, designer->controls[designer->wid_counter].name, x, y, w, h);
                wid[elems]->label = designer->controls[designer->wid_counter].name;
                set_controller_callbacks(designer, wid[elems], true);
                adj_set_value(designer->index->adj, adj_get_value(designer->index->adj)-1.0);
                designer->controls[designer->wid_counter-1].port_index = -1;
                add_to_list(designer, wid[elems], "add_lv2_image", false, IS_IMAGE);
                wid[elems]->parent_struct = designer;
                free(designer->controls[wid[elems]->data].image);
                designer->controls[wid[elems]->data].image = NULL;
                wid[elems]->func.expose_callback = draw_image;
                wid[elems]->func.enter_callback = null_callback;
                wid[elems]->func.leave_callback = null_callback;
                XLowerWindow(wi->app->dpy, wid[elems]->widget);
                win = wid[elems];
                free(type);
                type = NULL;
            } else {
                free(type);
                type = NULL;
                continue;
            }
            if (image != NULL) 
                load_single_controller_image(designer, image);
        } else if  (strstr(buf, "\"COLOR\"") != NULL) {
            read_controller_color(win, buf);
        }
    }
    free(type);
    free(label);
    free(image);
}

Widget_t *get_controller(XUiDesigner *designer, Widget_t *wid, Widget_t **elems, Widget_t **tabs, FILE *fp, char *buf) {
    char *type = NULL;
    char *label = NULL;
    int port = -1;
    char *symbol = NULL;
    int x = 1, y = 1, w = 1, h = 1;
    char *image = NULL;
    char *adjustment = NULL;
    int in_frame = 0;
    int in_tab = 0;
    int f = 0;
    int MIDI_PORT = -1;
    double std = 0.5, minvalue = 0.0, maxvalue = 1.0, stepsize = 0.01;
    char *entrys = NULL;
    Widget_t *wi = designer->ui;
    while (fgets(buf, 128, fp) != NULL) {
        if (strstr(buf, "{") != NULL) {
            continue;
        } else if  (strstr(buf, "\"Type\"") != NULL) {
            type =  get_key(buf, ":", ",");
        } else if  (strstr(buf, "\"Label\"") != NULL) {
            label =  get_string(buf, ":", ",");
        } else if  (strstr(buf, "\"Port\"") != NULL) {
            port =  (int)strtod(substr(buf, ":", ","), NULL);
        } else if  (strstr(buf, "\"Symbol\"") != NULL) {
            symbol =  get_string(buf, ":", ",");
        } else if (strstr(buf, "\"Size\"") != NULL) {
            char *ptr = strtok(buf, "[");
            ptr = strtok(NULL, ",");
            if (ptr != NULL)
                x = (int)strtod(ptr, NULL);
            ptr = strtok(NULL, ",");
            if (ptr != NULL)
                y = (int)strtod(ptr, NULL);
            ptr = strtok(NULL, ",");
            if (ptr != NULL)
                w = (int)strtod(ptr, NULL);
            ptr = strtok(NULL, "]");
            if (ptr != NULL)
                h = (int)strtod(ptr, NULL);      
        } else if  (strstr(buf, "\"Image\"") != NULL) {
            image =  get_resource_path(designer->resource_path, get_string(buf, ":", ","));
            if (strstr(image, "None") != NULL) {
                free(image);
                image = NULL;
            }
        } else if  (strstr(buf, "\"Parent\"") != NULL) {
            char *ptr = strtok(buf, "[");
            ptr = strtok(NULL, ",");
            if (ptr != NULL) {
                f = (int)strtod(ptr, NULL);
                wi = elems[f];
                in_frame = f+1;
            }
            ptr = strtok(NULL, "]");
            if (ptr != NULL) {
                int t = (int)strtod(ptr, NULL);
                if (t > -1) {
                    wi = tabs[t];
                    in_tab = t+1;
               }
            }
        } else if  (strstr(buf, "\"Adjustment\"") != NULL) {
            adjustment =  get_string(buf, ":", ",");
        } else if  (strstr(buf, "\"Default Value\"") != NULL) {
            std =  strtod(substr(buf, ":", ","), NULL);
        } else if  (strstr(buf, "\"Min Value\"") != NULL) {
            minvalue =  strtod(substr(buf, ":", ","), NULL);
        } else if  (strstr(buf, "\"Max Value\"") != NULL) {
            maxvalue =  strtod(substr(buf, ":", ","), NULL);
        } else if  (strstr(buf, "\"Step Size\"") != NULL) {
            stepsize =  strtod(substr(buf, ":", ","), NULL);
        } else if (strstr(buf, "\"Enums\"") != NULL) {
            entrys = get_key(buf, "[", "]");
        } else if (strstr(buf, "\"MIDIPORT\"") != NULL) {
            char *ptr = strtok(buf, ":");
            ptr = strtok(NULL, ",");
            if (ptr != NULL)
                MIDI_PORT = (int)strtod(ptr, NULL);
        } else if (strstr(buf, "}") != NULL) {
            //fprintf(stderr, "Stop object\n");
        } else if (strstr(buf, "]") != NULL) {            
            asprintf(&designer->controls[designer->wid_counter].name, "%s", label);
            if (strstr(type, "\"add_lv2_knob\"") != NULL) {
                wid = add_knob(wi, designer->controls[designer->wid_counter].name, x, y, w, h);
                set_controller_callbacks(designer, wid, true);
                add_to_list(designer, wid, "add_lv2_knob", true, IS_KNOB);
                wi = designer->ui;
            } else if (strstr(type, "\"add_lv2_hslider\"") != NULL) {
                wid = add_hslider(wi, designer->controls[designer->wid_counter].name, x, y, w, h);
                set_controller_callbacks(designer, wid, true);
                add_to_list(designer, wid, "\"add_lv2_hslider\"", true, IS_HSLIDER);
                wi = designer->ui;
            } else if (strstr(type, "\"add_lv2_vslider\"") != NULL) {
                wid = add_vslider(wi, designer->controls[designer->wid_counter].name, x, y, w, h);
                set_controller_callbacks(designer, wid, true);
                add_to_list(designer, wid, "\"add_lv2_vslider\"", true, IS_VSLIDER);
                wi = designer->ui;
            } else if (strstr(type, "\"add_lv2_button\"") != NULL) {
                fprintf(stderr, "add_lv2_button \n");
                wid = add_button(wi, designer->controls[designer->wid_counter].name, x, y, w, h);
                set_controller_callbacks(designer, wid, true);
                add_to_list(designer, wid, "add_lv2_button", false, IS_BUTTON);
                wi = designer->ui;
            } else if (strstr(type, "\"add_lv2_image_button\"") != NULL) {
                wid = add_image_button(wi, designer->controls[designer->wid_counter].name, x, y, w, h);
                set_controller_callbacks(designer, wid, true);
                add_to_list(designer, wid, "add_lv2_image_button", false, IS_IMAGE_BUTTON);
                wi = designer->ui;
            } else if (strstr(type, "\"add_lv2_toggle_button\"") != NULL) {
                wid = add_toggle_button(wi, designer->controls[designer->wid_counter].name, x, y, w, h);
                set_controller_callbacks(designer, wid, true);
                add_to_list(designer, wid, "add_lv2_toggle_button", false, IS_TOGGLE_BUTTON);
                wi = designer->ui;
            } else if (strstr(type, "\"add_lv2_image_toggle\"") != NULL) {
                wid = add_switch_image_button(wi, designer->controls[designer->wid_counter].name, x, y, w, h);
                set_controller_callbacks(designer, wid, true);
                add_to_list(designer, wid, "add_lv2_image_toggle", false, IS_IMAGE_TOGGLE);
                wi = designer->ui;
            } else if (strstr(type, "\"add_lv2_combobox\"") != NULL) {
                wid = add_combobox(wi, designer->controls[designer->wid_counter].name, x, y, w, h);
                set_controller_callbacks(designer, wid, true);
                add_to_list(designer, wid, "add_lv2_combobox", true, IS_COMBOBOX);
                char *ptr = strtok(entrys, ",");
                while(ptr != NULL) {
                    strdecode(ptr, "\"", "");
                    combobox_add_entry(wid, trim(ptr));
                    ptr = strtok(NULL, ",");
                }
                wi = designer->ui;
            } else if (strstr(type, "\"add_lv2_valuedisplay\"") != NULL) {
                wid = add_valuedisplay(wi, designer->controls[designer->wid_counter].name, x, y, w, h);
                set_controller_callbacks(designer, wid, true);
                add_to_list(designer, wid, "add_lv2_valuedisplay", true, IS_VALUE_DISPLAY);
                wi = designer->ui;
            } else if (strstr(type, "\"add_lv2_label\"") != NULL) {
                wid = add_label(wi, designer->controls[designer->wid_counter].name, x, y, w, h);
                set_controller_callbacks(designer, wid, true);
                add_to_list(designer, wid, "add_lv2_label", false, IS_LABEL);
                wi = designer->ui;
            } else if (strstr(type, "\"add_lv2_vmeter\"") != NULL) {
                wid = add_vmeter(wi, designer->controls[designer->wid_counter].name, false, x, y, w, h);
                set_controller_callbacks(designer, wid, true);
                add_to_list(designer, wid, "add_lv2_vmeter", true, IS_VMETER);
                wi = designer->ui;
            } else if (strstr(type, "\"add_lv2_hmeter\"") != NULL) {
                wid = add_hmeter(wi, designer->controls[designer->wid_counter].name, false, x, y, w, h);
                set_controller_callbacks(designer, wid, true);
                add_to_list(designer, wid, "add_lv2_hmeter", true, IS_HMETER);
                wi = designer->ui;
            } else if (strstr(type, "\"add_lv2_waveview\"") != NULL) {
                wid = add_waveview(wi, designer->controls[designer->wid_counter].name, x, y, w, h);
                set_controller_callbacks(designer, wid, true);
                add_to_list(designer, wid, "add_lv2_waveview", false, IS_WAVEVIEW);
                float v[9] = { 0.0,-0.5, 0.0, 0.5, 0.0, -0.5, 0.0, 0.5, 0.0};
                update_waveview(wid, &v[0],9);
                wi = designer->ui;
            } else if (strstr(type, "\"add_lv2_midikeyboard\"") != NULL) {
                wid = add_midi_keyboard(wi, designer->controls[designer->wid_counter].name, x, y, w, h);
                set_controller_callbacks(designer, wid, true);
                designer->controls[wid->data].is_midi_patch = true;
                designer->lv2c.midi_input = 1;
                designer->MIDIPORT = MIDI_PORT;
                wid->func.enter_callback = null_callback;
                wid->func.leave_callback = null_callback;
                add_to_list(designer, wid, "add_lv2_midikeyboard", false, IS_MIDIKEYBOARD);
                wi = designer->ui;
            } else {
                continue;
            }
            if (image != NULL) 
                load_single_controller_image(designer, image);
            if (symbol != NULL) {
                free(designer->controls[designer->active_widget_num].symbol);
                designer->controls[designer->active_widget_num].symbol = NULL;
                asprintf (&designer->controls[designer->active_widget_num].symbol, "%s",symbol);
            }
            if (adjustment != NULL) {
                set_adjustment(wid->adj,std, std, minvalue, maxvalue, stepsize, parse_adjusment(adjustment));
            }
            if (port != -1) {
                designer->controls[designer->active_widget_num].port_index = port;
            }
            if (in_frame) {
                designer->controls[wid->data].in_frame = in_frame;
                in_frame = 0;
            }
            if (in_tab) {
                designer->controls[wid->data].in_tab = in_tab;
                in_tab = 0;
            }
            break;
        }
    }
    free(type);
    free(label);
    free(symbol);
    free(image);
    free(adjustment);
    free(entrys);
    return wid;
}

void read_json(XUiDesigner *designer, const char *filename) {
    char* tmp = strdup(filename);
    free(designer->json_file_path);
    designer->json_file_path = NULL;
    asprintf(&designer->json_file_path, "%s/", dirname(dirname(tmp)));
    free(tmp);
    tmp = NULL;
    tmp = strdup(filename);
    free(designer->resource_path);
    designer->resource_path = NULL;
    asprintf(&designer->resource_path, "%s/", dirname(tmp));
    free(tmp);
    tmp = NULL;
    
    char* ui_image = NULL;
    char buf[128];
    static bool r = true;
    FILE *fp;
    Widget_t *wid = NULL;
    Widget_t **ui_elems;
    ui_elems = (Widget_t**)malloc(sizeof(Widget_t*) * 25);
    int i = 0;
    for(;i<25;i++) {
        ui_elems[i] = NULL;
    }
    Widget_t **ui_tabs;
    ui_tabs = (Widget_t**)malloc(sizeof(Widget_t*) * 25);
    i = 0;
    for(;i<25;i++) {
        ui_tabs[i] = NULL;
    }
    if((fp = fopen(filename, "r")) == NULL) {
        printf("Error opening pipe!\n");
        return;
    }
    designer->ui->flags |= FAST_REDRAW;
    while (fgets(buf, 128, fp) != NULL) {
        if (strstr(buf, "{") != NULL) {
            //fprintf(stderr, "Start object\n");
            continue;
        } else if (strstr(buf, "\"Project\"") != NULL) {
            designer->lv2c.ui_uri = get_string(buf, ":", ",");
        } else if (strstr(buf, "\"Name\"") != NULL) {
            designer->lv2c.name = get_string(buf, ":", ",");
            widget_set_title(designer->ui, designer->lv2c.name);
        } else if (strstr(buf, "\"Author\"") != NULL) {
            designer->lv2c.author = get_string(buf, ":", ",");
        } else if (strstr(buf, "\"Window size\"") != NULL) {
            designer->ui->width = (int)strtod(substr(buf, "[", ","), NULL);
            designer->ui->height = (int)strtod(substr(buf, ",", "]"), NULL);
            XResizeWindow(designer->ui->app->dpy, designer->ui->widget, designer->ui->width, designer->ui->height+1);
        } else if (strstr(buf, "\"Image\"") != NULL) {
            ui_image = get_resource_path(designer->resource_path, get_string(buf, ":", ","));
            //asprintf(&ui_image, "%s", get_string(buf, ":", ","));
            image_load_response(designer->image_loader, (void*)&ui_image);
        } else if (strstr(buf, "IS_") != NULL) {
            if (r) { // don't read again after fseek the read pointer
                long saved = ftell(fp);
                get_ui_elem(designer, ui_elems, ui_tabs, fp, buf);
                fseek(fp, saved, SEEK_SET);
                r = false;
            }
            wid = get_controller(designer, wid, ui_elems, ui_tabs, fp, buf);
        } else if (strstr(buf, "\"Colors\"") != NULL) {
            parse_colors(designer, fp, buf);
        } else if  (strstr(buf, "\"COLOR\"") != NULL) {
            read_controller_color(wid, buf);
        } else if (strstr(buf, "}") != NULL) {
            //fprintf(stderr, "Stop object\n");
        }
    }

    r = true; // reset static bool to allow rescan of json file
    free(ui_elems);
    free(ui_tabs);
    free(ui_image);
    designer->is_json_file = true;
    designer->is_project = false;
    XResizeWindow(designer->ui->app->dpy, designer->ui->widget, designer->ui->width, designer->ui->height-1);
    pthread_t rf;
    pthread_create(&rf, NULL, reset_flag, (void *)designer);
    if (fclose(fp)) {
        printf("Command not found or exited with error status\n");
        free(tmp);
        return;
    }
}
