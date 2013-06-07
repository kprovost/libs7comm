all: lib/libprofinet.so pnget/pnget analysis/analyze test/tests

include build/debug.mk
include build/rules.mk
include build/vars.mk

include lib/build.mk
include pnget/build.mk
include analysis/build.mk
include test/build.mk

lib/libprofinet.so: $(LIB_OBJS)

pnget/pnget: $(PNGET_OBJS)

analysis/analyze: $(ANALYSIS_OBJS)

test/tests: $(TEST_OBJS)
