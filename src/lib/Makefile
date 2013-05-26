all: libprofinet.so

CFLAGS = -g -Wall -Werror -I../include -fPIC
LDFLAGS = -shared

libprofinet.so: profinet.o
	$(CC) $(LDFLAGS) -o $@ $<

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY:
clean:
	rm -Rf *.so *.o
