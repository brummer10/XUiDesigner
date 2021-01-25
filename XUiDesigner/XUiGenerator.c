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

#include "XUiGenerator.h"


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

void print_list(XUiDesigner *designer) {
    int i = 0;
    int j = 0;
    for (;i<MAX_CONTROLS;i++) {
        if (designer->controls[i].wid != NULL) {
            j++;
        }
    }
    if (j) {
        Window w = (Window)designer->ui->widget;
        char *name;
        XFetchName(designer->ui->app->dpy, w, &name);

        printf ("\n#define CONTROLS %i\n\n", j);
        printf ("\n#define PLUGIN_UI_URI \"%s\"\n\n",designer->lv2c.ui_uri);
        printf ("\n#include \"lv2_plugin.h\"\n\n");
        print_colors(designer);
        printf ("#include \"%s\"\n\n\n"
        "void plugin_value_changed(X11_UI *ui, Widget_t *w, PortIndex index) {\n"
        "    // do special stuff when needed\n"
        "}\n\n"
        "void plugin_set_window_size(int *w,int *h,const char * plugin_uri) {\n"
        "    (*w) = %i; //set initial widht of main window\n"
        "    (*h) = %i; //set initial heigth of main window\n"
        "}\n\n"
        "const char* plugin_set_name() {\n"
        "    return \"%s\"; //set plugin name to display on UI\n"
        "}\n\n"
        "void plugin_create_controller_widgets(X11_UI *ui, const char * plugin_uri) {\n"
        "    set_costum_theme(&ui->main);\n"
        , designer->run_test? "ui_test.cc": "lv2_plugin.cc", designer->ui->width, designer->ui->height, name? name:"Test");
        if (designer->image && designer->run_test) printf ("    load_bg_image(ui,\"%s\");\n", designer->image);

    } else {
        return;
    }
    i = 0;
    j = 0;
    for (;i<MAX_CONTROLS;i++) {
        if (designer->controls[i].wid != NULL) {
            Widget_t * wid = designer->controls[i].wid;
            printf ("    ui->widget[%i] = %s (ui->widget[%i], %i, \"%s\", ui, %i,  %i, %i, %i);\n", 
                j, designer->controls[i].type, j,
                designer->controls[i].port_index, designer->controls[i].wid->label,
                designer->controls[i].wid->x, designer->controls[i].wid->y,
                designer->controls[i].wid-> width, designer->controls[i].wid->height);
            if (designer->controls[i].image != NULL && designer->run_test) {
                printf ("    load_controller_image(ui->widget[%i], \"%s\");\n", j, designer->controls[i].image);
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
                    printf ("    set_adjustment(ui->widget[%i]->adj, %.3f, %.3f, %.3f, %.3f, %.3f, %i);\n\n", 
                        j, powf(10,wid->adj->std_value), powf(10,wid->adj->std_value), powf(10,wid->adj->min_value),
                        powf(10,wid->adj->max_value), wid->adj->step, wid->adj->type);
                    
                } else {
                    printf ("    set_adjustment(ui->widget[%i]->adj, %.3f, %.3f, %.3f, %.3f, %.3f, %i);\n\n", 
                        j, wid->adj->std_value, wid->adj->std_value, wid->adj->min_value, wid->adj->max_value,
                        wid->adj->step, wid->adj->type);
                }
            }
            j++;
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
        char* name = "./Bundle/test.lv2/test/test.c";
        remove (name);
        FILE *fp;
        if((fp=freopen(name, "w" ,stdout))==NULL) {
            printf("open failed\n");
        }

        print_list(designer);
        fclose(fp);
        if (system(NULL)) {
            if ((int)adj_get_value(designer->color_chooser->adj)) {
                adj_set_value(designer->color_chooser->adj, 0.0);
            }
            widget_hide(designer->w);
            widget_hide(designer->ui);
            XFlush(designer->w->app->dpy);
            int ret = system("cd ./Bundle/test.lv2/test  && "
                "cc -O2 -D_FORTIFY_SOURCE=2 -Wall -fstack-protector "
                "`pkg-config lilv-0 --cflags` test.c -L. ../../../libxputty/libxputty/libxputty.a "
                "-o uitest  -fPIC -Wl,-z,noexecstack -Wl,--no-undefined -I./ -I../../../libxputty/libxputty/include/ "
                "`pkg-config --cflags --libs cairo x11 lilv-0` -lm && ./uitest");
            if (!ret) {
                designer->run_test = false;
                widget_show_all(designer->w);
                widget_show_all(designer->ui);
            } else {
                designer->run_test = false;
                widget_show_all(designer->w);
                widget_show_all(designer->ui);
                Widget_t *dia = open_message_dialog(designer->ui, INFO_BOX, _("INFO"),
                                                _("Test fail, sorry"),NULL);
                XSetTransientForHint(w->app->dpy, dia->widget, designer->ui->widget);
            }
        }
    }
    
}
