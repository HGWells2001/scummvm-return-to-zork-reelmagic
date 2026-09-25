# Architecture

## Build model

The project does not vendor ScummVM. `build.ps1` creates a filtered sparse checkout of the compatible ScummVM tree, fetches the two upstream ReelMagic-related commits, applies the local compatibility layer, validates the result, and builds a MADE-only Windows executable.

The sparse tree contains the shared components required to link a native ScummVM executable, including the Windows/SDL backend, GUI, audio, graphics, image and video subsystems, plus the MADE engine. Other engines are intentionally not materialized.

## ReelMagic video path

The local compatibility layer keeps ReelMagic-specific MPEG normalization in the shared `video/` subsystem rather than burying it inside MADE. MADE remains responsible for script semantics, movie lifecycle, layer placement and interaction with the game screen.

The effective path is:

```text
RTZ script call
  -> MADE script external
  -> MADE MPEG player
  -> ReelMagic MPEG normalizer
  -> ScummVM MPEG-PS decoder
  -> decoded frame
  -> MADE video layer
  -> MADE compositor
  -> ScummVM SurfaceSDL backend
```

## MADERM compatibility

The original MADERM wrapper exposes movie operations through MADE script externals. The compatibility layer preserves the observed behavior for:

- play/open
- state query
- close
- driver init/shutdown
- pause
- resume

The state presented to scripts is intentionally not the raw driver bitfield. It follows the mapping recovered from MADERM.

## Layering

Return to Zork ReelMagic can place MPEG video either in front of or behind the VGA graphics surface. When video is behind, VGA palette index `0` acts as transparent, allowing the MPEG frame to show through.

## Pause and timing

ReelMagic playback has two pause sources that must remain balanced:

1. script-level movie pause/resume
2. ScummVM global engine pause/resume

The compatibility layer freezes the MADE frame clock and pauses the ScummVM video decoder. A decoder created while the engine is already paused is paused immediately so audio cannot advance underneath a frozen frame.

## Windows presentation workaround

During testing, decoded ReelMagic frames were verified through MADE compositing and into the SDL rendering path, while the default Windows hardware renderer still presented a black result. The public build therefore selects SDL's software renderer on Windows for this custom executable.

This is deliberately localized to the custom build and should not be interpreted as a recommendation for upstream ScummVM globally.

## Launcher integration

The ReelMagic edition is detected as `made:rtzrm` and appears as **Return to Zork (ReelMagic)**. The launcher aliases its thumbnail to ScummVM's existing `made-rtz.png`, since it is a variant of the same game.
