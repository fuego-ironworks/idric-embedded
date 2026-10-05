# idric-embedded — WebAssembly

This branch is the WebAssembly architecture follower for Idriç. `ARCHITECTURE.md`
pins WebAssembly Core 3.0 and keeps the complete architecture inventory separate
from the first executable backend slice.

## First executable slice

The first implementation deliberately accepts only one shape:

```idris
%export "wasm:idric_answer"
idric_answer : Int32
idric_answer = 42
```

The real `.idric` source goes through the pinned Idriç frontend and
`Compiler.ANF`. The Wasm backend then accepts exactly one exported nullary ANF
function whose reachable body is one `Int32` literal and emits a binary module
containing only:

- one function type: `() -> i32`
- one function at type index 0
- one function export
- one code body: no locals, `i32.const`, `end`

The binary therefore contains exactly section IDs `1, 3, 7, 10`: type,
function, export, and code. It has no import, table, memory, global, start,
element, data, tag, or custom section.

WebAssembly Core 3.0 still uses binary-format version 1 in the module preamble;
`\0asm 01 00 00 00` is therefore the current Core binary format, not a claim
that this backend targets only Wasm 1.0.

## Verification

The hosted gate invokes the existing Make targets with the exact Idriç compiler and the verified Wasmtime executable.

The gate pins Idriç commit
`ef83e1627e0a8b84567ec1f461d3a85c26580019` and Wasmtime CLI `48.0.0`.
The release archive is SHA-256 checked before extraction. It:

1. bootstraps and tests that compiler, then typechecks and builds the backend;
2. compiles `tests/KnownInteger.idric` through its real frontend and ANF;
3. compares the generated module with an independently specified 45-byte Core
   binary: zero imports, section IDs `1, 3, 7, 10`, exactly one nullary
   `i32` function/export and the body `i32.const 42; end`;
4. asks the pinned Wasmtime executable to validate, instantiate, and invoke
   `idric_answer`, requiring stdout `42\n`;
5. recompiles twice and requires identical output bytes;
6. first typechecks a one-argument source with Idriç, then requires the Wasm
   backend's explicit reachable-program rejection and absence of an artifact.

The independent binary is an oracle only; no target copies it into the
candidate output. This is not a general WebAssembly binary inspector. The CLI
output protocol is pinned to this Wasmtime release, not assumed stable across
future versions. No first-party Python verifier or Python Wasmtime wrapper is
introduced.

Wasmtime is only the pinned independent validation/execution engine. The
generated module imports nothing and does not depend on JavaScript, browser
APIs, WASI, or any operating-system interface.

## Deliberate boundary

This PR proves only the first oracle from issue #7. It does **not** implement
linear memory, integer arithmetic beyond the literal, locals, comparisons,
structured control flow, direct calls, SIMD, GC/reference objects, exceptions,
tail calls, multiple memories, `memory64`, browser APIs, or WASI.

The next slice should remain inside the issue's initial Core surface and add the
memory store/load round trip before branch and direct-call fixtures.
