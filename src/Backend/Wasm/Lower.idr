module Backend.Wasm.Lower

import Backend.Wasm.IR
import Compiler.ANF
import Core.Name
import Core.TT

%default covering

private
isAsciiLetter : Char -> Bool
isAsciiLetter c =
  (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')

private
isAsciiDigit : Char -> Bool
isAsciiDigit c = c >= '0' && c <= '9'

private
isNameStart : Char -> Bool
isNameStart c = isAsciiLetter c || c == '_'

private
isNameRest : Char -> Bool
isNameRest c = isNameStart c || isAsciiDigit c

private
validateExportName : String -> Either String String
validateExportName name =
  case unpack name of
    [] => Left "The first Wasm export name cannot be empty"
    first :: rest =>
      if isNameStart first && all isNameRest rest
        then Right name
        else Left
          ("The first Wasm slice admits only ASCII identifier export names, got `" ++
           name ++ "`")

||| Lower exactly the first oracle from Compiler.ANF:
||| a nullary export whose entire reachable body is one Int32 literal.
public export
lowerExport : String -> Administrative_Normal_Form_Definition -> Either String ExportedI32Constant
lowerExport requestedName (Make_Administrative_Normal_Form_Function [] (Administrative_Normal_Form_Primitive_Value _ (I32 value))) = do
  name <- validateExportName requestedName
  Right (MkExportedI32Constant name (cast value))
lowerExport requestedName (Make_Administrative_Normal_Form_Function arguments body) =
  Left
    ("Export `" ++ requestedName ++
     "` is outside the first Wasm oracle: expected zero runtime parameters " ++
     "and one direct Int32 literal; got " ++ show (length arguments) ++
     " runtime parameters or a non-literal body")
lowerExport requestedName definition =
  Left
    ("Export `" ++ requestedName ++
     "` is not a function in the first Wasm oracle")
