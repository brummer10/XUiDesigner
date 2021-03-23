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

#include "XUiGenerator.h"
#include "XUiGridControl.h"


/*---------------------------------------------------------------------
-----------------------------------------------------------------------    
                         png2c
-----------------------------------------------------------------------
----------------------------------------------------------------------*/

void strtovar(char* c) {
    char* b = "_";
    int i = 0;
    for (i=0; c[i] != '\0'; i++) {
        if (!isalnum((unsigned char)c[i])) {
            c[i] = (*b);
        } else {
            c[i] = tolower((unsigned char)c[i]);
        }
    }
}

void strtoguard(char* c) {
    char* b = "_";
    int i = 0;
    for (i=0; c[i] != '\0'; i++) {
        if (!isalnum((unsigned char)c[i])) {
            c[i] = (*b);
        } else {
            c[i] = toupper((unsigned char)c[i]);
        }
    }
}

void png2c(char* image_name, char* filepath) {
    cairo_surface_t *image = cairo_image_surface_create_from_png(image_name);
    int w = cairo_image_surface_get_width(image);
    int h = cairo_image_surface_get_height(image);
    int stride = cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, w);

    char * xld = NULL;
    char * xldl = NULL;
    asprintf(&xld, "%s", basename(image_name));
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
                print widgets on exit
-----------------------------------------------------------------------
----------------------------------------------------------------------*/

void remove_from_list(XUiDesigner *designer, Widget_t *wid) {
    designer->controls[wid->data].wid = NULL;
    designer->controls[wid->data].have_adjustment = false;
    free(designer->controls[wid->data].image);
    designer->controls[wid->data].image = NULL;
    designer->controls[wid->data].grid_snap_option = 0;
    designer->controls[wid->data].in_frame = 0;
    designer->controls[wid->data].in_tab = 0;
}

void add_to_list(XUiDesigner *designer, Widget_t *wid, const char* type,
                                    bool have_adjustment, WidgetType is_type) {
    designer->controls[wid->data].wid = wid;
    designer->controls[wid->data].type = type;
    designer->controls[wid->data].have_adjustment = have_adjustment;
    designer->controls[wid->data].is_type = is_type;
}

