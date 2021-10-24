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

#include "XUiTextInput.h"


/*---------------------------------------------------------------------
-----------------------------------------------------------------------    
                label input widget
-----------------------------------------------------------------------
----------------------------------------------------------------------*/

size_t utf8ncpy(char* dst, const char* src, size_t sizeDest ) {
    if( sizeDest ){
        size_t sizeSrc = strlen(src);
        while( sizeSrc >= sizeDest ){
            const char* lastByte = src + sizeSrc;
            while( lastByte-- > src )
                if((*lastByte & 0xC0) != 0x80)
                    break;
            sizeSrc = lastByte - src;
        }
        memcpy(dst, src, sizeSrc);
        dst[sizeSrc] = '\0';
        return sizeSrc;
    }
    return 0;
}

void entry_set_text(XUiDesigner *designer, const char* label) {
    memset(designer->controller_label->input_label, 0,
        32 * (sizeof designer->controller_label->input_label[0]));
    if (strlen(label))
        utf8ncpy(designer->controller_label->input_label, label, 30);
    strcat(designer->controller_label->input_label, "|");
    expose_widget(designer->controller_label);
}

static void draw_entry(void *w_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    if (!w) return;
    XWindowAttributes attrs;
    XGetWindowAttributes(w->app->dpy, (Window)w->widget, &attrs);
    int width = attrs.width;
    int height = attrs.height;
    if (attrs.map_state != IsViewable) return;

    use_base_color_scheme(w, NORMAL_);
    cairo_rectangle(w->cr,0,0,width,height);
    cairo_fill_preserve (w->cr);
    use_text_color_scheme(w, NORMAL_);
    cairo_set_line_width(w->cr, 2.0);
    cairo_stroke(w->cr);

    cairo_set_font_size (w->cr, 9.0);

    cairo_move_to (w->cr, 2, 9);
    cairo_show_text(w->cr, " ");
}

void entry_add_text(void  *w_, void *label_) {
    Widget_t *w = (Widget_t*)w_;
    if (!w) return;
    char *label = (char*)label_;
    if (!label) {
        label = (char*)"";
    }
    draw_entry(w,NULL);
    cairo_text_extents_t extents;
    use_text_color_scheme(w, NORMAL_);
    cairo_set_font_size (w->cr, 11.0);
    if (strlen( w->input_label))
         w->input_label[strlen( w->input_label)-1] = 0;
    if (strlen( w->input_label)<30) {
        if (strlen(label))
        strcat( w->input_label, label);
    }
    w->label = w->input_label;
    strcat( w->input_label, "|");
    cairo_set_font_size (w->cr, 12.0);
    cairo_text_extents(w->cr, w->input_label , &extents);

    cairo_move_to (w->cr, 2, 12.0+extents.height);
    cairo_show_text(w->cr,  w->input_label);

}

static void entry_clip(Widget_t *w) {
    draw_entry(w,NULL);
    cairo_text_extents_t extents;
    use_text_color_scheme(w, NORMAL_);
    cairo_set_font_size (w->cr, 11.0);

    // check for UTF 8 char
    if (strlen( w->input_label)>=2) {
        int i = strlen( w->input_label)-1;
        int j = 0;
        int u = 0;
        for(;i>0;i--) {
            if(IS_UTF8(w->input_label[i])) {
                 u++;
            }
            j++;
            if (u == 1) break;
            if (j>2) break;
        }
        if (!u) j =2;

        if (IS_UTF8(w->input_label[0]) && (strlen( w->input_label)-(sizeof(char)*(j)) == 1)) 
            memset(&w->input_label[0],0,sizeof(char)*(j));
        else
            memset(&w->input_label[strlen( w->input_label)-(sizeof(char)*(j))],0,sizeof(char)*(j));
        strcat( w->input_label, "|");
    }
    cairo_set_font_size (w->cr, 12.0);
    cairo_text_extents(w->cr, w->input_label , &extents);

    cairo_move_to (w->cr, 2, 12.0+extents.height);
    cairo_show_text(w->cr,  w->input_label);

}

