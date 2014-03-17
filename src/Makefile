LIB_VERSION := 0.1
LIB_NAME := libs7comm.so.${LIB_VERSION}

all: lib/${LIB_NAME} s7get/s7get test/tests analysis/analyze

include build/debug.mk
include build/rules.mk
include build/vars.mk

include lib/build.mk
include s7get/build.mk
include analysis/build.mk
include test/build.mk

lib/${LIB_NAME}: $(LIB_OBJS)

s7get/s7get: $(S7GET_OBJS) lib/${LIB_NAME}

test/tests: $(TEST_OBJS) $(LIB_OBJS)

analysis/analyze: $(ANALYSIS_OBJS) $(LIB_OBJS)

# Dependencies
-include $(addsuffix .d, $(OBJS))
