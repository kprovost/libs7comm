all: lib/libs7comm.so s7get/s7get test/tests

include build/debug.mk
include build/rules.mk
include build/vars.mk

include lib/build.mk
include s7get/build.mk
include test/build.mk

lib/libs7comm.so: $(LIB_OBJS)

s7get/s7get: $(S7GET_OBJS)

test/tests: $(TEST_OBJS) lib/libs7comm.so

# Dependencies
-include $(addsuffix .d, $(OBJS))