static void update_label(XUiDesigner *designer, Widget_t *w) {
    if (designer->active_widget == NULL) return;
    if (strlen(w->input_label)) {
        if (designer->controls[designer->active_widget_num].is_type == IS_TABBOX) {
            int v = (int)adj_get_value(designer->active_widget->adj);
            if (designer->active_widget_num+v > MAX_CONTROLS) {
                Widget_t *dia = open_message_dialog(w, INFO_BOX, _("INFO"),
                    _("MAX CONTROL COUNTER OVERFLOW | Sorry, cant edit the label anymore"),NULL);
                XSetTransientForHint(w->app->dpy, dia->widget, w->widget);
                return;
            }
            free(designer->tab_label[designer->active_widget_num+v]);
            designer->tab_label[designer->active_widget_num+v] = NULL;
            asprintf (&designer->tab_label[designer->active_widget_num+v], "%s", w->input_label);
            designer->tab_label[designer->active_widget_num+v][strlen( w->input_label)-1] = 0;
            Widget_t *wi = designer->active_widget->childlist->childs[v];
            wi->label = (const char*)designer->tab_label[designer->active_widget_num+v];
        } else {
            free(designer->new_label[designer->active_widget_num]);
            designer->new_label[designer->active_widget_num] = NULL;
            asprintf (&designer->new_label[designer->active_widget_num], "%s", w->input_label);
            designer->new_label[designer->active_widget_num][strlen( w->input_label)-1] = 0;
            designer->active_widget->label = (const char*)designer->new_label[designer->active_widget_num];
        }
        expose_widget(designer->active_widget);
    }
}

void entry_get_text(void *w_, void *key_, void *user_data) {
    Widget_t *w = (Widget_t*)w_;
    if (!w) return;
    XUiDesigner *designer = (XUiDesigner*)w->parent_struct;
    XKeyEvent *key = (XKeyEvent*)key_;
    if (!key) return;
    int nk = key_mapping(w->app->dpy, key);
    if (nk == 11) {
        entry_clip(w);
        update_label(designer, w);
    } else {
        Status status;
        KeySym keysym;
        char buf[32];
        memset(buf, 0, 32 * sizeof(buf[0]));
        Xutf8LookupString(w->xic, key, buf, sizeof(buf) - 1, &keysym, &status);
        if (keysym == XK_Return) {
            update_label(designer, w);
            return;
        }
        if(status == XLookupChars || status == XLookupBoth){
            entry_add_text(w, buf);
            update_label(designer, w);
        }
    }
}

/*---------------------------------------------------------------------
-----------------------------------------------------------------------    
                text/numeric entry input widget
-----------------------------------------------------------------------
----------------------------------------------------------------------*/

// replace or insert a string at position pos. size is the size of string to be removed, could be zero.
void strreplace(char *target, size_t pos, size_t size, const char *replacement) {
    char buffer[1024] = { 0 };
    char *insert_point = &buffer[0];
    const char *tmp = target;
    size_t target_len = strlen(target);
    size_t repl_len = strlen(replacement);

    size_t new_point = utf8ncpy(insert_point, tmp, pos+1);
    if (new_point<pos && pos > 0){
        new_point = utf8ncpy(insert_point, tmp, pos+2);
    }
    insert_point += new_point;
    if (repl_len) {
        new_point = utf8ncpy(insert_point, replacement, repl_len+1);
        insert_point += new_point;
    }
    tmp += pos+size;
    utf8ncpy(insert_point, tmp, target_len+1-pos);
        
    utf8ncpy(target, buffer, 1024);
}

// check for UTF 8 char code point
static size_t findpos(const char* src, size_t sizeDest ) {
    size_t sizeSrc = strlen(src);
    while( sizeSrc > sizeDest ){
        const char* lastByte = src + sizeSrc;
        while( lastByte-- > src )
            if((*lastByte & 0xC0) != 0x80)
                break;
        sizeSrc = lastByte - src;
    }
    return sizeSrc;
}

