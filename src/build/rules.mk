# Don't delete intermediate files
.SECONDARY:

# Dependencies
-include $(addsuffix .d,$(OBJS))


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

%.so:
	@echo [LD] $(@F)
	$(Q)$(LD) -shared \
		$(LDFLAGS) \
		-o $(@) \
		$^

pnget/pnget analysis/analyze:
	@echo [LD] $(@F)
	$(Q)$(LD) \
		$(LDFLAGS) \
		-o $(@) \
		$^ \
		lib/libprofinet.so

.PHONY: clean

clean:
	rm -Rf lib/*.o lib/*.o.d lib/*.so
	rm -Rf analysis/*.o analysis/*.o.d analysis/analyze
	rm -Rf pnget/*.o pnget/*.o.d pnget/pnget
