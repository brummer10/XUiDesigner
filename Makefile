
include libxputty/Build/Makefile.base

BLUE = "\033[1;34m"
RED =  "\033[1;31m"
NONE = "\033[0m"

SUBDIR := XUiDesigner

.PHONY: $(SUBDIR) libxputty  recurse

$(MAKECMDGOALS) recurse: $(SUBDIR)

check-and-reinit-submodules :
	@if git submodule status 2>/dev/null | grep -E -q '^[-]|^[+]' ; then \
		echo "$(red)INFO: Need to reinitialize git submodules$(reset)"; \
		git submodule update --init; \
		echo "$(blue)Done$(reset)"; \
	else echo "$(blue)Submodule up to date$(reset)"; \
	fi

clean:

libxputty: check-and-reinit-submodules
ifneq ($(MAKECMDGOALS),debug)
	@exec $(MAKE) --no-print-directory -j 1 -C $@ $(MAKECMDGOALS) CFLAGS='-O3 -D_FORTIFY_SOURCE=2 -Wall \
	-fstack-protector -fno-ident -fno-asynchronous-unwind-tables -s -DNDEBUG -Wno-unused-result'
else
	@exec $(MAKE) --no-print-directory -j 1 -C $@ $(MAKECMDGOALS)
endif

$(SUBDIR): libxputty
	@exec $(MAKE) --no-print-directory -j 1 -C $@ $(MAKECMDGOALS)

