LIB_RELEASE := 1
LIB_API_VERSION := 0.0
LIB_VERSION := ${LIB_API_VERSION}.${LIB_RELEASE}
LIB_NAME := libs7comm.so
LIB_SONAME := ${LIB_NAME}.${LIB_API_VERSION}
LIB := ${LIB_SONAME}.${LIB_RELEASE}

all: core test/tests analysis/analyze

.PHONY: all core

core: lib/${LIB} s7get/s7get

include build/debug.mk
include build/rules.mk
include build/vars.mk

include lib/build.mk
include s7get/build.mk
include analysis/build.mk
include test/build.mk

lib/${LIB}: $(LIB_OBJS)

s7get/s7get: $(S7GET_OBJS) lib/${LIB_SONAME}

test/tests: $(TEST_OBJS) $(LIB_OBJS)

analysis/analyze: $(ANALYSIS_OBJS) $(LIB_OBJS)

# Dependencies
-include $(addsuffix .d, $(OBJS))
