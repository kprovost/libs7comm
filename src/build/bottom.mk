$(_module)_src := $(addprefix $(_module)/,$(srcs))

LIB_SRCS := $(LIB_SRCS) $(addprefix $(_module)/,$(lib_srcs))
PNGET_SRCS := $(PNGET_SRCS) $(addprefix $(_module)/,$(pnget_srcs))
TEST_SRCS := $(TEST_SRCS) $(addprefix $(_module)/,$(test_srcs))

$(_module)_includes := $(includes)
$(_module)_cflags := $(cflags)
$(_module)_ldflags := $(ldflags)

# Reset variables
lib_srcs :=
pnget_srcs :=
test_srcs :=
includes :=
cflags :=
ldflags :=
