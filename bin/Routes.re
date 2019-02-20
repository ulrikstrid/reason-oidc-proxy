open Httpaf;

type ctx = {
  discoveryDoc: Oidc.Discover.t,
  jwks: list(Oidc.Jwks.t),
  client_id: string,
  redirect_uri: string,
  client_secret: string,
  oidc_host: string,
};

let makeAuth = AuthRoute.make;

// let makeAuthCallback = AuthCallbackRoute.make;

let makeCallback = (reqd, mk_context) => {
  let {Httpaf.Request.target, meth, _} as request = Reqd.request(reqd);
  let req_uri = target |> Uri.of_string;
  let req_path = Uri.path(req_uri);
  Format.printf("Req: %a\n%!", Httpaf.Request.pp_hum, request);
  let path_parts = Str.(split(regexp("/"), req_path));

  let ctx = mk_context(reqd);

  switch (meth, path_parts) {
  | (`GET, ["validate"]) =>
    Headers.get(Reqd.request(reqd).headers, "Authorization")
    |> (
      authHeader =>
        switch (authHeader) {
        | Some(auth) =>
          switch (String.split_on_char(' ', auth)) {
          | ["Bearer", token] =>
            Jwt.t_of_token(token) |> Oidc.Jwks.validate(ctx.jwks)
              ? OkResponse.make(reqd)
              : UnauthorizedResponse.make(reqd, "Bad token")
          | _ => UnauthorizedResponse.make(reqd, "No bearer token")
          }
        | None => UnauthorizedResponse.make(reqd, "No Authorization header")
        }
    );

    Lwt.return_unit;
  | (`GET, ["auth"]) =>
    Headers.get(Reqd.request(reqd).headers, "Authorization")
    |> CCOpt.map(String.split_on_char(' '))
    |> (
      authHeader =>
        (
          switch (authHeader) {
          | Some(["Bearer", token]) => Some(token)
          | _ => None
          }
        )
        |> CCOpt.map(Jwt.t_of_token)
        |> CCOpt.map(Oidc.Jwks.validate(ctx.jwks))
        |> (
          okToken =>
            switch (okToken) {
            | Some(true) => OkResponse.make(reqd)
            | Some(false) =>
              RedirectAnswer.make(
                ~client_id=ctx.client_id,
                ~redirect_uri=ctx.redirect_uri,
                ~client_secret=ctx.client_secret,
                ~authorization_endpoint=
                  ctx.discoveryDoc.authorization_endpoint,
                ~state="12345",
                reqd,
              )
            | None =>
              RedirectAnswer.make(
                ~client_id=ctx.client_id,
                ~redirect_uri=ctx.redirect_uri,
                ~client_secret=ctx.client_secret,
                ~authorization_endpoint=
                  ctx.discoveryDoc.authorization_endpoint,
                ~state="12345",
                reqd,
              )
            }
        )
    );

    Lwt.return_unit;

  | (`POST, ["auth", "cb"]) =>
    open Lwt.Infix;

    let bodyBuffer =
      Headers.get(Reqd.request(reqd).headers, "content-length")
      |> CCOpt.map_or(~default=128, int_of_string)
      |> Buffer.create;

    let (bodyPromise, promiseResolver) = Lwt.wait();

    let request_body = Reqd.request_body(reqd);
    let rec on_read = (request_data, ~off, ~len) => {
      let request_data_bytes = Bytes.create(len);
      Lwt_bytes.blit_to_bytes(request_data, off, request_data_bytes, 0, len);
      Buffer.add_bytes(bodyBuffer, request_data_bytes);
      Body.schedule_read(~on_eof, ~on_read, request_body);
    }
    and on_eof = () => {
      Lwt.wakeup_later(promiseResolver, Buffer.contents(bodyBuffer));
    };

    Body.schedule_read(~on_read, ~on_eof, request_body);

    bodyPromise
    >|= (
      body => {
        CCString.split_on_char('=', body) |> (l => CCList.nth(l, 1));
      }
    )
    >>= (
      code =>
        {
          TradeCode.makeRequest(
            ~token_endpoint=ctx.discoveryDoc.token_endpoint,
            ~client_id=ctx.client_id,
            ~redirect_uri=ctx.redirect_uri,
            ~client_secret=ctx.client_secret,
            ~oidc_host=ctx.oidc_host,
            code,
          );
        }
        >|= (
          code => {
            Reqd.respond_with_string(
              reqd,
              Response.create(
                `OK,
                ~headers=
                  Headers.of_list([
                    ("Connection", "close"),
                    ("Content-Length", "4"),
                    /* ("Content-Type", "application/json"), */
                  ]),
              ),
              "code",
            );
          }
        )
    );
  | _ =>
    Reqd.respond_with_string(reqd, Response.create(`Not_found), "");
    Lwt.return_unit;
  };
};
