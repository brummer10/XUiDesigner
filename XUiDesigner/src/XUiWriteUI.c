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

#include "XUiWriteUI.h"
#include "XUiGenerator.h"


/*---------------------------------------------------------------------
-----------------------------------------------------------------------    
                parse adjustment type
-----------------------------------------------------------------------
----------------------------------------------------------------------*/

const char* parse_adjusment_type(CL_type cl_type) {
    switch(cl_type) {
        case CL_NONE:            return "CL_NONE";
        break;
        case CL_CONTINUOS:       return "CL_CONTINUOS";
        break;
        case CL_TOGGLE:          return "CL_TOGGLE";
        break;
        case CL_BUTTON:          return "CL_BUTTON";
        break;
        case CL_ENUM:            return "CL_ENUM";
        break;
        case CL_VIEWPORT:        return "CL_VIEWPORT";
        break;
        case CL_METER:           return "CL_METER";
        break;
        case CL_LOGARITHMIC:     return "CL_LOGARITHMIC";
        break;
        case CL_LOGSCALE:        return "CL_LOGSCALE";
        break;
        case CL_VIEWPORTSLIDER:  return "CL_VIEWPORTSLIDER";
        break;
        default: return NULL;
    }
}


/*---------------------------------------------------------------------
-----------------------------------------------------------------------    
                    png2c generate C file with cairo data
-----------------------------------------------------------------------
----------------------------------------------------------------------*/

void png2c(char* image_name, char* filepath) {
    cairo_surface_t *image = cairo_image_surface_create_from_png(image_name);
    int w = cairo_image_surface_get_width(image);
    int h = cairo_image_surface_get_height(image);
    int stride = cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, w);

    char * xld = NULL;
    char * xldl = NULL;
    char* tmp = strdup(image_name);
    asprintf(&xld, "%s", basename(tmp));
    free(tmp);
    tmp = NULL;
    strdecode(xld, ".png", ".c");
    strdecode(xld, "-", "_");
    strdecode(xld, " ", "_");
    asprintf(&xldl, "%s/%s",filepath, xld);
    FILE *fp;
    fp = fopen(xldl, "w");
    if (fp == NULL) {
        fprintf(stderr, "can't open file %s\n", xldl);
        return ;
    }

    const unsigned char *buff = cairo_image_surface_get_data(image);
    strdecode(xld, ".c", "");
    strtoguard(xld);
    fprintf(fp,
        "\n#pragma once\n\n"
        "#ifndef %s_H_\n"
        "#define %s_H_\n\n"
        "#ifdef __cplusplus\n"
        "extern \"C\" {\n"
        "#endif\n", xld, xld);

    strtovar(xld);
    fprintf(fp, "\n\n"
        "static const unsigned char %s_data[] = {\n", xld);
    int i = 0;
    int j = 0;
    int k = 1;
    int c = 4;
    for (; i < w * h * c; i++) {
        if (j == 15 ) {
            fprintf(fp, "0x%02x,\n", buff[i]);
            j = 0;
            k = 0;
        } else if (k == c ){
            fprintf(fp, "0x%02x,  ", buff[i]);
            j++;
            k = 0;
        } else if (j == 0) {
            fprintf(fp, "    0x%02x, ", buff[i]);
            j++;
        } else {
            fprintf(fp, "0x%02x, ", buff[i]);
            j++;
        }
        k++;
    }
    fprintf(fp, "};\n\n");
    fprintf(fp, "CairoImageData %s = (CairoImageData) {\n"
        "    .stride = %i,\n"
        "    .width  = %i,\n"
        "    .height = %i,\n"
        "    .data = (unsigned char*)%s_data,\n};\n\n",xld, stride, w, h, xld);

    fprintf(fp, "#ifdef __cplusplus\n"
    "}\n"
    "#endif\n"
    "#endif\n");

    fclose(fp);
    free(xld);
    free(xldl);
    cairo_surface_destroy(image);
}


/*---------------------------------------------------------------------
-----------------------------------------------------------------------    
                print color theme
-----------------------------------------------------------------------
----------------------------------------------------------------------*/

