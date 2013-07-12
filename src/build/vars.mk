CC = gcc
LD = gcc
OBJCOPY = objcopy
SIZE = size
NM = nm

topdir := $(shell pwd)/

src-to-obj = $(patsubst %.cpp,%.o,$(patsubst %.c,%.o,$(1)))

LIB_OBJS = $(call src-to-obj,$(LIB_SRCS))
S7GET_OBJS = $(call src-to-obj,$(S7GET_SRCS))
TEST_OBJS = $(call src-to-obj,$(TEST_SRCS))

module = $(<D)
module_includes = $(addprefix -I, $($(_module)_includes))
module_cflags = $($(module)_cflags)
module_ldflags = $($(module)_ldflags)

COMMON_FLAGS = -g -Wall -Werror $(module_includes)

CFLAGS = $(COMMON_FLAGS) -std=gnu99 $(module_cflags)
CPPFLAGS = $(COMMON_FLAGS)
LDFLAGS = $(module_ldflags)
