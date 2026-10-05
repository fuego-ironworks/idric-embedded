IDRIC ?= idris2
IDRIC_REVISION ?= ef83e1627
WASMTIME ?= wasmtime

DRIVER := build/exec/idric-wasm
MODULE := build/exec/known-integer.wasm
DETERMINISM_A := build/exec/known-integer-a.wasm
DETERMINISM_B := build/exec/known-integer-b.wasm
BACKEND_SOURCES := $(wildcard src/Backend/Wasm/*.idr) backend.ipkg

.PHONY: check-compiler check driver module shape runtime determinism verify clean

check-compiler:
	$(IDRIC) --version > build-compiler-version.txt
	grep -F '$(IDRIC_REVISION)' build-compiler-version.txt

check: check-compiler
	$(IDRIC) --typecheck backend.ipkg

driver: $(DRIVER)

$(DRIVER): $(BACKEND_SOURCES)
	$(IDRIC) --build backend.ipkg

$(MODULE): $(DRIVER) tests/KnownInteger.idric
	IDRIS2_PATH="$(CURDIR)/build/ttc:$${IDRIS2_PATH}" ./$(DRIVER) --cg wasm --source-dir tests tests/KnownInteger.idric -o known-integer

module: $(MODULE)

# This independent Core-format fixture is never copied into the generated output.
shape: $(MODULE)
	cmp $(MODULE) tests/fixtures/known-integer.wasm

runtime: $(MODULE)
	$(WASMTIME) run --invoke idric_answer $(MODULE) > build/exec/known-integer.stdout
	cmp build/exec/known-integer.stdout tests/fixtures/known-integer.stdout

$(DETERMINISM_A): $(DRIVER) tests/KnownInteger.idric
	IDRIS2_PATH="$(CURDIR)/build/ttc:$${IDRIS2_PATH}" ./$(DRIVER) --cg wasm --source-dir tests tests/KnownInteger.idric -o known-integer-a

$(DETERMINISM_B): $(DRIVER) tests/KnownInteger.idric
	IDRIS2_PATH="$(CURDIR)/build/ttc:$${IDRIS2_PATH}" ./$(DRIVER) --cg wasm --source-dir tests tests/KnownInteger.idric -o known-integer-b

determinism: $(DETERMINISM_A) $(DETERMINISM_B)
	cmp $(DETERMINISM_A) $(DETERMINISM_B)

verify: check shape determinism runtime

clean:
	rm -rf build build-compiler-version.txt
