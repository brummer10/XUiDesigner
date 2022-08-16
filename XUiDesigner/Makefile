
	# check CPU and supported optimization flags
	ifeq ($(UNAME), Linux)
		ifneq ($(shell cat /proc/cpuinfo | grep sse3 ) , )
			SSE_CFLAGS = -msse3 -mfpmath=sse
		else ifneq ($(shell cat /proc/cpuinfo | grep sse2 ) , )
			SSE_CFLAGS = -msse2 -mfpmath=sse
		else ifneq ($(shell cat /proc/cpuinfo | grep sse ) , )
			SSE_CFLAGS = -msse -mfpmath=sse
			else ifneq ($(shell cat /proc/cpuinfo | grep ARM ) , )
			ifneq ($(shell cat /proc/cpuinfo | grep ARMv7 ) , )
				ifneq ($(shell cat /proc/cpuinfo | grep vfpd32 ) , )
					SSE_CFLAGS = -march=armv7-a -mfpu=vfpv3 
				else ifneq ($(shell cat /proc/cpuinfo | grep vfpv3 ) , )
					SSE_CFLAGS = -march=armv7-a -mfpu=vfpv3
				endif
			else
				ARMCPU = "YES"
			endif
		else
			SSE_CFLAGS =
		endif
	endif

	# set bundle name
	NAME = XUiDesigner
	EXEC_NAME  = $(shell echo $(NAME) | tr A-Z a-z)
	BUILD_DIR = build
	VER = 0.5

	LIB_DIR := ../libxputty/libxputty/
	HEADER_DIR := $(LIB_DIR)include/
	LOCALIZE_DIR := ../libxputty/xputty/dialogs/
	PREFIX ?= /usr
	BIN_DIR ?= $(PREFIX)/bin
	SHARE_DIR ?= $(PREFIX)/share
	DESKAPPS_DIR ?= $(SHARE_DIR)/applications
	PIXMAPS_DIR ?= $(SHARE_DIR)/pixmaps
	MAN_DIR ?= $(SHARE_DIR)/man/man1
	LOCAL_DIR ?= $(SHARE_DIR)/locale
	SOURCE_DIR := ./src/

	# set compile flags
	LDFLAGS += -fPIC -Wl,-z,noexecstack -Wl,--no-undefined -I./include/ -I../libxputty/libxputty/include/ \
	`pkg-config --cflags --libs cairo x11 lilv-0` -lm
	CXXFLAGS += -O3 -D_FORTIFY_SOURCE=2 -Wall -Wextra -fstack-protector -fno-ident \
	-fno-asynchronous-unwind-tables -s -DNDEBUG -DSHARE_DIR=\"$(SHARE_DIR)\"
	# -pedantic -Wextra -Wshadow -Wpointer-arith -Wcast-qual -Wstrict-prototypes -Wmissing-prototypes
	DEBUG_CXXFLAGS += -g -D DEBUG -DSHARE_DIR=\"$(SHARE_DIR)\"
	# invoke build files
	OBJECTS = $(wildcard $(SOURCE_DIR)*.c)
	LOCALIZE = $(LOCALIZE_DIR)xfile-dialog.c $(LOCALIZE_DIR)xmessage-dialog.c $(LOCALIZE_DIR)xsavefile-dialoge.c
	## output style (bash colours)
	BLUE = "\033[1;34m"
	RED =  "\033[1;31m"
	NONE = "\033[0m"

.PHONY : $(HEADER_DIR)*.h all debug nls gettext updatepot po clean install uninstall 

all : check $(NAME)
	@cd ../libxputty/Build && $(MAKE) shared
	@mkdir -p ./$(BUILD_DIR)
	@mv ./$(EXEC_NAME) ./$(BUILD_DIR)
	@#if [ -f ./$(BUILD_DIR)/$(EXEC_NAME) ]; then echo $(BLUE)"build finish, now run make install"; \
	#else echo $(RED)"sorry, build failed"; fi
	@#echo $(NONE)

debug: CXXFLAGS = $(DEBUG_CXXFLAGS) 
debug: all

nls: LDFLAGS += -DENABLE_NLS -DGETTEXT_PACKAGE=\"$(EXEC_NAME)\" -DLOCAL_DIR=\"$(LOCAL_DIR)\"
nls: gettext all 

    #@localisation
