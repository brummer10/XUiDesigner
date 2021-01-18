

	LIB_DIR := ../libxputty/libxputty/
	HEADER_DIR := $(LIB_DIR)include/

	# include path for library headers
	INCLUDES := -I. -I$(HEADER_DIR) 
	# link flags to dynamic link cairo and X11 (default)
	LDFLAGS += -fPIC -Wl,-z,noexecstack -Wl,--no-undefined `pkg-config --cflags --libs cairo x11` -lm
	CXXFLAGS += -O2 -D_FORTIFY_SOURCE=2 -Wall -fstack-protector 
	DEBUG_CXXFLAGS += -g -D DEBUG

	BLUE = "\033[1;34m"
	RED =  "\033[1;31m"
	NONE = "\033[0m"

	XUIDESIGNER_SOURCES := $(wildcard *.c)
	XUIDESIGNER_OBJ := $(patsubst %.c,%.o,$(XUIDESIGNER_SOURCES))
	EXEC_FILES := $(patsubst %.c,%,$(XUIDESIGNER_SOURCES))

.PHONY : all clean

all: $(EXEC_FILES) 

debug: CXXFLAGS = $(DEBUG_CXXFLAGS) 
debug: all

clean:
	rm -f $(XUIDESIGNER_OBJ) $(EXEC_FILES)

$(HEADER_DIR)*.h:
	@echo $(RED)You need to build libxputty before build XFileSelet!!$(NONE)
	@echo $(RED)Just build from the top directory to keep the order$(NONE)
	@exit 1

$(XUIDESIGNER_OBJ): $(XUIDESIGNER_SOURCES) $(HEADER_DIR)*.h
	$(CC) -c $(patsubst %.o,%.c,$@) -o $@  $(INCLUDES) $(LDFLAGS)

$(EXEC_FILES): $(XUIDESIGNER_OBJ)
	$(CC) $(CXXFLAGS) -Wall $(patsubst %,%.c,$@) -L. $(LIB_DIR)libxputty.a -o $@  $(INCLUDES) $(LDFLAGS)


