all: lib/libprofinet.so pnget/pnget test/tests

include build/debug.mk
include build/rules.mk
include build/vars.mk

include lib/build.mk
include pnget/build.mk
include test/build.mk

lib/libprofinet.so: $(LIB_OBJS)

pnget/pnget: $(PNGET_OBJS)

test/tests: $(TEST_OBJS) lib/libprofinet.so

# Dependencies
-include $(addsuffix .d, $(OBJS))
