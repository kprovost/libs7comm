CC = gcc
LD = gcc
OBJCOPY = objcopy
SIZE = size
NM = nm

topdir := $(shell pwd)/

src-to-obj = $(patsubst %.cpp,%.o,$(patsubst %.c,%.o,$(1)))

LIB_OBJS = $(call src-to-obj,$(LIB_SRCS))
PNGET_OBJS = $(call src-to-obj,$(PNGET_SRCS))
ANALYSIS_OBJS = $(call src-to-obj,$(ANALYSIS_SRCS))
TEST_OBJS = $(call src-to-obj,$(TEST_SRCS))

module = $(<D)
module_includes = $(addprefix -I, $($(_module)_includes))
module_cflags = $($(module)_cflags)
module_ldflags = $($(module)_ldflags)

CFLAGS = -g -Wall -Werror \
		 -std=gnu99 \
		 $(module_includes) \
		 $(module_cflags)

CXXFLAGS = -g -Wall -Werror \
		 $(module_includes)

LDFLAGS = $(module_ldflags)
