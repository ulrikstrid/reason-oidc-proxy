exception Bad_payload;

module Header = {
  module Algorithm = {
    type t =
      | RS256(Nocrypto.Rsa.priv)
      | Unknown;

    let ofString =
      fun
      | "RSA" => RS256(Nocrypto.Rsa.generate(256))
      | _ => Unknown;
  };

  type t = {
    alg: Algorithm.t,
    typ: option(string),
  };

  let ofJson = json =>
    Yojson.Basic.Util.{
      alg: json |> member("alg") |> to_string |> Algorithm.ofString,
      typ: json |> member("typ") |> to_string_option,
    };

  let ofString = str => {
    Yojson.Basic.from_string(str) |> ofJson;
  };

  /* let ofB64 = b64 =>
     switch (B64.decode(b64)) {
     | Ok(str) => Ok(ofString(str))
     | Error(_) => Error()
     }; */

  let ofB64 = b => B64.decode(b) |> ofString;
};

module Payload = {
  type claim = string;

  type t = list((claim, string));

  // let ofB64 = b64 =>
  //   switch (B64.decode(b64)) {
  //   | Ok(str) => Ok(ofString(str))
  //   | Error(_) => Error()
  //   };

  let ofJson = json =>
    json
    |> Yojson.Basic.Util.to_assoc
    |> List.map(x =>
         switch (x) {
         | (claim, `String(value)) => (claim, value)
         | (claim, `Int(value)) => (claim, string_of_int(value))
         | _ => raise(Bad_payload)
         }
       );

  let ofString = str => {
    Yojson.Basic.from_string(str) |> ofJson;
  };

  let ofB64 = b => B64.decode(b) |> ofString;
};

module Signature = {
  type t = string;

  let ofB64 = B64.decode;
};

type t = {
  header: Header.t,
  payload: Payload.t,
  signature: Signature.t,
};

let opt_to_string =
  fun
  | Some(str) => str
  | None => "";

let tokenOfString = token => {
  token
  |> String.split_on_char('.')
  |> (
    parts =>
      switch (parts) {
      | [encodedHeader, encodedPayload, encodedSignature] =>
        Ok({
          header: Header.ofB64(encodedHeader),
          payload: Payload.ofB64(encodedPayload),
          signature: Signature.ofB64(encodedSignature),
        })
      | _ => Error("Not a valid JWT.")
      }
  );
};