// set the curser to a code point in the string
static void box_entry_set_curser_pos(Widget_t *w, int s) {
    TextBox_t *text_box = (TextBox_t*)w->private_struct;
    text_box->curser_size = findpos(text_box->input_label, text_box->curser_size + s);
    memset(text_box->selection, 0, 256 * (sizeof text_box->selection[0]));
    memcpy(text_box->selection,text_box->input_label , text_box->curser_size);
    text_box->selection[text_box->curser_size] = '\0';
    cairo_set_font_size (w->cr, 12.0);
    cairo_text_extents_t extents;
    cairo_text_extents(w->cr, text_box->selection , &extents);
    text_box->curser_pos = extents.width-1;
    expose_widget(w);
}

// public function, insert a value when used as numbox
void box_entry_set_value(Widget_t *w, float value) {
    TextBox_t *text_box = (TextBox_t*)w->private_struct;
    memset(text_box->input_label, 0, 256 * (sizeof text_box->input_label[0]));
    char buffer[30];
    snprintf(buffer, sizeof buffer, "%.3f", value);
    strcat(text_box->input_label, buffer);
    box_entry_set_curser_pos(w, strlen(text_box->input_label));
    expose_widget(w);
}

// public function, insert a string when used as textbox
void box_entry_set_text(Widget_t *w, const char* label) {
    TextBox_t *text_box = (TextBox_t*)w->private_struct;
    memset(text_box->input_label, 0, 256 * (sizeof text_box->input_label[0]));
    strncat(text_box->input_label, label, 254);
    box_entry_set_curser_pos(w, strlen(text_box->input_label));
    expose_widget(w);
}

// get the code point and string width on display for a mouse position
static void get_pos_for_x(Widget_t *w, int x, int *width, int *pos) {
    TextBox_t *text_box = (TextBox_t*)w->private_struct;
    cairo_text_extents_t extents;
    cairo_text_extents(w->cr, text_box->input_label , &extents);
    cairo_set_font_size (w->cr, 12.0);
    int i = 0;
    int j = 0;
    for (;i<=strlen(text_box->input_label);i++) {
        j = findpos(text_box->input_label, i);
        memset(text_box->selection, 0, 256 * (sizeof text_box->selection[0]));
        memcpy(text_box->selection,text_box->input_label , j);
        text_box->selection[j] = '\0';
        cairo_set_font_size (w->cr, 12.0);
        cairo_text_extents(w->cr, text_box->selection , &extents);
        if ((int)extents.width >= x) {
            (*pos) = j;
            (*width) = (int)extents.width;
            return;
        }
    }
    if (x > (int)extents.width) {
        (*pos) = j;
        (*width) = (int)extents.width;
    }
}

// set the curser mark position to the mouse pointer
static void text_box_button_pressed(void *w_, void* button_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    TextBox_t *text_box = (TextBox_t*)w->private_struct;
    XButtonEvent *xbutton = (XButtonEvent*)button_;
    if (w->flags & HAS_POINTER) {
        if(xbutton->button == Button1) {
            text_box->set_selection = 0;
            text_box->curser_mark = 0;
            text_box->curser_mark2 = 0;
            int x = xbutton->x;
            int j = 0;
            if (x>6) {
                int width = 0;
                get_pos_for_x(w, x, &width, &j);
                text_box->mark_pos = j;
                text_box->curser_mark = width;
                text_box->curser_mark2 = width;
            } else {
                text_box->mark_pos = 0;
                text_box->mark2_pos = 0;
                text_box->curser_mark = 0;
                text_box->curser_mark2 = 0;
            }
        }
    }
}

// set the curser to the mouse pointer
static void text_box_button_released(void *w_, void* button_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    TextBox_t *text_box = (TextBox_t*)w->private_struct;
    XButtonEvent *xbutton = (XButtonEvent*)button_;
    if (w->flags & HAS_POINTER) {
        if(xbutton->button == Button1) {
            int x = xbutton->x;
            int j = 0;
            if (x>6) {
                int width = 0;
                get_pos_for_x(w, x, &width, &j);
            } 
            box_entry_set_curser_pos(w, -(text_box->curser_size-j));
            expose_widget(w);
        } else if(xbutton->button == Button3) {
            if (!text_box->set_selection) text_box->item1->state = 4;
            else text_box->item1->state = 0;
            if (!have_paste(w)) text_box->item2->state = 4;
            else text_box->item2->state = 0;
            pop_menu_show(w, text_box->menu, 2, true);
        }
    }
}

