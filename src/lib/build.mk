include $(topdir)/build/head.mk

lib_srcs := \
	ppkt.c \
	s7comm.c \
	s7comm_debug.c \
	tcp.c  \
	tpkt.c \
	cotp.c \
	err.c

#includes := include
cflags := -fPIC
ldflags := -Wl,--version-script,lib/s7comm.map -Wl,-soname=${LIB_SONAME}

include $(topdir)/build/bottom.mk
