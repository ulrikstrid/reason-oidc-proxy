let make =
    (
      ~client_id,
      ~redirect_uri,
      ~client_secret,
      ~authorization_endpoint,
      ~state,
      reqd,
    ) => {
  open Httpaf;

  let queries = [
    ("client_id", client_id),
    ("response_type", "code id_token"),
    ("scope", "openid"),
    ("redirect_uri", redirect_uri),
    ("response_mode", "form_post"),
    ("client_secret", client_secret),
    ("nonce", "12345abcdef"),
    ("state", state),
  ];

  let auth_uri =
    Uri.(
      of_string(authorization_endpoint)
      |> (uri => add_query_params'(uri, queries) |> to_string)
    );

  Reqd.respond_with_string(
    reqd,
    Response.create(
      `Code(302),
      ~headers=
        Headers.of_list([
          ("Connection", "close"),
          ("Content-Length", "0"),
          ("Location", auth_uri),
        ]),
    ),
    "",
  );
};
