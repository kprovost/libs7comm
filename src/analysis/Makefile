
all: analyze

CFLAGS = -Wall -Werror
LDFLAGS = -lpcap

analyze: analyze.o
	$(CC) $(LDFLAGS) -o $@ $<

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

