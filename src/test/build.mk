include $(topdir)/build/head.mk

test_srcs := \
	main.cpp \
	ppkt.cpp

includes := include lib
ldflags := -lCppUTest -lCppUTestExt

include $(topdir)/build/bottom.mk
