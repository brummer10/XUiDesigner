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

#include "XUiWriteTurtle.h"
#include "XUiGenerator.h"


/*---------------------------------------------------------------------
-----------------------------------------------------------------------    
                generate manifest files for plugin
-----------------------------------------------------------------------
----------------------------------------------------------------------*/

void print_manifest(XUiDesigner *designer) {
    char *name = NULL;
    XFetchName(designer->ui->app->dpy, designer->ui->widget, &name);
    if (name == NULL) asprintf(&name, "%s", "noname");
    strdecode(name, " ", "_");

    if (!designer->generate_ui_only) {
        printf ("\n@prefix lv2:  <http://lv2plug.in/ns/lv2core#> .\n"
            "@prefix rdfs: <http://www.w3.org/2000/01/rdf-schema#> .\n"

            "<%s>\n"
            "    a lv2:Plugin ;\n"
            "    lv2:binary <%s.so> ;\n"
            "    rdfs:seeAlso <%s.ttl> .\n", designer->lv2c.uri, name, name);
    } else {
        printf ("\n@prefix guiext: <http://lv2plug.in/ns/extensions/ui#>.\n"
            "\n@prefix lv2:  <http://lv2plug.in/ns/lv2core#> .\n"
            "@prefix rdfs: <http://www.w3.org/2000/01/rdf-schema#> .\n"

            "<%s> guiext:ui <%s> .\n"
            "    <%s> a guiext:X11UI ;\n"
            "    lv2:binary <%s_ui.so> ;\n"
            "    rdfs:seeAlso <%s_ui.ttl> .\n", designer->lv2c.uri, designer->lv2c.ui_uri, designer->lv2c.ui_uri, name, name);
    }
    free(name);
}


/*---------------------------------------------------------------------
-----------------------------------------------------------------------    
                generate ttl files for plugin
-----------------------------------------------------------------------
----------------------------------------------------------------------*/

