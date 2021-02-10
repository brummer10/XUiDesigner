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

void utf8ncpy(char* dst, const char* src, size_t sizeDest ) {
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
    }
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

        memset(&w->input_label[strlen( w->input_label)-(sizeof(char)*(j))],0,sizeof(char)*(j));
        strcat( w->input_label, "|");
    }
    cairo_set_font_size (w->cr, 12.0);
    cairo_text_extents(w->cr, w->input_label , &extents);

    cairo_move_to (w->cr, 2, 12.0+extents.height);
    cairo_show_text(w->cr,  w->input_label);

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
    } else {
        Status status;
        KeySym keysym;
        char buf[32];
        Xutf8LookupString(w->xic, key, buf, sizeof(buf) - 1, &keysym, &status);
        if (keysym == XK_Return) {
            if (designer->active_widget != NULL) {
                if (strlen(w->input_label)) {
                    asprintf (&designer->new_label[designer->active_widget_num], "%s", w->input_label);
                    designer->new_label[designer->active_widget_num][strlen( w->input_label)-1] = 0;
                    designer->active_widget->label = (const char*)designer->new_label[designer->active_widget_num];
                    expose_widget(designer->active_widget);
                }
            }
            return;
        }
        if(status == XLookupChars || status == XLookupBoth){
            entry_add_text(w, buf);
            if (designer->active_widget != NULL) {
                if (strlen(w->input_label)) {
                    asprintf (&designer->new_label[designer->active_widget_num], "%s", w->input_label);
                    designer->new_label[designer->active_widget_num][strlen( w->input_label)-1] = 0;
                    designer->active_widget->label = (const char*)designer->new_label[designer->active_widget_num];
                    expose_widget(designer->active_widget);
                }
            }
        }
    }
}

/*---------------------------------------------------------------------
-----------------------------------------------------------------------    
                text/numeric entry input widget
-----------------------------------------------------------------------
----------------------------------------------------------------------*/

void box_entry_set_text(Widget_t *w, float value) {
    memset(w->input_label, 0, 32 * (sizeof w->input_label[0]));
    char buffer[30];
    snprintf(buffer, sizeof buffer, "%.3f", value);
    strcat(w->input_label, buffer);
    strcat(w->input_label, "|");
    expose_widget(w);
}

static void draw_box_entry(void *w_, void* user_data) {
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

static void box_entry_add_text(void  *w_, void *label_) {
    Widget_t *w = (Widget_t*)w_;
    if (!w) return;
    char *label = (char*)label_;
    if (!label) {
        label = (char*)"";
    }
    draw_box_entry(w,NULL);
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

static void box_entry_clip(Widget_t *w) {
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

        memset(&w->input_label[strlen( w->input_label)-(sizeof(char)*(j))],0,sizeof(char)*(j));
        strcat( w->input_label, "|");
    }
    cairo_set_font_size (w->cr, 12.0);
    cairo_text_extents(w->cr, w->input_label , &extents);

    cairo_move_to (w->cr, 2, 12.0+extents.height);
    cairo_show_text(w->cr,  w->input_label);

}

static void box_entry_get_text(void *w_, void *key_, void *user_data) {
    Widget_t *w = (Widget_t*)w_;
    if (!w) return;
    XKeyEvent *key = (XKeyEvent*)key_;
    if (!key) return;
    int nk = key_mapping(w->app->dpy, key);
    if (nk == 11) {
        box_entry_clip(w);
    } else {
        Status status;
        KeySym keysym;
        char buf[32];
        Xutf8LookupString(w->xic, key, buf, sizeof(buf) - 1, &keysym, &status);
        if (keysym == XK_Return) {
            if (strlen(w->input_label)>1) {
                w->func.user_callback(w, (void*)w->input_label);
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

// data = 0; textinput / data = 1; numeric input
Widget_t *add_input_box(Widget_t *parent, int data, int x, int y, int width, int height) {
    Widget_t *wid = create_widget(parent->app, parent, x, y, width, height);
    memset(wid->input_label, 0, 32 * (sizeof wid->input_label[0]) );
    wid->data = data;
    wid->func.expose_callback = box_entry_add_text;
    wid->func.key_press_callback = box_entry_get_text;
    wid->flags &= ~USE_TRANSPARENCY;
    //wid->scale.gravity = EASTWEST;
    Cursor c = XCreateFontCursor(parent->app->dpy, XC_xterm);
    XDefineCursor (parent->app->dpy, wid->widget, c);
    XFreeCursor(parent->app->dpy, c);
    return wid;
}
