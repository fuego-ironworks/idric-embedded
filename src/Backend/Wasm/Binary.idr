module Backend.Wasm.Binary

import Backend.Wasm.IR
import Data.Bits
import Data.Buffer
import System.File.Buffer

%default covering

private
uleb : Int -> List Int
uleb value =
  if value < 0
    then []
    else if value < 128
      then [value]
      else ((value .&. 127) .|. 128) :: uleb (shiftR value 7)

private
sleb32 : Int -> List Int
sleb32 value =
  let byte = value .&. 127
      rest = shiftR value 7
      signSet = (byte .&. 64) /= 0
      done = (rest == 0 && not signSet) || (rest == -1 && signSet)
  in if done
       then [byte]
       else (byte .|. 128) :: sleb32 rest

private
section : Int -> List Int -> List Int
section identifier payload =
  identifier :: uleb (cast (length payload)) ++ payload

private
encodeAsciiName : String -> Either String (List Int)
encodeAsciiName name = encode (unpack name)
  where
    encode : List Char -> Either String (List Int)
    encode [] = Right []
    encode (c :: rest) =
      let code = ord c in
        if code <= 127
          then do
            more <- encode rest
            Right (code :: more)
          else Left ("Non-ASCII Wasm export name is outside the first slice: " ++ name)

private
typeSection : List Int
typeSection =
  -- vec[1] functype 0x60, zero params, one i32 (0x7f) result
  section 1 [1, 0x60, 0, 1, 0x7f]

private
functionSection : List Int
functionSection =
  -- one function using type index zero
  section 3 [1, 0]

private
exportSection : String -> Either String (List Int)
exportSection name = do
  nameBytes <- encodeAsciiName name
  let payload =
        [1] ++
        uleb (cast (length nameBytes)) ++ nameBytes ++
        [0, 0] -- export kind function, function index zero
  Right (section 7 payload)

private
codeSection : Int -> List Int
codeSection value =
  let expression = [0, 0x41] ++ sleb32 value ++ [0x0b]
      body = uleb (cast (length expression)) ++ expression
  in section 10 ([1] ++ body)

||| Encode a Core 3.0-compatible binary module using binary-format version 1.
||| The first slice emits exactly type, function, export, and code sections.
public export
encodeModule : ExportedI32Constant -> Either String (List Int)
encodeModule function = do
  exports <- exportSection function.exportName
  Right
    ([0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00] ++
     typeSection ++
     functionSection ++
     exports ++
     codeSection function.value)

private
fillBuffer : Buffer -> Int -> List Int -> IO ()
fillBuffer buffer offset [] = pure ()
fillBuffer buffer offset (byte :: rest) = do
  setBits8 buffer offset (cast byte)
  fillBuffer buffer (offset + 1) rest

||| Write an already encoded module as raw bytes.
public export
writeModuleFile : String -> List Int -> IO (Either FileError ())
writeModuleFile path bytes = do
  let size = cast (length bytes)
  Just buffer <- newBuffer size
    | Nothing => pure (Left FileWriteError)
  fillBuffer buffer 0 bytes
  result <- writeBufferToFile path buffer size
  case result of
    Left (error, written) => pure (Left error)
    Right () => pure (Right ())