// maark the complete string on double click and set curser to end position
static void text_box_double_click(void *w_, void* button_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    TextBox_t *text_box = (TextBox_t*)w->private_struct;
    text_box->set_selection = 1;
    text_box->mark_pos = 0;
    text_box->curser_mark = 0;
    text_box->mark2_pos = findpos(text_box->input_label,strlen(text_box->input_label));
    
    cairo_text_extents_t extents;
    cairo_text_extents(w->cr, text_box->input_label , &extents);
    text_box->curser_mark2 = (int)extents.width;
    box_entry_set_curser_pos(w, strlen(text_box->input_label));
    expose_widget(w);
    //copy_to_clipboard(w, text_box->input_label, (int)text_box->mark2_pos);
}

void text_box_pop_menu(void *w_, void* item_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    Widget_t* p = (Widget_t*)w->parent;
    TextBox_t *text_box = (TextBox_t*)w->parent_struct;
    switch (*(int*)item_) {
        case 0:
        {
            if (text_box->set_selection) {
                memset(text_box->selection, 0, 256 * (sizeof text_box->selection[0]));
                int a = (int)text_box->mark_pos;
                int b = (int)text_box->mark2_pos;
                if (text_box->mark_pos > text_box->mark2_pos) {
                    a = (int)text_box->mark2_pos;
                    b = (int)text_box->mark_pos;
                }
                char* pos = &text_box->input_label[a];
                memcpy(text_box->selection, pos , b - a);
                copy_to_clipboard(p, text_box->selection, (int)strlen(text_box->selection));
            }
        }
        break;
        case 1:
            request_paste_from_clipboard(p);
        break;
        default:
        break;
    }
}


// mark the part of string were the mouse pointer hover over while pressed
static void text_box_button_motion(void *w_, void *xmotion_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    TextBox_t *text_box = (TextBox_t*)w->private_struct;
    text_box->set_selection = 1;
    XMotionEvent *xmotion = (XMotionEvent*)xmotion_;
    cairo_text_extents_t extents;
    cairo_text_extents(w->cr, text_box->input_label , &extents);
    cairo_set_font_size (w->cr, 12.0);
    if (w->flags & HAS_POINTER) {
        if(xmotion->state == Button1Mask) {
            int x = xmotion->x;
            int width = 0;
            int j = 0;
            get_pos_for_x(w, x, &width, &j);
            text_box->mark2_pos = j;
            text_box->curser_mark2 = width;
            expose_widget(w);
        }
    }
}

// draw the background of the textbox
static void draw_box_entry(void *w_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    if (!w) return;
    TextBox_t *text_box = (TextBox_t*)w->private_struct;
    XWindowAttributes attrs;
    XGetWindowAttributes(w->app->dpy, (Window)w->widget, &attrs);
    int width = attrs.width;
    int height = attrs.height;
    if (attrs.map_state != IsViewable) return;

    use_base_color_scheme(w, NORMAL_);
    cairo_rectangle(w->cr,0,0,width,height);
    cairo_fill_preserve (w->cr);
    use_text_color_scheme(w, NORMAL_);
    cairo_set_line_width(w->cr, 2.0);
    cairo_stroke(w->cr);

    cairo_set_font_size (w->cr, 9.0);

    cairo_move_to (w->cr, 2, 9);
    cairo_show_text(w->cr, " ");
    if (text_box->set_selection) {
        use_light_color_scheme(w, ACTIVE_);
        cairo_rectangle(w->cr,2+text_box->curser_mark, 2, 
            text_box->curser_mark2-text_box->curser_mark, height-4);
        cairo_fill (w->cr);
    }
}

