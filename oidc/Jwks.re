type t = {
  kty: string,
  use: string,
  kid: string,
  x5t: string,
  n: string,
  e: string,
  x5c: list(string),
  issuer: string,
};

let parseJwks = json =>
  Yojson.Basic.Util.{
    kty: json |> member("kty") |> to_string,
    use: json |> member("use") |> to_string,
    kid: json |> member("kid") |> to_string,
    x5t: json |> member("x5t") |> to_string,
    n: json |> member("n") |> to_string,
    e: json |> member("e") |> to_string,
    x5c: json |> member("x5c") |> to_list |> List.map(to_string),
    issuer: json |> member("issuer") |> to_string,
  };

let fromResponse = body =>
  Yojson.Basic.from_string(body)
  |> Yojson.Basic.Util.member("keys")
  |> Yojson.Basic.Util.to_list
  |> List.map(parseJwks);

/*
let makeRequest = (discoverData: Discover.t) => {
  open Lwt_result.Infix;

  Logs.app(m => m("Starting jwks request"));

  let https_url = discoverData.jwks_uri;
  Logs.app(m => m("Requesting: %s", https_url));

  let req =
    Httpkit.Client.Request.create(
      ~headers=[("User-Agent", "ReVouch")],
      `GET,
      https_url |> Uri.of_string,
    );

  Httpkit_lwt.Client.(Https.send(req) >>= Response.body)
  |> Lwt.map(res =>
       switch (res) {
       | Ok(body) => Ok(fromResponse(body))
       | Error(_) => Error("Could not get JWKS")
       }
     );
};
*/
let validate = token => {
  true;
};
