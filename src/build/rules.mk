# Don't delete intermediate files
.SECONDARY:

ifeq ($(V),1)
Q=
else
Q = @
endif

%.o: %.c
	@echo [CC] $(@F)
	$(Q)$(CC) -MD -MP -MF $(addsuffix .d, $@) \
		-c $(CFLAGS) \
		$< \
		-o $@

%.o: %.cpp
	@echo [CXX] $(@F)
	$(Q)$(CXX) -MD -MP -MF $(addsuffix .d, $@) \
		-c $(CPPFLAGS) \
		$< \
		-o $@

%.so:
	@echo [LD] $(@F)
	$(Q)$(LD) -shared \
		$(LDFLAGS) \
		-o $(@) \
		$^

pnget/pnget analysis/analyze test/tests:
	@echo [LD] $(@F)
	$(Q)$(CXX) \
		-o $(@) \
		$^ \
		lib/libprofinet.so \
		$(LDFLAGS)

.PHONY: test

test: test/tests
	./test/tests -v

.PHONY: clean

clean:
	rm -Rf lib/*.o lib/*.o.d lib/*.so
	rm -Rf analysis/*.o analysis/*.o.d analysis/analyze
	rm -Rf pnget/*.o pnget/*.o.d pnget/pnget
	rm -Rf test/*.o test/*.o.d test/tests
	rm -f cpputest*.xml
