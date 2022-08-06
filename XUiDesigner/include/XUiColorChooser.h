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

#ifndef XUICOLORCHOOSER_H_
#define XUICOLORCHOOSER_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    double center_x;
    double center_y;
    double radius;
    double red;
    double green;
    double blue;
    double alpha;
    double lum;
    double focus_x;
    double focus_y;
    Widget_t* al;
    Widget_t* lu;
} ColorChooser_t;

void create_color_chooser (XUiDesigner *designer);

void show_color_chooser(void *w_, void* user_data);

void set_focus_by_color(Widget_t* wid, const double r, const double g, const double b);

#ifdef __cplusplus
}
#endif

#endif //XUICOLORCHOOSER_H_
