open Httpaf;

let makeAuth = AuthRoute.make;

let makeAuthCallback = AuthCallbackRoute.make;

let testRequest = TestRequest.make;

let makeCallback = (reqd, _mk_context) => {
  let {Httpaf.Request.target, meth, _} as request = Reqd.request(reqd);
  let req_path = target |> Uri.of_string |> Uri.path;
  Format.printf("Req: %a\n%!", Httpaf.Request.pp_hum, request);
  let path_parts = Str.(split(regexp("/"), req_path));

  switch (meth, path_parts) {
  | (`GET, ["validate"]) =>
    Headers.get(Reqd.request(reqd).headers, "Authorization")
    |> (
      authHeader =>
        switch (authHeader) {
        | Some(auth) =>
          switch (String.split_on_char(' ', auth)) {
          | ["Bearer", token] =>
            Oidc.Jwks.validate(token)
              ? OkResponse.make(reqd) : UnauthorizedResponse.make(reqd)
          | _ => UnauthorizedResponse.make(reqd)
          }
        | None => UnauthorizedResponse.make(reqd)
        }
    );

    Lwt.return_unit;
  | (`GET, ["auth"]) =>
    Reqd.respond_with_string(
      reqd,
      Response.create(
        `Code(302),
        ~headers=
          Headers.of_list([
            ("Connection", "close"),
            ("Content-Length", "0"),
            ("Location", "http://localhost:3000/auth/cb"),
          ]),
      ),
      "",
    );
    Lwt.return_unit;

  | (`GET, ["auth", "cb"]) =>
    Reqd.respond_with_string(
      reqd,
      Response.create(
        `OK,
        ~headers=
          Headers.of_list([
            ("Connection", "close"),
            ("Content-Length", "8"),
            /* ("Content-Type", "application/json"), */
          ]),
      ),
      "callback",
    );
    Lwt.return_unit;

  | (`POST, ["foo"]) =>
    let response =
      Response.create(
        `OK,
        ~headers=
          Headers.of_list([
            ("Connection", "keep-alive"),
            ("Content-Length", "3"),
            /* ("Content-Type", "application/json"), */
          ]),
      );

    let request_body = Reqd.request_body(reqd);
    let response_body = Reqd.respond_with_streaming(reqd, response);
    let rec on_read = (_request_data, ~off as _, ~len as _) => {
      Body.write_string(response_body, "yay");
      Body.close_writer(response_body);

      Body.flush(response_body, () =>
        Body.schedule_read(request_body, ~on_eof, ~on_read)
      );
    }
    and on_eof = () => Body.close_writer(response_body);

    Body.schedule_read(~on_read, ~on_eof, request_body);
    Lwt.return_unit;
  | (_, ["bar"]) =>
    let response =
      Response.create(
        `OK,
        ~headers=
          Headers.of_list([
            ("Connection", "close"),
            ("Content-Type", "text/event-stream"),
          ]),
      );
    let request_body = Reqd.request_body(reqd);
    let response_body = Reqd.respond_with_streaming(reqd, response);
    let (finished, notify) = Lwt.wait();
    let rec on_read = (_request_data, ~off as _, ~len as _) =>
      Body.flush(response_body, () =>
        Body.schedule_read(request_body, ~on_eof, ~on_read)
      )
    and on_eof = () => {
      let _ = Body.write_string(response_body, "event: end\ndata: 1\n\n");
      Body.flush(
        response_body,
        () => {
          Body.close_writer(response_body);
          Lwt.wakeup_later(notify, ());
        },
      );
    };
    /* Body.flush(response_body, () => ()); */
    Body.schedule_read(~on_read, ~on_eof, request_body);
    finished;
  | _ =>
    Reqd.respond_with_string(reqd, Response.create(`Not_found), "");
    Lwt.return_unit;
  };
};
