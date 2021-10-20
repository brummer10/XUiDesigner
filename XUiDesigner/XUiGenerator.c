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

#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <libgen.h>
#include <ctype.h>

#include "XUiGenerator.h"
#include "XUiGridControl.h"
#include "XUiWriteTurtle.h"
#include "XUiWritePlugin.h"
#include "XUiWriteUI.h"


/*---------------------------------------------------------------------
-----------------------------------------------------------------------    
                    handle widget list
-----------------------------------------------------------------------
----------------------------------------------------------------------*/

void remove_from_list(XUiDesigner *designer, Widget_t *wid) {
    designer->controls[wid->data].wid = NULL;
    designer->controls[wid->data].have_adjustment = false;
    free(designer->controls[wid->data].image);
    designer->controls[wid->data].image = NULL;
    designer->controls[wid->data].grid_snap_option = 0;
    designer->controls[wid->data].in_frame = 0;
    designer->controls[wid->data].in_tab = 0;
    designer->controls[wid->data].is_atom_patch = false;
    designer->controls[wid->data].is_audio_input = false;
    designer->controls[wid->data].is_audio_output = false;
    designer->controls[wid->data].is_atom_input = false;
    designer->controls[wid->data].is_atom_output = false;
}

void add_to_list(XUiDesigner *designer, Widget_t *wid, const char* type,
                                    bool have_adjustment, WidgetType is_type) {
    designer->controls[wid->data].wid = wid;
    designer->controls[wid->data].type = type;
    designer->controls[wid->data].have_adjustment = have_adjustment;
    designer->controls[wid->data].is_type = is_type;
}

/*---------------------------------------------------------------------
-----------------------------------------------------------------------    
                    helper functions for generators
-----------------------------------------------------------------------
----------------------------------------------------------------------*/

void strtovar(char* c) {
    char* b = "_";
    int i = 0;
    for (i=0; c[i] != '\0'; i++) {
        if (!isalnum((unsigned char)c[i])) {
            c[i] = (*b);
        } else {
            c[i] = tolower((unsigned char)c[i]);
        }
    }
}

void strtosym(char* c) {
    char* b = "_";
    int i = 0;
    for (i=0; c[i] != '\0'; i++) {
        if (!isalnum((unsigned char)c[i])) {
            c[i] = (*b);
        }
    }
}

void strtoguard(char* c) {
    char* b = "_";
    int i = 0;
    for (i=0; c[i] != '\0'; i++) {
        if (!isalnum((unsigned char)c[i])) {
            c[i] = (*b);
        } else {
            c[i] = toupper((unsigned char)c[i]);
        }
    }
}

/*---------------------------------------------------------------------
-----------------------------------------------------------------------    
            generate a LV2 bunlde containing all needed files
-----------------------------------------------------------------------
----------------------------------------------------------------------*/

