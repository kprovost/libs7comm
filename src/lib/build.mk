include $(topdir)/build/head.mk

lib_srcs := \
	iso.c \
	pcap.c \
	pdu.c \
	ppkt.c \
	profinet.c \
	tcp.c  \
	tpkt.c \
	cotp.c

#includes := include
cflags := -fPIC
ldflags := -lpcap

include $(topdir)/build/bottom.mk
