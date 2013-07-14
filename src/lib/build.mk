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
ldflags := -Wl,--version-script,lib/s7comm.map -Wl,-soname=libs7comm.so.0

include $(topdir)/build/bottom.mk
