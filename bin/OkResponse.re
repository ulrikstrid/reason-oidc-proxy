open Httpaf;

let make = reqd =>
  Reqd.respond_with_string(
    reqd,
    Response.create(
      `OK,
      ~headers=
        Headers.of_list([("Connection", "close"), ("Content-Length", "2")]),
    ),
    "ok",
  );
