module Backend.Wasm.Main

import Backend.Wasm.Codegen
import Compiler.Common
import Idris.Driver

main : IO ()
main =
  mainWithCodegens
    [(backendName, wasmCodegen)]
