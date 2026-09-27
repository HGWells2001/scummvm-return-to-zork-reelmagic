# Zero Comico / ScummVM - Stage 6

Stage 6 turns the Stage 5 rendered main menu into an interactive runtime layer.

This directory is isolated on the scratch branch `scratch/zerocomico-stage6`.
The Return to Zork repository's `main` branch is untouched.

## Implemented

### Pixel-accurate picking

`ScenePicker` keeps an object-id buffer paired with the renderer's z-buffer.
When a fragment wins the depth test, its stable scene-object id is stored at the
same pixel. Mouse selection therefore resolves the actually visible mesh, not a
hand-authored rectangle.

### Mouse and menu hover

`MenuInput` handles ScummVM mouse move/down/up events and latches a click only
when press and release hit the same object.

The retail menu contains paired button textures:

```
*_spe.tga   // spento
*_acc.tga   // acceso
```

`MenuController` and `MenuMaterialState` switch these original resources on
hover without hard-coding the seven button names.

### Original Lucifer VM verbs recovered

The shipped `Zero Comico.exe` command table exposes the exact runtime names
used by this engine, including:

```
setfocus
E3D_hide
E3D_unhide
ifobjselected
play_open_cut
play_cut
loop_cut
wait_cut
stop_cut
if_cutisfinished
ChangeMainplace
ChangeMainplaceTTable
SaveMainplace
RestoreMainplace
GetSavedMainplace
```

The sibling Lucifer-engine title Blood & Lace proves `ifobjselected` has the
two-argument shape:

```
ifobjselected <owner/context> <object>
```

`ScriptBridge` and `script_opcodes.*` now expose/dispatch the relevant Stage
6 subset with these original names. The Stage 3 VM can pass its already
tokenized opcode and arguments directly to this adapter.

### ANJ animation runtime

The ANJ data decoded in Stage 4 can now run at runtime:

- Kochanek-Bartels TCB interpolation for translation and scale;
- axis-angle to quaternion conversion;
- quaternion slerp between rotation keys;
- visibility-key evaluation;
- named one-shot and looping clips;
- named cut-state queries for `wait_cut` and `if_cutisfinished`.

The exact retail quaternion-TCB/squad tangent construction is still a fidelity
task. Rotation currently uses slerp at the one documented substitution point.

### Stable scene registry

`SceneRegistry` owns transformable scene objects by stable name/id. Materials
remain separate because Stage 5 proved an F000 material and an F003 mesh may
share the same name.

`SceneRuntime` combines:

- script visibility;
- base transforms;
- evaluated ANJ transforms;
- active focus camera;
- active named cut;
- deferred MainPlace requests.

### Transactional MainPlace transition

`ChangeMainplace Mp1` is now represented by the real VM verb, not a made-up
engine API.

`MainPlaceTransitionController` performs the transition at a safe point:

1. keep the current scene alive;
2. read/decode the requested `MpN/gameplay/room.isc`;
3. parse `ge_MainPlace`, `StartPlace:` and all declared `Room` entries;
4. reject a mismatched/invalid target before touching the current scene;
5. ask the engine host to prepare and atomically activate the target;
6. clear the pending transition only after success.

The same mechanism applies to Mp1 through Mp5.

## Stage 5 integration points

### Renderer

At framebuffer creation:

```cpp
picker.resize(640, 480);
```

At frame start:

```cpp
picker.clear();
```

Whenever a rasterized fragment wins the Stage 5 z-test:

```cpp
picker.writePixel(x, y, depth, sceneObjectId);
```

### Texture lookup

Bind the base material texture once:

```cpp
menuMaterials.bindObjectTexture(meshName, materialTexture);
```

Render using:

```cpp
menuMaterials.effectiveTexture(meshName)
```

so an original `*_acc.tga` hover override can be temporary.

### Event loop

After the pick buffer represents the displayed frame:

```cpp
menuController.handleEvent(event, picker);
```

### Script VM

Route statements/conditions through `script_opcodes.*` first. A returned
`kStage6OpcodeUnhandled` falls back to the existing Stage 3 VM.

Important mappings already implemented:

```
E3D_hide object
E3D_unhide object
setfocus camera
play_open_cut name
play_cut name
loop_cut name
wait_cut name
stop_cut name
ChangeMainplace Mp1
ifobjselected owner object
if_cutisfinished name
```

`wait_cut` returns a yield result while the named cut is active.

## Validation

The branch CI clones current ScummVM and syntax-checks every Stage 6 C++ source
with:

```
-std=c++17 -Wall -Wextra -Werror
```

The current Stage 6 head is green.

`stage6_selftest.cpp` additionally describes regression checks for:

- z/id picking precedence;
- press/release selection on the same mesh;
- TCB interpolation;
- named `play_cut CLICK` / `wait_cut CLICK`;
- MainPlace descriptor parsing;
- the exact logical path
  `ChangeMainplace Mp1 -> parse StartPlace -> activate Mp1`.

GitHub Actions also builds a downloadable source ZIP artifact after successful
validation.

## Known boundaries

1. **Full retail `Mpx/gameplay/Interface.isc` contents**
   - The exact command vocabulary is now recovered from the executable.
   - The complete menu script is still needed to preserve the exact order in
     which `CLICK`, `SARACSU`, `SARACGIU`, `MAINESCE`, `MAINENTR` and
     other cuts are invoked by each button.
   - No guessed button sequence is hard-coded.

2. **Exact rotational TCB**
   - vec3 TCB is implemented.
   - rotation uses slerp until the DLL's quaternion tangent construction is
     recovered.

3. **ANJ special light channel `0x0E3D`**
   - Stage 5 resolves 261/263 P3D/ANJ pairs completely;
   - `c476.anj` and `c478.anj` retain this special case.

4. **Full Mp1 gameplay**
   - the safe Mp0 -> Mp1 transition path now exists;
   - the next major milestone is loading the complete Mp1 room, character,
     navigation and puzzle runtime after transition.

## Files

- `scene_picker.*` - z-buffer paired object picking
- `menu_input.*` - ScummVM mouse to selected object
- `menu_highlight.*` - generic *_spe / *_acc resource pairing
- `menu_controller.*` - hover controller
- `menu_material_state.*` - transient material overrides
- `timeline_eval.*` - TCB/slerp evaluation
- `animation_player.*` - named ANJ playback
- `scene_runtime.*` - registry and runtime state
- `script_bridge.*` - VM/runtime boundary
- `script_opcodes.*` - recovered Lucifer opcode adapter
- `mainplace.*` - data-driven MainPlace descriptor parser
- `mainplace_transition.*` - safe deferred MainPlace activation
- `stage6_selftest.cpp` - focused regression checks
