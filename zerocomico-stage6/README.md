# Zero Comico / ScummVM - Stage 6

Stage 6 turns the Stage 5 rendered main menu into an interactive runtime layer.

This directory is intentionally isolated from the Return to Zork work in this
repository. It is a scratch workspace while the normal local container runtime
is unavailable. The repository's `main` branch is not changed.

## Implemented in this stage

### Pixel-accurate scene picking

`ScenePicker` adds an integer object-id buffer beside the renderer's depth
buffer. Whenever a triangle fragment wins the normal depth test, the renderer
writes the stable scene-registry id to the same pixel.

This means menu clicks are resolved against the surface that is actually
visible at the mouse pixel. No 2D hand-authored rectangles and no approximate
screen-space bounding boxes are required.

### ScummVM mouse bridge

`MenuInput` consumes:

- `EVENT_MOUSEMOVE`
- `EVENT_LBUTTONDOWN`
- `EVENT_LBUTTONUP`

A click is latched only when press and release resolve to the same scene
object. The selected object name is then available to the script VM.

### Original `ifobjselected` shape

The sibling Lucifer-engine game Blood & Lace proves the original statement is
two-argument:

```
ifobjselected <owner/context> <object>
```

Stage 6 therefore exposes both the compatibility one-argument helper and the
faithful two-argument form. The context is preserved in the API; current menu
selection uses the second argument, the actual selected scene object.

### Menu hover from original assets

The retail menu stores button states as paired resources:

```
*_spe.tga   // spento
*_acc.tga   // acceso
```

Stage 6 derives these names generically. It does not hard-code NUOVO, CARICA,
SALVA, AIUTI, CREDITS, CONTINUA or ABBANDONA.

`MenuController` switches only the hovered object's texture override and
restores the previous object. `MenuMaterialState` provides the small material
API needed by the Stage 5 renderer.

### TCB animation runtime

The ANJ data decoded in Stage 4 is now consumable at runtime:

- Kochanek-Bartels TCB interpolation for vec3 translation;
- Kochanek-Bartels TCB interpolation for vec3 scale;
- axis-angle to quaternion conversion;
- quaternion slerp between rotational keys;
- visibility-key evaluation;
- named animation clips and per-object tracks;
- looped and one-shot playback.

The rotation path is deliberately marked as an approximation at one precise
point: the retail engine's exact quaternion TCB/squad tangent construction has
not yet been recovered from japotek3d.dll. The public API is shaped so the
exact evaluator can replace slerp later without changing call sites.

### Stable scene registry

`SceneRegistry` owns transformable scene objects by stable name/id. Materials
are deliberately excluded from this registry because Stage 5 proved that F000
materials and F003 meshes can share the same name.

`SceneRuntime` combines:

- script visibility;
- base transforms;
- evaluated ANJ transforms;
- active focus camera;
- named animation playback;
- deferred MainPlace transition requests.

### MainPlace transition infrastructure

`parseMainPlaceDescriptor()` reads the already-decoded `gameplay/room.isc`
structure and extracts:

- `ge_MainPlace`
- `StartPlace:`
- declared `Room` entries

The same mechanism is intended for Mp1 through Mp5 instead of hard-coding room
names.

A requested transition is deferred until a safe point in the engine loop. The
next integration step is therefore:

1. VM requests MainPlace change;
2. current frame finishes;
3. runtime unloads current scene;
4. decode target `room.isc` and `scene.isc`;
5. parse MainPlace descriptor;
6. load its declared StartPlace;
7. build the new scene registry.

## Stage 5 integration points

### Renderer

At framebuffer creation:

```cpp
picker.resize(640, 480);
```

At the beginning of each rendered frame:

```cpp
picker.clear();
```

When a rasterized fragment wins the normal z-test:

```cpp
picker.writePixel(x, y, depth, sceneObjectId);
```

The same stable `sceneObjectId` must identify that P3D mesh in
`SceneRegistry`.

### Renderer texture lookup

Bind the mesh/object to its normal material texture once:

```cpp
menuMaterials.bindObjectTexture(meshName, materialTexture);
```

For actual rendering, ask:

```cpp
menuMaterials.effectiveTexture(meshName)
```

instead of reading the base texture directly. Non-menu objects simply have no
override and therefore render normally.

### Input loop

Feed ScummVM events after the pick buffer represents the currently displayed
frame:

```cpp
menuController.handleEvent(event, picker);
```

### Script VM

The existing Stage 3 VM should route the corresponding original verbs through
`ScriptBridge`:

```
ifobjselected owner object
e3d_Hide object
e3d_UnHide object
SetFocus camera
<animation-start verb>
<animation-wait verb>
<MainPlace-transition verb>
```

The exact spelling/argument shape of the last three menu statements must come
from the retail `Mpx/Interface.isc` rather than being invented.

## Focused self-test

`stage6_selftest.cpp` covers:

- depth/id replacement in `ScenePicker`;
- press+release selection on the same mesh;
- a simple TCB interpolation midpoint;
- MainPlace descriptor parsing;
- script visibility and deferred MainPlace requests.

A branch-only GitHub Actions job also syntax-checks every Stage 6 C++ source
against the current ScummVM headers with `-Wall -Wextra -Werror`.

## Known boundaries

1. **Retail `Interface.isc` opcode spelling**
   - The Stage 5 archive contains the original decoded-data work, but the local
     archive runtime is currently unavailable.
   - Public documentation does not publish the full file.
   - Therefore Stage 6 does not fabricate the exact animation/transition
     statement syntax.

2. **Exact rotational TCB**
   - Vec3 TCB is implemented.
   - Rotations currently interpolate key quaternions with slerp.
   - Recovering the original quaternion tangent construction remains a fidelity
     task.

3. **ANJ special light channel `0x0E3D`**
   - Stage 5 resolves 261/263 P3D/ANJ pairs completely.
   - `c476.anj` and `c478.anj` share this remaining special case.

4. **Full Mp1 gameplay**
   - Stage 6 provides the safe transition machinery.
   - Loading and executing the complete Mp1 room/character/puzzle script set is
     the next milestone after the exact menu transition statement is recovered.

## Files

- `scene_picker.*` - z-buffer paired object picking
- `menu_input.*` - ScummVM mouse to selected object
- `menu_highlight.*` - generic *_spe / *_acc resource pairing
- `menu_controller.*` - hover controller
- `menu_material_state.*` - transient material overrides
- `timeline_eval.*` - TCB/slerp evaluation
- `animation_player.*` - named ANJ clip playback
- `scene_runtime.*` - registry and runtime state
- `script_bridge.*` - VM/runtime boundary
- `mainplace.*` - data-driven MainPlace descriptor parsing
- `stage6_selftest.cpp` - focused regression checks

