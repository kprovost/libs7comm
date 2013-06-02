
all: analyze

CFLAGS = -g -Wall -Werror -I../include -I../lib
LDFLAGS = -lpcap -L../lib -lprofinet

analyze: analyze.o
	$(CC) $(LDFLAGS) -o $@ $<

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY:
clean:
	rm -Rf analyze analyze.o