void print_ttl(XUiDesigner *designer) {
    char *name = NULL;
    XFetchName(designer->ui->app->dpy, designer->ui->widget, &name);
    if (name == NULL) asprintf(&name, "%s", "noname");
    printf ("\n@prefix doap:  <http://usefulinc.com/ns/doap#> .\n"
        "@prefix foaf:   <http://xmlns.com/foaf/0.1/> .\n"
        "@prefix lv2:    <http://lv2plug.in/ns/lv2core#> .\n"
        "@prefix rdf:    <http://www.w3.org/1999/02/22-rdf-syntax-ns#> .\n"
        "@prefix rdfs:   <http://www.w3.org/2000/01/rdf-schema#> .\n"
        "@prefix guiext: <http://lv2plug.in/ns/extensions/ui#>.\n"
        "@prefix opts:   <http://lv2plug.in/ns/ext/options#> .\n"
        "@prefix time:   <http://lv2plug.in/ns/ext/time#>.\n"
        "@prefix units:  <http://lv2plug.in/ns/extensions/units#> .\n"
        "@prefix atom:   <http://lv2plug.in/ns/ext/atom#> .\n"
        "@prefix urid:   <http://lv2plug.in/ns/ext/urid#> .\n"
        "@prefix pprop:  <http://lv2plug.in/ns/ext/port-props#> .\n"
        "@prefix midi:   <http://lv2plug.in/ns/ext/midi#> .\n"
        "@prefix patch:  <http://lv2plug.in/ns/ext/patch#> .\n\n\n");

    if (!designer->generate_ui_only) {
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
        for (;i<MAX_CONTROLS;i++) {
            if (designer->controls[i].is_atom_patch) {
                Widget_t * wid = designer->controls[i].wid;
                const char* uri = (const char*) wid->parent_struct;
                printf("\n<%s>\n"
                       "    a lv2:Parameter ;\n"
                       "        rdfs:label \"%s\" ;\n", uri, wid->label);
                if (designer->controls[i].port_index == -1) printf("        rdfs:range atom:Float");
                else if (designer->controls[i].port_index == -2) printf("        rdfs:range atom:Int");
                else if (designer->controls[i].port_index == -3) printf("        rdfs:range atom:Bool");
                else if (designer->controls[i].port_index == -4) printf("        rdfs:range atom:Path");
                if (designer->controls[i].have_adjustment) {
                    printf(" ;\n"
                           "        lv2:default %f ;\n"
                           "        lv2:minimum %f ;\n"
                           "        lv2:maximum %f .\n", adj_get_std_value(wid->adj),
                                    adj_get_min_value(wid->adj), adj_get_max_value(wid->adj));
                } else {
                    printf(" .\n");
                }
            }
        }
        
        i = 0;
        int p = 0;
        bool add_comma = false;
        printf ("\n   lv2:port ");
        if (designer->is_project) {
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
        }
        if (designer->is_faust_file) {
            i = 0;
            bool have_bypass = false;
            for (;i<MAX_CONTROLS;i++) {
                if (designer->controls[i].wid != NULL) {
                    if (designer->controls[i].destignation_enabled) {
                        have_bypass = true;
                        break;
                    }
                }
            }
            if (!have_bypass) {
                printf (", [\n"
                    "      a lv2:InputPort ,\n"
                    "          lv2:ControlPort ;\n"
                    "      lv2:index %i ;\n"
                    "      lv2:designation lv2:enabled ;\n"
                    "      lv2:portProperty lv2:toggled, pprop:trigger ;\n"
                    "      lv2:symbol \"Bypass\" ;\n"
                    "      lv2:name \"bypass\" ;\n"
                    "      lv2:default 1 ;\n"
                    "      lv2:minimum 0 ;\n"
                    "      lv2:maximum 1 ;\n"
                    "   ]", p);
            }
        }
        i = 0;
        for (;i<MAX_CONTROLS;i++) {
            if (designer->controls[i].wid != NULL) {
                Widget_t * wid = designer->controls[i].wid;
                if (designer->controls[i].is_type == IS_FRAME ||
                    designer->controls[i].is_type == IS_IMAGE ||
                    designer->controls[i].is_type == IS_TABBOX) {
                    continue;
                } else {
                    if (designer->controls[i].is_atom_patch) {
                        continue;
                    }
                    strtosym(designer->controls[i].symbol);
                    char *xldl = NULL;
                    asprintf(&xldl, "%s", designer->controls[i].wid->label);
                    strtovar(xldl);
                    if (designer->controls[i].have_adjustment) {
                        if (designer->controls[i].is_type == IS_COMBOBOX) {
                            Widget_t *menu = wid->childlist->childs[1];
                            Widget_t* view_port =  menu->childlist->childs[0];
                            ComboBox_t *comboboxlist = (ComboBox_t*)view_port->parent_struct;
                            printf ("%s [\n"
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
                                    , add_comma ? ",": "", designer->is_project ? p : designer->controls[i].port_index,
                                    designer->controls[i].symbol, xldl, adj_get_std_value(wid->adj),
                                    adj_get_min_value(wid->adj), adj_get_max_value(wid->adj));
                                add_comma = true;
                            unsigned int k = 0;
                            int l = (int)adj_get_min_value(wid->adj);
                            for(; k<comboboxlist->list_size;k++) {
                                printf ("      lv2:scalePoint [rdfs:label \"%s\"; rdf:value %i];\n", comboboxlist->list_names[k],l);
                                l++;
                            }
                            printf ("]");
                        } else if (designer->controls[i].is_type == IS_VMETER ||
                                designer->controls[i].is_type == IS_HMETER) {
                            printf ("%s [\n"
                                "      a lv2:OutputPort ,\n"
                                "          lv2:ControlPort ;\n"
                                "      lv2:index %i ;\n"
                                "      lv2:symbol \"%s\" ;\n"
                                "      lv2:name \"%s\" ;\n"
                                "      lv2:default %f ;\n"
                                "      lv2:minimum %f ;\n"
                                "      lv2:maximum %f ;\n"
                                "   ]", add_comma ? ",": "", designer->is_project ? p : designer->controls[i].port_index,
                                    designer->controls[i].symbol, xldl, adj_get_std_value(wid->adj),
                                    adj_get_min_value(wid->adj), adj_get_max_value(wid->adj));
                                add_comma = true;
                        } else {
                            printf ("%s [\n"
                                "      a lv2:InputPort ,\n"
                                "          lv2:ControlPort ;\n"
                                "      lv2:index %i ;\n"
                                "      lv2:symbol \"%s\" ;\n"
                                "      lv2:name \"%s\" ;\n"
                                "      lv2:default %f ;\n"
                                "      lv2:minimum %f ;\n"
                                "      lv2:maximum %f ;\n"
                                "   ]", add_comma ? ",": "", designer->is_project ? p : designer->controls[i].port_index,
                                    designer->controls[i].symbol, xldl, adj_get_std_value(wid->adj),
                                    adj_get_min_value(wid->adj), adj_get_max_value(wid->adj));
                                add_comma = true;
                        }
                    } else if (designer->controls[i].is_audio_input) {
                        printf ("%s [\n"
                            "       a lv2:AudioPort ,\n"
                            "          lv2:InputPort ;\n"
                            "      lv2:index %i ;\n"
                            "      lv2:symbol \"%s\" ;\n"
                            "      lv2:name \"%s\" ;\n"
                            "   ]", add_comma ? ",": "", designer->is_project ? p : designer->controls[i].port_index,
                                designer->controls[i].symbol, xldl);
                            add_comma = true;
                    } else if (designer->controls[i].is_audio_output) {
                        printf ("%s [\n"
                            "       a lv2:AudioPort ,\n"
                            "          lv2:OutputPort ;\n"
                            "      lv2:index %i ;\n"
                            "      lv2:symbol \"%s\" ;\n"
                            "      lv2:name \"%s\" ;\n"
                            "   ]", add_comma ? ",": "", designer->is_project ? p : designer->controls[i].port_index,
                                designer->controls[i].symbol, xldl);                        
                            add_comma = true;
                    } else if (designer->controls[i].is_atom_output) {
                        printf ("%s [\n"
                            "      a lv2:OutputPort ,\n"
                            "          atom:AtomPort ;\n"
                            "      atom:bufferType atom:Sequence ;\n"
                            "      atom:supports midi:MidiEvent ,\n"
                            "           patch:Message ;\n"
                            "      lv2:designation lv2:control ;\n"
                            "      lv2:index %i ;\n"
                            "      lv2:symbol \"%s\" ;\n"
                            "      lv2:name \"%s\" ;\n"
                            "   ]", add_comma ? ",": "", designer->is_project ? p : designer->controls[i].port_index,
                                designer->controls[i].symbol, xldl);
                            add_comma = true;
                    } else if (designer->controls[i].is_atom_input) {
                        printf ("%s [\n"
                            "      a lv2:InputPort ,\n"
                            "          atom:AtomPort ;\n"
                            "      atom:bufferType atom:Sequence ;\n"
                            "      atom:supports midi:MidiEvent ,\n"
                            "           patch:Message ;\n"
                            "      lv2:designation lv2:control ;\n"
                            "      lv2:index %i ;\n"
                            "      lv2:symbol \"%s\" ;\n"
                            "      lv2:name \"%s\" ;\n"
                            "   ]", add_comma ? ",": "", designer->is_project ? p : designer->controls[i].port_index,
                                designer->controls[i].symbol, xldl);
                            add_comma = true;
                    } else if (designer->controls[i].is_type == IS_TOGGLE_BUTTON ||
                            designer->controls[i].is_type == IS_IMAGE_TOGGLE) {
                        printf ("%s [\n"
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
                            "   ]", add_comma ? ",": "", designer->is_project ? p : designer->controls[i].port_index,
                                designer->controls[i].destignation_enabled ? "      lv2:designation lv2:enabled;\n" : "",
                                designer->controls[i].symbol, xldl,
                                designer->controls[i].destignation_enabled ? 1 : 0);
                            add_comma = true;
                    } else if (designer->controls[i].is_type == IS_BUTTON ||
                            designer->controls[i].is_type == IS_IMAGE_BUTTON) {
                        printf ("%s [\n"
                            "      a lv2:InputPort ,\n"
                            "          lv2:ControlPort ;\n"
                            "      lv2:index %i ;\n"
                            "      lv2:portProperty lv2:toggled, pprop:trigger ;\n"
                            "      lv2:symbol \"%s\" ;\n"
                            "      lv2:name \"%s\" ;\n"
                            "      lv2:default 0 ;\n"
                            "      lv2:minimum 0 ;\n"
                            "      lv2:maximum 1 ;\n"
                            "   ]", add_comma ? ",": "", designer->is_project ? p : designer->controls[i].port_index,
                                 designer->controls[i].symbol, xldl);
                            add_comma = true;
                    }
                    free(xldl);
                }
            }
            p++;
        }
        i = 0;
        for (;i<MAX_CONTROLS;i++) {
            if (designer->controls[i].is_atom_patch) {
                Widget_t * wid = designer->controls[i].wid;
                printf (" ;\npatch:writable <%s>", (const char*) wid->parent_struct);
            }
        }
        printf (" .\n\n");
        strdecode(name, " ", "_");
        printf ("\n<%s>\n"
            "   a guiext:X11UI;\n"
            "   guiext:binary <%s_ui.so> ;\n"
            "       lv2:extensionData guiext::idle ;\n"
            "       lv2:extensionData guiext:resize ;\n"
            "       lv2:extensionData guiext:idleInterface ;\n"
            "       lv2:requiredFeature guiext:idleInterface ;\n"
            "       lv2:optionalFeature opts:options ;\n"
            "       opts:supportedOption guiext:scaleFactor ;"
            , designer->lv2c.ui_uri, name);
        i = 0;
        for (;i<MAX_CONTROLS;i++) {
            if (designer->controls[i].wid != NULL) {
                if (designer->controls[i].is_atom_output) {
                    printf ("\n       guiext:portNotification [\n"
                        "           guiext:plugin  <%s> ;\n"
                        "           lv2:symbol \"%s\" ;\n"
                        "           guiext:notifyType atom:Blank\n"
                        "       ] " , designer->lv2c.uri, designer->controls[i].symbol);
                }
            }
        }
        printf (" .\n");
    } else {
        int i = 0;
        printf ("<%s>\n"
            "   lv2:extensionData guiext::idle ;\n"
            "   lv2:extensionData guiext:resize ;\n"
            "   lv2:extensionData guiext:idleInterface ;\n"
            "   lv2:requiredFeature guiext:idleInterface ;\n"
            "   lv2:optionalFeature opts:options ;\n"
            "   opts:supportedOption guiext:scaleFactor ;"
            , designer->lv2c.ui_uri);
        for (;i<MAX_CONTROLS;i++) {
            if (designer->controls[i].wid != NULL) {
                if (designer->controls[i].is_atom_output) {
                    printf ("\n       guiext:portNotification [\n"
                        "           guiext:plugin  <%s> ;\n"
                        "           lv2:symbol \"%s\" ;\n"
                        "           guiext:notifyType atom:Blank\n"
                        "       ] " , designer->lv2c.uri, designer->controls[i].symbol);
                }
            }
        }
        printf (" .\n");
        
    }
    free(name);
}
