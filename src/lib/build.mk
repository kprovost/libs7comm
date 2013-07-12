include $(topdir)/build/head.mk

lib_srcs := \
	ppkt.c \
	s7comm.c \
	s7comm_debug.c \
	tcp.c  \
	tpkt.c \
	cotp.c

#includes := include
cflags := -fPIC
ldflags := -lpcap

include $(topdir)/build/bottom.mk