// add text to the textbox at curser position, or if text is marked, replace it with the input
static void box_entry_add_text(void  *w_, void *label_) {
    Widget_t *w = (Widget_t*)w_;
    if (!w) return;
    TextBox_t *text_box = (TextBox_t*)w->private_struct;
    char *label = (char*)label_;
    if (!label) {
        label = (char*)"";
    }
    draw_box_entry(w,NULL);
    cairo_text_extents_t extents;
    use_text_color_scheme(w, NORMAL_);
    cairo_set_font_size (w->cr, 11.0);
    if (strlen( text_box->input_label)<254) {
        if (strlen(label)) {
            size_t p = 0;
            if (text_box->set_selection) {
                if (text_box->curser_mark < text_box->curser_mark2) {
                    p = (text_box->mark2_pos - text_box->mark_pos)-1;
                } else {
                    p = (text_box->mark_pos - text_box->mark2_pos);
                }
            }
            size_t j = findpos(text_box->input_label, text_box->curser_size-1);
            if (text_box->set_selection && (text_box->curser_mark > text_box->curser_mark2)) {
                strreplace(text_box->input_label, text_box->curser_size, p, label);
            } else if (text_box->set_selection && (text_box->curser_mark < text_box->curser_mark2)) {
                strreplace(text_box->input_label, max(0,j-p), text_box->curser_size-j+p, label);
            } else {
                strreplace(text_box->input_label, text_box->curser_size, 0, label);
            }
            text_box->set_selection = 0;
            text_box->mark_pos = 0;
            text_box->mark2_pos = 0;
            text_box->curser_mark = 0;
            text_box->curser_mark2 = 0;
            box_entry_set_curser_pos(w, strlen(label));
        }
    }
    w->label = text_box->input_label;
    cairo_set_font_size (w->cr, 12.0);
    cairo_text_extents(w->cr, text_box->input_label , &extents);

    cairo_move_to (w->cr, 2, 20);
    cairo_show_text(w->cr,  text_box->input_label);

    cairo_move_to (w->cr, 2+text_box->curser_pos, 20);
    cairo_show_text(w->cr,  w->input_label);
    w->func.value_changed_callback(w, NULL);
}

// clip the marked part of the text, or one char at curser position
static void box_entry_clip(Widget_t *w) {
    TextBox_t *text_box = (TextBox_t*)w->private_struct;
    draw_entry(w,NULL);
    cairo_text_extents_t extents;
    use_text_color_scheme(w, NORMAL_);
    cairo_set_font_size (w->cr, 11.0);

    if (strlen( text_box->input_label)>=1) {
        size_t p = 0;
        if (text_box->set_selection) {
            if (text_box->curser_mark < text_box->curser_mark2) {
                p = (text_box->mark2_pos - text_box->mark_pos)-1;
            } else {
                p = (text_box->mark_pos - text_box->mark2_pos);
            }
        }
        size_t j = findpos(text_box->input_label, text_box->curser_size-1);
        int m = (IS_UTF8(text_box->input_label[j])) ? -2:-1;
        if (text_box->set_selection && (text_box->curser_mark > text_box->curser_mark2)) {
            strreplace(text_box->input_label, text_box->curser_size, p, "");
            m = 0;
        } else {
            strreplace(text_box->input_label, j-p, text_box->curser_size-j+p, "");
        }
        box_entry_set_curser_pos(w, m);
        text_box->set_selection = 0;
        text_box->mark_pos = 0;
        text_box->mark2_pos = 0;
        text_box->curser_mark = 0;
        text_box->curser_mark2 = 0;
        
    }
    cairo_set_font_size (w->cr, 12.0);
    cairo_text_extents(w->cr, text_box->input_label , &extents);

    cairo_move_to (w->cr, 2, 20);
    cairo_show_text(w->cr, text_box->input_label);

    cairo_move_to (w->cr, 2+text_box->curser_pos, 20);
    cairo_show_text(w->cr,  w->input_label);
    w->func.value_changed_callback(w, NULL);
}

