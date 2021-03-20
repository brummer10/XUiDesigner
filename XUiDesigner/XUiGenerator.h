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

#ifndef XUIGENERATOR_H_
#define XUIGENERATOR_H_

#ifdef __cplusplus
extern "C" {
#endif

void remove_from_list(XUiDesigner *designer, Widget_t *wid);

void add_to_list(XUiDesigner *designer, Widget_t *wid, const char* type,
                                    bool have_adjustment, WidgetType is_type);

void print_list(XUiDesigner *designer);

void print_ttl(XUiDesigner *designer);

void run_test(void *w_, void* user_data);

void run_save(void *w_, void* user_data);

#ifdef __cplusplus
}
#endif

#endif //XUIGENERATOR_H_
