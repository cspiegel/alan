# Generic make rules for alan compiler
# Set:
#	COMPILER : which command to run the C compiler
#	LINKER : which command to run the linker
#	OS_FLAGS : what flags must be passed to both compiler and linker
#	INCLUDES : directives to include the required directories
#	EXTRA_COMPILER_FLAGS : what extra flags to pass to the compiler
#	EXTRA_LINKER_FLAGS : what extra flags to pass to the linker

CC = $(COMPILER)
CFLAGS = $(INCLUDES) -I../interpreter $(OS_FLAGS) $(EXTRA_COMPILER_FLAGS)

LINK = $(LINKER)
LINKFLAGS = $(OS_FLAGS) $(EXTRA_LINKER_FLAGS)

#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#
# Main target to do everything
#
.PHONY: all
all: unit build test

# Target to just build
.PHONY: build
build: alan

#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#
# Run all tests!
#
ifneq ($(EMACS),)
JREGROUTPUT = -noansi
endif
.PHONY: test
test:
	../bin/jregr -bin ../bin -dir testing $(JREGROUTPUT)
	../bin/jregr -bin ../bin -dir testing/positions $(JREGROUTPUT)

# Clean
.PHONY: clean
clean:
	-rm *.o .*/*.o .*/*.d

#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#
# Alan command line compiler
#
ALANOBJDIR = .alan
ALANOBJECTS = $(addprefix $(ALANOBJDIR)/,${ALANSRCS:.c=.o}) $(ALANOBJDIR)/alan.version.o
-include $(ALANOBJECTS:.o=.d)
$(ALANOBJECTS): $(ALANOBJDIR)/%.o: %.c
	$(CC) $(CFLAGS) -MMD -o $@ -c $<

$(ALANOBJDIR):
	@mkdir $(ALANOBJDIR)

alan: $(ALANOBJDIR) $(ALANOBJECTS)
	$(LINK) -o alan $(ALANOBJECTS) $(LINKFLAGS)
	-@if ! test -d ../bin; then mkdir ../bin 2> /dev/null ; fi
	cp alan ../bin/alan


#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#
# Unit testing
#
.PHONY: unit
unit: COMPILER = gcc
unit: LINKER = gcc
unit: unittests
	@./unittests $(UNITOUTPUT)

UNITTESTSOBJDIR = .unittests
UNITTESTSOBJECTS = $(addprefix $(UNITTESTSOBJDIR)/,${UNITTESTSSRCS:.c=.o}) $(UNITTESTSOBJDIR)/alan.version.o
UNITTESTSDLLOBJECTS = $(addprefix $(UNITTESTSOBJDIR)/,${UNITTESTSDLLSRCS:.c=.o}) $(UNITTESTSOBJDIR)/alan.version.o
-include $(UNITTESTSOBJECTS:.o=.d)
$(UNITTESTSOBJECTS): $(UNITTESTSOBJDIR)/%.o: %.c
	$(CC) $(CFLAGS) -MMD -o $@ -c $<

$(UNITTESTSOBJDIR):
	@mkdir $(UNITTESTSOBJDIR)

unittests: CFLAGS += $(CGREENINCLUDE)
unittests: LIBS = $(CGREENLIB) $(ALLOCLIBS)
unittests: $(UNITTESTSOBJDIR) $(UNITTESTSOBJECTS)
	$(LINK) -o unittests $(UNITTESTSOBJECTS) $(LINKFLAGS) $(LIBS)
	$(LINK) -shared -o unittests.dll $(UNITTESTSDLLOBJECTS) $(LINKFLAGS) $(LIBS)

