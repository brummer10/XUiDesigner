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

#ifndef XUITEXTINPUT_H_
#define XUITEXTINPUT_H_

#ifdef __cplusplus
extern "C" {
#endif


typedef struct {
    char input_label[256];
    int curser_pos;
    int curser_mark;
    size_t mark_pos;
    int curser_mark2;
    size_t mark2_pos;
    int set_selection;
    size_t curser_size;
} TextBox_t;


void utf8ncpy(char* dst, const char* src, size_t sizeDest );

void entry_set_text(XUiDesigner *designer, const char* label);

void entry_add_text(void  *w_, void *label_);

void entry_get_text(void *w_, void *key_, void *user_data);

void box_entry_set_value(Widget_t *w, float value);

void box_entry_set_text(Widget_t *w, const char* label);

Widget_t *add_input_box(Widget_t *parent, int data, int x, int y, int width, int height);

#ifdef __cplusplus
}
#endif

#endif //XUITEXTINPUT_H_
