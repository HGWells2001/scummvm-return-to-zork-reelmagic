# Zero Comico / ScummVM - Stage 11

Stage 11 joins the previously separate gameplay, actor and dialogue layers.

Branch: `scratch/zerocomico-stage11`.

## Actor motion without guessing `.seq`

The original executable distinguishes high-level character animation states
such as `walk:`, `standby:`, `special_standby:` and `Blend:`, while
`japotek3d.dll` exposes a `SequenceTable` loader for the actor `.seq`
files.

The actual Giovanni SequenceTable grammar is still not available in the
public corpus. Stage 11 therefore introduces `ActorMotionController`, whose
semantic mapping is **injected**:

```
ActorMotionClips {
    idle = <resolved clip name>
    walk = <resolved clip name>
}
```

The controller validates those names against the clips already decoded from
`Giovanni.anj`.

It never falls back to a guessed clip named `Walk` or `Stay`.

When the SequenceTable is decoded later, it only has to populate this mapping;
the gameplay/pathfinding code does not change.

## Integrated gameplay host

`IntegratedGameplayHost` implements the Stage 8 `GameplayRuntimeHost` and
combines:

- Stage 9 packed actor resource loading;
- Giovanni P3D/ANJ model;
- actor JGF5 texture loading;
- classic-mesh render catalog;
- scene object registration;
- per-actor ANJ animation runtime;
- Stage 11 motion controller;
- Stage 10 dialogue service;
- renderer/world callbacks for floor position and examine text.

The renderer-specific BSP-floor -> P3D-world conversion remains behind
`GameplaySceneAdapter`. Stage 11 does not guess which P3D axes correspond
to the BSP x/y plane.

## Gameplay session

`GameplaySession` makes the frame order explicit:

1. Stage 8 advances pathfinding and the puzzle VM;
2. walking state changes reach the integrated host;
3. Stage 9/11 advances the selected actor ANJ animation.

The session also loads the decoded `dialog.isc` before starting gameplay,
so `start_dialog` and `wait_last_dialog` use the real Stage 10 service.

## Rendering boundary

Stage 9 already supplies:

- `ActorModel`;
- `ActorTextureSet`;
- `ActorRenderCatalog`;
- `registerActorSceneObjects()`.

Stage 11 exposes all of these through the integrated host. The remaining
renderer work is to submit the catalog's classic mesh batches to the same
software rasterizer used for the Stage 5 menu, and later resolve shared /
deformer geometry ownership.

## Validation

The Stage 11 focused test intentionally uses non-retail synthetic clip names
(`gio_idle_resolved`, `gio_walk_resolved`) to prove the motion controller
is mapping-driven rather than name-driven.

It also verifies:

- invalid/unresolved walk mappings are rejected;
- repeating a walking state does not restart it;
- an unresolved idle mapping safely stops animation instead of inventing one.

CI syntax-checks Stages 6 through 11 with `-Werror`, builds ScummVM's real
`libcommon`, runs the Stage 11 motion test, and packages all experimental
modules.
