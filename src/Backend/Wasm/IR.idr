module Backend.Wasm.IR

%default total

||| The deliberately tiny first Wasm IR: one exported nullary function that
||| returns one i32 constant.  This is an executable acceptance seam, not the
||| complete WebAssembly architecture inventory.
public export
record ExportedI32Constant where
  constructor MkExportedI32Constant
  exportName : String
  value : Int
