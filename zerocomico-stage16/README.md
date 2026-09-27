# Zero Comico / ScummVM - Stage 16

Stage 16 resolves the retail character start-position reference without
inventing its numeric coordinate semantics.

Branch: `scratch/zerocomico-stage16`.

## Retail chain now resolved

The game scripts contain:

```
SetCharPos_Vector <character> <helper>
```

and the shape script declares helpers through:

```
ge_Shape <name> Position
```

Stage 16 connects those two structures.

For a character the runtime can now prove:

```
character
  -> SetCharPos_Vector
  -> helper name
  -> Shape.shp declaration
  -> ge_Shape ... Position
```

A Portal, Range, Entity or unknown shape is rejected as a character start
helper.

## ShapeScriptParser

The parser currently promotes only top-level forms already established by the
retail script corpus:

- `ge_Shape ... Position`
- `ge_Shape ... Portal`
- `ge_Shape ... Range`
- `ge_Shape ... Entity`
- `ge_Vector ...`

All declaration-tail tokens are preserved but deliberately remain opaque.

That distinction matters because the public corpus does not expose enough
retail Shape.shp body text to prove which token/field is the numeric world
position.

## Transactional loader

`CharacterStartBindingLoader` reads:

```
MpN/gameplay/char.isc
MpN/gameplay/Shape.shp
```

through the existing decoded-text resource host and publishes the binding only
after both scripts parse and the referenced helper is proven to be a Position.

## Remaining boundary

Stage 16 resolves **which** retail shape positions Giovanni, but not yet the
three-dimensional value represented by that shape.

The next fidelity step is to recover the Position shape body or helper P3D
payload and then convert it to:

```
CharacterStartBinding
  -> proven position/orientation
  -> GameplayRuntime::setActorStartPosition()
  -> actor-root transform
```

No zero-vector or hand-authored room coordinate is used as fallback.

## Validation

The focused self-test checks:

- Shape.shp Position / Portal / Vector parsing;
- case-insensitive helper lookup;
- SetCharPos_Vector resolution;
- MainPlace path construction;
- rejection of a Portal used as a start Position;
- transactional loading of char.isc + Shape.shp.

CI syntax-checks Stages 6 through 16 with `-Werror`, builds ScummVM's real
`libcommon`, runs the Stage 16 test and packages the complete experimental
module set.
