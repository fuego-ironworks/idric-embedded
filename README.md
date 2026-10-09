# idric-embedded

Experimental Idriç backend work for embedded processor families.

Architecture branches are intentionally separated so ISA and ABI research can proceed independently before any shared backend design is assumed.

The shared [compact unit direction codec](shared/compact-unit-direction/README.md)
now composes Icky C chart and storage values. Its native test selects actual ICK
explicitly; each embedded branch keeps its separate acceptance boundary.
