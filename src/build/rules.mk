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

%.so.${LIB_VERSION}:
	@echo [LD] $(@F)
	$(Q)$(LD) -shared \
		$(LDFLAGS) \
		-o $(@) \
		$^

s7get/s7get test/tests analysis/analyze:
	@echo [LD] $(@F)
	$(Q)$(CXX) \
		-o $(@) \
		$^ \
		$(LDFLAGS)

.PHONY: test

test: test/tests
	./test/tests -v

.PHONY: clean

clean:
	rm -Rf lib/*.o lib/*.o.d lib/*.so.*
	rm -Rf s7get/*.o s7get/*.o.d s7get/s7get
	rm -Rf test/*.o test/*.o.d test/tests
	rm -Rf analysis/*.o analysis/*.o.d analysis/analyze
	rm -f cpputest*.xml
