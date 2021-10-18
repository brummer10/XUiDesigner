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
#include "XUiWriteTurtle.h"
#include "XUiWritePlugin.h"


/*---------------------------------------------------------------------
-----------------------------------------------------------------------    
                    handle widget list
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
    designer->controls[wid->data].is_atom_patch = false;
}

void add_to_list(XUiDesigner *designer, Widget_t *wid, const char* type,
                                    bool have_adjustment, WidgetType is_type) {
    designer->controls[wid->data].wid = wid;
    designer->controls[wid->data].type = type;
    designer->controls[wid->data].have_adjustment = have_adjustment;
    designer->controls[wid->data].is_type = is_type;
}

/*---------------------------------------------------------------------
-----------------------------------------------------------------------    
                    helper functions for generators
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
                print color theme
-----------------------------------------------------------------------
----------------------------------------------------------------------*/

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

/*---------------------------------------------------------------------
-----------------------------------------------------------------------    
                print C file for widgets
-----------------------------------------------------------------------
----------------------------------------------------------------------*/

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
        if (designer->controls[i].is_atom_patch && designer->controls[i].is_type == IS_FILE_BUTTON) {
            have_atom_in = true;
        } else if (designer->controls[i].is_atom_patch && designer->controls[i].is_type != IS_FILE_BUTTON) {
            have_atom_out = true;
        }
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
    if (have_atom_in || have_atom_out) {
        printf ( "\n#define OBJ_BUF_SIZE 1024");
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
        printf ("#endif\n");
    }
    if (j) {
        Window w = (Window)designer->ui->widget;
        char *name;
        XFetchName(designer->ui->app->dpy, w, &name);
        
        if (have_image && !designer->run_test) printf ("\n#include \"xresources.h\"\n\n");
        print_colors(designer);
        printf ("#include \"%s\"\n\n\n"
        , designer->run_test? "ui_test.cc": "lv2_plugin.cc");
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
            printf("#endif\n");
        }
        if (have_atom_out) {
            
        }
        printf ("void plugin_value_changed(X11_UI *ui, Widget_t *w, PortIndex index) {\n"
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
        , designer->ui->width, designer->ui->height, name? name:"Test");
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


