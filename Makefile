# Direct build-tool invocations at make's fixed interface.
ICK ?= ick
ICK_LINK_FLAGS ?= -fno-link-libatomic
BUILD ?= _/build
CFLAGS ?= -O2
WARN = -std=c17 -Wall -Wextra -Werror -Wpedantic -Wshadow
SOURCE_ROOT = $(abspath $(dir $(lastword $(MAKEFILE_LIST))))
CODEC = $(SOURCE_ROOT)/shared/compact-unit-direction
TESTS = $(SOURCE_ROOT)/_/tests

.PHONY: test
test: $(BUILD)/compact-direction $(BUILD)/direction-equivalence
	$(BUILD)/compact-direction
	$(BUILD)/direction-equivalence
$(BUILD):
	mkdir -p "$@"
$(BUILD)/compact-direction: $(CODEC)/compact_unit_direction_test.c $(CODEC)/compact_unit_direction.h | $(BUILD)
	$(ICK) $(ICK_LINK_FLAGS) $(WARN) $(CFLAGS) -I"$(CODEC)" $< -lm -o $@
$(BUILD)/reference.o: $(TESTS)/reference_direction.c $(TESTS)/reference/compact_unit_direction.h | $(BUILD)
	$(ICK) $(WARN) $(CFLAGS) -c $< -o $@
$(BUILD)/direction-equivalence: $(TESTS)/direction_equivalence.c $(CODEC)/compact_unit_direction.h $(BUILD)/reference.o | $(BUILD)
	$(ICK) $(ICK_LINK_FLAGS) $(WARN) $(CFLAGS) -I"$(CODEC)" $< $(BUILD)/reference.o -lm -o $@
