include $(topdir)/build/head.mk

s7get_srcs := \
	s7get.c

includes := include
ldflags := lib/${LIB}
include $(topdir)/build/bottom.mk
