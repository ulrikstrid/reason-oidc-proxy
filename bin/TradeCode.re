let makeRequest =
    (~token_endpoint, ~client_id, ~redirect_uri, ~client_secret, code) => {
  open Lwt_result.Infix;

  Logs.app(m => m("Trading code for tokens"));

  let body =
    [
      ("client_id", client_id),
      ("scope", "openid"),
      ("code", code),
      ("redirect_uri", redirect_uri),
      ("grant_type", "authorization_code"),
      ("client_secret", client_secret),
    ]
    |> Uri.add_query_params'(Uri.empty)
    |> Uri.to_string
    |> (s => String.sub(s, 1, String.length(s) - 1));

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
       | Ok(body) => body
       | Error(_) => "Something went wrong!"
       }
     );
};
