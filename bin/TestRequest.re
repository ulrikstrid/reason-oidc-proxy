let make = () => {
  open Lwt_result.Infix;

  let https_url = "https://api.github.com/users/ulrikstrid";
  Logs.app(m => m("Requesting: %s", https_url));

  let req =
    Httpkit.Client.Request.create(
      ~headers=[("User-Agent", "Reason HttpKit")],
      `GET,
      https_url |> Uri.of_string,
    );

  Httpkit_lwt.Client.(Https.send(req) >>= Response.body)
  |> Lwt.map(res =>
       switch (res) {
       | Ok(body) =>
         Logs.app(m => m("Response: %s", body));
         ();
       | Error(_) =>
         Logs.err(m => m("Something went wrong!!!"));
         ();
       }
     );
};
