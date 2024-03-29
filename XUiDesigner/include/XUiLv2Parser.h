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



#include "XUiDesigner.h"


#pragma once

#ifndef XUILV2PARSER_H_
#define XUILV2PARSER_H_

#ifdef __cplusplus
extern "C" {
#endif

int load_plugin_ui(Widget_t *w);

void filter_uris_by_word(Widget_t *lv2_uris, Widget_t *lv2_names,
                        const LilvPlugins* lv2_plugins, const char* word);

void filter_uris(Widget_t *lv2_uris, Widget_t *lv2_names, const LilvPlugins* lv2_plugins);

void load_uris(Widget_t *lv2_uris, Widget_t *lv2_names, const LilvPlugins* lv2_plugins);

void set_path(LilvWorld* world, const char* workdir);

void reset_plugin_ui(XUiDesigner *designer);

#ifdef __cplusplus
}
#endif

#endif //XUILV2PARSER_H_
