include $(topdir)/build/head.mk

lib_srcs := \
	ppkt.c \
	profinet.c \
	profinet_debug.c \
	tcp.c  \
	tpkt.c \
	cotp.c

#includes := include
cflags := -fPIC
ldflags := -lpcap

include $(topdir)/build/bottom.mk
