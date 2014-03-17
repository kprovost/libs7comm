$(_module)_src := $(addprefix $(_module)/,$(srcs))

LIB_SRCS := $(LIB_SRCS) $(addprefix $(_module)/,$(lib_srcs))
S7GET_SRCS := $(S7GET_SRCS) $(addprefix $(_module)/,$(s7get_srcs))
TEST_SRCS := $(TEST_SRCS) $(addprefix $(_module)/,$(test_srcs))
ANALYSIS_SRCS := $(ANALYSIS_SRCS) $(addprefix $(_module)/,$(analysis_srcs))

$(_module)_includes := $(includes)
$(_module)_cflags := $(cflags)
$(_module)_ldflags := $(ldflags)

# Reset variables
lib_srcs :=
s7get_srcs :=
test_srcs :=
analysis_srcs :=
includes :=
cflags :=
ldflags :=
