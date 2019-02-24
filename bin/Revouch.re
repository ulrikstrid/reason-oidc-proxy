/** Setup loggers */
Fmt_tty.setup_std_outputs();
Logs.set_level(Some(Logs.Debug));
Logs.set_reporter(Logs_fmt.reporter());

let [discoveryUrl, client_id, redirect_uri, client_secret] =
  Sys.[
    getenv_opt("DISCOVERY_URL"),
    getenv_opt("CLIENT_ID"),
    getenv_opt("REDIRECT_URI"),
    getenv_opt("CLIENT_SECRET"),
  ]
  |> CCOpt.sequence_l
  |> CCOpt.get_exn;

let main = () => {
  Lwt.Infix.(
    Oidc.Discover.makeRequest(discoveryUrl)
    >>= (
      discoveryDoc => {
        Oidc.Jwks.makeRequest(discoveryDoc)
        >|= CCResult.map(jwks =>
              Routes.{
                jwks,
                discoveryDoc,
                client_id,
                redirect_uri,
                client_secret,
              }
            );
      }
    )
    |> Lwt_main.run
    |> (
      jwksResult =>
        switch (jwksResult) {
        | Ok(ctx) => Httpaf_server.start(~port=3000, ~ctx=_req => ctx, ())
        | Error(message) => Logs.err(m => m("%s", message))
        }
    )
  );
};

main();
