all: libprofinet.so

OBJS =\
	  profinet.o \
	  ppkt.o \
	  pdu.o \
	  iso.o \
	  tcp.o \
	  tpkt.o \
	  pcap.o

CFLAGS = -g -Wall -Werror -I../include -fPIC -std=gnu99
LDFLAGS = -shared -lpcap

libprofinet.so: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY:
clean:
	rm -Rf *.so *.o
