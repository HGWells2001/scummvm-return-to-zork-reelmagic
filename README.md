# Return to Zork ReelMagic for ScummVM

Unofficial compatibility project for running the **ReelMagic edition of Return to Zork** through ScummVM's MADE engine.

The goal is a complete Windows ScummVM build with all engines enabled, plus native support for Return to Zork ReelMagic as its own MADE target. It launches from the standard ScummVM frontend, keeps the original MPEG presentation semantics, and avoids external wrappers once the custom build is installed.

> This project is experimental and is **not an official ScummVM release**. It does not include Return to Zork, ReelMagic drivers, movies, data files, or any other proprietary game content.

## Current status

The build currently provides a full ScummVM x64 application with all available engines enabled, plus:

- separate `made:rtzrm` detection for **Return to Zork (ReelMagic)**
- ReelMagic MPEG normalization/descrambling in the shared ScummVM video layer
- single-frame MPEG pacing support required by ReelMagic streams
- MADERM external-call behavior for movie play/state/close/pause/resume
- front/behind VGA compositing behavior
- pause/resume clock handling for both script pauses and ScummVM global pauses
- `SAMPLE.AD` extraction fallback from `rtzrm.red`
- central ReelMagic video pumping in MADE
- `Space` to skip non-looping ReelMagic cutscenes
- standard ScummVM launcher integration and the official Return to Zork launcher icon
- Windows SDL software-renderer workaround for the presentation issue found during compatibility testing

The project is based on upstream ScummVM work and then applies a small compatibility/hardening layer on top.

## Upstream basis

This repository builds from the ScummVM-compatible base commit:

`d16c0c72818d46526406921c25f47432939eb4d7`

and applies these upstream changes first:

- [ScummVM PR #7848](https://github.com/scummvm/scummvm/pull/7848) — `VIDEO: Add opt-in single-frame MPEG decoding`
- [ScummVM PR #7849](https://github.com/scummvm/scummvm/pull/7849) — `MADE: Support Return to Zork ReelMagic`

Pinned commits used by the build:

- `79d7ee1af3847c28eeee5d90d6cda0252cf5fcb2`
- `5c82a3dc97f4768b319fca2afddf436bfae4aca7`

## Build on Windows

Requirements:

- Windows 64-bit
- Git for Windows
- Visual Studio with **Desktop development with C++**
- Internet access for the ScummVM sparse checkout and vcpkg dependencies
- Python 3 is recommended for the validation suite

Then run:

```text
BUILD.cmd
```

The script prepares the complete ScummVM source tree because every engine is compiled. It still uses Git transfer filtering where possible, then applies the ReelMagic changes on top.

The resulting portable build is written to:

```text
BUILD_OUTPUT\ScummVM-RTZ-ReelMagic\
```

Launch:

```text
scummvm.exe
```

Use **Add Game...** with any game supported by ScummVM. For a legally obtained ReelMagic installation, ScummVM should additionally detect:

```text
Return to Zork (ReelMagic)
```

## ScummVM integration

The custom build keeps the standard frontend and standard per-game graphics options. The ReelMagic target appears as a normal MADE entry and uses the regular Return to Zork launcher artwork already distributed by ScummVM.

The engine also exposes the ReelMagic-specific option:

```text
Reduce ReelMagic video to 200 lines
```

This controls whether the 240-line MPEG frame is reduced to the game's 200-line graphics height or the graphics are stretched to preserve the full video frame.

## Game files are not included

You must provide your own copy of Return to Zork ReelMagic. No proprietary `.MPG`, `.RED`, `.PRJ`, executable, driver, CD image, or game-data file is stored in this repository.

The `.gitignore` intentionally rejects common game/media/driver formats to make accidental commits harder.

## Technical notes

The compatibility work was validated against behavior recovered from the original MADERM wrapper and ReelMagic 1.11 driver generation. Notes are kept in [`docs/`](docs/).

Key recovered behavior includes:

- ReelMagic RTZ magic key `0x40044041`
- MPEG frame-rate encoding using the ReelMagic high-bit/reserved-code convention
- 56-frame delta pattern used to reconstruct MPEG P/B-frame parameters
- MADERM state mapping for stopped/playing/paused
- Z-order values for MPEG in front of or behind VGA graphics
- palette index `0` as the transparent/alpha VGA index for behind-video compositing
- resume-once / resume-loop behavior used by MADERM external 106

See [`docs/ARCHITECTURE.md`](docs/ARCHITECTURE.md) for the compatibility pipeline.

## Repository layout

```text
BUILD.cmd              one-click Windows build
build.ps1              complete ScummVM x64 + ReelMagic build pipeline
scripts/               patch application and PowerShell validators
tests/                 Python source/behavior checks
docs/                  reverse-engineering and architecture notes
.github/workflows/      lightweight repository checks
```

## Credits

This project builds on the work of the **ScummVM project** and the ReelMagic work submitted upstream by **synacktic**. Compatibility research also relied on behavior observed in the original Return to Zork MADERM wrapper and ReelMagic 1.11 driver generation.

Please prefer upstreaming generally useful fixes to ScummVM rather than keeping long-lived forks where possible.

## License

Code and scripts in this repository are distributed under **GPL-3.0-or-later**, consistent with ScummVM. See [`LICENSE`](LICENSE).

Return to Zork and its original assets are not covered by this repository's license and are not distributed here.
