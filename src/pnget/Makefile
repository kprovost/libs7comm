all: pnget

CFLAGS = -g -Wall -Werror -I../include
LDFLAGS = -L../lib -lprofinet

pnget: pnget.o
	$(CC) $(LDFLAGS) -o $@ $<

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY:
clean:
	rm -Rf *.so *.o
