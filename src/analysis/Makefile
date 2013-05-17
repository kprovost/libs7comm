
all: analyze

CFLAGS = -g -Wall -Werror
LDFLAGS = -lpcap

analyze: analyze.o
	$(CC) $(LDFLAGS) -o $@ $<

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY:
clean:
	rm -Rf analyze analyze.o