void run_save(void *w_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    XUiDesigner *designer = (XUiDesigner*)w->parent_struct;
    if(user_data !=NULL) {
        if( access(*(const char**)user_data, F_OK ) == -1 ) {
            Widget_t *dia = open_message_dialog(w, ERROR_BOX, *(const char**)user_data,
                                                _("Couldn't access file, sorry"),NULL);
            XSetTransientForHint(w->app->dpy, dia->widget, w->widget);
            return;
        }
        
        int i = 0;
        int j = 0;
        bool have_image = false;
        bool use_atom = false;
        for (;i<MAX_CONTROLS;i++) {
            if (designer->controls[i].wid != NULL) {
                j++;
            }
            if (designer->controls[i].image) {
                have_image = true;
            }
            if (designer->controls[i].is_atom_patch) {
                use_atom = true;
            }
        }
        if (!j) {
            Widget_t *dia = open_message_dialog(designer->ui, INFO_BOX, _("INFO"),
                                            _("Please create at least one Controller,|or load a LV2 URI to save a build "),NULL);
            XSetTransientForHint(w->app->dpy, dia->widget, designer->ui->widget);
            return;
        }
        Window w = (Window)designer->ui->widget;
        char *name = NULL;
        XFetchName(designer->ui->app->dpy, w, &name);
        if (name == NULL) asprintf(&name, "%s", "noname");
        strdecode(name, " ", "_");
        char* filepath = NULL;
        asprintf(&filepath, "%s%s_ui",*(const char**)user_data,name);
        struct stat st = {0};

        if (stat(filepath, &st) == -1) {
            mkdir(filepath, 0700);
        }

        char* cmd = NULL;
        asprintf(&cmd, "cd %s && git init", filepath);
        int ret = system(cmd);
        free(cmd);
        cmd = NULL;
        asprintf(&cmd, "cd %s && git submodule add https://github.com/brummer10/libxputty.git", filepath);
        ret = system(cmd);
        free(cmd);
        cmd = NULL;

        asprintf(&cmd, "SUBDIR := %s\n\n"

            ".PHONY: $(SUBDIR) libxputty  recurse\n\n"

            "$(MAKECMDGOALS) recurse: $(SUBDIR)\n\n"

            "clean:\n\n"

            "libxputty:\n"
            "	@exec $(MAKE) -j 1 -C $@ $(MAKECMDGOALS)\n\n"

            "$(SUBDIR): libxputty\n"
            "	@exec $(MAKE) -j 1 -C $@ $(MAKECMDGOALS)\n\n", name);

        char* makefile = NULL;
        asprintf(&makefile, "%s/makefile",filepath);
        FILE *fpm;
        if((fpm=freopen(makefile, "w" ,stdout))==NULL) {
            printf("open failed\n");
        }
        printf(cmd);
        fclose(fpm);
        free(makefile);
        free(cmd);
        cmd = NULL;
       

        free(filepath);
        filepath = NULL;
        asprintf(&filepath, "%s%s_ui/%s",*(const char**)user_data, name, name);

        if (stat(filepath, &st) == -1) {
            mkdir(filepath, 0700);
        }

        char* filename = NULL;
        asprintf(&filename, "%s%s_ui/%s/%s.c",*(const char**)user_data,name,name, name );
        remove (filename);

        FILE *fp;
        if((fp=freopen(filename, "w" ,stdout))==NULL) {
            printf("open failed\n");
        }
        fprintf(stderr, "save to %s\n", filename);
        print_list(designer);
        fclose(fp);
        fp = NULL;
        if (!designer->generate_ui_only) {
            strdecode(filename, ".c", ".cpp");
            if((fp=freopen(filename, "w" ,stdout))==NULL) {
                printf("open failed\n");
            }
            print_plugin(designer);
            fclose(fp);
            fp = NULL;
            strdecode(filename, ".cpp", ".ttl");
        } else {
            free(filename);
            filename = NULL;
            asprintf(&filename, "%s%s_ui/%s/%s_ui.ttl",*(const char**)user_data,name,name,name);
        }
        
        if((fp=freopen(filename, "w" ,stdout))==NULL) {
            printf("open failed\n");
        }
        print_ttl(designer);
        fclose(fp);
        fp = NULL;
        free(filename);
        filename = NULL;
        asprintf(&filename, "%s%s_ui/%s/manifest.ttl",*(const char**)user_data,name,name);
        if((fp=freopen(filename, "w" ,stdout))==NULL) {
            printf("open failed\n");
        }
        print_manifest(designer);
        fclose(fp);
        fp = NULL;
        free(filename);
        filename = NULL;
        if (system(NULL)) {
            cmd = NULL;
            asprintf(&cmd, "cp /usr/share/XUiDesigner/wrapper/libxputty/lv2_plugin.* \'%s\'", filepath);
            ret = system(cmd);
            if (!ret) {
                free(cmd);
                cmd = NULL;
                if (!designer->generate_ui_only) {
                    asprintf(&cmd,"\n\n	# check if user is root\n"
                        "	user = $(shell whoami)\n"
                        "	ifeq ($(user),root)\n"
                        "	INSTALL_DIR = /usr/lib/lv2\n"
                        "	else \n"
                        "	INSTALL_DIR = ~/.lv2\n"
                        "	endif\n"
                        "\n\n	# check LD version\n"
                        "	ifneq ($(shell xxd --version 2>&1 | head -n 1 | grep xxd),)\n"
                        "		USE_XXD = 1\n"
                        "	else ifneq ($(shell $(LD) --version 2>&1 | head -n 1 | grep LLD),)\n"
                        "		ifneq ($(shell uname -a | grep  x86_64), )\n"
                        "			LDEMULATION := elf_x86_64\n"
                        "		else ifneq ($(shell uname -a | grep amd64), )\n"
                        "			LDEMULATION := elf_x86_64\n"
                        "		else ifneq ($(shell uname -a | grep i386), )\n"
                        "			LDEMULATION := elf_i386\n"
                        "		endif\n"
                        "		USE_LDD = 1\n"
                        "	else ifneq ($(shell gold --version 2>&1 | head -n 1 | grep gold),)\n"
                        "		LD = gold\n"
                        "	endif\n"

                        "\n\n	NAME = %s\n"
                        "	space := $(subst ,, )\n"
                        "	EXEC_NAME := $(subst $(space),_,$(NAME))\n"
                        "	BUNDLE = $(EXEC_NAME).lv2\n"
                        "	RESOURCES_DIR :=./resources/\n"
                        "	LIB_DIR := ../libxputty/libxputty/\n"
                        "	HEADER_DIR := $(LIB_DIR)include/\n"
                        "	UI_LIB:= $(LIB_DIR)libxputty.a\n"
                        "	STRIP ?= strip\n\n"
                        "	RESOURCES := $(wildcard $(RESOURCES_DIR)*.png)\n"
                        "	RESOURCES_OBJ := $(notdir $(patsubst %s.png,%s.o,$(RESOURCES)))\n"
                        "	RESOURCES_LIB := $(notdir $(patsubst %s.png,%s.a,$(RESOURCES)))\n"
                        "	RESOURCE_EXTLD := $(notdir $(patsubst %s.png,%s_png,$(RESOURCES)))\n"
                        "	RESOURCEHEADER := xresources.h\n"
                        "	LDFLAGS += -fvisibility=hidden -Wl,-Bstatic `pkg-config --cflags --libs xputty` \\\n"
                        "	-Wl,-Bdynamic `pkg-config --cflags --libs cairo x11 lilv-0` \\\n"
                        "	-shared -lm -fPIC -Wl,-z,noexecstack -Wl,--no-undefined -Wl,--gc-sections\n"
                        "	CFLAGS := -O2 -D_FORTIFY_SOURCE=2 -Wall -fstack-protector -fvisibility=hidden \\\n"
                        "	-fdata-sections -Wl,--gc-sections -Wl,-z,relro,-z,now -Wl,--exclude-libs,ALL\n\n"
                        ".PHONY : all install uninstall\n\n"
                        ".NOTPARALLEL:\n\n"
                        "all: $(RESOURCEHEADER) $(EXEC_NAME)\n\n"
                        "	@mkdir -p ./$(BUNDLE)\n"
                        "	@cp ./*.ttl ./$(BUNDLE)\n"
                        "	@cp ./*.so ./$(BUNDLE)\n"
                        "	@if [ -f ./$(BUNDLE)/$(EXEC_NAME).so ]; then echo \"build finish, now run make install\"; \\\n"
                        "	else echo \"sorry, build failed\"; fi\n\n"
                        "$(RESOURCEHEADER): $(RESOURCES_OBJ)\n"
                        "	rm -f $(RESOURCEHEADER)\n"
                        "	for f in $(RESOURCE_EXTLD); do \\\n"
                        "		echo 'EXTLD('$${f}')' >> $(RESOURCEHEADER) ; \\\n"
                        "	done\n\n"
                        "ifdef USE_XXD\n"
                        "$(RESOURCES_OBJ): $(RESOURCES)\n"
                        "	@#use this line to regenerate the *.c files from used images\n"
                        "	@#cd $(RESOURCES_DIR) && xxd -i $(patsubst %s.o,%s.png,$@) > $(patsubst %s.o,%s.c,$@)\n"
                        "	$(CC) -c $(RESOURCES_DIR)$(patsubst %s.o,%s.c,$@) -o $@\n"
                        "	$(AR) rcs $(patsubst %s.o,%s.a,$@) $@\n"
                        "else ifdef USE_LDD\n"
                        "$(RESOURCES_OBJ): $(RESOURCES)\n"
                        "	cd $(RESOURCES_DIR) && $(LD) -r -b binary -m $(LDEMULATION) -z noexecstack $(patsubst %s.o,%s.png,$@) -o ../$@\n"
                        "	$(AR) rcs $(patsubst %s.o,%s.a,$@) $@\n"
                        "	LDFLAGS += -DUSE_LD=1\n"
                        "else\n"
                        "$(RESOURCES_OBJ): $(RESOURCES)\n"
                        "	cd $(RESOURCES_DIR) && $(LD) -r -b binary -z noexecstack --strip-all $(patsubst %s.o,%s.png,$@) -o ../$@\n"
                        "	$(AR) rcs $(patsubst %s.o,%s.a,$@) $@\n"
                        "	LDFLAGS += -DUSE_LD=1\n"
                        "endif\n\n"
                        "$(EXEC_NAME):$(RESOURCES_OBJ)\n"
                        "	@# use this line when you include libxputty as submodule\n"
                        "	@$(CC) $(CFLAGS) \'$(NAME).c\' -L. $(RESOURCES_LIB) $(UI_LIB) -o \'$(EXEC_NAME)_ui.so\' $(LDFLAGS) -I./ -I$(HEADER_DIR)\n"
                        "	$(CXX) $(CXXFLAGS) $(EXEC_NAME).cpp $(LDFLAGS) -o $(EXEC_NAME).so\n"
                        "	$(CC) %s $(CFLAGS) \'$(NAME).c\' -L. $(RESOURCES_LIB) -o \'$(EXEC_NAME)_ui.so\' $(LDFLAGS) -I./\n"
                        "	$(STRIP) -s -x -X -R .comment -R .note.ABI-tag $(EXEC_NAME).so\n"
                        "	$(STRIP) -s -x -X -R .comment -R .note.ABI-tag $(EXEC_NAME)_ui.so\n\n"
                        "install :\n"
                        "ifneq (\"$(wildcard ./$(BUNDLE))\",\"\")\n"
                        "	@mkdir -p $(DESTDIR)$(INSTALL_DIR)/$(BUNDLE)\n"
                        "	cp -r ./$(BUNDLE)/* $(DESTDIR)$(INSTALL_DIR)/$(BUNDLE)\n"
                        "	@echo \". ., done\"\n"
                        "else\n"
                        "	@echo \". ., you must build first\"\n"
                        "endif\n\n"
                        "uninstall :\n"
                        "	@rm -rf $(INSTALL_DIR)/$(BUNDLE)\n"
                        "	@echo \". ., done\"\n\n"
                        "clean:\n"
                        "	rm -f *.a *.o *.so xresources.h\n\n",
                        name, "%%","%%","%%","%%","%%","%%","%%","%%","%%","%%","%%","%%","%%","%%","%%","%%","%%","%%","%%","%%","%%","%%", use_atom ? "-DUSE_ATOM" : "");
                } else {
                    asprintf(&cmd,"\n\n	# check if user is root\n"
                        "	user = $(shell whoami)\n"
                        "	ifeq ($(user),root)\n"
                        "	INSTALL_DIR = /usr/lib/lv2\n"
                        "	else \n"
                        "	INSTALL_DIR = ~/.lv2\n"
                        "	endif\n"
                        "\n\n	# check LD version\n"
                        "	ifneq ($(shell xxd --version 2>&1 | head -n 1 | grep xxd),)\n"
                        "		USE_XXD = 1\n"
                        "	else ifneq ($(shell $(LD) --version 2>&1 | head -n 1 | grep LLD),)\n"
                        "		ifneq ($(shell uname -a | grep  x86_64), )\n"
                        "			LDEMULATION := elf_x86_64\n"
                        "		else ifneq ($(shell uname -a | grep amd64), )\n"
                        "			LDEMULATION := elf_x86_64\n"
                        "		else ifneq ($(shell uname -a | grep i386), )\n"
                        "			LDEMULATION := elf_i386\n"
                        "		endif\n"
                        "		USE_LDD = 1\n"
                        "	else ifneq ($(shell gold --version 2>&1 | head -n 1 | grep gold),)\n"
                        "		LD = gold\n"
                        "	endif\n"

                        "\n\n	NAME = %s\n"
                        "	space := $(subst ,, )\n"
                        "	EXEC_NAME := $(subst $(space),_,$(NAME))\n"
                        "	BUNDLE = $(EXEC_NAME)_ui.lv2\n"
                        "	RESOURCES_DIR :=./resources/\n"
                        "	LIB_DIR := ../libxputty/libxputty/\n"
                        "	HEADER_DIR := $(LIB_DIR)include/\n"
                        "	UI_LIB:= $(LIB_DIR)libxputty.a\n"
                        "	STRIP ?= strip\n\n"
                        "	RESOURCES := $(wildcard $(RESOURCES_DIR)*.png)\n"
                        "	RESOURCES_OBJ := $(notdir $(patsubst %s.png,%s.o,$(RESOURCES)))\n"
                        "	RESOURCES_LIB := $(notdir $(patsubst %s.png,%s.a,$(RESOURCES)))\n"
                        "	RESOURCE_EXTLD := $(notdir $(patsubst %s.png,%s_png,$(RESOURCES)))\n"
                        "	RESOURCEHEADER := xresources.h\n"
                        "	LDFLAGS += -fvisibility=hidden -Wl,-Bstatic `pkg-config --cflags --libs xputty` \\\n"
                        "	-Wl,-Bdynamic `pkg-config --cflags --libs cairo x11 lilv-0` \\\n"
                        "	-shared -lm -fPIC -Wl,-z,noexecstack -Wl,--no-undefined -Wl,--gc-sections\n"
                        "	CFLAGS := -O2 -D_FORTIFY_SOURCE=2 -Wall -fstack-protector -fvisibility=hidden \\\n"
                        "	-fdata-sections -Wl,--gc-sections -Wl,-z,relro,-z,now -Wl,--exclude-libs,ALL\n\n"
                        ".PHONY : all install uninstall\n\n"
                        ".NOTPARALLEL:\n\n"
                        "all: $(RESOURCEHEADER) $(EXEC_NAME)\n\n"
                        "	@mkdir -p ./$(BUNDLE)\n"
                        "	@cp ./*.ttl ./$(BUNDLE)\n"
                        "	@cp ./*.so ./$(BUNDLE)\n"
                        "	@if [ -f ./$(BUNDLE)/$(EXEC_NAME)_ui.so ]; then echo \"build finish, now run make install\"; \\\n"
                        "	else echo \"sorry, build failed\"; fi\n\n"
                        "$(RESOURCEHEADER): $(RESOURCES_OBJ)\n"
                        "	rm -f $(RESOURCEHEADER)\n"
                        "	for f in $(RESOURCE_EXTLD); do \\\n"
                        "		echo 'EXTLD('$${f}')' >> $(RESOURCEHEADER) ; \\\n"
                        "	done\n\n"
                        "ifdef USE_XXD\n"
                        "$(RESOURCES_OBJ): $(RESOURCES)\n"
                        "	@#use this line to regenerate the *.c files from used images\n"
                        "	@#cd $(RESOURCES_DIR) && xxd -i $(patsubst %s.o,%s.png,$@) > $(patsubst %s.o,%s.c,$@)\n"
                        "	$(CC) -c $(RESOURCES_DIR)$(patsubst %s.o,%s.c,$@) -o $@\n"
                        "	$(AR) rcs $(patsubst %s.o,%s.a,$@) $@\n"
                        "else ifdef USE_LDD\n"
                        "$(RESOURCES_OBJ): $(RESOURCES)\n"
                        "	cd $(RESOURCES_DIR) && $(LD) -r -b binary -m $(LDEMULATION) -z noexecstack $(patsubst %s.o,%s.png,$@) -o ../$@\n"
                        "	$(AR) rcs $(patsubst %s.o,%s.a,$@) $@\n"
                        "	LDFLAGS += -DUSE_LD=1\n"
                        "else\n"
                        "$(RESOURCES_OBJ): $(RESOURCES)\n"
                        "	cd $(RESOURCES_DIR) && $(LD) -r -b binary -z noexecstack --strip-all $(patsubst %s.o,%s.png,$@) -o ../$@\n"
                        "	$(AR) rcs $(patsubst %s.o,%s.a,$@) $@\n"
                        "	LDFLAGS += -DUSE_LD=1\n"
                        "endif\n\n"
                        "$(EXEC_NAME):$(RESOURCES_OBJ)\n"
                        "	@# use this line when you include libxputty as submodule\n"
                        "	@$(CC) $(CFLAGS) \'$(NAME).c\' -L. $(RESOURCES_LIB) $(UI_LIB) -o \'$(EXEC_NAME)_ui.so\' $(LDFLAGS) -I./ -I$(HEADER_DIR)\n"
                        "	$(CC) %s $(CFLAGS) \'$(NAME).c\' -L. $(RESOURCES_LIB) -o \'$(EXEC_NAME)_ui.so\' $(LDFLAGS) -I./\n"
                        "	$(STRIP) -s -x -X -R .comment -R .note.ABI-tag $(EXEC_NAME)_ui.so\n\n"
                        "install :\n"
                        "ifneq (\"$(wildcard ./$(BUNDLE))\",\"\")\n"
                        "	@mkdir -p $(DESTDIR)$(INSTALL_DIR)/$(BUNDLE)\n"
                        "	cp -r ./$(BUNDLE)/* $(DESTDIR)$(INSTALL_DIR)/$(BUNDLE)\n"
                        "	@echo \". ., done\"\n"
                        "else\n"
                        "	@echo \". ., you must build first\"\n"
                        "endif\n\n"
                        "uninstall :\n"
                        "	@rm -rf $(INSTALL_DIR)/$(BUNDLE)\n"
                        "	@echo \". ., done\"\n\n"
                        "clean:\n"
                        "	rm -f *.a *.o *.so xresources.h\n\n",
                        name, "%%","%%","%%","%%","%%","%%","%%","%%","%%","%%","%%","%%","%%","%%","%%","%%","%%","%%","%%","%%","%%","%%", use_atom ? "-DUSE_ATOM" : "");                    
                }
                makefile = NULL;
                asprintf(&makefile, "%s/makefile",filepath);
                FILE *fpm;
                if((fpm=freopen(makefile, "w" ,stdout))==NULL) {
                    printf("open failed\n");
                }
                printf(cmd);
                fclose(fpm);
                free(makefile);
                free(cmd);
                cmd = NULL;
                //asprintf(&cmd, "cd \'%s\'  && make", filepath);
                //ret = system(cmd);
                //free(cmd);
                //cmd = NULL;
            } else {
                free(cmd);
                cmd = NULL;
            }
        }
        free(filepath);
        
        cmd = NULL;
        if (have_image || designer->image != NULL) {
            filepath = NULL;
            asprintf(&filepath, "%s%s_ui/resources",*(const char**)user_data,name);
            if (stat(filepath, &st) == -1) {
                mkdir(filepath, 0700);
            } else {
                asprintf(&cmd, "rm -rf \'%s\'", filepath);
                int ret = system(cmd);
                if (!ret) {
                    free(cmd);
                    cmd = NULL;
                    mkdir(filepath, 0700);
                } else {
                    free(cmd);
                    cmd = NULL;
                }
            }
        }
        if (designer->image != NULL) {
            //png2c(designer->image,filepath);
            char* xldl = strdup(basename(designer->image));
            strdecode(xldl, "-", "_");
            strdecode(xldl, " ", "_");
            strtovar(xldl);
            strdecode(xldl, "_png", ".png");
            char* fxldl = NULL;
            asprintf(&fxldl, "%s/%s", filepath, xldl);
            asprintf(&cmd, "cp \'%s\' \'%s\'", designer->image,fxldl);
            int ret = system(cmd);
            if (!ret) {
                char* xldc =  strdup(xldl);
                strdecode(xldc, ".png", ".c");
                free(cmd);
                cmd = NULL;
                asprintf(&cmd, "cd %s && xxd -i %s > %s", filepath, xldl, xldc);
                ret = system(cmd);
                free(xldc);
                free(cmd);
                cmd = NULL;
            } else {
                free(cmd);
                cmd = NULL;
                fprintf(stderr, "Fail to copy image\n");
            }
            free(xldl);
            free(fxldl);
        }
        if (have_image) {
            i = 0;
            for (;i<MAX_CONTROLS;i++) {
                if (designer->controls[i].image != NULL) {
                    //png2c(designer->controls[i].image,filepath);
                    char* xldl = strdup(basename(designer->controls[i].image));
                    strdecode(xldl, "-", "_");
                    strdecode(xldl, " ", "_");
                    strtovar(xldl);
                    strdecode(xldl, "_png", ".png");
                    char* fxldl = NULL;
                    asprintf(&fxldl, "%s/%s", filepath, xldl);
                    asprintf(&cmd, "cp \'%s\' \'%s\'", designer->controls[i].image,fxldl);
                    int ret = system(cmd);
                    if (!ret) {
                        char* xldc = strdup(xldl);
                        strdecode(xldc, ".png", ".c");
                        free(cmd);
                        cmd = NULL;
                        asprintf(&cmd, "cd %s && xxd -i %s > %s", filepath, xldl, xldc);
                        ret = system(cmd);
                        free(xldc);
                        free(cmd);
                        cmd = NULL;
                    } else {
                        free(cmd);
                        cmd = NULL;
                        fprintf(stderr, "Fail to copy image\n");
                    }
                    free(xldl);
                    free(fxldl);
                }
            }
            free(filepath);
            filepath = NULL;
            asprintf(&filepath, "%s%s_ui",*(const char**)user_data,name);
            char* cmd = NULL;
            asprintf(&cmd, "cd %s && git add .", filepath);
            ret = system(cmd);
            free(cmd);
            cmd = NULL;
            free(filepath);
            free(name);
        }
    }
}


