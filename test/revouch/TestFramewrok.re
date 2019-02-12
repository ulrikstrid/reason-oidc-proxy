include Rely.Make({
  let config =
    Rely.TestFrameworkConfig.initialize({
      snapshotDir: "test/oidc/__snapshots__",
      projectDir: "oidc"
    });
});
