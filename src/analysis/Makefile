
all: analyze

CFLAGS = -g -Wall -Werror -I../lib
LDFLAGS = -lpcap

analyze: analyze.o
	$(CC) $(LDFLAGS) -o $@ $<

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY:
clean:
	rm -Rf analyze analyze.o

