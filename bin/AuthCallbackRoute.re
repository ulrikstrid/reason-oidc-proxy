let make = _state => {
  open Lwt_result.Infix;

  let https_url = "https://api.github.com/users/ulrikstrid";
  Logs.app(m => m("Requesting: %s", https_url));

  Httpkit_lwt.Client.(
    Httpkit.Client.Request.create(
      ~headers=[("User-Agent", "Reason HttpKit")],
      `GET,
      https_url |> Uri.of_string,
    )
    |> Https.send
    >>= Response.body
  )
  |> Lwt.map(requestPromise =>
       switch (requestPromise) {
       | Ok(body) =>
         Logs.app(m => m("Response: %s", body));
         `OK(body);
       | Error(_) =>
         Logs.err(m => m("Something went wrong!!!"));
         `With_status((`Code(500), "error"));
       }
     );
};
