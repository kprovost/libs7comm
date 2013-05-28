all: libprofinet.so

OBJS =\
	  profinet.o \
	  ppkt.o \
	  pdu.o \
	  iso.o

CFLAGS = -g -Wall -Werror -I../include -fPIC -std=c99
LDFLAGS = -shared

libprofinet.so: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY:
clean:
	rm -Rf *.so *.o
