include $(topdir)/build/head.mk

analysis_srcs := \
	analyze.c \
	pcap.c

includes := include lib
ldflags := -lpcap

include $(topdir)/build/bottom.mk
