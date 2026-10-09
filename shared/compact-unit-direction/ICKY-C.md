# Functorial Icky C direction codec

The maintained inline model and tests use ← assignment, × multiplication, and ÷ division.
Pointer stars and the three-byte layout stay intact. Geometric chart
coordinates and signed storage codes have distinct types. Encode composes
direction → octahedral chart → signed Q0.11 codes → three bytes. Decode
composes bytes → codes → lifted chart → unit direction. Reflection and each
stage return values; the existing pointer APIs own the output writes.

Keep the six principal-axis encodings, finite/nonzero policy, overflow-safe
scaling, Q0.11 rounding and clamp, negative-hemisphere seam, sign extension,
and untouched output on rejected input. Value-level encode/project operations
have the same nonzero-finite precondition as the existing unchecked helper;
the status API validates it first.

`make test ICK=/absolute/path/to/ick` replaces the stock-C POSIX test launcher.
This fixed Makefile interface compiles every maintained test and the frozen
comparison control with actual ICK. The native producer pins ICK
c61e448251744a2f40ad743ebef1a027bdcd2f9d, declares prebuilt GCC 13 startup/libgcc
and glibc/libm, and uses host GCC only to bootstrap the compiler.

The inherited independent axis/rejection/131,072-point sphere checks remain.
The additional test compares all 16,777,216 storage values bitwise with the
original decoder, checks each result is finite and unit length, and preserves
pack/unpack bytes. Encode compares 131,072 sampled binary32 bit-pattern triples
and all 2,197 triples from signed zero, subnormal, minimum/maximum finite,
infinite, and NaN inputs, plus independent rounding/sign-extension cases.
Assertions must remain enabled. The assignment guard is policy evidence;
these native executions establish the numerical composition boundary.

The frozen header is exactly the original default-branch source at
a526dbe18bc545a9c53319749e8344cec23490bd. Other repositories retain their own
copies; this conversion preserves the byte representation and is no longer a
claim of identical source text. No platform branch, embedded backend,
accelerometer consumer, firmware package, or physical sensor is qualified by
the native host result.

The 2026-10-09 division migration changes seventeen binary divisions across
the maintained header and tests. A forced rebuild with the current ICK
frontend passed every storage-value, sampled-input, extreme-input, and
independent sphere check above. The frozen comparison header is unchanged.
