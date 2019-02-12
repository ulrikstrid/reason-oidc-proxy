/** Setup loggers */
Fmt_tty.setup_std_outputs();
Logs.set_level(Some(Logs.Debug));
Logs.set_reporter(Logs_fmt.reporter());

let main = () => {
  Lwt.Infix.(
    Oidc.Discover.makeRequest()
    >>= Oidc.Jwks.makeRequest
    |> Lwt_main.run
    |> (
      fun
      | Ok(_jwks) => Httpaf_server.start(~port=3000, ~ctx=_req => (), ())
      | Error(message) => Logs.err(m => m("%s", message))
    )
  );
};

main();
