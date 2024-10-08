
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
	BUILD_DIR = build/
	VER = 1.1

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
	`pkg-config --cflags --libs cairo x11 lilv-0` -lm -lpthread
	MY_CFLAGS += -O3 $(CFLAGS) -D_FORTIFY_SOURCE=2 -Wall -Wextra -fstack-protector -fno-ident -Wno-unused-result \
	-fno-asynchronous-unwind-tables -s -DNDEBUG -DSHARE_DIR=\"$(SHARE_DIR)\"
	# -pedantic -Wextra -Wshadow -Wpadded -Wpointer-arith -Wcast-qual -Wstrict-prototypes -Wmissing-prototypes
	DEBUG_MY_CFLAGS += $(CFLAGS) -g -DDEBUG -DSHARE_DIR=\"$(SHARE_DIR)\"
	# invoke build files
	SOURCE = $(wildcard $(SOURCE_DIR)*.c)
	OBJECTS := $(addprefix $(BUILD_DIR),$(notdir $(patsubst %.c,%.o,$(SOURCE))))
	LOCALIZE = $(LOCALIZE_DIR)xfile-dialog.c $(LOCALIZE_DIR)xmessage-dialog.c $(LOCALIZE_DIR)xsavefile-dialoge.c
	## output style (bash colours)
	BLUE = "\033[1;34m"
	RED =  "\033[1;31m"
	NONE = "\033[0m"

.PHONY : $(HEADER_DIR)*.h all debug nls gettext updatepot po clean install uninstall 

all : check $(NAME)
	@cd ../libxputty/Build && $(MAKE) shared
	@mv ./$(EXEC_NAME) ./$(BUILD_DIR)
	@if [ -f ./$(BUILD_DIR)$(EXEC_NAME) ]; then \
		$(B_ECHO) "=================== DONE =======================$(reset)"; \
	else \
		$(R_ECHO)"sorry, build failed$(reset)"; \
	fi 

debug: MY_CFLAGS = $(DEBUG_MY_CFLAGS) 
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
	$(QUIET)mkdir -p $(dir $@)
	$(QUIET)msgfmt -c -o $@ po/$*.po

updatepot:
	$(QUIET)mkdir -p ./po/
	$(QUIET)xgettext --keyword=_ --language=C++ --add-comments --sort-output --package-name=$(EXEC_NAME) --package-version=$(VER) -o po/$(EXEC_NAME).pot $(SOURCE) $(LOCALIZE)
	$(QUIET)for POFILE in $(MSGLANGS) ; do msgmerge --update po/$$POFILE po/$(EXEC_NAME).pot ; done

po:
	$(QUIET)msginit --input=po/$(EXEC_NAME).pot --locale=$(LANG) --output=po/$(basename $(LANG)).po
	@$(ECHO) "$(LANG)$(reset)"

check :
	@mkdir -p ./$(BUILD_DIR)
ifdef ARMCPU
	@$(R_ECHO) "ARM CPU DEDECTED, please check the optimization flags$(reset)"
endif

clean :
	$(QUIET)rm -f ./$(BUILD_DIR)$(EXEC_NAME)
	$(QUIET)rm -f ./$(OBJECTS)
	$(QUIET)rm -rf ./$(BUILD_DIR)
	@$(ECHO) "clean up$(reset)"

install :
ifneq ("$(wildcard ./$(BUILD_DIR))","")
	$(QUIET)cd ../libxputty/Build && $(MAKE) install
	@$(B_ECHO) "Install XUiDesigner$(reset)"
	$(QUIET)mkdir -p $(DESTDIR)$(BIN_DIR)
	$(QUIET)cp ./$(BUILD_DIR)$(EXEC_NAME) $(DESTDIR)$(BIN_DIR)/$(EXEC_NAME)
	$(QUIET)install -m 0755 ../tools/dsp2cc $(DESTDIR)$(BIN_DIR)/dsp2cc
	$(QUIET)mkdir -p $(DESTDIR)$(DESKAPPS_DIR)
	$(QUIET)cp $(NAME).desktop $(DESTDIR)$(DESKAPPS_DIR)
	$(QUIET)mkdir -p $(DESTDIR)$(PIXMAPS_DIR)
	$(QUIET)cp ../$(NAME).svg $(DESTDIR)$(PIXMAPS_DIR)
	$(QUIET)mkdir -p $(DESTDIR)$(SHARE_DIR)/XUiDesigner/wrapper
	$(QUIET)cp -r ../Bundle/wrapper/* $(DESTDIR)$(SHARE_DIR)/XUiDesigner/wrapper
	$(QUIET)if [ ${LANGW} -gt 1 ]; then \
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
	$(QUIET)update-desktop-database || true
	@$(B_ECHO) "=================== DONE =======================$(reset)"
else
	@$(R_ECHO) ". . , you must build first$(reset)"
endif

uninstall :
	$(QUIET)rm -rf $(DESTDIR)$(BIN_DIR)/$(EXEC_NAME)
	$(QUIET)rm -rf $(DESTDIR)$(BIN_DIR)/dsp2cc
	$(QUIET)rm -rf $(DESTDIR)$(DESKAPPS_DIR)/$(NAME).desktop
	$(QUIET)rm -rf $(DESTDIR)$(PIXMAPS_DIR)/$(NAME).svg
	$(QUIET)rm -rf $(DESTDIR)$(SHARE_DIR)/XUiDesigner
	$(QUIET)rm -rf $(addprefix $(SHARE_DIR)/, $(MSGOBJS))
	@$(B_ECHO) ". . , done$(reset)"

$(OBJECTS) : $(SOURCE)
	@$(ECHO) "Building object file $@ $(reset)"
	$(QUIET)$(CC) $(MY_CFLAGS) -Wall -c $(addprefix $(SOURCE_DIR),$(notdir $(patsubst %.o,%.c,$@))) -o $@ $(LDFLAGS)

$(NAME) : $(OBJECTS)
	@$(B_ECHO) "Compile $(EXEC_NAME) $(reset)"
	$(QUIET)$(CC) $(MY_CFLAGS) $(OBJECTS) -L. ../libxputty/libxputty/libxputty.a -o $(EXEC_NAME) $(LDFLAGS)

doc:
	#pass