// textbox receive a keypress
static void box_entry_get_text(void *w_, void *key_, void *user_data) {
    Widget_t *w = (Widget_t*)w_;
    if (!w) return;
    TextBox_t *text_box = (TextBox_t*)w->private_struct;
    XKeyEvent *key = (XKeyEvent*)key_;
    if (!key) return;
    int nk = key_mapping(w->app->dpy, key);
    if (nk == 11) {
        box_entry_clip(w);
    } else if (nk == 6) {
        box_entry_set_curser_pos(w, -1);
    } else if (nk == 4) {
        if(IS_UTF8(text_box->input_label[text_box->curser_size])) 
            box_entry_set_curser_pos(w, 2);
        else
            box_entry_set_curser_pos(w, 1);
    } else {
        Status status;
        KeySym keysym;
        char buf[32];
        memset(buf, 0, 32 * sizeof(buf[0]));
        Xutf8LookupString(w->xic, key, buf, sizeof(buf) - 1, &keysym, &status);
        if (keysym == XK_Return) {
            if (strlen(text_box->input_label)>1) {
                w->func.user_callback(w, (void*)text_box->input_label);
            }
            return;
        }
        if(status == XLookupChars || status == XLookupBoth){
            if (w->data) {
                if (key->keycode == XKeysymToKeycode(w->app->dpy, XK_0) ||
                    key->keycode == XKeysymToKeycode(w->app->dpy, XK_1) ||
                    key->keycode == XKeysymToKeycode(w->app->dpy, XK_2) ||
                    key->keycode == XKeysymToKeycode(w->app->dpy, XK_3) ||
                    key->keycode == XKeysymToKeycode(w->app->dpy, XK_4) ||
                    key->keycode == XKeysymToKeycode(w->app->dpy, XK_5) ||
                    key->keycode == XKeysymToKeycode(w->app->dpy, XK_6) ||
                    key->keycode == XKeysymToKeycode(w->app->dpy, XK_7) ||
                    key->keycode == XKeysymToKeycode(w->app->dpy, XK_8) ||
                    key->keycode == XKeysymToKeycode(w->app->dpy, XK_9) ||
                    key->keycode == XKeysymToKeycode(w->app->dpy, XK_period)) {
                    box_entry_add_text(w, buf);
                }
            } else {
                box_entry_add_text(w, buf);
            }
        }
    }
}

static void text_box_paste(void *w_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    //TextBox_t *text_box = (TextBox_t*)w->private_struct;
    if(user_data !=NULL) {
        box_entry_add_text(w, *(char**)user_data);
    }
}


// free the internal used memory of the textbox
static void text_box_mem_free(void *w_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    TextBox_t *text_box = (TextBox_t*)w->private_struct;
    free(text_box);
}

// data = 0; textinput / data = 1; numeric input
Widget_t *add_input_box(Widget_t *parent, int data, int x, int y, int width, int height) {
    Widget_t *wid = create_widget(parent->app, parent, x, y, width, height);
    TextBox_t* text_box = (TextBox_t*)malloc(sizeof(TextBox_t));
    wid->private_struct = text_box;
    memset(text_box->input_label, 0, 256 * (sizeof text_box->input_label[0]));
    memset(text_box->selection, 0, 256 * (sizeof text_box->selection[0]));
    text_box->curser_pos = 0;
    text_box->curser_mark = 0;
    text_box->mark_pos = 0;
    text_box->curser_mark2 = 0;
    text_box->mark2_pos = 0;
    text_box->set_selection = 0;
    text_box->curser_size = 0;
    strcat(wid->input_label, "|");
    wid->flags |= HAS_MEM;
    wid->data = data;
    wid->func.expose_callback = box_entry_add_text;
    wid->func.key_press_callback = box_entry_get_text;
    wid->func.mem_free_callback = text_box_mem_free;
    wid->func.button_press_callback = text_box_button_pressed;
    wid->func.button_release_callback = text_box_button_released;
    wid->func.motion_callback = text_box_button_motion;
    wid->func.double_click_callback = text_box_double_click;
    wid->xpaste_callback = text_box_paste;
    wid->flags &= ~USE_TRANSPARENCY;

    text_box->menu = create_menu(wid, 25);
    text_box->menu->parent = wid;
    text_box->menu->parent_struct = text_box;
    text_box->item1 = menu_add_item(text_box->menu, _("Copy"));
    text_box->item2 = menu_add_item(text_box->menu, _("Paste"));
    text_box->menu->func.button_release_callback = text_box_pop_menu;

    //wid->scale.gravity = EASTWEST;
    Cursor c = XCreateFontCursor(parent->app->dpy, XC_xterm);
    XDefineCursor (parent->app->dpy, wid->widget, c);
    XFreeCursor(parent->app->dpy, c);
    return wid;
}