/*---------------------------------------------------------------------
-----------------------------------------------------------------------    
            generate a LV2 bunlde containing all needed files
-----------------------------------------------------------------------
----------------------------------------------------------------------*/

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
        bool use_atom = false;
        for (;i<MAX_CONTROLS;i++) {
            if (designer->controls[i].wid != NULL) {
                j++;
            }
            if (designer->controls[i].image) {
                have_image = true;
            }
            if (designer->controls[i].is_atom_patch) {
                use_atom = true;
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

        char* cmd = NULL;
        asprintf(&cmd, "cd %s && git init", filepath);
        int ret = system(cmd);
        free(cmd);
        cmd = NULL;
        asprintf(&cmd, "cd %s && git submodule add https://github.com/brummer10/libxputty.git", filepath);
        ret = system(cmd);
        free(cmd);
        cmd = NULL;

        asprintf(&cmd, "SUBDIR := %s\n\n"

            ".PHONY: $(SUBDIR) libxputty  recurse\n\n"

            "$(MAKECMDGOALS) recurse: $(SUBDIR)\n\n"

            "clean:\n\n"

            "libxputty:\n"
            "	@exec $(MAKE) -j 1 -C $@ $(MAKECMDGOALS)\n\n"

            "$(SUBDIR): libxputty\n"
            "	@exec $(MAKE) -j 1 -C $@ $(MAKECMDGOALS)\n\n", name);

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
       

        free(filepath);
        filepath = NULL;
        asprintf(&filepath, "%s%s_ui/%s",*(const char**)user_data, name, name);

        if (stat(filepath, &st) == -1) {
            mkdir(filepath, 0700);
        }

        char* filename = NULL;
        asprintf(&filename, "%s%s_ui/%s/%s.c",*(const char**)user_data,name,name, name );
        remove (filename);

        FILE *fp;
        if((fp=freopen(filename, "w" ,stdout))==NULL) {
            printf("open failed\n");
        }
        fprintf(stderr, "save to %s\n", filename);
        print_list(designer);
        fclose(fp);
        fp = NULL;
        if (!designer->generate_ui_only) {
            strdecode(filename, ".c", ".cpp");
            if((fp=freopen(filename, "w" ,stdout))==NULL) {
                printf("open failed\n");
            }
            print_plugin(designer);
            fclose(fp);
            fp = NULL;
            strdecode(filename, ".cpp", ".ttl");
        } else {
            free(filename);
            filename = NULL;
            asprintf(&filename, "%s%s_ui/%s/%s_ui.ttl",*(const char**)user_data,name,name,name);
        }
        
        if((fp=freopen(filename, "w" ,stdout))==NULL) {
            printf("open failed\n");
        }
        print_ttl(designer);
        fclose(fp);
        fp = NULL;
        free(filename);
        filename = NULL;
        asprintf(&filename, "%s%s_ui/%s/manifest.ttl",*(const char**)user_data,name,name);
        if((fp=freopen(filename, "w" ,stdout))==NULL) {
            printf("open failed\n");
        }
        print_manifest(designer);
        fclose(fp);
        fp = NULL;
        free(filename);
        filename = NULL;
        if (system(NULL)) {
            cmd = NULL;
            asprintf(&cmd, "cp /usr/share/XUiDesigner/wrapper/libxputty/lv2_plugin.* \'%s\'", filepath);
            ret = system(cmd);
            if (!ret) {
                free(cmd);
                cmd = NULL;
                if (!designer->generate_ui_only) {
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
                        "	LIB_DIR := ../libxputty/libxputty/\n"
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
                        "	@$(CC) $(CFLAGS) \'$(NAME).c\' -L. $(RESOURCES_LIB) $(UI_LIB) -o \'$(EXEC_NAME)_ui.so\' $(LDFLAGS) -I./ -I$(HEADER_DIR)\n"
                        "	$(CXX) $(CXXFLAGS) $(EXEC_NAME).cpp $(LDFLAGS) -o $(EXEC_NAME).so\n"
                        "	$(CC) %s $(CFLAGS) \'$(NAME).c\' -L. $(RESOURCES_LIB) -o \'$(EXEC_NAME)_ui.so\' $(LDFLAGS) -I./\n"
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
                        name, "%%","%%","%%","%%","%%","%%","%%","%%","%%","%%","%%","%%","%%","%%","%%","%%","%%","%%","%%","%%","%%","%%", use_atom ? "-DUSE_ATOM" : "");
                } else {
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
                        "	BUNDLE = $(EXEC_NAME)_ui.lv2\n"
                        "	RESOURCES_DIR :=./resources/\n"
                        "	LIB_DIR := ../libxputty/libxputty/\n"
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
                        "	@if [ -f ./$(BUNDLE)/$(EXEC_NAME)_ui.so ]; then echo \"build finish, now run make install\"; \\\n"
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
                        "	@$(CC) $(CFLAGS) \'$(NAME).c\' -L. $(RESOURCES_LIB) $(UI_LIB) -o \'$(EXEC_NAME)_ui.so\' $(LDFLAGS) -I./ -I$(HEADER_DIR)\n"
                        "	$(CC) %s $(CFLAGS) \'$(NAME).c\' -L. $(RESOURCES_LIB) -o \'$(EXEC_NAME)_ui.so\' $(LDFLAGS) -I./\n"
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
                        name, "%%","%%","%%","%%","%%","%%","%%","%%","%%","%%","%%","%%","%%","%%","%%","%%","%%","%%","%%","%%","%%","%%", use_atom ? "-DUSE_ATOM" : "");                    
                }
                makefile = NULL;
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
                cmd = NULL;
            }
        }
        free(filepath);
        
        cmd = NULL;
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
            filepath = NULL;
            asprintf(&filepath, "%s%s_ui",*(const char**)user_data,name);
            char* cmd = NULL;
            asprintf(&cmd, "cd %s && git add .", filepath);
            ret = system(cmd);
            free(cmd);
            cmd = NULL;
            free(filepath);
            free(name);
        }
    }
}


/*---------------------------------------------------------------------
-----------------------------------------------------------------------    
            do a test build and run the GUI
-----------------------------------------------------------------------
----------------------------------------------------------------------*/

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
