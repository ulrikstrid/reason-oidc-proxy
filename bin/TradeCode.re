let makeRequest =
    (~token_endpoint, ~client_id, ~redirect_uri, ~client_secret, code) => {
  open Lwt_result.Infix;

  Logs.app(m => m("Trading code for tokens"));

  let body =
    [
      ("grant_type", "authorization_code"),
      ("client_id", client_id),
      ("code", code),
      ("redirect_uri", redirect_uri),
      ("client_secret", client_secret),
    ]
    |> CCList.fold_left(
         (b, (key, value)) => b ++ key ++ "=" ++ value ++ "&",
         "",
       )
    |> (b => CCString.sub(b, 0, CCString.length(b) - 1));

  let req =
    Httpkit.Client.Request.create(
      ~headers=[
        ("User-Agent", "ReVouch"),
        ("Content-Type", "application/x-www-form-urlencoded"),
      ],
      ~body,
      `POST,
      token_endpoint |> Uri.of_string,
    );

  Httpkit_lwt.Client.(Https.send(req) >>= Response.body)
  |> Lwt.map(res =>
       switch (res) {
       | Ok(body) => Logs.app(m => m("Response: %s", body))
       | Error(_) => Logs.err(m => m("Something went wrong!!!"))
       }
     );
};
