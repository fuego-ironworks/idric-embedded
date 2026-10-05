# First Wasm oracle repair

Purpose: retire August PR #9 by executing the first real Idriç-to-Wasm module
without widening the backend. This example uses the current pinned compiler,
not a hand-authored binary as a fallback.

Type-system sketch: the source exports a nullary Int32 value. The backend admits
only ANF Make_Administrative_Normal_Form_Function []
(Administrative_Normal_Form_Primitive_Value _ (I32 value)) and an ASCII export name. Its artifact
ABI is () -> i32 with zero imports. The one-argument Int32 fixture is well typed
source but must be rejected by this backend, leaving no artifact.

Observed repair history: Lower lacked Core.TT's I32 constructor. Adding the
import exposed an inference failure in the monadic tuple pattern used to select
exports. A typed selectSingleExport now keeps zero/multiple exports fail closed.
Fresh current-compiler testing then exposed the old ANF constructor names; the
backend now uses the current Administrative_Normal_Form_Definition vocabulary
and constructors directly, retaining the same accepted/rejected shapes.

The original new Python shape/runtime verifiers were removed under current
repository policy. An independent 45-byte Core-format fixture and cmp check
the entire positive artifact. Digest-pinned Wasmtime CLI is the foreign
validation/execution oracle. No alternative compiler/backend generates the
candidate. Hosted results qualify only these positive and negative fixtures,
not broader Wasm support or physical-device acceptance.

Sources: tests/KnownInteger.idric and tests/UnsupportedFunction.idric.
Resulting language work: none asserted; backend integration bugs remain visible
in the same PR and fresh checks decide whether the repair is accepted.
