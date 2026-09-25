# Contributing

Contributions are welcome, especially when they reduce the local compatibility layer and move generally useful fixes upstream to ScummVM.

Please keep contributions focused on compatibility code, tests, documentation, or reproducible build logic.

Do not commit copyrighted game data, movies, executables, ReelMagic driver binaries, CD images, extracted assets, or other proprietary files.

For code changes:

1. keep the build reproducible;
2. add or update a source-level test where practical;
3. preserve the separation between generic ReelMagic MPEG handling in `video/` and Return to Zork/MADE-specific script or compositing behavior;
4. document reverse-engineered behavior with enough detail to reproduce the conclusion without distributing the original binary.