static void print_colors(XUiDesigner *designer) {
    //Xputty * main = designer->w->app;
    Colors *c = &designer->ui->color_scheme->normal;
    printf (
    "void set_costum_theme(Widget_t *w) {\n"
    "    w->color_scheme->normal = (Colors) {\n"
    "         /* cairo    / r  / g  / b  / a  /  */\n"
    "        .fg =       { %.3f, %.3f, %.3f, %.3f},\n"
    "        .bg =       { %.3f, %.3f, %.3f, %.3f},\n"
    "        .base =     { %.3f, %.3f, %.3f, %.3f},\n"
    "        .text =     { %.3f, %.3f, %.3f, %.3f},\n"
    "        .shadow =   { %.3f, %.3f, %.3f, %.3f},\n"
    "        .frame =    { %.3f, %.3f, %.3f, %.3f},\n"
    "        .light =    { %.3f, %.3f, %.3f, %.3f}\n"
    "    };\n\n",c->fg[0],c->fg[1],c->fg[2],c->fg[3],
                c->bg[0],c->bg[1],c->bg[2],c->bg[3],
                c->base[0],c->base[1],c->base[2],c->base[3],
                c->text[0],c->text[1],c->text[2],c->text[3],
                c->shadow[0],c->shadow[1],c->shadow[2],c->shadow[3],
                c->frame[0],c->frame[1],c->frame[2],c->frame[3],
                c->light[0],c->light[1],c->light[2],c->light[3]);

    c = &designer->ui->color_scheme->prelight;
    printf (
    "    w->color_scheme->prelight = (Colors) {\n"
    "         /* cairo    / r  / g  / b  / a  /  */\n"
    "        .fg =       { %.3f, %.3f, %.3f, %.3f},\n"
    "        .bg =       { %.3f, %.3f, %.3f, %.3f},\n"
    "        .base =     { %.3f, %.3f, %.3f, %.3f},\n"
    "        .text =     { %.3f, %.3f, %.3f, %.3f},\n"
    "        .shadow =   { %.3f, %.3f, %.3f, %.3f},\n"
    "        .frame =    { %.3f, %.3f, %.3f, %.3f},\n"
    "        .light =    { %.3f, %.3f, %.3f, %.3f}\n"
    "    };\n\n",c->fg[0],c->fg[1],c->fg[2],c->fg[3],
                c->bg[0],c->bg[1],c->bg[2],c->bg[3],
                c->base[0],c->base[1],c->base[2],c->base[3],
                c->text[0],c->text[1],c->text[2],c->text[3],
                c->shadow[0],c->shadow[1],c->shadow[2],c->shadow[3],
                c->frame[0],c->frame[1],c->frame[2],c->frame[3],
                c->light[0],c->light[1],c->light[2],c->light[3]);

    c = &designer->ui->color_scheme->selected;
    printf (
    "    w->color_scheme->selected = (Colors) {\n"
    "         /* cairo    / r  / g  / b  / a  /  */\n"
    "        .fg =       { %.3f, %.3f, %.3f, %.3f},\n"
    "        .bg =       { %.3f, %.3f, %.3f, %.3f},\n"
    "        .base =     { %.3f, %.3f, %.3f, %.3f},\n"
    "        .text =     { %.3f, %.3f, %.3f, %.3f},\n"
    "        .shadow =   { %.3f, %.3f, %.3f, %.3f},\n"
    "        .frame =    { %.3f, %.3f, %.3f, %.3f},\n"
    "        .light =    { %.3f, %.3f, %.3f, %.3f}\n"
    "    };\n\n",c->fg[0],c->fg[1],c->fg[2],c->fg[3],
                c->bg[0],c->bg[1],c->bg[2],c->bg[3],
                c->base[0],c->base[1],c->base[2],c->base[3],
                c->text[0],c->text[1],c->text[2],c->text[3],
                c->shadow[0],c->shadow[1],c->shadow[2],c->shadow[3],
                c->frame[0],c->frame[1],c->frame[2],c->frame[3],
                c->light[0],c->light[1],c->light[2],c->light[3]);

    c = &designer->ui->color_scheme->active;
    printf (
    "    w->color_scheme->active = (Colors) {\n"
    "         /* cairo    / r  / g  / b  / a  /  */\n"
    "        .fg =       { %.3f, %.3f, %.3f, %.3f},\n"
    "        .bg =       { %.3f, %.3f, %.3f, %.3f},\n"
    "        .base =     { %.3f, %.3f, %.3f, %.3f},\n"
    "        .text =     { %.3f, %.3f, %.3f, %.3f},\n"
    "        .shadow =   { %.3f, %.3f, %.3f, %.3f},\n"
    "        .frame =    { %.3f, %.3f, %.3f, %.3f},\n"
    "        .light =    { %.3f, %.3f, %.3f, %.3f}\n"
    "    };\n\n",c->fg[0],c->fg[1],c->fg[2],c->fg[3],
                c->bg[0],c->bg[1],c->bg[2],c->bg[3],
                c->base[0],c->base[1],c->base[2],c->base[3],
                c->text[0],c->text[1],c->text[2],c->text[3],
                c->shadow[0],c->shadow[1],c->shadow[2],c->shadow[3],
                c->frame[0],c->frame[1],c->frame[2],c->frame[3],
                c->light[0],c->light[1],c->light[2],c->light[3]);

    c = &designer->ui->color_scheme->insensitive;
    printf (
    "    w->color_scheme->insensitive = (Colors) {\n"
    "         /* cairo    / r  / g  / b  / a  /  */\n"
    "        .fg =       { %.3f, %.3f, %.3f, %.3f},\n"
    "        .bg =       { %.3f, %.3f, %.3f, %.3f},\n"
    "        .base =     { %.3f, %.3f, %.3f, %.3f},\n"
    "        .text =     { %.3f, %.3f, %.3f, %.3f},\n"
    "        .shadow =   { %.3f, %.3f, %.3f, %.3f},\n"
    "        .frame =    { %.3f, %.3f, %.3f, %.3f},\n"
    "        .light =    { %.3f, %.3f, %.3f, %.3f}\n"
    "    };\n"
    "}\n\n",c->fg[0],c->fg[1],c->fg[2],c->fg[3],
            c->bg[0],c->bg[1],c->bg[2],c->bg[3],
            c->base[0],c->base[1],c->base[2],c->base[3],
            c->text[0],c->text[1],c->text[2],c->text[3],
            c->shadow[0],c->shadow[1],c->shadow[2],c->shadow[3],
            c->frame[0],c->frame[1],c->frame[2],c->frame[3],
            c->light[0],c->light[1],c->light[2],c->light[3]);

}

static double *get_selected_color(Colors *c, int s) {
    double *use = NULL;
    switch (s) {
        case 0: use = c->fg;
        break;
        case 1: use = c->bg;
        break;
        case 2: use = c->base;
        break;
        case 3: use = c->text;
        break;
        case 4: use = c->shadow;
        break;
        case 5: use = c->frame;
        break;
        case 6: use = c->light;
        break;
        default:
        break;
    }
    return use;
}

static void check_for_Widget_colors(XUiDesigner *designer) {
    int j = 0;  // Color_state
    for(;j<5;j++) {
        int k = 0; // Color_mod
        for(;k<6;k++) {
            double *c =  get_selected_color(get_color_scheme(designer->ui, j), k);
            int i = 0;
            int a = 0;
            int x = 0;
            for (;i<MAX_CONTROLS;i++) {
                if (designer->controls[i].wid != NULL) {
                    if (designer->controls[i].is_audio_output || designer->controls[i].is_audio_input ||
                        designer->controls[i].is_atom_output || designer->controls[i].is_atom_input ||
                        designer->controls[i].is_type == IS_FRAME ||
                        designer->controls[i].is_type == IS_IMAGE ||
                        designer->controls[i].is_type == IS_TABBOX) {
                        continue;
                    }
                    Widget_t * wid = designer->controls[i].wid;
                    double *b = get_selected_color(get_color_scheme(wid, j), k);
                    a = memcmp(c, b, 4 * sizeof(double));
                    if (a != 0) {
                        printf("\n    set_widget_color(ui->widget[%i], %i, %i,"
                                "%.3f, %.3f, %.3f, %.3f);\n", x, j, k, b[0], b[1], b[2], b[3]);
                    }
                    x++;
                }
            }
        }
    }
}

