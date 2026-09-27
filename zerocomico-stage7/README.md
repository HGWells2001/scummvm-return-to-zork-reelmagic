# Zero Comico / ScummVM - Stage 7

Stage 7 moves the experimental port from the interactive Mp0 menu into the
first real gameplay MainPlace.

The branch is `scratch/zerocomico-stage7`; `main` remains untouched.

## What is implemented

### MainPlace -> start Room gameplay preparation

Stage 6 already performs a safe `ChangeMainplace Mp1` transition. Stage 7
adds the next transaction:

1. parse the real `Mp1/gameplay/room.isc`;
2. resolve its `StartPlace`;
3. parse that Room's resource fields;
4. resolve the room navigation BSP;
5. parse and validate the 2D BSP/pathfinding graph;
6. parse the MainPlace's `puzzle.isc` interactive `Object` declarations;
7. only publish the prepared gameplay state after all required resources pass.

No room name, NPC, hotspot rectangle or route is hard-coded into the runtime.

### BSP navigation and weighted pathfinding

`BspNavigation` ports the already verified retail text format:

- room outline + holes;
- BSP points/edges/cells;
- preorder BSP tree closure check;
- weighted navigation graph;
- support section consumption;
- index validation.

Path search currently uses Dijkstra over the shipped weights.

The retail Mp1 data documented by the reverse-engineering corpus includes:

- `r11_Map00.bsp`: 40 graph nodes, 630 arcs;
- `r11_MapCam00.bsp`: 28 graph nodes, 430 arcs;
- room maps `r11` through `r15`.

### Character movement primitive

`PathFollower` advances a 2D character position along a BSP route at a
caller-supplied engine-units-per-second speed. The renderer/character layer can
map this 2D floor position back to the 3D actor transform.

### Retail interactive Object registry

`GameplayObjectRegistry` reads `Object` declarations from decoded
`puzzle.isc` and retains the fields Stage 7 needs for interaction:

- `entity:`
- `polygon:`
- `range:`
- `oprange:`
- `size:`
- `roomscope:`
- `examine_text:`
- `PICKABLE`
- `EXAMINABLE`
- `OPERATED`
- `ENABLED`
- presence of `examine` / `operate` handlers.

The retail script census contains 469 such objects over the game.

### Picked mesh -> puzzle handler

Stage 6 already writes the front-most rendered mesh id into a pick buffer.
Stage 7 now resolves that picked scene entity against the retail Object's
`entity:` field.

For an operation:

1. resolve picked mesh -> Object;
2. check the original object capability flags;
3. use `range` / `oprange`;
4. if necessary, request BSP movement toward the target;
5. suspend the interaction while walking;
6. when the target is reached, invoke the Object's original `operate`,
   `examine` or take handler through the VM host.

This is the gameplay equivalent of Stage 6's menu picking: no hand-authored
screen rectangles.

## Mp1 facts used as ground truth

The public format/census work and the user's retail image establish the
following resource topology for Mp1:

- 5 Room declarations: `Room1_1` .. `Room1_5`;
- navigation files `r11_Map00.bsp` .. `r15_Map00.bsp`;
- `puzzle.isc`, `char.isc`, `dialog.isc`, `scene.isc`;
- NPC resources including Pacman, Dinky, Operaio and Cocco;
- Pacman is bound to scene entity `pac_pacman`;
- its script logic includes original `start_dialog`, `hide`, `mov`
  and state variables such as `guarito_PacMan` and `parlato_PacMan`.

Stage 7 does **not** hard-code those puzzle outcomes. They remain data-driven
VM work.

## Integration with Stage 6

After `MainPlaceTransitionController` activates Mp1, the engine-side host
should prepare a `GameplayMainPlaceState` with `GameplayMainPlaceLoader`.

The Stage 6 scene picker can feed its selected mesh name to
`GameplayInteractionController::request()`.

The engine host supplies:

- actual player-to-object distance;
- object world position / walk destination;
- `PathFollower` or equivalent character locomotion;
- dispatch back into the existing script VM for the chosen Object handler.

## Current boundary

Stage 7 establishes the complete infrastructure from Mp1 transition to
walk-to-hotspot and Object-handler dispatch.

Still to implement for full fidelity:

1. actor/model loading for Giovanni in the gameplay renderer;
2. mapping the exact retail start helper/vector to his initial 3D transform;
3. room-to-room portals and camera switching;
4. full `operate` / `examine` block execution in the Stage 3 VM;
5. dialogue/audio runtime;
6. inventory/combine semantics;
7. combat/minigame-specific verbs.

## Validation

`stage7_selftest.cpp` exercises:

- BSP parse, closure and shortest path;
- Room resource parsing;
- automatic `r11_Map00.bsp` path resolution;
- Object metadata parsing;
- mesh/entity -> object interaction;
- deferred operation while walking;
- path following;
- transactional start-room gameplay loading.

GitHub Actions syntax-checks Stage 6 and Stage 7 against current ScummVM
headers with `-Wall -Wextra -Werror`, then packages both module directories.
