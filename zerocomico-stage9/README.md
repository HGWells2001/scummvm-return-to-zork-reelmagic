# Zero Comico / ScummVM - Stage 9

Stage 9 moves the shared protagonist from an opaque resource bundle to typed
JapoTek model and animation data that can be consumed by the renderer.

Branch: `scratch/zerocomico-stage9`.

## P3D model loader

`P3DModelParser` implements the layouts recovered in Stages 3 and 4:

- F000 materials, including the 48/92-byte texture-resource forms;
- F001 cameras;
- F002 lights and optional linked-name table;
- all three retail F003 storage forms:
  - classic;
  - shared-vertex (`0x10000`);
  - deformer (`0x20000`);
- optional F003 UV data (`0x20`);
- optional structurally decoded auxiliary data (`0x40`).

Classic mesh vertices use the original loader operation already recovered from
`japotek3d.dll`:

```
loadedVertex = rawVertex + vectorB - headerVector
```

The unsupported `0x80000000` F003 branch is rejected explicitly. The earlier
retail corpus analysis found no Zero Comico P3D using it.

### Record terminator correction

The current cross-disc structural parser establishes:

- named record terminator: **`ED FF FF`**;
- file trailer: **`00 ED FF FF`**;
- F044 group: authoritative u32 length.

This supersedes the earlier Stage 2 wording that treated the four-byte trailer
as every record's terminator.

## Recursive ANJ document

`ANJDocumentParser` implements the corrected F044 group walk:

```
BB AA 44 F0
u32 length
children...
```

Named-record boundaries use constrained backtracking: a candidate
`ED FF FF` is accepted only when the remainder of the enclosing range also
tiles. This protects against terminator-like bytes inside float data.

The parser exposes:

- complete flattened record graph with nesting/parent groups;
- 68-byte object bindings;
- F007 timeline headers;
- animation name, duration, first/last frame and payload range.

## TCB animation decoding

`ANJTrackDecoder` resolves each F007 target by **target name against the P3D
scene/object table**, not just by the surrounding F044 type.

This preserves the Stage 5 fix for mixed timelines such as `c416.anj`.

Implemented target schemas:

- F003 / F032: translation + scale + axis-angle rotation + visibility;
- F011: one vec3 channel (camera target);
- F001: vec3 + two scalar channels (camera);
- F002: two vec3 channels (light);
- F022: structurally one vec3 channel, semantic class still intentionally
  unnamed.

The key formats are the layouts recovered earlier:

- vec3 TCB key: 28 bytes;
- scalar TCB key: 20 bytes;
- axis-angle TCB key: 32 bytes;
- visibility key: frame + boolean.

Transform tracks are emitted directly as Stage 6 `AnimationClip` /
`ObjectTimeline` data, so the existing TCB interpolation code can play them.

## Combined Giovanni ActorModel

`ActorModelLoader` now performs one transaction:

```
Mpx/bodies/Giovanni/Giovanni.p3d
      -> P3DModel

Mpx/bodies/Giovanni/Giovanni.anj
      -> ANJDocument
      -> AnimationClip[]
```

Nothing is published if either side fails.

`registerActorSceneObjects()` registers mesh, camera, light and transform
target names in the common SceneRegistry.

## Per-actor animation player

`ActorAnimationRuntime` gives the protagonist a dedicated
`AnimationPlayer`. A Giovanni walk therefore no longer has to share the
single Stage 6 scene/cutscene player.

The remaining missing link is the retail `.seq` mapping that says which ANJ
clip(s) make up high-level states such as Walk/Standby. The DLL proves a
SequenceTable and contains the tokens `fromseq`, `start`, `endseq` /
`end_seq`, and blending-table diagnostics, but the public reverse-engineering
repository does not contain the decoded `.seq` bodies. Stage 9 therefore
does **not** guess that grammar.

## Renderer handoff

`ActorRenderCatalog` converts classic F003 meshes into render batches:

```
mesh
 -> triangle range
 -> material group
 -> P3D material
 -> texture resource
```

Shared/deformer meshes remain loaded losslessly but are not falsely
rasterized before their ownership/deformation relationship is proven.

## Validation

The Stage 9 workflow syntax-checks Stages 6 through 9 against current ScummVM
headers with:

```
-std=c++17 -Wall -Wextra -Werror
```

`stage9_selftest.cpp` also constructs a small valid P3D and ANJ in memory and
covers the intended path:

```
P3D -> ANJ F044/F007 -> target resolver -> TCB keys
    -> AnimationClip -> render catalog
```

## Next boundary

For the first visually animated Giovanni in Mp1:

1. replay the Stage 9 parser against the retail Giovanni P3D/ANJ once the
   uploaded archive is available to the execution runtime again;
2. link classic render batches to the Stage 5 software rasterizer;
3. resolve shared/deformer F003 ownership for the actor;
4. decode the retail `Giovanni.seq` SequenceTable mapping;
5. resolve the exact retail start helper/vector and BSP-to-P3D axis mapping;
6. bind Stage 8 `setActorWalking()` to the decoded Walk sequence.