MSGLANGS=$(notdir $(wildcard po/*po))
MSGOBJS=$(addprefix locale/,$(MSGLANGS:.po=/LC_MESSAGES/$(EXEC_NAME).mo))
LANGS=$(addprefix locale/,$(MSGLANGS:.po=/LC_MESSAGES/))
LANGW=$(words  $(MSGLANGS))

gettext: $(MSGOBJS)

locale/%/LC_MESSAGES/$(EXEC_NAME).mo: po/%.po
	mkdir -p $(dir $@)
	msgfmt -c -o $@ po/$*.po

updatepot:
	@mkdir -p ./po/
	xgettext --keyword=_ --language=C++ --add-comments --sort-output --package-name=$(EXEC_NAME) --package-version=$(VER) -o po/$(EXEC_NAME).pot $(OBJECTS) $(LOCALIZE)
	for POFILE in $(MSGLANGS) ; do msgmerge --update po/$$POFILE po/$(EXEC_NAME).pot ; done

po:
	msginit --input=po/$(EXEC_NAME).pot --locale=$(LANG) --output=po/$(basename $(LANG)).po
	echo $(LANG)

check :
ifdef ARMCPU
	@echo $(RED)ARM CPU DEDECTED, please check the optimization flags
	@echo $(NONE)
endif

clean :
	@rm -f ./$(BUILD_DIR)/$(EXEC_NAME)
	@rm -rf ./$(BUILD_DIR)
	@echo ". ." $(BLUE)", clean up"$(NONE)

install :
ifneq ("$(wildcard ./$(BUILD_DIR))","")
	@cd ../libxputty/Build && $(MAKE) install
	@mkdir -p $(DESTDIR)$(BIN_DIR)
	@cp ./$(BUILD_DIR)/$(EXEC_NAME) $(DESTDIR)$(BIN_DIR)/$(EXEC_NAME)
	@cp ../tools/dsp2cc $(DESTDIR)$(BIN_DIR)/dsp2cc
	@mkdir -p $(DESTDIR)$(DESKAPPS_DIR)
	@cp $(NAME).desktop $(DESTDIR)$(DESKAPPS_DIR)
	@mkdir -p $(DESTDIR)$(PIXMAPS_DIR)
	@cp ../$(NAME).svg $(DESTDIR)$(PIXMAPS_DIR)
	@mkdir -p $(DESTDIR)$(SHARE_DIR)/XUiDesigner/wrapper
	@cp -r ../Bundle/wrapper/* $(DESTDIR)$(SHARE_DIR)/XUiDesigner/wrapper
	@if [ ${LANGW} -gt 1 ]; then \
		for lang in $(LANGS) ; \
		do \
			if [ -f $$lang/$(EXEC_NAME).mo ]; then \
				mkdir -p $(DESTDIR)$(SHARE_DIR)/$$lang; \
				cp $$lang/$(EXEC_NAME).mo $(DESTDIR)$(SHARE_DIR)/$$lang; \
			fi \
		done \
	elif [ ${LANGW} -gt 0 ]; then \
			if [ -f $(LANGS)/$(EXEC_NAME).mo ]; then \
				mkdir -p $(DESTDIR)$(SHARE_DIR)/$(LANGS); \
				cp $(LANGS)/$(EXEC_NAME).mo $(DESTDIR)$(SHARE_DIR)/$(LANGS); \
			fi \
	fi
	@update-desktop-database || true
	@echo ". ." $(BLUE)", done"$(NONE)
else
	@echo ". ." $(BLUE)", you must build first"$(NONE)
endif

uninstall :
	@rm -rf $(DESTDIR)$(BIN_DIR)/$(EXEC_NAME)
	@rm -rf $(DESTDIR)$(BIN_DIR)/dsp2cc
	@rm -rf $(DESTDIR)$(DESKAPPS_DIR)/$(NAME).desktop
	@rm -rf $(DESTDIR)$(PIXMAPS_DIR)/$(NAME).svg
	@rm -rf $(DESTDIR)$(SHARE_DIR)/XUiDesigner
	@rm -rf $(addprefix $(SHARE_DIR)/, $(MSGOBJS))
	@echo ". ." $(BLUE)", done"$(NONE)

$(NAME) :
	$(CC) $(CXXFLAGS) $(OBJECTS) -L. ../libxputty/libxputty/libxputty.a -o $(EXEC_NAME) $(LDFLAGS)

doc:
	#pass
