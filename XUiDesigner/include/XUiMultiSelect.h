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

#ifndef XUIMULTISELECT_H_
#define XUIMULTISELECT_H_

#ifdef __cplusplus
extern "C" {
#endif

void fix_pos_for_all(XUiDesigner *designer, WidgetType is_type);

void move_all_for_type(XUiDesigner *designer, WidgetType is_type, int x, int y);

void resize_all_for_type(XUiDesigner *designer, Widget_t *wi, WidgetType is_type, int w, int h);

void set_ratio(Widget_t *w);

int is_in_selection(XUiDesigner *designer, int x, int y);

void fix_pos_for_selection(XUiDesigner *designer);

void move_for_selection(XUiDesigner *designer, int x, int y);

void move_selection(XUiDesigner *designer, XMotionEvent *xmotion);

void reset_selection(XUiDesigner *designer);

#ifdef __cplusplus
}
#endif

#endif //XUIMULTISELECT_H_
