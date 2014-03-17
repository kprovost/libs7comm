include $(topdir)/build/head.mk

analysis_srcs := \
	analyze.c

includes := include lib
ldflags := -lpcap -Llib lib/libs7comm.so.0.1

include $(topdir)/build/bottom.mk