void print_colors(XUiDesigner *designer) {
    Xputty * main = designer->w->app;
    Colors *c = &main->color_scheme->normal;
    printf (
    "void set_costum_theme(Xputty *main) {\n"
    "    main->color_scheme->normal = (Colors) {\n"
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

    c = &main->color_scheme->prelight;
    printf (
    "    main->color_scheme->prelight = (Colors) {\n"
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

    c = &main->color_scheme->selected;
    printf (
    "    main->color_scheme->selected = (Colors) {\n"
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

    c = &main->color_scheme->active;
    printf (
    "    main->color_scheme->active = (Colors) {\n"
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

    c = &main->color_scheme->insensitive;
    printf (
    "    main->color_scheme->insensitive = (Colors) {\n"
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

void print_plugin(XUiDesigner *designer) {
    char *name = NULL;
    XFetchName(designer->ui->app->dpy, designer->ui->widget, &name);
    if (name == NULL) asprintf(&name, "%s", "noname");
    strtovar(name);

    printf ("\n#include <cstdlib>\n"
    "#include <cmath>\n"
    "#include <iostream>\n"
    "#include <cstring>\n"
    "#include <unistd.h>\n\n"
    "#include <lv2.h>\n\n");
    if (designer->lv2c.midi_input || designer->lv2c.midi_output) {
        printf ("#include <lv2/lv2plug.in/ns/ext/atom/atom.h>\n"
        "#include <lv2/lv2plug.in/ns/ext/atom/util.h>\n"
        "#include <lv2/lv2plug.in/ns/ext/midi/midi.h>\n"
        "#include <lv2/lv2plug.in/ns/ext/urid/urid.h>\n\n");
    }

    printf (""///////////////////////// MACRO SUPPORT ////////////////////////////////\n\n"
    "#define PLUGIN_URI \"%s\"\n\n"
    "#ifndef max\n"
    "#define max(x, y) (((x) > (y)) ? (x) : (y))\n"
    "#endif\n"
    "#ifndef min\n"
    "#define min(x, y) (((x) < (y)) ? (x) : (y))\n"
    "#endif\n\n",  designer->lv2c.uri);
    printf ("typedef int PortIndex;\n\n"
    "////////////////////////////// PLUG-IN CLASS ///////////////////////////\n\n"
    "namespace %s {\n\n"
    "class X%s\n"
    "{\n"
    "private:\n", name, name);

    if (designer->lv2c.midi_input) {
        printf ("    LV2_URID midi_MidiEvent;\n"
        "    LV2_URID_Map* map;\n");
    }

    int i = 0;
    for (;i<designer->lv2c.audio_input;i++) {
        printf ("    float* input%i;\n", i);
    }
    i = 0;
    for (;i<designer->lv2c.audio_output;i++) {
        printf ("    float* output%i;\n", i);
    }
    if (designer->lv2c.midi_input) {
        printf ("    const LV2_Atom_Sequence* midi_in;\n");
    }
    if (designer->lv2c.midi_output) {
        printf ("    LV2_Atom_Sequence* midi_out;\n");
    }
    i = 0;
    for (;i<MAX_CONTROLS;i++) {
        if (designer->controls[i].wid != NULL) {
            if (designer->controls[i].is_type == IS_FRAME ||
                designer->controls[i].is_type == IS_TABBOX) {
                continue;
            }
            if (!designer->controls[i].destignation_enabled) {
                char* var = strdup(designer->controls[i].wid->label);
                strtovar(var),
                printf ("    float* %s;\n"
                "    float %s_;\n", var, var);
                free(var);
                var = NULL;
            } else {
                printf ("    float* bypass;\n"
                "    float bypass_;\n");
            }
        }
    }
    if (designer->lv2c.bypass) {
        printf ("    // bypass ramping\n"
        "    bool needs_ramp_down;\n"
        "    bool needs_ramp_up;\n"
        "    float ramp_down;\n"
        "    float ramp_up;\n"
        "    float ramp_up_step;\n"
        "    float ramp_down_step;\n"
        "    bool bypassed;\n\n");
    }

    printf ("    // private functions\n"
    "    inline void run_dsp_(uint32_t n_samples);\n"
    "    inline void connect_(uint32_t port,void* data);\n"
    "    inline void init_dsp_(uint32_t rate);\n"
    "    inline void connect_all__ports(uint32_t port, void* data);\n"
    "    inline void activate_f();\n"
    "    inline void clean_up();\n"
    "    inline void deactivate_f();\n"

    "public:\n"
    "    // LV2 Descriptor\n"
    "    static const LV2_Descriptor descriptor;\n"
    "    // static wrapper to private functions\n"
    "    static void deactivate(LV2_Handle instance);\n"
    "    static void cleanup(LV2_Handle instance);\n"
    "    static void run(LV2_Handle instance, uint32_t n_samples);\n"
    "    static void activate(LV2_Handle instance);\n"
    "    static void connect_port(LV2_Handle instance, uint32_t port, void* data);\n"
    "    static LV2_Handle instantiate(const LV2_Descriptor* descriptor,\n"
    "                                double rate, const char* bundle_path,\n"
    "                                const LV2_Feature* const* features);\n"
    "    X%s();\n"
    "    ~X%s();\n"
    "};\n\n", name, name);

    printf ("// constructor\n"
    "X%s::X%s() :\n", name, name);
    bool add_comma = false;
    i = 0;
    for (;i<MAX_CONTROLS;i++) {
        if (designer->controls[i].wid != NULL) {
            if (designer->controls[i].is_type == IS_FRAME ||
                designer->controls[i].is_type == IS_TABBOX) {
                continue;
            }
            if (!designer->controls[i].destignation_enabled) {
                char* var = strdup(designer->controls[i].wid->label);
                strtovar(var);
                printf ("%s\n    %s(NULL)",add_comma ? "," : "", var);
                free(var);
                var = NULL;
                add_comma = true;
            } else {
                printf ("%s\n    bypass(NULL)",add_comma ? "," : "");
                add_comma = true;
                printf ("%s\n    bypass_(2)",add_comma ? "," : "");
            }
        }
    }
    i = 0;
    for (;i<designer->lv2c.audio_input;i++) {
        printf ("%s\n    input%i(NULL)", add_comma ? "," : "", i);
        add_comma = true;
    }
    i = 0;
    for (;i<designer->lv2c.audio_output;i++) {
        printf ("%s\n    output%i(NULL)", add_comma ? "," : "", i);
        add_comma = true;
    }
    if (designer->lv2c.midi_input) {
        printf ("%s\n    midi_in(NULL)", add_comma ? "," : "");
        add_comma = true;
    }
    if (designer->lv2c.midi_output) {
        printf ("%s\n    midi_out(NULL)", add_comma ? "," : "");
        add_comma = true;
    }
    if (designer->lv2c.bypass) {
        printf ("%s\n    needs_ramp_down(false),\n"
        "    needs_ramp_up(false),\n"
        "    bypassed(false)", add_comma ? "," : "");
    }
    printf (" {};\n\n");

    printf ("// destructor\n"
    "X%s::~X%s() { };\n\n", name, name);


    printf ("///////////////////////// PRIVATE CLASS  FUNCTIONS /////////////////////\n\n"
    "void X%s::init_dsp_(uint32_t rate)\n"
    "{\n", name);
    if (designer->lv2c.bypass) {
        printf ("    // set values for internal ramping\n"
        "    ramp_down_step = 32 * (256 * rate) / 48000; \n"
        "    ramp_up_step = ramp_down_step;\n"
        "    ramp_down = ramp_down_step;\n"
        "    ramp_up = 0.0;\n");
    }
    printf ("}\n\n");

    printf ("// connect the Ports used by the plug-in class\n"
    "void X%s::connect_(uint32_t port,void* data)\n"
    "{\n"
    "    switch ((PortIndex)port)\n"
    "    {\n", name);
    i = 0;
    int p = 0;
    for (;i<designer->lv2c.audio_input;i++) {
        printf ("        case %i:\n"
                "            input%i = static_cast<float*>(data);\n"
                "            break;\n", p, i);
        p++;
    }
    i = 0;
    for (;i<designer->lv2c.audio_output;i++) {
        printf ("        case %i:\n"
                "            output%i = static_cast<float*>(data);\n"
                "            break;\n", p, i);
        p++;
    }
    if (designer->lv2c.midi_input) {
        printf ("        case %i:\n"
                "            midi_in = (const LV2_Atom_Sequence*)data;\n"
                "            break;\n", p);
        p++;
    }
    if (designer->lv2c.midi_output) {
        printf ("        case %i:\n"
                "            midi_out = (LV2_Atom_Sequence*)data;\n"
                "            break;\n", p);
        p++;
    }
    i = 0;
    for (;i<MAX_CONTROLS;i++) {
        if (designer->controls[i].wid != NULL) {
            if (designer->controls[i].is_type == IS_FRAME ||
                designer->controls[i].is_type == IS_TABBOX) {
                continue;
            }
            if (!designer->controls[i].destignation_enabled) {
                char* var = strdup(designer->controls[i].wid->label);
                strtovar(var);
                printf ("        case %i:\n"
                        "            %s = static_cast<float*>(data);\n"
                        "            break;\n", p, var);
                p++;
                free(var);
                var = NULL;
            } else {
                printf ("        case %i:\n"
                        "            bypass = static_cast<float*>(data);\n"
                        "            break;\n", p);
                p++;
            }
        }
    }
        printf ("        default:\n"
                "            break;\n"
                "    }\n"
    "}\n\n");

    printf ("void X%s::activate_f()\n"
    "{\n"
    "    // allocate the internal DSP mem\n"
    "}\n\n"

    "void X%s::clean_up()\n"
    "{\n"
    "    // delete the internal DSP mem\n"
    "}\n\n"

    "void X%s::deactivate_f()\n"
    "{\n"
    "    // delete the internal DSP mem\n"
    "}\n\n", name, name, name);

    printf ("void X%s::run_dsp_(uint32_t n_samples)\n"
    "{\n"
    "    if(n_samples<1) return;\n\n", name);

    i = 0;
    printf ("    // get controller values\n");
    for (;i<MAX_CONTROLS;i++) {
        if (designer->controls[i].wid != NULL) {
            if (designer->controls[i].is_type == IS_FRAME ||
                designer->controls[i].is_type == IS_TABBOX) {
                continue;
            }
            if (!designer->controls[i].destignation_enabled) {
                char* var = strdup(designer->controls[i].wid->label);
                strtovar(var);
                printf ("#define  %s_ (*(%s));\n", var, var);
                free(var);
                var = NULL;
            }
        }
    }

    if (designer->lv2c.midi_input) {
        printf ("    LV2_ATOM_SEQUENCE_FOREACH(midi_in, ev) {\n"
        "        if (ev->body.type == midi_MidiEvent) {\n"
        "            const uint8_t* const msg = (const uint8_t*)(ev + 1);\n"
        "            switch (lv2_midi_message_type(msg)) {\n"
        "            case LV2_MIDI_MSG_NOTE_ON:\n"
        "                //note_on = msg[1];\n"
        "            break;\n"
        "            case LV2_MIDI_MSG_NOTE_OFF:\n"
        "                //note_off = msg[1];\n"
        "            break;\n"
        "            case LV2_MIDI_MSG_CONTROLLER:\n"
        "                switch (msg[1]) {\n"
        "                    case LV2_MIDI_CTL_MSB_MODWHEEL:\n"
        "                    case LV2_MIDI_CTL_LSB_MODWHEEL:\n"
        "                        //vowel = (float) (msg[2]);\n"
        "                    break;\n"
        "                    case LV2_MIDI_CTL_ALL_SOUNDS_OFF:\n"
        "                    case LV2_MIDI_CTL_ALL_NOTES_OFF:\n"
        "                        //\n"
        "                    break;\n"
        "                    case LV2_MIDI_CTL_RESET_CONTROLLERS:\n"
        "                        //pitchbend = 0.0;\n"
        "                        //vowel = 0.0;\n"
        "                    break;\n"
        "                    default:\n"
        "                    break;\n"
        "                }\n"
        "            break;\n"
        "            case LV2_MIDI_MSG_BENDER:\n"
        "                //pitchbend = ((msg[2] << 7 | msg[1]) - 8192) * 0.00146484375;\n"
        "            break;\n"
        "            default:\n"
        "            break;\n"
        "            }\n"
        "        }\n"
        "    }\n");
    }

    i = 0;
    for (;i<designer->lv2c.audio_input;i++) {
        if (i <=designer->lv2c.audio_output) {
            printf ("\n    // do inplace processing on default\n"
            "    if(output%i != input%i)\n"
            "        memcpy(output%i, input%i, n_samples*sizeof(float));\n\n", i,i,i,i);
        } else {
            printf ("    // audio input and output count is not equal\n"
            "    // you must handle them yourself\n\n");
        }
    }

    if (designer->lv2c.bypass) {
        if (designer->lv2c.audio_input != designer->lv2c.audio_output) {
            printf ("     // audio input and output count is not equal\n"
            "    // you must handle bypassing yourself\n\n");
        } else {
            i = 0;
            for (;i<designer->lv2c.audio_input;i++) {
                printf ("    float buf%i[n_samples];\n", i);
            }

            printf ("    // check if bypass is pressed\n"
            "    if (bypass_ != static_cast<uint32_t>(*(bypass))) {\n"
            "        bypass_ = static_cast<uint32_t>(*(bypass));\n"
            "        if (!bypass_) {\n"
            "            needs_ramp_down = true;\n"
            "            needs_ramp_up = false;\n"
            "        } else {\n"
            "            needs_ramp_down = false;\n"
            "            needs_ramp_up = true;\n"
            "            bypassed = false;\n"
            "        }\n"
            "    }\n\n"

            "    if (needs_ramp_down || needs_ramp_up) {\n");
            i = 0;
            for (;i<designer->lv2c.audio_input;i++) {
                printf ("         memcpy(buf%i, input%i, n_samples*sizeof(float));\n", i, i);
            }
            printf ("    }\n"

            "    if (!bypassed) {\n    ");
        }
    }
    printf ("    for (uint32_t i = 0; i<n_samples; i++) {\n");
    i = 0;
    for (;i<designer->lv2c.audio_output;i++) {
        printf("        float tmp%i = output%i[i];\n"
        "        //do your dsp\n"
        "        output%i[i] = tmp%i;\n", i, i, i, i);
    }
    printf ("    }\n\n");
    if (designer->lv2c.bypass) {
        if (designer->lv2c.audio_input != designer->lv2c.audio_output) {
            printf ("     // audio input and output count is not equal\n"
            "    // you must handle bypassing yourself\n\n");
        } else {
        
            printf ("    }\n"

            "    // check if ramping is needed\n"
            "    if (needs_ramp_down) {\n"
            "        float fade = 0;\n"
            "        for (uint32_t i=0; i<n_samples; i++) {\n"
            "            if (ramp_down >= 0.0) {\n"
            "                --ramp_down; \n"
            "            }\n"
            "            fade = max(0.0,ramp_down) /ramp_down_step ;\n");
            i = 0;
            for (;i<designer->lv2c.audio_input;i++) {            
                printf ("            output%i[i] = output%i[i] * fade + buf%i[i] * (1.0 - fade);\n", i, i, i);
            }
            printf ("        }\n"

            "        if (ramp_down <= 0.0) {\n"
            "            // when ramped down, clear buffer from dsp\n"
            "            needs_ramp_down = false;\n"
            "            bypassed = true;\n"
            "            ramp_down = ramp_down_step;\n"
            "            ramp_up = 0.0;\n"
            "        } else {\n"
            "            ramp_up = ramp_down;\n"
            "        }\n"

            "    } else if (needs_ramp_up) {\n"
            "        float fade = 0;\n"
            "        for (uint32_t i=0; i<n_samples; i++) {\n"
            "            if (ramp_up < ramp_up_step) {\n"
            "                ++ramp_up ;\n"
            "            }\n"
            "            fade = min(ramp_up_step,ramp_up) /ramp_up_step ;\n");
            i = 0;
            for (;i<designer->lv2c.audio_input;i++) {            
                printf ("            output%i[i] = output%i[i] * fade + buf%i[i] * (1.0 - fade);\n", i, i, i);
            }
            printf ("        }\n"

            "        if (ramp_up >= ramp_up_step) {\n"
            "            needs_ramp_up = false;\n"
            "            ramp_up = 0.0;\n"
            "            ramp_down = ramp_down_step;\n"
            "        } else {\n"
            "            ramp_down = ramp_up;\n"
            "        }\n"
            "    }\n");
        }
    }
    i = 0;
    for (;i<MAX_CONTROLS;i++) {
        if (designer->controls[i].wid != NULL) {
            if (designer->controls[i].is_type == IS_FRAME ||
                designer->controls[i].is_type == IS_TABBOX) {
                continue;
            }
            if (!designer->controls[i].destignation_enabled) {
                char* var = strdup(designer->controls[i].wid->label);
                strtovar(var);
                printf ("#undef  %s_\n", var);
                free(var);
                var = NULL;
            }
        }
    }
    printf("}\n\n"

    "void X%s::connect_all__ports(uint32_t port, void* data)\n"
    "{\n"
    "    // connect the Ports used by the plug-in class\n"
    "    connect_(port,data); \n"
    "}\n\n", name);

    printf("////////////////////// STATIC CLASS  FUNCTIONS  ////////////////////////\n\n"

    "LV2_Handle \n"
    "X%s::instantiate(const LV2_Descriptor* descriptor,\n"
    "                            double rate, const char* bundle_path,\n"
    "                            const LV2_Feature* const* features)\n"
    "{\n",name);
    if (designer->lv2c.midi_input) {
        printf("    LV2_URID_Map* map = NULL;\n"
        "    for (int i = 0; features[i]; ++i) {\n"
        "        if (!strcmp(features[i]->URI, LV2_URID__map)) {\n"
        "            map = (LV2_URID_Map*)features[i]->data;\n"
        "            break;\n"
        "        }\n"
        "    }\n"
        "    if (!map) {\n"
        "        return NULL;\n"
        "    }\n");
    }
    printf("    // init the plug-in class\n"
    "    X%s *self = new X%s();\n"
    "    if (!self) {\n"
    "        return NULL;\n"
    "    }\n", name, name);
    if (designer->lv2c.midi_input) {
        printf("    self->map = map;\n"
        "    self->midi_MidiEvent = map->map(map->handle, LV2_MIDI__MidiEvent);\n");
    }
    printf("    self->init_dsp_((uint32_t)rate);\n"

    "    return (LV2_Handle)self;\n"
    "}\n\n");

    printf("void X%s::connect_port(LV2_Handle instance, \n"
    "                                   uint32_t port, void* data)\n"
    "{\n"
    "    // connect all ports\n"
    "    static_cast<X%s*>(instance)->connect_all__ports(port, data);\n"
    "}\n\n"

    "void X%s::activate(LV2_Handle instance)\n"
    "{\n"
    "    // allocate needed mem\n"
    "    static_cast<X%s*>(instance)->activate_f();\n"
    "}\n\n"

    "void X%s::run(LV2_Handle instance, uint32_t n_samples)\n"
    "{\n"
    "    // run dsp\n"
    "    static_cast<X%s*>(instance)->run_dsp_(n_samples);\n"
    "}\n\n"

    "void X%s::deactivate(LV2_Handle instance)\n"
    "{\n"
    "    // free allocated mem\n"
    "    static_cast<X%s*>(instance)->deactivate_f();\n"
    "}\n\n"

    "void X%s::cleanup(LV2_Handle instance)\n"
    "{\n"
    "    // well, clean up after us\n"
    "    X%s* self = static_cast<X%s*>(instance);\n"
    "    self->clean_up();\n"
    "    delete self;\n"
    "}\n\n", name, name, name, name, name, name, name, name, name, name, name);

    printf("const LV2_Descriptor X%s::descriptor =\n"
    "{\n"
    "    PLUGIN_URI ,\n"
    "    X%s::instantiate,\n"
    "    X%s::connect_port,\n"
    "    X%s::activate,\n"
    "    X%s::run,\n"
    "    X%s::deactivate,\n"
    "    X%s::cleanup,\n"
    "    NULL\n"
    "};\n\n", name, name, name, name, name, name, name);


    printf("} // end namespace %s\n\n"

    "////////////////////////// LV2 SYMBOL EXPORT ///////////////////////////\n\n", name);

    printf("extern \"C\"\n"
    "LV2_SYMBOL_EXPORT\n"
    "const LV2_Descriptor*\n"
    "lv2_descriptor(uint32_t index)\n"
    "{\n"
    "    switch (index)\n"
    "    {\n"
    "        case 0:\n"
    "            return &%s::X%s::descriptor;\n"
    "        default:\n"
    "            return NULL;\n"
    "    }\n"
    "}\n\n"

    "///////////////////////////// FIN //////////////////////////////////////\n", name, name);
}

void print_manifest(XUiDesigner *designer) {
    char *name = NULL;
    XFetchName(designer->ui->app->dpy, designer->ui->widget, &name);
    if (name == NULL) asprintf(&name, "%s", "noname");
    strdecode(name, " ", "_");

    printf ("\n@prefix lv2:  <http://lv2plug.in/ns/lv2core#> .\n"
        "@prefix rdfs: <http://www.w3.org/2000/01/rdf-schema#> .\n"

        "<%s>\n"
        "    a lv2:Plugin ;\n"
        "    lv2:binary <%s.so> ;\n"
        "    rdfs:seeAlso <%s.ttl> .\n", designer->lv2c.uri, name, name);
    free(name);
}

void print_ttl(XUiDesigner *designer) {
    char *name = NULL;
    XFetchName(designer->ui->app->dpy, designer->ui->widget, &name);
    if (name == NULL) asprintf(&name, "%s", "noname");
    printf ("\n@prefix doap: <http://usefulinc.com/ns/doap#> .\n"
        "@prefix foaf: <http://xmlns.com/foaf/0.1/> .\n"
        "@prefix lv2: <http://lv2plug.in/ns/lv2core#> .\n"
        "@prefix rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#> .\n"
        "@prefix rdfs: <http://www.w3.org/2000/01/rdf-schema#> .\n"
        "@prefix guiext: <http://lv2plug.in/ns/extensions/ui#>.\n"
        "@prefix time: <http://lv2plug.in/ns/ext/time#>.\n"
        "@prefix units: <http://lv2plug.in/ns/extensions/units#> .\n"
        "@prefix atom:  <http://lv2plug.in/ns/ext/atom#> .\n"
        "@prefix urid:  <http://lv2plug.in/ns/ext/urid#> .\n"
        "@prefix pprop: <http://lv2plug.in/ns/ext/port-props#> .\n"
        "@prefix midi:  <http://lv2plug.in/ns/ext/midi#> .\n"
        "@prefix patch: <http://lv2plug.in/ns/ext/patch#> .\n\n\n");


    printf ("<urn:name#me>\n"
            "   a foaf:Person ;\n"
            "   foaf:name \"%s\" .\n\n"

        "<%s>\n"
            "   a lv2:Plugin ,\n"
            "       lv2:%s ;\n"
            "   doap:maintainer <urn:name#me> ;\n"
            "   doap:name \"%s\" ;\n"
            "   lv2:project <%s> ;\n"
            "   lv2:requiredFeature urid:map ;\n"
            "   lv2:optionalFeature lv2:hardRTCapable ;\n"
              
            "   lv2:minorVersion 1 ;\n"
            "   lv2:microVersion 0 ;\n\n"

        "guiext:ui <%s> ;\n\n"
            
        "rdfs:comment \"\"\"\n"

        "...\n"

        "\"\"\";\n\n", designer->lv2c.author, designer->lv2c.uri, designer->lv2c.plugintype,
                name, designer->lv2c.uri, designer->lv2c.ui_uri );


    int i = 0;
    int p = 0;
    bool add_comma = false;
    printf ("   lv2:port ");
    for (;i<designer->lv2c.audio_input;i++) {
        printf ("%s [\n"
        "       a lv2:AudioPort ,\n"
        "          lv2:InputPort ;\n"
        "      lv2:index %i ;\n"
        "      lv2:symbol \"in%i\" ;\n"
        "      lv2:name \"In%i\" ;\n"
        "   ]", add_comma ? ",": "", p, i, i);
        p++;
        add_comma = true;
    }
    i = 0;
    for (;i<designer->lv2c.audio_output;i++) {
        printf ("%s [\n"
        "      a lv2:AudioPort ,\n"
        "           lv2:OutputPort ;\n"
        "      lv2:index %i ;\n"
        "      lv2:symbol \"out%i\" ;\n"
        "      lv2:name \"Out%i\" ;\n"
        "   ]", add_comma ? ",": "", p, i, i);
        p++;
        add_comma = true;
    }
    if (designer->lv2c.midi_input) {

        printf ("%s [\n"
        "      a lv2:InputPort ,\n"
        "          atom:AtomPort ;\n"
        "      atom:bufferType atom:Sequence ;\n"
        "      atom:supports midi:MidiEvent ,\n"
        "           patch:Message ;\n"
        "      lv2:designation lv2:control ;\n"
        "      lv2:index %i ;\n"
        "      lv2:symbol \"MIDI_IN\" ;\n"
        "      lv2:name \"MIDI_IN\" ;\n"
        "   ]", add_comma ? ",": "", p);
        p++;
        add_comma = true;
    }
    if (designer->lv2c.midi_output) {

        printf ("%s [\n"
        "      a lv2:OutputPort ,\n"
        "          atom:AtomPort ;\n"
        "      atom:bufferType atom:Sequence ;\n"
        "      atom:supports midi:MidiEvent ,\n"
        "           patch:Message ;\n"
        "      lv2:designation lv2:control ;\n"
        "      lv2:index %i ;\n"
        "      lv2:symbol \"MIDI_OUT\" ;\n"
        "      lv2:name \"MIDI_OUT\" ;\n"
        "   ]", add_comma ? ",": "", p);
        p++;
        add_comma = true;
    }
    i = 0;
    for (;i<MAX_CONTROLS;i++) {
        if (designer->controls[i].wid != NULL) {
            Widget_t * wid = designer->controls[i].wid;
            if (designer->controls[i].is_type == IS_FRAME ||
                designer->controls[i].is_type == IS_TABBOX) {
                continue;
            } else {
                if (designer->controls[i].have_adjustment) {
                    if (designer->controls[i].is_type == IS_COMBOBOX) {
                        Widget_t *menu = wid->childlist->childs[1];
                        Widget_t* view_port =  menu->childlist->childs[0];
                        ComboBox_t *comboboxlist = (ComboBox_t*)view_port->parent_struct;
                        printf (", [\n"
                            "      a lv2:InputPort ,\n"
                            "          lv2:ControlPort ;\n"
                            "      lv2:index %i ;\n"
                            "      lv2:symbol \"%s\" ;\n"
                            "      lv2:name \"%s\" ;\n"
                            "      lv2:default %.1f ;\n"
                            "      lv2:minimum %.1f ;\n"
                            "      lv2:maximum %.1f ;\n"
                            "      lv2:portProperty lv2:integer ;\n"
                            "      lv2:portProperty lv2:enumeration ;\n"
                                , designer->is_project ? p : designer->controls[i].port_index, designer->controls[i].wid->label,
                                designer->controls[i].wid->label, wid->adj->std_value,
                                wid->adj->min_value, wid->adj->max_value);
                        unsigned int k = 0;
                        int l = (int)wid->adj->min_value;
                        for(; k<comboboxlist->list_size;k++) {
                            printf ("      lv2:scalePoint [rdfs:label \"%s\"; rdf:value %i];\n", comboboxlist->list_names[k],l);
                            l++;
                        }
                        printf ("]");
                    } else if (designer->controls[i].is_type == IS_VMETER ||
                            designer->controls[i].is_type == IS_HMETER) {
                        printf (", [\n"
                            "      a lv2:OutputPort ,\n"
                            "          lv2:ControlPort ;\n"
                            "      lv2:index %i ;\n"
                            "      lv2:symbol \"%s\" ;\n"
                            "      lv2:name \"%s\" ;\n"
                            "      lv2:default %f ;\n"
                            "      lv2:minimum %f ;\n"
                            "      lv2:maximum %f ;\n"
                            "   ]", designer->is_project ? p : designer->controls[i].port_index, designer->controls[i].wid->label,
                                designer->controls[i].wid->label, wid->adj->std_value,
                                wid->adj->min_value, wid->adj->max_value);
                    } else {
                        printf (", [\n"
                            "      a lv2:InputPort ,\n"
                            "          lv2:ControlPort ;\n"
                            "      lv2:index %i ;\n"
                            "      lv2:symbol \"%s\" ;\n"
                            "      lv2:name \"%s\" ;\n"
                            "      lv2:default %f ;\n"
                            "      lv2:minimum %f ;\n"
                            "      lv2:maximum %f ;\n"
                            "   ]", designer->is_project ? p : designer->controls[i].port_index, designer->controls[i].wid->label,
                                designer->controls[i].wid->label, wid->adj->std_value,
                                wid->adj->min_value, wid->adj->max_value);
                    }
                } else if (designer->controls[i].is_type == IS_TOGGLE_BUTTON ||
                        designer->controls[i].is_type == IS_IMAGE_TOGGLE) {
                    printf (", [\n"
                        "      a lv2:InputPort ,\n"
                        "          lv2:ControlPort ;\n"
                        "      lv2:index %i ;\n"
                        "%s"
                        "      lv2:portProperty lv2:toggled ;\n"
                        "      lv2:symbol \"%s\" ;\n"
                        "      lv2:name \"%s\" ;\n"
                        "      lv2:default %i ;\n"
                        "      lv2:minimum 0 ;\n"
                        "      lv2:maximum 1 ;\n"
                        "   ]", designer->is_project ? p : designer->controls[i].port_index,
                            designer->controls[i].destignation_enabled ? "      lv2:designation lv2:enabled;\n" : "",
                            designer->controls[i].wid->label, designer->controls[i].wid->label,
                            designer->controls[i].destignation_enabled ? 1 : 0);
                } else if (designer->controls[i].is_type == IS_BUTTON) {
                    printf (", [\n"
                        "      a lv2:InputPort ,\n"
                        "          lv2:ControlPort ;\n"
                        "      lv2:index %i ;\n"
                        "      lv2:portProperty lv2:toggled, pprop:trigger ;\n"
                        "      lv2:symbol \"%s\" ;\n"
                        "      lv2:name \"%s\" ;\n"
                        "      lv2:default 0 ;\n"
                        "      lv2:minimum 0 ;\n"
                        "      lv2:maximum 1 ;\n"
                        "   ]", designer->is_project ? p : designer->controls[i].port_index,
                             designer->controls[i].wid->label, designer->controls[i].wid->label);
                }
            }
        }
        p++;
    }
    printf (" .\n\n");

    strdecode(name, " ", "_");
    printf ("<%s>\n"
        "   a guiext:X11UI;\n"
        "   guiext:binary <%s_ui.so> ;\n"
        "       lv2:extensionData guiext::idle ;\n"
        "       lv2:extensionData guiext:resize ;\n"
        "       lv2:extensionData guiext:idleInterface ;\n"
        "       lv2:requiredFeature guiext:idleInterface ;\n"
        "   .\n", designer->lv2c.ui_uri, name);
    free(name);
}

void print_list(XUiDesigner *designer) {
    int i = 0;
    int j = 0;
    int k = 0;
    int l = 0;
    int p = designer->lv2c.audio_input + designer->lv2c.audio_output +
        designer->lv2c.midi_input + designer->lv2c.midi_output;
    bool have_image = false;
    for (;i<MAX_CONTROLS;i++) {
        if (designer->controls[i].wid != NULL && (designer->controls[i].is_type != IS_FRAME &&
                                                designer->controls[i].is_type != IS_TABBOX)) {
            j++;
        } else if (designer->controls[i].wid != NULL && (designer->controls[i].is_type == IS_FRAME ||
                                                        designer->controls[i].is_type == IS_TABBOX)) {
            k++;
            if (designer->controls[i].is_type == IS_TABBOX) {
                l += designer->controls[i].wid->childlist->elem;
            }
        }
        if (designer->controls[i].image) {
            have_image = true;
        }
    }
    if (designer->image != NULL) {
        have_image = true;
    }
    if (j) {
        Window w = (Window)designer->ui->widget;
        char *name;
        XFetchName(designer->ui->app->dpy, w, &name);

        printf ("\n#define CONTROLS %i\n", j);
        printf ("\n#define GUI_ELEMENTS %i\n", k);
        printf ("\n#define TAB_ELEMENTS %i\n\n", l);
        printf ("\n#define PLUGIN_UI_URI \"%s\"\n\n",designer->lv2c.ui_uri);
        printf ("\n#include \"lv2_plugin.h\"\n\n");
        
        if (have_image && !designer->run_test) printf ("\n#include \"xresources.h\"\n\n");
        print_colors(designer);
        printf ("#include \"%s\"\n\n\n"
        "void plugin_value_changed(X11_UI *ui, Widget_t *w, PortIndex index) {\n"
        "    // do special stuff when needed\n"
        "}\n\n"
        "void plugin_set_window_size(int *w,int *h,const char * plugin_uri) {\n"
        "    (*w) = %i; //set initial width of main window\n"
        "    (*h) = %i; //set initial height of main window\n"
        "}\n\n"
        "const char* plugin_set_name() {\n"
        "    return \"%s\"; //set plugin name to display on UI\n"
        "}\n\n"
        "void plugin_create_controller_widgets(X11_UI *ui, const char * plugin_uri) {\n"
        "    set_costum_theme(&ui->main);\n"
        , designer->run_test? "ui_test.cc": "lv2_plugin.cc", designer->ui->width, designer->ui->height, name? name:"Test");
        if (designer->image != NULL) {
            if (designer->run_test) {
                printf ("    load_bg_image(ui,\"%s\");\n", designer->image);
            } else {
                char * xldl = strdup(basename(designer->image));
                strdecode(xldl, ".", "_");
                strdecode(xldl, "-", "_");
                strdecode(xldl, " ", "_");
                strtovar(xldl);
                printf ("    widget_get_scaled_png(ui->win, LDVAR(%s));\n", xldl);
                free(xldl);
                //printf ("    load_bg_image(ui,\"./resources/%s\");\n", basename(designer->image));
            }
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
            if (designer->controls[i].is_type == IS_FRAME) {
                printf ("    ui->elem[%i] = %s (ui->elem[%i], ui->win, %i, \"%s\", ui, %i,  %i, %i, %i);\n", 
                    j, designer->controls[i].type, j,
                    designer->is_project ? p : designer->controls[i].port_index, designer->controls[i].wid->label,
                    designer->controls[i].wid->x, designer->controls[i].wid->y,
                    designer->controls[i].wid-> width, designer->controls[i].wid->height);
                if (designer->controls[i].image != NULL ) {
                    if (designer->run_test) {
                        printf ("    load_controller_image(ui->elem[%i], \"%s\");\n",
                                            j, designer->controls[i].image);
                    } else {
                        char * xldl = strdup(basename(designer->controls[i].image));
                        strdecode(xldl, ".", "_");
                        strdecode(xldl, "-", "_");
                        strdecode(xldl, " ", "_");
                        strtovar(xldl);
                        printf ("    widget_get_scaled_png(ui->elem[%i], LDVAR(%s));\n",
                                j, xldl);
                        free(xldl);
                       // printf ("    load_controller_image(ui->elem[%i], \"./resources/%s\");\n",
                       //                         j, basename(designer->controls[i].image));
                    }
                }
                j++;
            } else if (designer->controls[i].is_type == IS_TABBOX) {
                printf ("    ui->elem[%i] = %s (ui->elem[%i], ui->win, %i, \"%s\", ui, %i,  %i, %i, %i);\n", 
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
            Widget_t * wid = designer->controls[i].wid;
            if (designer->controls[i].is_type == IS_FRAME ||
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
                printf ("    ui->widget[%i] = %s (ui->widget[%i], %s, %i, \"%s\", ui, %i,  %i, %i, %i);\n", 
                    j, designer->controls[i].type, j, parent,
                    designer->is_project ? p : designer->controls[i].port_index, designer->controls[i].wid->label,
                    designer->controls[i].wid->x, designer->controls[i].wid->y,
                    designer->controls[i].wid-> width, designer->controls[i].wid->height);
                free(parent);
            }
            if (designer->controls[i].is_atom_patch && designer->controls[i].is_type != IS_FILE_BUTTON) {
                const char* uri = (const char*) wid->parent_struct;
                printf("    ui->widget[%i]->parent_struct = (void*)\"%s\";\n", j, uri);
            }
            if (designer->controls[i].image != NULL ) {
                if (designer->run_test) {
                    printf ("    load_controller_image(ui->widget[%i], \"%s\");\n",
                            j, designer->controls[i].image);
                } else {
                    char * xldl = strdup(basename(designer->controls[i].image));
                    strdecode(xldl, ".", "_");
                    strdecode(xldl, "-", "_");
                    strdecode(xldl, " ", "_");
                    strtovar(xldl);
                    printf ("    widget_get_png(ui->widget[%i], LDVAR(%s));\n",
                    //printf ("    load_controller_image(ui->widget[%i], \"./resources/%s\");\n",
                            j, xldl);
                    free(xldl);
                }
            }
            if (designer->controls[i].is_type == IS_COMBOBOX) {
                Widget_t *menu = wid->childlist->childs[1];
                Widget_t* view_port =  menu->childlist->childs[0];
                ComboBox_t *comboboxlist = (ComboBox_t*)view_port->parent_struct;
                unsigned int k = 0;
                for(; k<comboboxlist->list_size;k++) {
                    printf ("    combobox_add_entry (ui->widget[%i], \"%s\");\n", j, comboboxlist->list_names[k]);
                }
            }
            if (designer->controls[i].have_adjustment) {
                if (wid->adj->type == CL_LOGARITHMIC) {
                    printf ("    set_adjustment(ui->widget[%i]->adj, %.3f, %.3f, %.3f, %.3f, %.3f, %s);\n", 
                        j, powf(10,wid->adj->std_value), powf(10,wid->adj->std_value), powf(10,wid->adj->min_value),
                        powf(10,wid->adj->max_value), wid->adj->step, parse_adjusment_type(wid->adj->type));
                    
                } else if (wid->adj->type == CL_LOGSCALE) {
                    printf ("    set_adjustment(ui->widget[%i]->adj, %.3f, %.3f, %.3f, %.3f, %.3f, %s);\n", 
                        j, log10(wid->adj->std_value)*wid->adj->log_scale, log10(wid->adj->std_value)*wid->adj->log_scale,
                        log10(wid->adj->min_value)*wid->adj->log_scale, log10(wid->adj->max_value)*wid->adj->log_scale,
                        wid->adj->step, parse_adjusment_type(wid->adj->type));
                    
                } else {
                    printf ("    set_adjustment(ui->widget[%i]->adj, %.3f, %.3f, %.3f, %.3f, %.3f, %s);\n", 
                        j, wid->adj->std_value, wid->adj->std_value, wid->adj->min_value, wid->adj->max_value,
                        wid->adj->step, parse_adjusment_type(wid->adj->type));
                }
            }
            printf ("\n");
            if (designer->controls[i].is_type != IS_FRAME) j++;
            if (designer->controls[i].is_type != IS_FRAME) p++;
        }
    }
    printf ("}\n\n"
    "void plugin_cleanup(X11_UI *ui) {\n"
    "    // clean up used sources when needed\n"
    "}\n\n"

    "void plugin_port_event(LV2UI_Handle handle, uint32_t port_index,\n"
    "                        uint32_t buffer_size, uint32_t format,\n"
    "                        const void * buffer) {\n"
    "    // port value change message from host\n"
    "    // do special stuff when needed\n"
    "}\n\n");

}

void run_save(void *w_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    XUiDesigner *designer = (XUiDesigner*)w->parent_struct;
    if(user_data !=NULL) {
        if( access(*(const char**)user_data, F_OK ) == -1 ) {
            Widget_t *dia = open_message_dialog(w, ERROR_BOX, *(const char**)user_data,
                                                _("Couldn't access file, sorry"),NULL);
            XSetTransientForHint(w->app->dpy, dia->widget, w->widget);
            return;
        }
        
        int i = 0;
        int j = 0;
        bool have_image = false;
        for (;i<MAX_CONTROLS;i++) {
            if (designer->controls[i].wid != NULL) {
                j++;
            }
            if (designer->controls[i].image) {
                have_image = true;
            }
        }
        if (!j) {
            Widget_t *dia = open_message_dialog(designer->ui, INFO_BOX, _("INFO"),
                                            _("Please create at least one Controller,|or load a LV2 URI to save a build "),NULL);
            XSetTransientForHint(w->app->dpy, dia->widget, designer->ui->widget);
            return;
        }
        Window w = (Window)designer->ui->widget;
        char *name = NULL;
        XFetchName(designer->ui->app->dpy, w, &name);
        if (name == NULL) asprintf(&name, "%s", "noname");
        strdecode(name, " ", "_");
        char* filepath = NULL;
        asprintf(&filepath, "%s%s_ui",*(const char**)user_data,name);
        struct stat st = {0};

        if (stat(filepath, &st) == -1) {
            mkdir(filepath, 0700);
        }

        char* filename = NULL;
        asprintf(&filename, "%s%s_ui/%s.c",*(const char**)user_data,name, name );
        remove (filename);
        FILE *fp;
        if((fp=freopen(filename, "w" ,stdout))==NULL) {
            printf("open failed\n");
        }
        fprintf(stderr, "save to %s\n", filename);
        print_list(designer);
        fclose(fp);
        fp = NULL;
        strdecode(filename, ".c", ".cpp");
        if((fp=freopen(filename, "w" ,stdout))==NULL) {
            printf("open failed\n");
        }
        print_plugin(designer);
        fclose(fp);
        fp = NULL;
        strdecode(filename, ".cpp", ".ttl");
        if((fp=freopen(filename, "w" ,stdout))==NULL) {
            printf("open failed\n");
        }
        print_ttl(designer);
        fclose(fp);
        fp = NULL;
        free(filename);
        filename = NULL;
        asprintf(&filename, "%s%s_ui/manifest.ttl",*(const char**)user_data,name);
        if((fp=freopen(filename, "w" ,stdout))==NULL) {
            printf("open failed\n");
        }
        print_manifest(designer);
        fclose(fp);
        fp = NULL;
        free(filename);
        filename = NULL;
        if (system(NULL)) {
            char* cmd = NULL;
            asprintf(&cmd, "cp /usr/share/XUiDesigner/wrapper/libxputty/lv2_plugin.* \'%s\'", filepath);
            int ret = system(cmd);
            if (!ret) {
                free(cmd);
                asprintf(&cmd,"\n\n	# check if user is root\n"
                    "	user = $(shell whoami)\n"
                    "	ifeq ($(user),root)\n"
                    "	INSTALL_DIR = /usr/lib/lv2\n"
                    "	else \n"
                    "	INSTALL_DIR = ~/.lv2\n"
                    "	endif\n"
                    "\n\n	# check LD version\n"
                    "	ifneq ($(shell xxd --version 2>&1 | head -n 1 | grep xxd),)\n"
                    "		USE_XXD = 1\n"
                    "	else ifneq ($(shell $(LD) --version 2>&1 | head -n 1 | grep LLD),)\n"
                    "		ifneq ($(shell uname -a | grep  x86_64), )\n"
                    "			LDEMULATION := elf_x86_64\n"
                    "		else ifneq ($(shell uname -a | grep amd64), )\n"
                    "			LDEMULATION := elf_x86_64\n"
                    "		else ifneq ($(shell uname -a | grep i386), )\n"
                    "			LDEMULATION := elf_i386\n"
                    "		endif\n"
                    "		USE_LDD = 1\n"
                    "	else ifneq ($(shell gold --version 2>&1 | head -n 1 | grep gold),)\n"
                    "		LD = gold\n"
                    "	endif\n"

                    "\n\n	NAME = %s\n"
                    "	space := $(subst ,, )\n"
                    "	EXEC_NAME := $(subst $(space),_,$(NAME))\n"
                    "	BUNDLE = $(EXEC_NAME).lv2\n"
                    "	RESOURCES_DIR :=./resources/\n"
                    "	LIB_DIR := ../../../libxputty/libxputty/\n"
                    "	HEADER_DIR := $(LIB_DIR)include/\n"
                    "	UI_LIB:= $(LIB_DIR)libxputty.a\n"
                    "	STRIP ?= strip\n\n"
                    "	RESOURCES := $(wildcard $(RESOURCES_DIR)*.png)\n"
                    "	RESOURCES_OBJ := $(notdir $(patsubst %s.png,%s.o,$(RESOURCES)))\n"
                    "	RESOURCES_LIB := $(notdir $(patsubst %s.png,%s.a,$(RESOURCES)))\n"
                    "	RESOURCE_EXTLD := $(notdir $(patsubst %s.png,%s_png,$(RESOURCES)))\n"
                    "	RESOURCEHEADER := xresources.h\n"
                    "	LDFLAGS += -fvisibility=hidden -Wl,-Bstatic `pkg-config --cflags --libs xputty` \\\n"
                    "	-Wl,-Bdynamic `pkg-config --cflags --libs cairo x11 lilv-0` \\\n"
                    "	-shared -lm -fPIC -Wl,-z,noexecstack -Wl,--no-undefined -Wl,--gc-sections\n"
                    "	CFLAGS := -O2 -D_FORTIFY_SOURCE=2 -Wall -fstack-protector -fvisibility=hidden \\\n"
                    "	-fdata-sections -Wl,--gc-sections -Wl,-z,relro,-z,now -Wl,--exclude-libs,ALL\n\n"
                    ".PHONY : all install uninstall\n\n"
                    ".NOTPARALLEL:\n\n"
                    "all: $(RESOURCEHEADER) $(EXEC_NAME)\n\n"
                    "	@mkdir -p ./$(BUNDLE)\n"
                    "	@cp ./*.ttl ./$(BUNDLE)\n"
                    "	@cp ./*.so ./$(BUNDLE)\n"
                    "	@if [ -f ./$(BUNDLE)/$(EXEC_NAME).so ]; then echo \"build finish, now run make install\"; \\\n"
                    "	else echo \"sorry, build failed\"; fi\n\n"
                    "$(RESOURCEHEADER): $(RESOURCES_OBJ)\n"
                    "	rm -f $(RESOURCEHEADER)\n"
                    "	for f in $(RESOURCE_EXTLD); do \\\n"
                    "		echo 'EXTLD('$${f}')' >> $(RESOURCEHEADER) ; \\\n"
                    "	done\n\n"
                    "ifdef USE_XXD\n"
                    "$(RESOURCES_OBJ): $(RESOURCES)\n"
                    "	@#use this line to regenerate the *.c files from used images\n"
                    "	@#cd $(RESOURCES_DIR) && xxd -i $(patsubst %s.o,%s.png,$@) > $(patsubst %s.o,%s.c,$@)\n"
                    "	$(CC) -c $(RESOURCES_DIR)$(patsubst %s.o,%s.c,$@) -o $@\n"
                    "	$(AR) rcs $(patsubst %s.o,%s.a,$@) $@\n"
                    "else ifdef USE_LDD\n"
                    "$(RESOURCES_OBJ): $(RESOURCES)\n"
                    "	cd $(RESOURCES_DIR) && $(LD) -r -b binary -m $(LDEMULATION) -z noexecstack $(patsubst %s.o,%s.png,$@) -o ../$@\n"
                    "	$(AR) rcs $(patsubst %s.o,%s.a,$@) $@\n"
                    "	LDFLAGS += -DUSE_LD=1\n"
                    "else\n"
                    "$(RESOURCES_OBJ): $(RESOURCES)\n"
                    "	cd $(RESOURCES_DIR) && $(LD) -r -b binary -z noexecstack --strip-all $(patsubst %s.o,%s.png,$@) -o ../$@\n"
                    "	$(AR) rcs $(patsubst %s.o,%s.a,$@) $@\n"
                    "	LDFLAGS += -DUSE_LD=1\n"
                    "endif\n\n"
                    "$(EXEC_NAME):$(RESOURCES_OBJ)\n"
                    "	@# use this line when you include libxputty as submodule\n"
                    "	@#$(CC) $(CFLAGS) \'$(NAME).c\' -L. $(RESOURCES_LIB) $(UI_LIB) -o \'$(EXEC_NAME)_ui.so\' $(LDFLAGS) -I./ -I$(HEADER_DIR)\n"
                    "	$(CXX) $(CXXFLAGS) $(EXEC_NAME).cpp $(LDFLAGS) -o $(EXEC_NAME).so\n"
                    "	$(CC) $(CFLAGS) \'$(NAME).c\' -L. $(RESOURCES_LIB) -o \'$(EXEC_NAME)_ui.so\' $(LDFLAGS) -I./\n"
                    "	$(STRIP) -s -x -X -R .comment -R .note.ABI-tag $(EXEC_NAME).so\n"
                    "	$(STRIP) -s -x -X -R .comment -R .note.ABI-tag $(EXEC_NAME)_ui.so\n\n"
                    "install :\n"
                    "ifneq (\"$(wildcard ./$(BUNDLE))\",\"\")\n"
                    "	@mkdir -p $(DESTDIR)$(INSTALL_DIR)/$(BUNDLE)\n"
                    "	cp -r ./$(BUNDLE)/* $(DESTDIR)$(INSTALL_DIR)/$(BUNDLE)\n"
                    "	@echo \". ., done\"\n"
                    "else\n"
                    "	@echo \". ., you must build first\"\n"
                    "endif\n\n"
                    "uninstall :\n"
                    "	@rm -rf $(INSTALL_DIR)/$(BUNDLE)\n"
                    "	@echo \". ., done\"\n\n"
                    "clean:\n"
                    "	rm -f *.a *.o *.so xresources.h\n\n",
                    name, "%%","%%","%%","%%","%%","%%","%%","%%","%%","%%","%%","%%","%%","%%","%%","%%","%%","%%","%%","%%","%%","%%");
                char* makefile = NULL;
                asprintf(&makefile, "%s/makefile",filepath);
                FILE *fpm;
                if((fpm=freopen(makefile, "w" ,stdout))==NULL) {
                    printf("open failed\n");
                }
                printf(cmd);
                fclose(fpm);
                free(makefile);
                free(cmd);
                cmd = NULL;
                //asprintf(&cmd, "cd \'%s\'  && make", filepath);
                //ret = system(cmd);
                //free(cmd);
                //cmd = NULL;
            } else {
                free(cmd);
            }
        }
        free(filepath);
        
        char* cmd = NULL;
        if (have_image || designer->image != NULL) {
            filepath = NULL;
            asprintf(&filepath, "%s%s_ui/resources",*(const char**)user_data,name);
            if (stat(filepath, &st) == -1) {
                mkdir(filepath, 0700);
            } else {
                asprintf(&cmd, "rm -rf \'%s\'", filepath);
                int ret = system(cmd);
                if (!ret) {
                    free(cmd);
                    cmd = NULL;
                    mkdir(filepath, 0700);
                } else {
                    free(cmd);
                    cmd = NULL;
                }
            }
        }
        if (designer->image != NULL) {
            //png2c(designer->image,filepath);
            char* xldl = strdup(basename(designer->image));
            strdecode(xldl, "-", "_");
            strdecode(xldl, " ", "_");
            strtovar(xldl);
            strdecode(xldl, "_png", ".png");
            char* fxldl = NULL;
            asprintf(&fxldl, "%s/%s", filepath, xldl);
            asprintf(&cmd, "cp \'%s\' \'%s\'", designer->image,fxldl);
            int ret = system(cmd);
            if (!ret) {
                char* xldc =  strdup(xldl);
                strdecode(xldc, ".png", ".c");
                free(cmd);
                cmd = NULL;
                asprintf(&cmd, "cd %s && xxd -i %s > %s", filepath, xldl, xldc);
                ret = system(cmd);
                free(xldc);
                free(cmd);
                cmd = NULL;
            } else {
                free(cmd);
                cmd = NULL;
                fprintf(stderr, "Fail to copy image\n");
            }
            free(xldl);
            free(fxldl);
        }
        if (have_image) {
            i = 0;
            for (;i<MAX_CONTROLS;i++) {
                if (designer->controls[i].image != NULL) {
                    //png2c(designer->controls[i].image,filepath);
                    char* xldl = strdup(basename(designer->controls[i].image));
                    strdecode(xldl, "-", "_");
                    strdecode(xldl, " ", "_");
                    strtovar(xldl);
                    strdecode(xldl, "_png", ".png");
                    char* fxldl = NULL;
                    asprintf(&fxldl, "%s/%s", filepath, xldl);
                    asprintf(&cmd, "cp \'%s\' \'%s\'", designer->controls[i].image,fxldl);
                    int ret = system(cmd);
                    if (!ret) {
                        char* xldc = strdup(xldl);
                        strdecode(xldc, ".png", ".c");
                        free(cmd);
                        cmd = NULL;
                        asprintf(&cmd, "cd %s && xxd -i %s > %s", filepath, xldl, xldc);
                        ret = system(cmd);
                        free(xldc);
                        free(cmd);
                        cmd = NULL;
                    } else {
                        free(cmd);
                        cmd = NULL;
                        fprintf(stderr, "Fail to copy image\n");
                    }
                    free(xldl);
                    free(fxldl);
                }
            }
            free(filepath);
            free(name);
        }
    }
}

void run_test(void *w_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    XUiDesigner *designer = (XUiDesigner*)w->parent_struct;
    if (w->flags & HAS_POINTER && !adj_get_value(w->adj_y)) {
        int i = 0;
        int j = 0;
        for (;i<MAX_CONTROLS;i++) {
            if (designer->controls[i].wid != NULL) {
                j++;
            }
        }
        if (!j) {
            Widget_t *dia = open_message_dialog(designer->ui, INFO_BOX, _("INFO"),
                                            _("Please create at least one Controller,|or load a LV2 URI to run a test build "),NULL);
            XSetTransientForHint(w->app->dpy, dia->widget, designer->ui->widget);
            return;
        }
        designer->run_test = true;
        char* name = "/tmp/test.c";
        remove (name);
        FILE *fp;
        if((fp=freopen(name, "w" ,stdout))==NULL) {
            fprintf(stderr,"open failed\n");
            return;
        }

        print_list(designer);
        fclose(fp);
        if (system(NULL)) {
            if ((int)adj_get_value(designer->color_chooser->adj)) {
                adj_set_value(designer->color_chooser->adj, 0.0);
            }
            widget_hide(designer->w);
            widget_hide(designer->ui);
            widget_hide(designer->set_project);
            XFlush(designer->w->app->dpy);
            int ret = system("cd /tmp/  && "
                "cc -O2 -D_FORTIFY_SOURCE=2 -Wall -fstack-protector "
                "`pkg-config lilv-0 --cflags` test.c "
                "-o uitest  -fPIC -Wl,-z,noexecstack -Wl,--no-undefined -I./ "
                "-I/usr/share/XUiDesigner/wrapper/libxputty "
                "-Wl,-Bstatic `pkg-config --cflags --libs xputty` "
                "-Wl,-Bdynamic `pkg-config --cflags --libs cairo x11 lilv-0` -lm ");
            if (!ret) {
                ret = system("cd /tmp/  && ./uitest");
            }
            if (!ret) {
                designer->run_test = false;
                widget_show_all(designer->w);
                widget_show_all(designer->ui);
                use_grid(designer->grid, NULL);
                widget_hide(designer->controller_settings);
                widget_hide(designer->combobox_settings);
            } else {
                designer->run_test = false;
                widget_show_all(designer->w);
                widget_show_all(designer->ui);
                use_grid(designer->grid, NULL);
                widget_hide(designer->controller_settings);
                widget_hide(designer->combobox_settings);
                Widget_t *dia = open_message_dialog(designer->ui, INFO_BOX, _("INFO"),
                                                _("Test fail, sorry"),NULL);
                XSetTransientForHint(w->app->dpy, dia->widget, designer->ui->widget);
            }
        }
    }
}
