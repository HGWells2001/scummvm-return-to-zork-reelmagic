# Zero Comico / ScummVM - Stage 12

Stage 12 creates the first backend-neutral render frame for Giovanni.

Branch: `scratch/zerocomico-stage12`.

## What is now render-ready

The Stage 9 P3D loader already proves classic F003 vertex/index/UV/material
layouts. Stage 9 also decodes Giovanni's ANJ transform tracks. Stage 12 joins
those two data streams.

`ActorRenderFrameBuilder` emits one world-space triangle record containing:

- three transformed positions;
- per-corner UVs when present;
- mesh and source triangle indices;
- material index/name;
- original texture resource name;
- visibility filtering from `SceneRegistry`.

The transform order is:

```
P3D classic vertex
 -> current ANJ mesh transform
 -> injected actor-root transform
 -> world-space render vertex
```

The actor-root transform is intentionally supplied by the engine. This is the
hook where the proven BSP floor position can later be converted to the
renderer's world coordinate system.

## What remains deliberately outside

Stage 12 does **not** silently infer:

- the semantic role of the 3x3 matrix stored in each P3D mesh record;
- P3D coordinate handedness;
- whether camera FOV is vertical or horizontal;
- the BSP x/y to P3D world-axis mapping;
- shared/deformer mesh ownership.

Those are the remaining fidelity questions before final pixel projection.

## Renderer boundary

`ActorTriangleSink` is a small rendering interface:

```
beginActor()
submitTriangle(triangle, texture)
endActor()
```

`ActorRenderSubmitter` resolves the Stage 9 `ActorTextureSet` material
binding and sends the render frame to that sink.

This means a ScummVM software rasterizer, TinyGL backend, debug wireframe
renderer or capture tool can consume exactly the same Giovanni geometry.

`GameplayActorRenderer` connects this to the Stage 11
`IntegratedGameplayHost`, so the live animated actor, textures and render
catalog stay synchronized with gameplay.

## Validation

The Stage 12 executable self-test constructs a synthetic classic P3D triangle,
then verifies numerically:

- non-uniform render metadata survives;
- scale is applied;
- a 90-degree quaternion rotation is applied;
- mesh translation is applied;
- actor-root translation is applied afterwards;
- all three UV corners remain correctly indexed;
- hidden meshes produce no triangles;
- an invisible actor root produces no triangles;
- malformed UV tables are rejected instead of read out of bounds.

CI syntax-checks Stages 6 through 12 with `-Werror`, builds ScummVM's real
`libcommon`, executes the transform/render-frame test and packages all
experimental modules.