/*---------------------------------------------------------------------
-----------------------------------------------------------------------    
            do a test build and run the GUI
-----------------------------------------------------------------------
----------------------------------------------------------------------*/

void run_test(void *w_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    XUiDesigner *designer = (XUiDesigner*)w->parent_struct;
    if (w->flags & HAS_POINTER && !adj_get_value(w->adj_y)) {
        int i = 0;
        int j = 0;
        for (;i<MAX_CONTROLS;i++) {
            if (designer->controls[i].wid != NULL) {
                j++;
            }
        }
        if (!j) {
            Widget_t *dia = open_message_dialog(designer->ui, INFO_BOX, _("INFO"),
                                            _("Please create at least one Controller,|or load a LV2 URI to run a test build "),NULL);
            XSetTransientForHint(w->app->dpy, dia->widget, designer->ui->widget);
            return;
        }
        designer->run_test = true;
        char* name = "/tmp/test.c";
        remove (name);
        FILE *fp;
        if((fp=freopen(name, "w" ,stdout))==NULL) {
            fprintf(stderr,"open failed\n");
            return;
        }

        print_list(designer);
        fclose(fp);
        if (system(NULL)) {
            if ((int)adj_get_value(designer->color_chooser->adj)) {
                adj_set_value(designer->color_chooser->adj, 0.0);
            }
            widget_hide(designer->w);
            widget_hide(designer->ui);
            widget_hide(designer->set_project);
            XFlush(designer->w->app->dpy);
            int ret = system("cd /tmp/  && "
                "cc -O2 -D_FORTIFY_SOURCE=2 -Wall -fstack-protector "
                "`pkg-config lilv-0 --cflags` test.c "
                "-o uitest  -fPIC -Wl,-z,noexecstack -Wl,--no-undefined -I./ "
                "-I/usr/share/XUiDesigner/wrapper/libxputty "
                "-Wl,-Bstatic `pkg-config --cflags --libs xputty` "
                "-Wl,-Bdynamic `pkg-config --cflags --libs cairo x11 lilv-0` -lm ");
            if (!ret) {
                ret = system("cd /tmp/  && ./uitest");
            }
            if (!ret) {
                designer->run_test = false;
                widget_show_all(designer->w);
                widget_show_all(designer->ui);
                use_grid(designer->grid, NULL);
                widget_hide(designer->controller_settings);
                widget_hide(designer->combobox_settings);
            } else {
                designer->run_test = false;
                widget_show_all(designer->w);
                widget_show_all(designer->ui);
                use_grid(designer->grid, NULL);
                widget_hide(designer->controller_settings);
                widget_hide(designer->combobox_settings);
                Widget_t *dia = open_message_dialog(designer->ui, INFO_BOX, _("INFO"),
                                                _("Test fail, sorry"),NULL);
                XSetTransientForHint(w->app->dpy, dia->widget, designer->ui->widget);
            }
        }
    }
}
