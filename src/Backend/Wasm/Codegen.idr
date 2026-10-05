module Backend.Wasm.Codegen

import Backend.Wasm.Binary
import Backend.Wasm.IR
import Backend.Wasm.Lower
import Compiler.ANF
import Compiler.Common
import Core.Context
import Core.Core
import Core.Env
import Core.TT
import Idris.Syntax
import Libraries.Utils.Path
import System
import System.File.Error

%default covering

-- Driver post-options may display UserError and still exit zero.
-- Backend refusal and artifact I/O failure must fail the command.
private
rejectBackend : String -> Core value
rejectBackend explanation = coreLift $ die ("Error: " ++ explanation)

public export
backendName : String
backendName = "wasm"

private
lookupDefinition : Name -> List (Name, Administrative_Normal_Form_Definition) -> Maybe Administrative_Normal_Form_Definition
lookupDefinition requested [] = Nothing
lookupDefinition requested ((name, definition) :: rest) =
  if requested == name
    then Just definition
    else lookupDefinition requested rest

private
fullyQualifiedExport :
  {auto c : Ref Ctxt Defs} -> (Name, String) -> Core (Name, String)
fullyQualifiedExport (internalName, externalName) = do
  qualifiedName <- toFullNames internalName
  pure (qualifiedName, externalName)

private
selectSingleExport : List (Name, String) -> Core (Name, String)
selectSingleExport [selected] = pure selected
selectSingleExport [] =
  rejectBackend
    "wasm: no export selected; add %export \"wasm:<name>\" to the first oracle"
selectSingleExport _ =
  rejectBackend
    "wasm: the first executable slice admits exactly one exported function"

private
compileWasm :
  Ref Ctxt Defs -> Ref Syn SyntaxInfo ->
  (temporaryDirectory : String) -> (outputDirectory : String) ->
  ClosedTerm -> (requestedOutputName : String) -> Core (Maybe String)
compileWasm definitions syntax temporaryDirectory outputDirectory
            term requestedOutputName = do
  compileData <- getCompileDataWith [backendName] False Compiler.Common.Administrative_Normal_Form term
  qualifiedExports <- traverse fullyQualifiedExport (exported compileData)
  selectedExport <- selectSingleExport qualifiedExports
  let (internalName, externalName) = selectedExport
  definition <-
    case lookupDefinition internalName (anf compileData) of
      Nothing =>
        rejectBackend
          ("wasm: no ANF definition was produced for exported function `" ++
           show internalName ++ "`")
      Just found => pure found
  lowered <-
    case lowerExport externalName definition of
      Left explanation =>
        rejectBackend ("wasm rejected reachable program: " ++ explanation)
      Right function => pure function
  bytes <-
    case encodeModule lowered of
      Left explanation =>
        rejectBackend ("wasm binary encoding failed: " ++ explanation)
      Right encoded => pure encoded
  let outputFile = outputDirectory </> (requestedOutputName ++ ".wasm")
  writeResult <- coreLift $ writeModuleFile outputFile bytes
  case writeResult of
    Left error =>
      rejectBackend
        ("wasm: could not write `" ++ outputFile ++ "`: " ++ show error)
    Right () => pure (Just outputFile)

private
executeWasm :
  Ref Ctxt Defs -> Ref Syn SyntaxInfo -> String -> ClosedTerm -> Core ()
executeWasm definitions syntax temporaryDirectory term =
  rejectBackend
    "wasm emits a core module; execute it with an explicitly chosen host runtime"

public export
wasmCodegen : Codegen
wasmCodegen = MkCG compileWasm executeWasm Nothing Nothing
