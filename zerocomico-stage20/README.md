# Zero Comico / ScummVM - Stage 20

Stage 20 carries the numeric `Shape.shp` information decoded in Stage 19
through the character-start and world-opcode runtime layers.

Branch: `scratch/zerocomico-stage20`.

## Transactional placement chain

Stages 16-19 establish:

```
SetCharPos_Vector <character> <helper>
 -> ge_Shape <helper> Position
 -> optional numeric A/B payloads
```

`CharacterStartPlacementLoader` joins those views transactionally. A
published placement contains both the symbolic binding and its exact A/B
records. Failure at any step publishes nothing.

The Mp1 regression uses the retail symbolic chain:

```
SetCharPos_Vector Pacman r12_Start_Pacman
ge_Shape r12_Start_Pacman Position
```

The numeric A/B values in the focused test are synthetic because the public
corpus does not expose the full body of that retail helper.

## Geometry-aware world opcode

`GeometryWorldOpcodeService` upgrades the Stage 17
`SetCharPos_Vector` dispatch. Before calling the world adapter it requires:

1. a symbolic shape declaration;
2. type `Position`;
3. a Stage 19 geometry record with the same helper;
4. matching `Position` type.

The host then receives the character, symbolic definition and exact geometry.

No other world opcode is promoted in Stage 20. In particular
`portals_on/off` remain unhandled because they are not established by the
public retail grammar used for this port.

## Fidelity boundary

Stage 20 still does **not** turn A/B into an actor transform.

It does not assume:

- A = position;
- B = facing target;
- B-A = heading;
- Shape axes equal P3D axes;
- Shape units equal BSP units.

This is intentional. Stage 20 gets the retail numeric evidence all the way to
the renderer/world boundary without hard-coding a plausible interpretation.

## Validation

The executable test verifies:

- transactional char.isc + Shape.shp loading;
- the retail Pacman helper chain;
- numeric A/B surviving the full runtime path;
- geometry-aware `SetCharPos_Vector`;
- Portal rejection;
- missing geometry rejection;
- bad argument counts;
- unsupported portal toggles remain unhandled.

The previous divergent Stage 20 branch is preserved as
`scratch/zerocomico-stage20-legacy`; the active branch is based on the
latest green Stage 19 line.
