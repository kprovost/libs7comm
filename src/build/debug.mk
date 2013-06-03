
print-%:
	@echo $* = $($*)

.PHONY: printvars printvars-short
printvars:
	@$(foreach V,$(sort $(.VARIABLES)), $(warning $V=$($V)))

printvars-short:
	@$(foreach V,$(sort $(.VARIABLES)), \
	$(if $(filter-out environment% default automatic, \
	$(origin $V)),$(warning $V=$($V) ($(value $V)))))


