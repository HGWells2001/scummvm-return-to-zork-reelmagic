# Zero Comico / ScummVM - Stage 19

Stage 19 begins decoding the numeric body of retail `Shape.shp` helpers
without assigning unproven semantics to the two endpoint records.

Branch: `scratch/zerocomico-stage19`.

## Ground truth

The public retail grammar census shows that `.shp` has only four
body/declaration-leading forms:

- `ge_Shape`
- `ge_Polygon`
- `A`
- `B`

The executable also names the shape kinds:

- `Position`
- `Portal`
- `Range`
- `Entity`

and Stage 16 already proves the chain:

```
SetCharPos_Vector <character> <helper>
  -> ge_Shape <helper> Position
```

Stage 19 adds the next safe layer.

## ShapeGeometryParser

For every `ge_Shape`, the parser records the following `A` and `B`
lines until a new top-level `ge_Shape`, `ge_Polygon` or `ge_Vector`
begins.

Each A/B payload is preserved losslessly as tokens.

It is promoted to a numeric `ShapeVec3` **only** if the line contains
exactly three syntactically valid floating-point values.

Accepted numeric forms include signs, decimals and scientific notation.

Malformed or differently shaped records remain opaque instead of being
coerced.

## Semantics deliberately not assigned

Stage 19 does **not** claim that:

- A is the character position;
- B is the facing direction;
- B-A is a heading vector;
- either vector uses the same axes as P3D;
- the values are already in BSP floor coordinates.

Those are plausible interpretations, but not yet proven by the available
retail evidence.

The runtime can now know the exact numeric A/B payload of a Position helper
while keeping the final world mapping behind an explicit boundary.

## Character start geometry

`resolveCharacterStartGeometry()` connects a proven Position helper name to
its decoded A/B geometry and rejects Portal/Range/Entity shapes.

The intended chain is now:

```
char.isc SetCharPos_Vector
  -> Stage 16 Position helper binding
  -> Shape.shp ge_Shape Position
  -> Stage 19 numeric A/B records
  -> future proven A/B semantics
  -> GameplayRuntime actor root
```

## Validation

The focused executable test covers:

- CRLF physical source locations;
- signed integers and decimals;
- scientific notation;
- leading-dot decimals;
- inline comments;
- case-insensitive shape lookup;
- opaque nonnumeric A/B preservation;
- ge_Polygon boundary isolation;
- Position-only start resolution;
- duplicate A/B rejection;
- malformed numeric tokens staying opaque.

CI syntax-checks Stages 6 through 19 with `-Werror`, builds ScummVM's real
`libcommon`, executes the Stage 19 geometry test and packages all current
experimental modules.


## Mp1 regression target

The helper name used by the focused regression is the retail Mp1 chain already
proven by Stages 16-17:

```
SetCharPos_Vector Pacman r12_Start_Pacman
ge_Shape r12_Start_Pacman Position
```

The numeric A/B values in the focused unit fixture remain synthetic because
the public corpus does not expose the full retail body of that shape. Stage 19
therefore tests numeric parsing without claiming those synthetic numbers are
Pacman's real coordinates.

The previous divergent Stage 19 branch is preserved as
`scratch/zerocomico-stage19-legacy`; the active branch is based on the
latest green Stage 18 line.
