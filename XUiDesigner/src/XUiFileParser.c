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


#include "XUiFileParser.h"
#include "XUiGenerator.h"
#include "XUiImageLoader.h"
#include "XUiLv2Parser.h"
#include "XUiTurtleView.h"
#include "XUiWritePlugin.h"
#include "XUiReadJson.h"

char *substr(const char *str, const char *p1, const char *p2) {
    const char *i1 = strstr(str, p1);
    if(i1 != NULL) {
        const size_t pl1 = strlen(p1);
        const char *i2 = strstr(i1 + pl1, p2);
        if(p2 != NULL) {
            /* Found both markers, extract text. */
            const size_t mlen = i2 - (i1 + pl1);
            char *ret = malloc(mlen + 1);
            if(ret != NULL) {
                memcpy(ret, i1 + pl1, mlen);
                ret[mlen] = '\0';
                return ret;
            }
        }
    }
    return NULL;
}
/*---------------------------------------------------------------------
-----------------------------------------------------------------------    
                load faust dsp file
-----------------------------------------------------------------------
----------------------------------------------------------------------*/

static bool check_synth(XUiDesigner *designer, const char* filename) {
    bool is_synth = false;
    char* cmd = NULL;
    char * voices = NULL;
    char* tmp = strdup(filename);
    char buf[128];
    FILE *fp;
    if((fp = fopen(tmp, "r")) == NULL) {
        printf("Error opening pipe!\n");
        return false;
    }
    while (fgets(buf, 128, fp) != NULL) {
        if (strstr(buf, "[midi:on]") != NULL) {
            fprintf(stderr, "is synth\n");
            is_synth = true;
        } else if (strstr(buf, "[nvoices:") != NULL) {
            voices = substr(buf, ":", "]");
            fprintf(stderr, "is %s nvoices synth\n", voices); //declare options "[nvoices:6]";
            is_synth = true;
            break;
        }
    }
    if (fclose(fp)) {
        printf("Command not found or exited with error status\n");
        free(voices);
        return false;
    }
    if (is_synth) {
        if (voices == NULL) voices = strdup("8");
        char *tmp2 = strdup(tmp);
        char *b = basename(tmp2);
        asprintf(&cmd, "cp %s /tmp/%s", tmp, b);
        int ret = system(cmd);
        free(cmd);
        cmd = NULL;
        asprintf(&cmd, "faust2lv2 -nvoices %s -keep /tmp/%s", voices, b);
        ret = system(cmd);
        if (ret) {
            open_message_dialog(designer->ui, ERROR_BOX, "",
                "Fail to parse faust file", NULL);        
            free(voices);
            free(tmp);
            return false;
        }
        if (designer->world) {
            lilv_world_free(designer->world);
            designer->world = NULL;
        }
        strdecode(b, ".dsp", ".cpp");
        strdecode(tmp, ".dsp", "");
        char *folder = basename(tmp);
        free(designer->faust_synth_voices);
        designer->faust_synth_voices = NULL;
        asprintf(&designer->faust_synth_voices, "%s", voices);
        free(designer->faust_synth_file);
        designer->faust_synth_file = NULL;
        asprintf(&designer->faust_synth_file, "/tmp/%s/%s", folder, b);
        designer->is_faust_synth_file = true;

        combobox_delete_entrys(designer->lv2_uris);
        combobox_delete_entrys(designer->lv2_names);
        combobox_add_entry(designer->lv2_uris,_("--"));
        combobox_add_entry(designer->lv2_names,_("--"));
        designer->world = lilv_world_new();
        free(designer->path);
        designer->path = NULL;
        asprintf(&designer->path, "/tmp/%s", folder);
        load_lv2_uris (designer);
        combobox_set_active_entry(designer->lv2_uris, 1);
        combobox_set_active_entry(designer->lv2_names, 1);

        free(designer->lv2c.ui_uri);
        designer->lv2c.ui_uri = NULL;
        asprintf(&designer->lv2c.ui_uri, "urn:%s:%s%s", getUserName(), folder,"_ui");

        free(voices);
        voices = NULL;
        free(cmd);
        cmd = NULL;
        free(tmp2);
        tmp2 = NULL;
    }
    free(tmp);
    return is_synth;
}

