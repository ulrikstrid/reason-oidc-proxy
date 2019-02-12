/* /** Handle sigpipe internally */
   Sys.(set_signal(sigpipe, Signal_ignore));

   /** Setup loggers */
   Fmt_tty.setup_std_outputs();
   Logs.set_level(Some(Logs.Debug));
   Logs.set_reporter(Logs_fmt.reporter());

   open Httpkit;

   Routes.makeAuthCallback() |> Lwt_main.run |> ignore;

   module App = {
     type state = {req_count: int};
     let initial_state = {req_count: 0};
     type other = {
       req_count: int,
       name: string,
     };
     let inc: Server.Middleware.t(state, other) =
       ctx => {req_count: ctx.state.req_count + 1, name: "what"};
     let json: Server.Middleware.t(other, unit) =
       ctx => {
         let str = ctx.state.req_count |> string_of_int;
         ctx.respond(~status=`OK, str);
       };

     let route_handler = (state, path) =>
       switch (path) {
       | [""] => `OK("hello world")
       | ["auth"] => Routes.makeAuth(state)
       | ["auth", "cb"] =>
         Routes.makeAuthCallback() |> Lwt_main.run |> (_ => `OK("ok"))
       | _ => `Unmatched
       };
   };

   let on_start = () => Logs.app(m => m("Running on localhost:3000"));

   Lwt.Infix.(
     Httpkit.Server.(
       make(App.initial_state)
       |> use(Common.log)
       |> reply(Common.router(App.route_handler))
       |> Httpkit_lwt.Server.Http.listen(~port=3000, ~on_start)
       |> Lwt_main.run
     )
   );
    */