static void check_for_elem_colors(XUiDesigner *designer) {
    int j = 0;  // Color_state
    for(;j<5;j++) {
        int k = 0; // Color_mod
        for(;k<6;k++) {
            double *c =  get_selected_color(get_color_scheme(designer->ui, j), k);
            int i = 0;
            int a = 0;
            int x = 0;
            for (;i<MAX_CONTROLS;i++) {
                if (designer->controls[i].wid != NULL) {
                    if (designer->controls[i].is_type == IS_FRAME ||
                        designer->controls[i].is_type == IS_IMAGE ||
                        designer->controls[i].is_type == IS_TABBOX) {

                        Widget_t * wid = designer->controls[i].wid;
                        double *b = get_selected_color(get_color_scheme(wid, j), k);
                        a = memcmp(c, b, 4 * sizeof(double));
                        if (a != 0) {
                            printf("\n    set_widget_color(ui->elem[%i], %i, %i,"
                                    "%.3f, %.3f, %.3f, %.3f);\n", x, j, k, b[0], b[1], b[2], b[3]);
                        }
                        x++;
                    }
                }
            }
        }
    }
}

/*---------------------------------------------------------------------
-----------------------------------------------------------------------    
                print C file for UI widgets 
-----------------------------------------------------------------------
----------------------------------------------------------------------*/

int format(float value) {
    float v = value - (int)value;
    char s[30] = {0};
    snprintf(s, 29, "%f", v);
    unsigned int l = strlen(s)-1;
    for(;l>0;l--) {
        if (strstr(&s[l],"0")) {
            s[l] = '\0';
        } else {
            break;
        }
    }
    if (strstr(&s[l],".")) {
        l++;
    }
    return l-1;
}

