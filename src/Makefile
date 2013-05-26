
all: pnget/pnget

pnget/pnget: lib/libprofinet.so FORCE
	$(MAKE) -C pnget

lib/libprofinet.so: FORCE
	$(MAKE) -C lib

.PHONY: clean FORCE
clean:
	$(MAKE) -C pnget clean
	$(MAKE) -C lib clean
