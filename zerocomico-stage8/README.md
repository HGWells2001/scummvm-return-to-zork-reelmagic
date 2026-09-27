# Zero Comico / ScummVM - Stage 8

Stage 8 turns the Stage 7 Mp1 navigation layer into a gameplay runtime for
the shared protagonist and retail Object handlers.

Branch: `scratch/zerocomico-stage8`.

## Shared protagonist

The retail disc stores Giovanni outside the individual MainPlaces:

- `Mpx/bodies/Giovanni/Giovanni.p3d`
- `Mpx/bodies/Giovanni/Giovanni.anj`
- `Mpx/bodies/Giovanni/Giovanni.seq`
- `Mpx/bodies/Giovanni/Giovanni.mat`

`makeSharedActorAssets()` generates those paths from the actor name. Mp1
therefore does not invent or duplicate a local Giovanni resource.

The exact initial room coordinate is still intentionally external to this
module: it must come from the original helper/vector or preceding cut-scene.
There is no guessed fallback coordinate.

## Gameplay variables

`GameplayVariables` imports retail `Variable name value` declarations from
the decoded scripts. Values remain strings because the Lucifer scripts compare
literal tokens.

This is enough for original state such as:

- `guarito_PacMan`
- `parlato_PacMan`
- `Side_Pitfall`

## Object handler extraction

`ObjectHandlerRegistry` indexes `examine:` and `operate:` bodies under
their original `Object` names.

The Stage 7 mesh-to-Object mapping can therefore proceed all the way from a
picked rendered entity to the actual retail script body.

## First gameplay VM

`ObjectHandlerVM` currently executes the high-value control-flow/opcode
subset found in Mp1 puzzle logic:

- `begin_thread` / `end_thread`
- `if_e`
- `else`
- `endif`
- `mov`
- `hide`
- `unhide`
- `start_dialog`
- `wait_last_dialog`
- all Stage 6 opcodes already implemented by `executeStage6Opcode()`.

Unknown opcodes are not silently ignored. The VM stops with
`kObjectHandlerVmBlockedOpcode` and exposes the opcode name so the next
reverse-engineering step is explicit.

## Giovanni + BSP + Object handler runtime

`GameplayRuntime` combines:

- Stage 7's retail BSP/pathfinding graph;
- shared Giovanni resources;
- actor floor position;
- path following;
- Object interaction ranges;
- Object handler lookup;
- gameplay variables;
- the handler VM;
- dialog yielding.

Interaction path:

```
pick-buffer mesh
  -> Object entity:
  -> capability/range check
  -> compute a stop point inside oprange
  -> BSP route
  -> Giovanni walking
  -> arrive
  -> Object operate:/examine:
  -> if_e / mov / hide / dialog / waits
```

A direct floor click is also represented by `walkToFloorPoint()`.

## No guessed world mapping

The BSP data is explicitly a 2D plan. Stage 8 keeps it as `NavVec2`.
The host renderer is responsible for mapping that floor coordinate to the
engine's 3D actor transform. This avoids pretending that the documented BSP
x/y pair is definitely a particular pair of P3D axes before that mapping is
proven from the original loader/runtime.

## Current boundary

Stage 8 provides the runtime plumbing required for the first real puzzle
interaction. Remaining work for a visually playable Mp1 build:

1. decode/load Giovanni's P3D geometry into the renderer;
2. decode the needed ANJ/SEQ actor locomotion semantics;
3. resolve the exact retail Giovanni start helper/vector;
4. room camera switching and portals;
5. implement additional puzzle opcodes as the VM blocks on them;
6. real dialog subtitle/speech playback;
7. inventory/combine UI and semantics.

## Validation

`stage8_selftest.cpp` covers:

- exact shared Giovanni asset paths;
- Variable declarations;
- Object handler extraction;
- `if_e/else/endif`;
- `mov`;
- `hide`;
- dialog start + wait/yield;
- BSP walk to `oprange`;
- automatic handler start when the actor arrives.

GitHub Actions syntax-checks Stages 6, 7 and 8 against current ScummVM headers
with `-Wall -Wextra -Werror` and packages the module set.
