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

#include "XUiConfig.h"


/*---------------------------------------------------------------------
-----------------------------------------------------------------------    
                save/load config file
-----------------------------------------------------------------------
----------------------------------------------------------------------*/


void save_designer_config(XUiDesigner *designer) {
    char* config_file = NULL;
    asprintf(&config_file, "%s/.config/xuidesigner.conf", getenv("HOME"));
    FILE *fpm;
    if((fpm=freopen(config_file, "w" ,stdout))==NULL) {
        printf("Error opening config file\n");
        return;
    }
    if (designer->global_knob_image_file) {
        printf("[Global Knob Image]=%s\n", designer->global_knob_image_file);
    }
    if (designer->global_button_image_file) {
        printf("[Global Button Image]=%s\n", designer->global_button_image_file);
    }
    if (designer->global_switch_image_file) {
        printf("[Global Switch Image]=%s\n", designer->global_switch_image_file);
    }
    if (designer->global_vslider_image_file) {
        printf("[Global VSlider Image]=%s\n", designer->global_vslider_image_file);
    }
    if (designer->global_hslider_image_file) {
        printf("[Global HSlider Image]=%s\n", designer->global_hslider_image_file);
    }
    printf("[Use Global Knob Image]=%f\n", adj_get_value(designer->global_knob_image->adj));
    printf("[Use Global Button Image]=%f\n", adj_get_value(designer->global_button_image->adj));
    printf("[Use Global Switch Image]=%f\n", adj_get_value(designer->global_switch_image->adj));
    printf("[Use Global VSlider Image]=%f\n", adj_get_value(designer->global_vslider_image->adj));
    printf("[Use Global HSlider Image]=%f\n", adj_get_value(designer->global_hslider_image->adj));
    printf("[Global VSlider Sprites]=%i\n", designer->global_vslider_image_sprites);
    printf("[Global HSlider Sprites]=%i\n", designer->global_hslider_image_sprites);
    printf("[Keep Aspect Ratio]=%f\n", adj_get_value(designer->aspect_ratio->adj));
    printf("[Use Global Size]=%f\n", adj_get_value(designer->resize_all->adj));
    printf("[Show Plugin Name]=%f\n", adj_get_value(designer->display_name->adj));
    fclose(fpm);
    free(config_file);
}

void read_designer_config(XUiDesigner *designer) {
    char* config_file = NULL;
    asprintf(&config_file, "%s/.config/xuidesigner.conf", getenv("HOME"));
    FILE *fpm;
    char buf[128];
    if((fpm = fopen(config_file, "r")) == NULL) {
        printf("Error opening config file!\n");
        return;
    }
    while (fgets(buf, 128, fpm) != NULL) {
        char *ptr = strtok(buf, "=");
        while(ptr != NULL) {
            if (strstr(ptr, "[Global Knob Image]") != NULL) {
                free(designer->global_knob_image_file);
                designer->global_knob_image_file = NULL;
                ptr = strtok(NULL, "\n");
                asprintf(&designer->global_knob_image_file, "%s", ptr);
            } else if (strstr(ptr, "[Global Button Image]") != NULL) {
                free(designer->global_button_image_file);
                designer->global_button_image_file = NULL;
                ptr = strtok(NULL, "\n");
                asprintf(&designer->global_button_image_file, "%s", ptr);
            } else if (strstr(ptr, "[Global Switch Image]") != NULL) {
                free(designer->global_switch_image_file);
                designer->global_switch_image_file = NULL;
                ptr = strtok(NULL, "\n");
                asprintf(&designer->global_switch_image_file, "%s", ptr);
            } else if (strstr(ptr, "[Global VSlider Image]") != NULL) {
                free(designer->global_vslider_image_file);
                designer->global_vslider_image_file = NULL;
                ptr = strtok(NULL, "\n");
                asprintf(&designer->global_vslider_image_file, "%s", ptr);
            } else if (strstr(ptr, "[Global HSlider Image]") != NULL) {
                free(designer->global_hslider_image_file);
                designer->global_hslider_image_file = NULL;
                ptr = strtok(NULL, "\n");
                asprintf(&designer->global_hslider_image_file, "%s", ptr);
            } else if (strstr(ptr, "[Use Global Knob Image]") != NULL) {
                ptr = strtok(NULL, "\n");
                adj_set_value(designer->global_knob_image->adj, strtod(ptr, NULL));
            } else if (strstr(ptr, "[Use Global Button Image]") != NULL) {
                ptr = strtok(NULL, "\n");
                adj_set_value(designer->global_button_image->adj, strtod(ptr, NULL));
            } else if (strstr(ptr, "[Use Global Switch Image]") != NULL) {
                ptr = strtok(NULL, "\n");
                adj_set_value(designer->global_switch_image->adj, strtod(ptr, NULL));
            } else if (strstr(ptr, "[Use Global VSlider Image]") != NULL) {
                ptr = strtok(NULL, "\n");
                adj_set_value(designer->global_vslider_image->adj, strtod(ptr, NULL));
            } else if (strstr(ptr, "[Use Global HSlider Image]") != NULL) {
                ptr = strtok(NULL, "\n");
                adj_set_value(designer->global_hslider_image->adj, strtod(ptr, NULL));
            } else if (strstr(ptr, "[Keep Aspect Ratio]") != NULL) {
                ptr = strtok(NULL, "\n");
                adj_set_value(designer->aspect_ratio->adj, strtod(ptr, NULL));
            } else if (strstr(ptr, "[Use Global Size]") != NULL) {
                ptr = strtok(NULL, "\n");
                adj_set_value(designer->resize_all->adj, strtod(ptr, NULL));
            } else if (strstr(ptr, "[Show Plugin Name]") != NULL) {
                ptr = strtok(NULL, "\n");
                adj_set_value(designer->display_name->adj, strtod(ptr, NULL));
            } else if (strstr(ptr, "[Global VSlider Sprites]") != NULL) {
                ptr = strtok(NULL, "\n");
                designer->global_vslider_image_sprites = strtod(ptr, NULL);
            } else if (strstr(ptr, "[Global HSlider Sprites]") != NULL) {
                ptr = strtok(NULL, "\n");
                designer->global_hslider_image_sprites = (int)strtod(ptr, NULL);
            }
            ptr = strtok(NULL, "=");
        }
    }
    fclose(fpm);
    free(config_file);
}