void parse_faust_file (XUiDesigner *designer, const char* filename) {
    if (check_synth(designer, filename)) return;
    char* cmd = NULL;
    char* tmp = strdup(filename);
    strdecode(tmp, ".dsp", ".cc");
    char* outname = NULL;
    asprintf(&outname, "/tmp/%s", basename(tmp));
    free(tmp);
    tmp = NULL;
    tmp = strdup(filename);
    free(designer->faust_path);
    designer->faust_path = NULL;
    asprintf(&designer->faust_path, "%s/",dirname(tmp));
    free(tmp);
    tmp = NULL;    
    if (access("./tools/dsp2cc", F_OK) == 0) {
        asprintf(&cmd, "./tools/dsp2cc -d %s -b -o %s", filename, outname);
    } else {
        asprintf(&cmd, "dsp2cc -d %s -b -o %s", filename, outname);
    }

    int ret = system(cmd);
    if (ret) {
        open_message_dialog(designer->ui, ERROR_BOX, "",
            "Fail to parse faust file", NULL);        
        free(outname);
        outname = NULL;
        free(cmd);
        cmd = NULL;
        return;
    }
    free(designer->faust_file);
    designer->faust_file = NULL;
    asprintf(&designer->faust_file, "%s", outname);
    free(cmd);
    cmd = NULL;
    char buf[128];
    FILE *fp;
    asprintf(&cmd, "cat %s | sed -n '/enum/,/PortIndex/p' |  sed '/enum/d;/PortIndex/d;/{/d;/}/d'", outname);
    if((fp = popen(cmd, "r")) == NULL) {
        printf("Error opening pipe!\n");
        return;
    }
    int p = 1;
    int j = 0;
    designer->lv2c.audio_input = 0;
    designer->lv2c.audio_output = 0;
    designer->ui->flags |= FAST_REDRAW;
    while (fgets(buf, 128, fp) != NULL) {
        if (strstr(buf, "input") != NULL) {
            designer->lv2c.audio_input += 1;
            j++;
        } else if (strstr(buf, "output") != NULL) {
            designer->lv2c.audio_output += 1;
            j++;
        } else if (strstr(buf, "bypass") != NULL) {
            designer->lv2c.bypass = 1;
            if (designer->lv2c.bypass) {
                Widget_t *wid = add_toggle_button(designer->ui, "Bypass", 60*p, 60, 60, 60);
                set_controller_callbacks(designer, wid, true);
                designer->controls[designer->active_widget_num].destignation_enabled = true;
                add_to_list(designer, wid, "add_lv2_toggle_button", false, IS_TOGGLE_BUTTON);
                designer->controls[designer->active_widget_num].port_index = j;
                if (designer->global_switch_image_file != NULL && adj_get_value(designer->global_switch_image->adj))
                    load_single_controller_image(designer, designer->global_switch_image_file);
                designer->prev_active_widget = wid;
                p++;
                j++;
            }
        } else {
            char *ptr = strtok(buf, ",");
            strdecode(ptr, " ", "");
            char *label;
            float v[8] = {0,0,0,0};
            int i = 0;
            asprintf(&label, "%s", ptr);
            while(ptr != NULL) {
                ptr = strtok(NULL, ",");
                if (ptr != NULL) {
                    if (strstr(ptr, "//") == NULL) {
                        v[i] = strtod(ptr, NULL);
                        i++;
                    }
                }
            }
            
            asprintf(&designer->controls[designer->wid_counter].name, "%s", label);
            Widget_t *wid = add_knob(designer->ui, designer->controls[designer->wid_counter].name, 60*p + 10*p, 60, 60, 80);
            set_adjustment(wid->adj, v[0], v[0], v[1], v[2], v[3], CL_CONTINUOS);
            set_controller_callbacks(designer, wid, true);
            tooltip_set_text(wid, wid->label);
            add_to_list(designer, wid, "add_lv2_knob", true, IS_KNOB);
            designer->controls[designer->active_widget_num].port_index = j;
            if (designer->global_knob_image_file != NULL && adj_get_value(designer->global_knob_image->adj)) 
                load_single_controller_image(designer, designer->global_knob_image_file);
            p++;
            j++;

            free(label);
            label = NULL;
        }
        //printf("OUTPUT: %s", buf);
    }
    if (pclose(fp)) {
        printf("Command not found or exited with error status\n");
        return;
    }
    designer->ui->width = min(1200, 60 + 70*p);
    designer->ui->height = 120;
    XResizeWindow(designer->ui->app->dpy, designer->ui->widget, designer->ui->width, designer->ui->height);

    strdecode(outname, ".cc", "");
    widget_set_title(designer->ui,basename(outname));
    free(designer->lv2c.ui_uri);
    designer->lv2c.ui_uri = NULL;
    asprintf(&designer->lv2c.ui_uri, "urn:%s:%s%s", getUserName(), basename(outname),"_ui");
    free(designer->lv2c.uri);
    designer->lv2c.uri = NULL;
    asprintf(&designer->lv2c.uri, "urn:%s:%s", getUserName(), basename(outname));
    designer->is_faust_file = true;
    free(cmd);
    cmd = NULL;    
    free(outname);
    outname = NULL;
    if (!designer->ttlfile_view) create_text_view_window(designer);
    XWindowAttributes attrs;
    XGetWindowAttributes(designer->ttlfile_view->app->dpy, (Window)designer->ttlfile_view->widget, &attrs);
    if (attrs.map_state == IsViewable) {
        run_generate_ttl(designer->ttlfile, NULL);
    }
    pthread_t rf;
    pthread_create(&rf, NULL, reset_flag, (void *)designer);
    //print_ttl(designer);
    //print_plugin(designer);
    //print_makefile(designer);
}

static void parse_c_file (XUiDesigner *designer, char* filename) {
    fprintf(stderr, "Parse C file\n");
    char buf[128];
    FILE *fp;
    if((fp = fopen(filename, "r")) == NULL) {
        printf("Error opening pipe!\n");
        return;
    }

    designer->lv2c.audio_input = 0;
    designer->lv2c.audio_output = 0;
    while (fgets(buf, 128, fp) != NULL) {
        if (strstr(buf, "input") != NULL) {
            designer->lv2c.audio_input += 1;
        } else if (strstr(buf, "output") != NULL) {
            designer->lv2c.audio_output += 1;
        }
        fprintf(stderr, "%s", buf);
    }
  
    if (fclose(fp)) {
        printf("Command not found or exited with error status\n");
        return;
    }
}

void dnd_load_response(void *w_, void* user_data) {
    if(user_data !=NULL) {
        Widget_t *w = (Widget_t*)w_;
        XUiDesigner *designer = (XUiDesigner*)w->parent_struct;
        reset_plugin_ui(designer);
        char* dndfile = NULL;
        dndfile = strtok(*(char**)user_data, "\r\n");
        while (dndfile != NULL) {
            if (strstr(dndfile, ".dsp") ) {
                parse_faust_file (designer, dndfile);
            } else if (strstr(dndfile, ".c") ) {
                parse_c_file (designer, dndfile);
            } else if (strstr(dndfile, ".json") ) {
                read_json (designer, dndfile);
            }
            dndfile = strtok(NULL, "\r\n");
        }
    }
}