void print_list(XUiDesigner *designer) {
    int i = 0;
    int j = 0;
    int k = 0;
    int l = 0;
    int p = designer->lv2c.audio_input + designer->lv2c.audio_output +
        designer->lv2c.midi_input + designer->lv2c.midi_output;
    bool have_image = false;
    bool have_atom_in = false;
    bool have_atom_out = false;
    bool have_midi_in = false;
    int MIDI_PORT = -1;
    int MIDIKEYBOARD = -1;
    if (designer->lv2c.midi_input) {
        MIDI_PORT = designer->lv2c.audio_input + designer->lv2c.audio_output +
                    designer->lv2c.midi_input -1;
    }
    
    i = 0;
    for (;i<MAX_CONTROLS;i++) {
        if (designer->controls[i].wid != NULL && (designer->controls[i].is_type != IS_FRAME &&
                                                designer->controls[i].is_type != IS_IMAGE &&
                                                designer->controls[i].is_type != IS_TABBOX &&
                                                !designer->controls[i].is_audio_input &&
                                                !designer->controls[i].is_audio_output &&
                                                !designer->controls[i].is_atom_input &&
                                                !designer->controls[i].is_atom_output)) {
            j++;
        } else if (designer->controls[i].wid != NULL && (designer->controls[i].is_type == IS_FRAME ||
                                                        designer->controls[i].is_type == IS_IMAGE ||
                                                        designer->controls[i].is_type == IS_TABBOX)) {
            k++;
            if (designer->controls[i].is_type == IS_TABBOX) {
                l += designer->controls[i].wid->childlist->elem;
            }
        }
        if (designer->controls[i].is_midi_patch) {
            have_midi_in = true;
            MIDIKEYBOARD = j-1;
        }
        if (designer->controls[i].is_atom_patch && designer->controls[i].is_type == IS_FILE_BUTTON) {
            have_atom_in = true;
        } else if (designer->controls[i].is_atom_patch && designer->controls[i].is_type != IS_FILE_BUTTON) {
            have_atom_out = true;
        }
    }
    i = 0;
    int m = 0;
    for (;i<MAX_CONTROLS;i++) {
        if (designer->controls[i].wid != NULL && (designer->controls[i].is_type != IS_FRAME &&
                                                designer->controls[i].is_type != IS_IMAGE &&
                                                designer->controls[i].is_type != IS_TABBOX &&
                                                designer->controls[i].is_type != IS_MIDIKEYBOARD)) {
            if (designer->controls[i].is_atom_input) {
                MIDI_PORT = m;
                break;
            }
            m++;
        }
    }
    
    if (have_midi_in && designer->MIDIPORT > -1) {
        MIDI_PORT = designer->MIDIPORT;
    }
    if (!adj_get_value(designer->display_name->adj)) {
        printf ("\n#define HIDE_NAME \n");
    }
    if (j) {
        printf ("\n#define CONTROLS %i\n", j);
        printf ("\n#define GUI_ELEMENTS %i\n", k);
        printf ("\n#define TAB_ELEMENTS %i\n\n", l);
        printf ("\n#define PLUGIN_UI_URI \"%s\"\n\n",designer->lv2c.ui_uri);
        printf ("\n#include \"lv2_plugin.h\"\n\n");
    }
    if (have_atom_in || have_atom_out) {
        printf("#ifdef USE_ATOM\n");
    }
    i = 0;
    for (;i<MAX_CONTROLS;i++) {
        if (designer->controls[i].image) {
            have_image = true;
        }
        if (designer->controls[i].is_atom_patch && designer->controls[i].is_type == IS_FILE_BUTTON) {
            Widget_t * wid = designer->controls[i].wid;
            const char* uri = (const char*) wid->parent_struct;
            char *xldl = NULL;
            asprintf(&xldl, "%s", wid->label);
            strtovar(xldl);
            printf ("\n#define XLV2__%s \"%s\"", xldl, uri);
            free(xldl);
        } else if (designer->controls[i].is_atom_patch && designer->controls[i].is_type != IS_FILE_BUTTON) {
            Widget_t * wid = designer->controls[i].wid;
            const char* uri = (const char*) wid->parent_struct;
            char *xldl = NULL;
            asprintf(&xldl, "%s", wid->label);
            strtovar(xldl);
            printf ("\n#define XLV2__%s \"%s\"", xldl, uri);
            free(xldl);
        }
    }
    if (designer->image != NULL) {
        have_image = true;
    }
    if (have_atom_in || have_atom_out || have_midi_in) {
        printf ( "\n#define OBJ_BUF_SIZE 1024\n");
    }
    if (have_atom_in || have_atom_out) {
        printf ( "\n\ntypedef struct {\n");
        i = 0;
        for (;i<MAX_CONTROLS;i++) {
            if (designer->controls[i].is_atom_patch) {
                char *xldl = NULL;
                asprintf(&xldl, "%s", designer->controls[i].wid->label);
                strtovar(xldl);
                printf ("    LV2_URID %s;\n", xldl);
                free(xldl);
            }
        }


        printf ("    LV2_URID atom_Object;\n"
                "    LV2_URID atom_Int;\n"
                "    LV2_URID atom_Float;\n"
                "    LV2_URID atom_Bool;\n"
                "    LV2_URID atom_Vector;\n"
                "    LV2_URID atom_Path;\n"
                "    LV2_URID atom_String;\n"
                "    LV2_URID atom_URID;\n"
                "    LV2_URID atom_eventTransfer;\n"
                "    LV2_URID patch_Put;\n"
                "    LV2_URID patch_Get;\n"
                "    LV2_URID patch_Set;\n"
                "    LV2_URID patch_property;\n"
                "    LV2_URID patch_value;\n" );


        printf ( "} X11LV2URIs;\n");
        
        printf ("\ntypedef struct {\n"
                "    LV2_Atom_Forge forge;\n"
                "    X11LV2URIs   uris;\n"
                "    char *filename;\n"
                "} X11_UI_Private_t;\n");

        printf ("\nstatic inline void map_x11ui_uris(LV2_URID_Map* map, X11LV2URIs* uris) {\n");
        i = 0;
        for (;i<MAX_CONTROLS;i++) {
            if (designer->controls[i].is_atom_patch) {
                char *xldl = NULL;
                asprintf(&xldl, "%s", designer->controls[i].wid->label);
                strtovar(xldl);
                printf ("    uris->%s = map->map(map->handle, XLV2__%s);\n", xldl, xldl);
                free(xldl);
            }
        }
        printf ("    uris->atom_Object = map->map(map->handle, LV2_ATOM__Object);\n"
                "    uris->atom_Int = map->map(map->handle, LV2_ATOM__Int);\n"
                "    uris->atom_Float = map->map(map->handle, LV2_ATOM__Float);\n"
                "    uris->atom_Bool = map->map(map->handle, LV2_ATOM__Bool);\n"
                "    uris->atom_Vector = map->map(map->handle, LV2_ATOM__Vector);\n"
                "    uris->atom_Path = map->map(map->handle, LV2_ATOM__Path);\n"
                "    uris->atom_String = map->map(map->handle, LV2_ATOM__String);\n"
                "    uris->atom_URID = map->map(map->handle, LV2_ATOM__URID);\n"
                "    uris->atom_eventTransfer = map->map(map->handle, LV2_ATOM__eventTransfer);\n"
                "    uris->patch_Put = map->map(map->handle, LV2_PATCH__Put);\n"
                "    uris->patch_Get = map->map(map->handle, LV2_PATCH__Get);\n"
                "    uris->patch_Set = map->map(map->handle, LV2_PATCH__Set);\n"
                "    uris->patch_property = map->map(map->handle, LV2_PATCH__property);\n"
                "    uris->patch_value = map->map(map->handle, LV2_PATCH__value);\n");
        printf ("}\n");
        printf ("#endif\n\n");
    }
    if (j) {
        Window w = (Window)designer->ui->widget;
        char *name;
        XFetchName(designer->ui->app->dpy, w, &name);
        
        if (have_image && !designer->run_test) printf ("\n#include \"xresources.h\"\n\n");
        print_colors(designer);
        printf ("#include \"%s\"\n\n\n"
        , designer->run_test? "ui_test.cc": "lv2_plugin.cc");
        
        if (have_midi_in && MIDI_PORT > -1) {
            printf ("#ifdef USE_MIDI\n");
            printf("static void send_midi_data(Widget_t *w, const int *key, const int control) {\n"
            "    X11_UI *ui = (X11_UI*) w->parent_struct;\n"
            "    MidiKeyboard *keys = (MidiKeyboard*)ui->widget[%i]->private_struct;\n"
            "    uint8_t obj_buf[OBJ_BUF_SIZE];\n"
            "    uint8_t vec[3];\n"
            "    vec[0] = (int)control;\n"// Note On/Off or controller number
            "    vec[0] |= keys->channel;\n" //channel
            "    vec[1] = (*key);\n" // note
            "    vec[2] = keys->velocity;\n" // velocity
            "    lv2_atom_forge_set_buffer(&ui->forge, obj_buf, OBJ_BUF_SIZE);\n\n"

            "    lv2_atom_forge_frame_time(&ui->forge,0);\n"
            "    LV2_Atom* msg = (LV2_Atom*)lv2_atom_forge_raw(&ui->forge,&ui->midiatom,sizeof(LV2_Atom));\n"
            "    lv2_atom_forge_raw(&ui->forge,vec, sizeof(vec));\n"
            "    lv2_atom_forge_pad(&ui->forge,sizeof(vec)+sizeof(LV2_Atom));\n\n"
            "    ui->write_function(ui->controller, %i, lv2_atom_total_size(msg),\n"
            "                       ui->atom_eventTransfer, msg);\n"
            "}\n\n", MIDIKEYBOARD, MIDI_PORT);

            printf("static void send_all_notes_off(Widget_t *w, const int *value){\n"
            "        X11_UI *ui = (X11_UI*) w->parent_struct;\n"
            "        int key = 120;\n"
            "        send_midi_data(ui->widget[%i], &key, 0xB0);\n"
            "}\n\n", MIDIKEYBOARD);

            printf("static void xkey_press(void *w_, void *key_, void *user_data) {\n"
            "        Widget_t *w = (Widget_t*)w_;\n"
            "        X11_UI *ui = (X11_UI*) w->parent_struct;\n"
            "        ui->widget[%i]->func.key_press_callback(ui->widget[%i], key_, user_data);\n"
            "}\n\n", MIDIKEYBOARD, MIDIKEYBOARD);

            printf("static void xkey_release(void *w_, void *key_, void *user_data) {\n"
            "        Widget_t *w = (Widget_t*)w_;\n"
            "        X11_UI *ui = (X11_UI*) w->parent_struct;\n"
            "        ui->widget[%i]->func.key_release_callback(ui->widget[%i], key_, user_data);\n"
            "}\n\n", MIDIKEYBOARD, MIDIKEYBOARD);
            
            printf("#endif\n");
        }
        
        if (have_atom_in) {
            printf ("#ifdef USE_ATOM\n");

            printf ("\nstatic inline LV2_Atom* write_set_file(LV2_Atom_Forge* forge, const LV2_URID control,\n"
                    "                        const X11LV2URIs* uris, const char* filename) {\n"
                    "    LV2_Atom_Forge_Frame frame;\n"
                    "    LV2_Atom* set = (LV2_Atom*)lv2_atom_forge_object(\n"
                    "                        forge, &frame, 1, uris->patch_Set);\n"
                    "    lv2_atom_forge_key(forge, uris->patch_property);\n"
                    "    lv2_atom_forge_urid(forge, control);\n"
                    "    lv2_atom_forge_key(forge, uris->patch_value);\n"
                    "    lv2_atom_forge_path(forge, filename, strlen(filename));\n"
                    "    lv2_atom_forge_pop(forge, &frame);\n"
                    "    return set;\n"
                    "}\n");

           printf ("\nstatic void file_load_response(void *w_, void* user_data) {\n"
                    "    Widget_t *w = (Widget_t*)w_;\n"
                    "    Widget_t *p = (Widget_t*)w->parent;\n"
                    "    X11_UI *ui = (X11_UI*) p->parent_struct;\n"
                    "    X11_UI_Private_t *ps = (X11_UI_Private_t*)ui->private_ptr;\n"
                    "    const LV2_URID urid = *(const LV2_URID*)w->parent_struct;\n"
                    "    if(user_data !=NULL) {\n"
                    "        free(ps->filename);\n"
                    "        ps->filename = NULL;\n"
                    "        ps->filename = strdup(*(const char**)user_data);\n"
                    "        uint8_t obj_buf[OBJ_BUF_SIZE];\n"
                    "        lv2_atom_forge_set_buffer(&ps->forge, obj_buf, OBJ_BUF_SIZE);\n"

                    "        LV2_Atom* msg = (LV2_Atom*)write_set_file(&ps->forge, urid, &ps->uris, ps->filename);\n"

                    "        ui->write_function(ui->controller, %i, lv2_atom_total_size(msg),\n"
                    "                           ps->uris.atom_eventTransfer, msg);\n"
                    "        free(ps->filename);\n"
                    "        ps->filename = NULL;\n"
                    "        ps->filename = strdup(\"None\");\n"
                    "    }\n"
                    "}\n", designer->lv2c.atom_input_port);

           printf ("\nvoid send_controller_message(Widget_t *w, const LV2_URID control) {\n"
                    "    Widget_t *p = (Widget_t*)w->parent;\n"
                    "    X11_UI *ui = (X11_UI*) p->parent_struct;\n"
                    "    X11_UI_Private_t *ps = (X11_UI_Private_t*)ui->private_ptr;\n"
                    "    const X11LV2URIs* uris = &ps->uris;\n"
                    "    const float value = adj_get_value(w->adj);\n"
                    "    uint8_t obj_buf[OBJ_BUF_SIZE];\n"
                    "    lv2_atom_forge_set_buffer(&ps->forge, obj_buf, OBJ_BUF_SIZE);\n"

                    "    LV2_Atom_Forge_Frame frame;\n"
                    "    LV2_Atom* msg = (LV2_Atom*)lv2_atom_forge_object(&ps->forge, &frame, 0, uris->patch_Set);\n"
                    "    lv2_atom_forge_key(&ps->forge, uris->patch_property);\n"
                    "    lv2_atom_forge_urid(&ps->forge, control);\n"
                    "    lv2_atom_forge_key(&ps->forge, uris->patch_value);\n"
                    "    switch(w->data) {\n"
                    "        case -2:\n"
                    "             lv2_atom_forge_int(&ps->forge, (int)value);\n"
                    "        break;\n"
                    "        case -3:\n"
                    "             lv2_atom_forge_bool(&ps->forge, (int)value);\n"
                    "        break;\n"
                    "        default:\n"
                    "            lv2_atom_forge_float(&ps->forge, value);\n"
                    "        break;\n"
                    "    }\n"
                    "    lv2_atom_forge_pop(&ps->forge, &frame);\n"

                    "    ui->write_function(ui->controller, %i, lv2_atom_total_size(msg),\n"
                    "                       ps->uris.atom_eventTransfer, msg);\n"
                    "}\n"

                    "\nvoid controller_callback(void *w_, void* user_data) {\n"
                    "    Widget_t *w = (Widget_t*)w_;\n"
                    "    const LV2_URID urid = *(const LV2_URID*)w->parent_struct;\n"
                    "    if (w->data == -4) {\n"
                    "        file_load_response(w, user_data);\n"
                    "    } else {\n"
                    "        send_controller_message(w, urid);\n"
                    "    }\n"
                    "}\n", designer->lv2c.atom_input_port);

            printf ("\nstatic void dummy_callback(void *w_, void* user_data) {\n"
                    "}\n");

            printf ("\nvoid set_ctl_val_from_host(Widget_t *w, float value) {\n"
                    "    xevfunc store = w->func.value_changed_callback;\n"
                    "    w->func.value_changed_callback = dummy_callback;\n"
                    "    adj_set_value(w->adj, value);\n"
                    "    w->func.value_changed_callback = *(*store);\n"
                    "}\n");

            printf("#endif\n");
        }
        if (have_atom_out) {
            
        }
        printf ("\nvoid plugin_value_changed(X11_UI *ui, Widget_t *w, PortIndex index) {\n"
        "    // do special stuff when needed\n"
        "}\n\n"
        "void plugin_set_window_size(int *w,int *h,const char * plugin_uri, float scale) {\n"
        "    (*w) = %i * scale; //set initial width of main window\n"
        "    (*h) = %i * scale; //set initial height of main window\n"
        "}\n\n"
        "const char* plugin_set_name() {\n"
        "    return \"%s\"; //set plugin name to display on UI\n"
        "}\n\n"
        "void plugin_create_controller_widgets(X11_UI *ui, const char * plugin_uri, float scale) {\n"
        "    set_costum_theme(ui->win);\n"
        , designer->ui->width, designer->ui->height, name? name:"Test");

        if (have_midi_in && MIDI_PORT > -1) {
                printf ("#ifdef USE_MIDI\n"
                "    XSelectInput(ui->win->app->dpy, ui->win->widget,StructureNotifyMask|ExposureMask|KeyPressMask \n"
                "        |EnterWindowMask|LeaveWindowMask|ButtonReleaseMask|KeyReleaseMask\n"
                "        |ButtonPressMask|Button1MotionMask|PointerMotionMask);\n"
                "    ui->win->flags |= NO_AUTOREPEAT | NO_PROPAGATE;\n"
                "    ui->win->func.key_press_callback = xkey_press;\n"
                "    ui->win->func.key_release_callback = xkey_release;\n"
                "#endif\n\n");
        }

        if (designer->image != NULL) {
            if (designer->run_test) {
                printf ("    load_bg_image(ui,\"%s\");\n", designer->image);
            } else {
                char* tmp = strdup(designer->image);
                char * xldl = strdup(basename(tmp));
                strdecode(xldl, ".", "_");
                strdecode(xldl, "-", "_");
                strdecode(xldl, " ", "_");
                strtovar(xldl);
                if (strstr(designer->image, ".png")) {
                    printf ("    widget_get_scaled_png(ui->win, LDVAR(%s));\n", xldl);
                } else if (strstr(designer->image, ".svg")) {
                    printf ("    widget_get_scaled_svg(ui->win, %s);\n", xldl);
                }
                free(xldl);
                xldl = NULL;
                free(tmp);
                tmp = NULL;
                //printf ("    load_bg_image(ui,\"./resources/%s\");\n", basename(designer->image));
            }
        }

        if (have_atom_in || have_atom_out) {
            printf ("\n#ifdef USE_ATOM\n"
                    "    X11_UI_Private_t *ps =(X11_UI_Private_t*)malloc(sizeof(X11_UI_Private_t));\n"
                    "    ui->private_ptr = (void*)ps;\n"
                    "    map_x11ui_uris(ui->map, &ps->uris);\n"
                    "    lv2_atom_forge_init(&ps->forge, ui->map);\n"
                    "    const X11LV2URIs* uris = &ps->uris;\n"
                    "    ps->filename = strdup(\"None\");\n" 
                    "#endif\n\n");
        }

    } else {
        return;
    }
    i = 0;
    j = 0;
    l = 0;
    int ttb[k] ;
    memset(ttb, 0, k*sizeof(int));
    for (;i<MAX_CONTROLS;i++) {
        if (designer->controls[i].wid != NULL) {
            if (designer->controls[i].is_type == IS_FRAME || designer->controls[i].is_type == IS_IMAGE ) {
                printf ("    ui->elem[%i] = %s (ui->elem[%i], ui->win, %i, \"%s\", ui, %i,  %i, %i * scale, %i * scale);\n", 
                    j, designer->controls[i].type, j,
                    designer->is_project ? p : designer->controls[i].port_index, designer->controls[i].wid->label,
                    designer->controls[i].wid->x, designer->controls[i].wid->y,
                    designer->controls[i].wid->width, designer->controls[i].wid->height);
                if (designer->controls[i].image != NULL ) {
                    if (designer->run_test) {
                        printf ("    load_controller_image(ui->elem[%i], \"%s\");\n",
                                            j, designer->controls[i].image);
                    } else {
                        char* tmp = strdup(designer->controls[i].image);
                        char * xldl = strdup(basename(tmp));
                        strdecode(xldl, ".", "_");
                        strdecode(xldl, "-", "_");
                        strdecode(xldl, " ", "_");
                        strtovar(xldl);
                        if (strstr(designer->controls[i].image, ".png")) {
                            printf ("    widget_get_scaled_png(ui->elem[%i], LDVAR(%s));\n",
                                    j, xldl);
                        } else if (strstr(designer->controls[i].image, ".svg")) {
                            printf ("    widget_get_scaled_svg(ui->widget[%i], %s);\n", j, xldl);
                        }
                        free(xldl);
                        xldl = NULL;
                        free(tmp);
                        tmp = NULL;
                       // printf ("    load_controller_image(ui->elem[%i], \"./resources/%s\");\n",
                       //                         j, basename(designer->controls[i].image));
                    }
                }
                j++;
            } else if (designer->controls[i].is_type == IS_TABBOX) {
                printf ("    ui->elem[%i] = %s (ui->elem[%i], ui->win, %i, \"%s\", ui, %i,  %i, %i * scale, %i * scale);\n", 
                    j, designer->controls[i].type, j,
                    designer->is_project ? p : designer->controls[i].port_index, designer->controls[i].wid->label,
                    designer->controls[i].wid->x, designer->controls[i].wid->y,
                    designer->controls[i].wid-> width, designer->controls[i].wid->height);
                ttb[j] = l;
                int elem = designer->controls[i].wid->childlist->elem;
                int t = 0;
                for(;t<elem;t++) {
                    Widget_t *wi = designer->controls[i].wid->childlist->childs[t];
                    printf ("    ui->tab_elem[%i] = add_lv2_tab (ui->tab_elem[%i], ui->elem[%i], -1, \"%s\", ui);\n", 
                        l, l, j, wi->label);
                    l++;
                }
                printf ("\n");
                j++;
            }
            
        }
    }
    i = 0;
    j = 0;
    for (;i<MAX_CONTROLS;i++) {
        if (designer->controls[i].wid != NULL) {
            if (designer->controls[i].is_audio_output || designer->controls[i].is_audio_input ||
                designer->controls[i].is_atom_output || designer->controls[i].is_atom_input) {
                continue;
            }
            Widget_t * wid = designer->controls[i].wid;
            if (designer->controls[i].is_type == IS_FRAME ||
                designer->controls[i].is_type == IS_IMAGE ||
                designer->controls[i].is_type == IS_TABBOX) {
                continue;
            } else {
                char* parent = NULL;
                if (designer->controls[i].in_tab) {
                    int atb = ttb[designer->controls[i].in_frame-1];
                    asprintf(&parent,"ui->tab_elem[%i]", atb + designer->controls[i].in_tab-1);
                } else {
                    designer->controls[i].in_frame ? asprintf(&parent,"ui->elem[%i]", designer->controls[i].in_frame-1) :
                        asprintf(&parent,"%s", "ui->win");
                }
                printf ("    ui->widget[%i] = %s (ui->widget[%i], %s, %i, \"%s\", ui, %i,  %i, %i * scale, %i * scale);\n", 
                    j, designer->controls[i].type, j, parent,
                    designer->controls[i].is_midi_patch ? -1 : designer->is_project ? designer->is_faust_file ?
                    designer->controls[i].port_index : p : designer->controls[i].port_index,
                    designer->controls[i].wid->label,
                    designer->controls[i].wid->x, designer->controls[i].wid->y,
                    designer->controls[i].wid-> width, designer->controls[i].wid->height);
                free(parent);
            }
            if (designer->controls[i].is_atom_patch ) {
                //const char* uri = (const char*) wid->parent_struct;
                printf("#ifdef USE_ATOM\n");
                char* xldl = NULL;
                asprintf(&xldl, "%s", wid->label);
                strtovar(xldl);
                printf("    ui->widget[%i]->parent_struct = (void*)&uris->%s;\n", j, xldl);
                free(xldl);
                if (designer->controls[i].is_type == IS_FILE_BUTTON ) {
                    printf("    ui->widget[%i]->func.user_callback = controller_callback;\n", j);
                } else {
                    printf("    ui->widget[%i]->func.value_changed_callback = controller_callback;\n", j);
                }
                printf("#endif\n");
            }
            if (designer->controls[i].image != NULL ) {
                if (designer->run_test) {
                    printf ("    load_controller_image(ui->widget[%i], \"%s\");\n",
                            j, designer->controls[i].image);
                } else {
                    char* tmp = strdup(designer->controls[i].image);
                    char * xldl = strdup(basename(tmp));
                    strdecode(xldl, ".", "_");
                    strdecode(xldl, "-", "_");
                    strdecode(xldl, " ", "_");
                    strtovar(xldl);
                    if (strstr(designer->controls[i].image, ".png")) {
                        printf ("    widget_get_png(ui->widget[%i], LDVAR(%s));\n", j, xldl);
                    } else if (strstr(designer->controls[i].image, ".svg")) {
                        printf ("    widget_get_svg(ui->widget[%i], %s);\n", j, xldl);
                    }
                    free(xldl);
                    xldl = NULL;
                    free(tmp);
                    tmp = NULL;
                }
                if (designer->controls[i].is_type == IS_VSLIDER ||
                        designer->controls[i].is_type == IS_HSLIDER) {
                    printf ("    set_slider_image_frame_count(ui->widget[%i], %i);\n",j,
                        designer->controls[i].slider_image_sprites);
                }
            }
            if (designer->controls[i].is_type == IS_COMBOBOX) {
                Widget_t *menu = wid->childlist->childs[1];
                Widget_t* view_port =  menu->childlist->childs[0];
                ComboBox_t *comboboxlist = (ComboBox_t*)view_port->parent_struct;
                unsigned int ka = 0;
                for(; ka<comboboxlist->list_size;ka++) {
                    printf ("    combobox_add_entry (ui->widget[%i], \"%s\");\n", j, comboboxlist->list_names[ka]);
                }
            }
            if (designer->controls[i].have_adjustment && !designer->controls[i].is_midi_patch) {
                printf ("    set_adjustment(ui->widget[%i]->adj, %.*f, %.*f, %.*f, %.*f, %.*f, %s);\n", 
                    j, format(adj_get_std_value(wid->adj)),adj_get_std_value(wid->adj),
                    format(adj_get_std_value(wid->adj)), adj_get_std_value(wid->adj),
                    format(adj_get_min_value(wid->adj)),adj_get_min_value(wid->adj),
                    format(adj_get_max_value(wid->adj)), adj_get_max_value(wid->adj),
                    format(wid->adj->step), wid->adj->step,
                    parse_adjusment_type(wid->adj->type));
            }
            if (designer->controls[i].is_midi_patch && MIDI_PORT > -1) {
                printf ("#ifdef USE_MIDI\n"
                        "    MidiKeyboard *keys = (MidiKeyboard*)ui->widget[%i]->private_struct;\n"
                        "    keys->mk_send_note = send_midi_data;\n"
                        "    keys->mk_send_all_sound_off = send_all_notes_off;\n"
                        "#endif\n", j);
                p--;
            }
            if (have_midi_in && MIDI_PORT > -1 && ! designer->controls[i].is_midi_patch) {
                    printf ("#ifdef USE_MIDI\n"
                            "    ui->widget[%i]->func.key_press_callback = xkey_press;\n"
                            "    ui->widget[%i]->func.key_release_callback = xkey_release;\n"
                            "#endif\n", j, j);

            }
            printf ("\n");
            if (designer->controls[i].is_type != IS_FRAME) {
                j++;
                p++;
            } else if (designer->controls[i].is_type != IS_IMAGE) {
                j++;
                p++;
            }
        }
    }
    check_for_elem_colors(designer);
    check_for_Widget_colors(designer);
    printf ("}\n\n"
    "void plugin_cleanup(X11_UI *ui) {\n");
    if (have_atom_in || have_atom_out) {
        printf ("#ifdef USE_ATOM\n"
                "    X11_UI_Private_t *ps = (X11_UI_Private_t*)ui->private_ptr;\n"
                "    free(ps->filename);\n"
                "#endif\n");
    }
    printf ("    // clean up used sources when needed\n"
            "}\n\n");

    if (have_atom_in || have_atom_out) {
        printf ("#ifdef USE_ATOM\n"
                "Widget_t *get_widget_from_urid(X11_UI *ui, const LV2_URID urid) {\n"
                "    int i = 0;\n"
                "    for(; i<CONTROLS; i++) {\n"
                "        if (*(const LV2_URID*)ui->widget[i]->parent_struct == urid) {\n"
                "            return ui->widget[i];\n"
                "        }\n"
                "    }\n"
                "    return NULL;\n"
                "}\n"

                "\nstatic inline const LV2_Atom* read_set_file(const X11LV2URIs* uris, X11_UI *ui,\n"
                "                                            const LV2_Atom_Object* obj) {\n"
                "    if (obj->body.otype != uris->patch_Set) {\n"
                "        return NULL;\n"
                "    }\n"
                "    const LV2_Atom* property = NULL;\n"
                "    lv2_atom_object_get(obj, uris->patch_property, &property, 0);\n"
                "    if (property == NULL) return NULL;\n"
                "    Widget_t *w = get_widget_from_urid(ui,((LV2_Atom_URID*)property)->body);\n"
                "    if (!w || (property->type != uris->atom_URID)) {\n"
                "        return NULL;\n"
                "    }\n"
                "    const LV2_Atom* file_path = NULL;\n"
                "    lv2_atom_object_get(obj, uris->patch_value, &file_path, 0);\n"
                "    if (!file_path || (file_path->type != uris->atom_Path)) {\n"
                "        return NULL;\n"
                "    }\n"
                "    return file_path;\n"
                "}\n"
                "#endif\n");
    }
    printf ("\nvoid plugin_port_event(LV2UI_Handle handle, uint32_t port_index,\n"
            "                        uint32_t buffer_size, uint32_t format,\n"
            "                        const void * buffer) {\n");

    if (have_atom_in || have_atom_out) {
        printf ("#ifdef USE_ATOM\n"
                "    X11_UI* ui = (X11_UI*)handle;\n"
                "    X11_UI_Private_t *ps = (X11_UI_Private_t*)ui->private_ptr;\n"
                "    const X11LV2URIs* uris = &ps->uris;\n"

                "    if (format == ps->uris.atom_eventTransfer) {\n"
                "        const LV2_Atom* atom = (LV2_Atom*)buffer;\n"
                "        if (atom->type == ps->uris.atom_Object) {\n"
                "            const LV2_Atom_Object* obj      = (LV2_Atom_Object*)atom;\n"
                "            if (obj->body.otype == uris->patch_Set) {\n"
                "                const LV2_Atom*  file_uri = read_set_file(uris, ui, obj);\n"
                "                if (file_uri) {\n"
                "                    const char* uri = (const char*)LV2_ATOM_BODY(file_uri);\n"
                "                    if (strlen(uri)) {\n"
                "                        if (strcmp(uri, (const char*)ps->filename) !=0) {\n"
                "                            free(ps->filename);\n"
                "                            ps->filename = NULL;\n"
                "                            ps->filename = strdup(uri);\n"
                "                            expose_widget(ui->win);\n"
                "                        }\n"
                "                    }\n"
                "                } else {\n"
                "                    const LV2_Atom* value = NULL;\n"
                "                    const LV2_Atom* property = NULL;\n"
                "                    lv2_atom_object_get(obj, uris->patch_value, &value, \n"
                "                                    uris->patch_property, &property, 0);\n"
                "                    if (value == NULL) return;\n"
                "                    if (property == NULL) return;\n"
                "                    Widget_t *w = get_widget_from_urid(ui,((LV2_Atom_URID*)property)->body);\n"
                "                    if (w) {\n"
                "                        if (value->type == uris->atom_Float ) {\n"
                "                            float* val = (float*)LV2_ATOM_BODY(value);\n"
                "                            set_ctl_val_from_host(w, (*val));\n"
                "                        } else if (value->type == uris->atom_Int ) {\n"
                "                            int* val = (int*)LV2_ATOM_BODY(value);\n"
                "                            set_ctl_val_from_host(w, (float)(*val));\n"
                "                        }else if (value->type == uris->atom_Bool ) {\n"
                "                            int* val = (int*)LV2_ATOM_BODY(value);\n"
                "                            set_ctl_val_from_host(w, (float)(*val));\n"
                "                        }\n"
                "                    }\n"
                "                }\n"
                "            }\n"
                "        }\n"
                "    }\n"
                "#endif\n");
    }
    printf ("    // port value change message from host\n"
            "    // do special stuff when needed\n"
            "}\n\n");

}

