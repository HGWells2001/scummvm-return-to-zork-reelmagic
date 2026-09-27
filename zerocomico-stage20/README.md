# Zero Comico / ScummVM - Stage 20

Stage 20 carries the numeric `Shape.shp` information decoded in Stage 19 through the character-start and world-opcode runtime layers.

Branch: `scratch/zerocomico-stage20`.

## CharacterStartPlacement

Stage 16 resolved the symbolic chain:

```
SetCharPos_Vector Giovanni R12pos
  -> ge_Shape R12pos Position
```

Stage 19 decoded the Position helper's A/B payload when it is an exact three-number vector.

Stage 20 joins both transactionally:

```
char.isc
Shape.shp symbolic parser
Shape.shp numeric A/B parser
  -> CharacterStartPlacement
```

The published placement therefore contains:

- the original `SetCharPos_Vector` binding;
- the validated Position shape definition;
- its numeric/opaque A record;
- its numeric/opaque B record.

If any required parse or binding fails, no partial placement is published.

## Geometry-aware world opcodes

`GeometryWorldOpcodeService` supersedes the Stage 17 service for `SetCharPos_Vector`.

Before calling the engine/world adapter it validates both views of the helper:

1. symbolic Shape.shp definition exists;
2. symbolic type is `Position`;
3. Stage 19 geometry record exists;
4. geometry record is also `Position`.

Only then does the host receive:

```
character name
ShapeDefinition
ShapeGeometryRecord
```

so the renderer has the exact retail A/B payload available.

`portals_on` and `portals_off` remain supported.

## Important fidelity boundary

Stage 20 still does **not** turn A/B into an actor transform.

The host sees both vectors and is responsible for interpreting them only after the original engine semantics are proven.

In particular Stage 20 does not assume:

- A = position;
- B = facing target;
- B-A = heading;
- P3D axes equal Shape.shp axes;
- Shape.shp units equal BSP units.

This preserves the new numeric evidence without silently baking a plausible but unverified coordinate convention into the engine.

## Validation

The focused test verifies:

- transactional `char.isc + Shape.shp` loading;
- numeric A/B surviving the full start-position chain;
- geometry-aware `SetCharPos_Vector` dispatch;
- Portal rejection;
- missing-geometry rejection;
- bad argument counts;
- portal enable/disable forwarding.

CI syntax-checks Stages 6 through 20 with `-Werror`, builds ScummVM's real `libcommon`, runs the Stage 20 test and packages all current modules.
