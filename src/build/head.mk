_mymakefile := $(word $(shell expr $(words $(MAKEFILE_LIST)) - 1), $(MAKEFILE_LIST))
_module := $(subst /build.mk,,$(_mymakefile))
