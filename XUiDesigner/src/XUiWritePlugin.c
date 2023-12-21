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

#include "XUiWritePlugin.h"
#include "XUiGenerator.h"

static void append(char **str, const char *buf) {
    char *nstr;
    if (*str == NULL) {
        asprintf(&nstr, ", %s", buf);
    } else {
        asprintf(&nstr, "%s, %s", *str, buf);
        free(*str);
    }
    *str = nstr;
}

/*---------------------------------------------------------------------
-----------------------------------------------------------------------    
                generate C++ plugin sceleton for plugin
-----------------------------------------------------------------------
----------------------------------------------------------------------*/

void print_plugin(XUiDesigner *designer) {
    char * a_inputs[16];
    int a = 0;
    char * a_outputs[16];
    int o = 0;
    bool parse_file = designer->is_faust_file ? true : designer->is_cc_file ? true : false;
    bool is_project = designer->is_project || designer->is_json_file;
    char *name = NULL;
    XFetchName(designer->ui->app->dpy, designer->ui->widget, &name);
    if (name == NULL) asprintf(&name, "%s", "noname");
    strovar(name);

    printf ("\n#include <cstdlib>\n"
    "#include <cmath>\n"
    "#include <iostream>\n"
    "#include <cstring>\n"
    "#include <unistd.h>\n\n"
    "#include <lv2/core/lv2.h>\n");
    if (designer->lv2c.midi_input || designer->lv2c.midi_output) {
        printf ("#include <lv2/atom/atom.h>\n"
        "#include <lv2/atom/util.h>\n"
        "#include <lv2/midi/midi.h>\n"
        "#include <lv2/urid/urid.h>\n\n");
    }
    if (designer->lv2c.midi_output) {
        printf ("#include <lv2/atom/forge.h>\n");
    }

    printf ("///////////////////////// MACRO SUPPORT ////////////////////////////////\n\n"
    "#define PLUGIN_URI \"%s\"\n\n"
    "using std::min;\n"
    "using std::max;\n\n",  designer->lv2c.uri);
    if (designer->is_faust_file) {
        char* tmp = strdup(designer->faust_file);
        printf ("#define __rt_data __attribute__((section(\".rt.data\")))\n");
        printf ("#include \"%s\"\n\n", basename(tmp));
        free(tmp);
        tmp = NULL;
    } else if (designer->is_cc_file) {
        char* tmp = strdup(designer->cc_file);
        printf ("#include \"%s\"\n\n", basename(tmp));
        free(tmp);
        tmp = NULL;
    } else {
        printf ("typedef int PortIndex;\n\n");
    }
    printf ("////////////////////////////// PLUG-IN CLASS ///////////////////////////\n\n"
    "namespace %s {\n\n"
    "class X%s\n"
    "{\n"
    "private:\n", name, name);

    if (designer->lv2c.midi_input) {
        printf ("    LV2_URID midi_MidiEvent;\n"
        "    LV2_URID_Map* map;\n");
    }

    int i = 0;
    if (is_project) {
        for (;i<designer->lv2c.audio_input;i++) {
            printf ("    float* input%i;\n", i);
            asprintf((char**)&a_inputs[a],"input%i", i);
            a++;
        }
        i = 0;
        for (;i<designer->lv2c.audio_output;i++) {
            printf ("    float* output%i;\n", i);
            asprintf((char**)&a_outputs[o],"output%i", i);
            o++;
        }
        if (designer->lv2c.midi_input) {
            printf ("    const LV2_Atom_Sequence* midi_in;\n");
        }
        if (designer->lv2c.midi_output) {
            printf ("    LV2_Atom_Sequence* midi_out;\n");
            printf ("    LV2_Atom midiatom;\n");
            printf ("    LV2_Atom_Forge forge;\n");
            printf ("    LV2_Atom_Forge_Frame frame;\n");
            printf ("    uint8_t data[3];\n");

        }
        i = 0;
    }
    bool have_bypass = false;
    for (;i<MAX_CONTROLS;i++) {
        if (designer->controls[i].wid != NULL) {
            if (designer->controls[i].is_type == IS_FRAME ||
                designer->controls[i].is_type == IS_IMAGE ||
                designer->controls[i].is_type == IS_TABBOX) {
                continue;
            }
            if (!designer->controls[i].destignation_enabled) {
                char* var = strdup(designer->controls[i].wid->label);
                strtovar(var);
                if (designer->controls[i].is_audio_input) {
                    printf ("    float* %s;\n", var);
                    a_inputs[a] = strdup(var);
                    a++;
                } else if (designer->controls[i].is_audio_output) {
                    printf ("    float* %s;\n", var);
                    a_outputs[o] = strdup(var);
                    o++;
                } else if (!parse_file) {
                    printf ("    float* %s;\n"
                    "    float %s_;\n", var, var);
                }
                free(var);
                var = NULL;
            } else {
                designer->lv2c.bypass = 1;
                have_bypass = true;
                printf ("    float* bypass;\n"
                "    float bypass_;\n");
            }
        }
    }
    if (parse_file && !have_bypass) {
        printf ("    float* bypass;\n"
        "    float bypass_;\n");
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
    if (parse_file) {
        printf ("    %s::Dsp* plugin;\n\n", name);
    }

    printf ("    // private functions\n");
    if (designer->lv2c.midi_output) {
         printf ("    void send_midi_data(int count, uint8_t controller,\n"
         "                         uint8_t note, uint8_t velocity);\n");
    }
    printf ("    inline void run_dsp_(uint32_t n_samples);\n"
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
    if (is_project) {
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
    }
    i = 0;
    for (;i<MAX_CONTROLS;i++) {
        if (designer->controls[i].wid != NULL) {
            if (designer->controls[i].is_type == IS_FRAME ||
                designer->controls[i].is_type == IS_IMAGE ||
                designer->controls[i].is_type == IS_TABBOX) {
                continue;
            }
            if (!designer->controls[i].destignation_enabled && !parse_file) {
                char* var = strdup(designer->controls[i].wid->label);
                strtovar(var);
                printf ("%s\n    %s(NULL)",add_comma ? "," : "", var);
                free(var);
                var = NULL;
                add_comma = true;
            } else if (designer->controls[i].destignation_enabled) {
                printf ("%s\n    bypass(NULL)",add_comma ? "," : "");
                add_comma = true;
                printf ("%s\n    bypass_(2)",add_comma ? "," : "");
            }
        }
    }
    if (parse_file && !have_bypass) {
        printf ("%s\n    bypass(NULL)",add_comma ? "," : "");
        add_comma = true;
        printf ("%s\n    bypass_(2)",add_comma ? "," : "");
    }
    if (designer->lv2c.bypass) {
        printf ("%s\n    needs_ramp_down(false),\n"
        "    needs_ramp_up(false),\n"
        "    bypassed(false)", add_comma ? "," : "");
    }
    if (parse_file) {
        printf (",\n"
        "    plugin(%s::plugin())", name);
    }
    printf (" {};\n\n");

    printf ("// destructor\n"
    "X%s::~X%s() {", name, name);
    if (parse_file) {
        printf ("\n    plugin->del_instance(plugin);\n");
    }
    printf ("};\n\n");


    printf ("///////////////////////// PRIVATE CLASS  FUNCTIONS /////////////////////\n\n"
    "void X%s::init_dsp_(uint32_t rate)\n"
    "{\n", name);
    if (parse_file) {
        printf ("    plugin->init(rate);\n");
    }
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
    if (is_project) {
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
    }
    i = 0;
    for (;i<MAX_CONTROLS;i++) {
        if (designer->controls[i].wid != NULL) {
            if (designer->controls[i].is_type == IS_FRAME ||
                designer->controls[i].is_type == IS_IMAGE ||
                designer->controls[i].is_type == IS_TABBOX ||
                designer->controls[i].is_atom_input ||
                designer->controls[i].is_atom_output ) {
                continue;
            }
            if (!designer->controls[i].destignation_enabled && !parse_file) {
                char* var = strdup(designer->controls[i].wid->label);
                strtovar(var);
                printf ("        case %i:\n"
                        "            %s = static_cast<float*>(data);\n"
                        "            break;\n", p, var);
                p++;
                free(var);
                var = NULL;
            } else if (designer->controls[i].destignation_enabled) {
                printf ("        case %i:\n"
                        "            bypass = static_cast<float*>(data);\n"
                        "            break;\n", p);
                p++;
            }
        }
    }
    if (parse_file && !have_bypass) {
        printf ("        case %i:\n"
                "            bypass = static_cast<float*>(data);\n"
                "            break;\n", p);
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

    if (designer->lv2c.midi_output) {
        printf ("// send midi data to the midi output port\n"
    "void X%s::send_midi_data(int count, uint8_t controller,\n"
    "                             uint8_t note, uint8_t velocity)\n"
    "{\n"
    "    if(!midi_out) return;\n"
    "    data[0] = controller;\n"
    "    data[1] = note;\n"
    "    data[2] = velocity;\n"
    "    lv2_atom_forge_frame_time(&forge,count);\n"
    "    lv2_atom_forge_raw(&forge,&midiatom,sizeof(LV2_Atom));\n"
    "    lv2_atom_forge_raw(&forge,data, sizeof(data));\n"
    "    lv2_atom_forge_pad(&forge,sizeof(data)+sizeof(LV2_Atom));\n"
    "}\n", name);

    }

    printf ("void X%s::run_dsp_(uint32_t n_samples)\n"
    "{\n"
    "    if(n_samples<1) return;\n\n", name);

    if (designer->lv2c.midi_output) {
        printf ("    lv2_atom_forge_set_buffer(&forge,(uint8_t*)midi_out, midi_out->atom.size);\n"
        "    lv2_atom_forge_sequence_head(&forge, &frame, 0);\n");
    }

    i = 0;
    if (!designer->controls[i].destignation_enabled && !parse_file) {
        printf ("    // get controller values\n");
        for (;i<MAX_CONTROLS;i++) {
            if (designer->controls[i].wid != NULL) {
                if (designer->controls[i].is_type == IS_FRAME ||
                    designer->controls[i].is_type == IS_IMAGE ||
                    designer->controls[i].is_type == IS_TABBOX ||
                    designer->controls[i].is_audio_input ||
                    designer->controls[i].is_audio_output ||
                    designer->controls[i].is_atom_input ||
                    designer->controls[i].is_atom_output ) {
                    continue;
                }
                char* var = strdup(designer->controls[i].wid->label);
                strtovar(var);
                printf ("#define  %s_ (*(%s))\n", var, var);
                free(var);
                var = NULL;
            }
        }
    }

    if (designer->lv2c.midi_input) {
        printf ("    LV2_ATOM_SEQUENCE_FOREACH(midi_in, ev) {\n"
        "        if (ev->body.type == midi_MidiEvent) {\n"
        "            const uint8_t* const msg = (const uint8_t*)(ev + 1);\n");
        if (designer->lv2c.midi_output) {
            printf ("            //forward all incoming MIDI data to output\n"
            "            send_midi_data(0, msg[0], msg[1], msg[2]);\n");
        }
        printf ("            switch (lv2_midi_message_type(msg)) {\n"
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

    char* oports = NULL;
    i = 0;
    if (designer->lv2c.audio_input == designer->lv2c.audio_output) {
        for (;i<designer->lv2c.audio_input;i++) {
            if (i < designer->lv2c.audio_output) {
                printf ("\n    // do inplace processing on default\n"
                "    if(%s != %s)\n"
                "        memcpy(%s, %s, n_samples*sizeof(float));\n\n",
                        a_outputs[i],a_inputs[i],a_outputs[i],a_inputs[i]);
                asprintf(&oports, ", %s, %s",a_outputs[i], a_outputs[i]);
            } else {
                printf ("    // audio input and output count is not equal\n"
                "    // you must handle them yourself\n\n");
            }
        }
    } else {
        for (;i<designer->lv2c.audio_input;i++) {
            append(&oports, a_inputs[i]);
        }
        i = 0;
        for (;i<designer->lv2c.audio_output;i++) {
            append(&oports, a_outputs[i]);
        }
    }

    if (designer->lv2c.bypass) {
        if (designer->lv2c.audio_input != designer->lv2c.audio_output) {
            printf ("     // audio input and output count is not equal\n"
            "    // you must handle ramping yourself\n\n");
        } else {
            i = 0;
            for (;i<designer->lv2c.audio_input;i++) {
                printf ("    float buf%i[n_samples];\n", i);
            }
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
        "    }\n\n");
        if (designer->lv2c.audio_input == designer->lv2c.audio_output) {
            printf ("    if (needs_ramp_down || needs_ramp_up) {\n");
            i = 0;
            for (;i<designer->lv2c.audio_input;i++) {
                printf ("         memcpy(buf%i, %s, n_samples*sizeof(float));\n", i, a_inputs[i]);
            }
            printf ("    }\n");
        }
        printf ("    if (!bypassed) {\n    ");
    }
    if (!parse_file) {
        printf ("    for (uint32_t i = 0; i<n_samples; i++) {\n");
        i = 0;
        for (;i<designer->lv2c.audio_output;i++) {
            printf("            float tmp%i = %s[i];\n"
            "            //do your dsp\n"
            "            %s[i] = tmp%i;\n", i, a_outputs[i], a_outputs[i], i);
        }
        printf ("        }\n\n");
    } else {
        //if (designer->lv2c.audio_input == designer->lv2c.audio_output) {
            
            printf ("    plugin->compute(n_samples%s);\n", oports);
        //}
        free(oports);
        oports = NULL;
    }
    
    if (designer->lv2c.bypass) {
        printf ("    }\n\n");
        if (designer->lv2c.audio_input != designer->lv2c.audio_output) {
            printf ("     // audio input and output count is not equal\n"
            "    // you must handle ramping yourself\n\n");
        } else {
        
            printf (
            "    // check if ramping is needed\n"
            "    if (needs_ramp_down) {\n"
            "        float fade = 0;\n"
            "        for (uint32_t i=0; i<n_samples; i++) {\n"
            "            if (ramp_down >= 0.0) {\n"
            "                --ramp_down; \n"
            "            }\n"
            "            fade = max(0.0f,ramp_down) /ramp_down_step ;\n");
            i = 0;
            for (;i<designer->lv2c.audio_input;i++) {            
                printf ("            %s[i] = %s[i] * fade + buf%i[i] * (1.0 - fade);\n",
                        a_outputs[i], a_outputs[i], i);
            }
            printf ("        }\n"

            "        if (ramp_down <= 0.0) {\n"
            "            // when ramped down, clear buffer from dsp\n"
            "            %s"
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
            "            fade = min(ramp_up_step,ramp_up) /ramp_up_step ;\n",
            parse_file ? "plugin->clear_state_f();\n" : "");
            i = 0;
            for (;i<designer->lv2c.audio_input;i++) {            
                printf ("            %s[i] = %s[i] * fade + buf%i[i] * (1.0 - fade);\n",
                        a_outputs[i], a_outputs[i], i);
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
                designer->controls[i].is_type == IS_IMAGE ||
                designer->controls[i].is_type == IS_TABBOX||
                designer->controls[i].is_audio_input ||
                designer->controls[i].is_audio_output ||
                designer->controls[i].is_atom_input ||
                designer->controls[i].is_atom_output) {
                continue;
            }
            if (!designer->controls[i].destignation_enabled && !parse_file) {
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
    "    connect_(port,data); \n", name);
    if (parse_file) {
        printf("    plugin->connect(port,data);");
    }
    
    printf("}\n\n");

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
    if (designer->lv2c.midi_output) {
        printf("    lv2_atom_forge_init(&self->forge,self->map);\n"
        "    self->midiatom.type  = self->midi_MidiEvent;\n"
        "    self->midiatom.size  = sizeof(self->data);\n");
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

    printf("LV2_SYMBOL_EXPORT\n"
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
    
    for (a-=1;a>-1;a--) {
        free((void*)a_inputs[a]);
    }
    for (o-=1;o>-1;o--) {
        free((void*)a_outputs[o]);
    }
}
