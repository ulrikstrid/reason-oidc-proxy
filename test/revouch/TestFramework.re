include Rely.Make({
  let config =
    Rely.TestFrameworkConfig.initialize({
      snapshotDir: "./test/revouch/__snapshots__",
      projectDir: "./"
    });
});
