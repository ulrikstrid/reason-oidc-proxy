open Httpaf;
open Lwt.Infix;

let json_err =
  fun
  | Ok(_) as ok => ok
  | Error(err) => Error(`String(err));

let mk_connection_handler = (mk_context: 'a => Routes.ctx) => {
  let connection_handler: (Unix.sockaddr, Lwt_unix.file_descr) => Lwt.t(unit) = {
    let request_handler: (Unix.sockaddr, Reqd.t(_)) => unit =
      (_client_address, request_descriptor) =>
        Lwt.async(() => Routes.makeCallback(request_descriptor, mk_context));

    let error_handler:
      (
        Unix.sockaddr,
        ~request: Httpaf.Request.t=?,
        _,
        Headers.t => Body.t([ | `write])
      ) =>
      unit =
      (_client_address, ~request as _=?, error, start_response) => {
        let response_body = start_response(Headers.empty);

        switch (error) {
        | `Exn(exn) =>
          Body.write_string(response_body, Printexc.to_string(exn));
          Body.write_string(response_body, "\n");

        | #Status.standard as error =>
          Body.write_string(
            response_body,
            Status.default_reason_phrase(error),
          )
        };

        Body.close_writer(response_body);
      };

    Httpaf_lwt.Server.create_connection_handler(
      ~config=?None,
      ~request_handler,
      ~error_handler,
    );
  };
  connection_handler;
};

let start = (~port=8080, ~ctx: 'a => Routes.ctx, ()) => {
  let listen_address = Unix.(ADDR_INET(inet_addr_loopback, port));

  Lwt.async(() =>
    Lwt_io.establish_server_with_client_socket(
      listen_address,
      mk_connection_handler(ctx),
    )
    >>= (_server => Lwt_io.printlf("Server started on port: %d", port))
  );

  let (forever, _) = Lwt.wait();

  Lwt_main.run(forever);
};
