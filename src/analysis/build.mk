include $(topdir)/build/head.mk

analysis_srcs := \
	analyze.c

includes := include lib
ldflags := -lpcap

include $(topdir)/build/bottom.mk
