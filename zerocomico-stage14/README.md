# Zero Comico / ScummVM - Stage 14

Stage 14 connects the retail character declarations to the JACS SequenceTable
work started in Stage 13.

Branch: `scratch/zerocomico-stage14`.

## Character script evidence

The retail corpus directly shows forms such as:

```
ge_Character Pacman
AnimSet Walk pac_pacman
SetCharPos_Vector Pacman r12_Start_Pacman
```

The retail executable also contains parser diagnostics immediately adjacent to
these AnimSet field labels:

- `special_standby:`
- `standby:`
- `Blend:`
- `turn:`
- `walk:`
- combat / jump / strafe / look-at fields.

Stage 14 promotes only those labels that are actually present in the
executable string table.

## CharacterScriptParser

The parser now records:

- `ge_Character` declarations;
- one or more `AnimSet <name> <entity>` bindings per character;
- proven AnimSet fields and their opaque argument lists;
- `SetCharPos_Vector <character> <helper>`;
- physical source line numbers, including blank lines.

For the known Mp1 evidence this means the parser can preserve the exact
relationship:

```
Pacman
  -> AnimSet Walk
  -> entity pac_pacman
  -> start helper r12_Start_Pacman
```

No helper coordinate is fabricated; Stage 14 stores the helper reference until
the matching shape/vector resource is decoded.

## Proven locomotion field arity

The executable diagnostics expose the order in which the engine enumerates
values after several fields:

`standby:`

1. standby animation
2. standby-after-run animation

`Blend:`

1. blend-start animation
2. blend-end animation

`turn:`

1. turn-left animation
2. turn-right animation

`walk:`

1. walk **animseq**
2. run **animseq**

That last distinction is important: walking references a JACS sequence, not
necessarily a raw ANJ clip.

`AnimSetMotionProfile` therefore keeps direct animations and sequence names
as different concepts instead of feeding `walk:` straight into the Stage 11
ANJ player.

## Runtime evidence layer

`AnimSetRuntimeEvidence` now validates the two different classes of
locomotion reference separately:

- `standby:` values must resolve directly to decoded ANJ clips;
- `walk:` / run values must occur as exact identifiers in the JACS
  SequenceTable.

A walk/run sequence is deliberately **not** converted to a raw ANJ clip yet.
Even a coincidental equal-name ANJ clip is insufficient evidence because the
retail executable calls these values `animseq`, not `anim`.

This means Stage 11 can already trust direct idle animation evidence, while
walking remains gated on actual SequenceTable transition decoding.

## Stage 13 correlation

`correlateMotionSequences()` checks whether the walk/run sequence identifiers
from the AnimSet occur anywhere in the conservatively parsed SequenceTable.

This is only evidence of a reference. It does not claim that an opaque JACS
line has already been decoded into a transition graph.

The intended chain is now explicit:

```
ge_Character
  -> AnimSet
  -> walk: <walk-seq> <run-seq>
  -> JACS SequenceTable
  -> resolved transition/animation names
  -> ANJ clip
  -> Stage 11 ActorMotionController
```

For idle animation, `standby:` is validated against the decoded ANJ clip
catalog. Walking remains intentionally unresolved until the JACS sequence
semantics are proven.

## Validation

The focused test covers:

- CRLF and blank-line-preserving source locations;
- multiple character blocks;
- `AnimSet Walk pac_pacman`;
- `SetCharPos_Vector Pacman r12_Start_Pacman`;
- standby/blend/turn/walk pair extraction;
- correlation of walk/run sequence identifiers with Stage 13;
- rejection of an unproven third value in a two-value locomotion field;
- direct standby-to-ANJ validation;
- exact walk/run SequenceTable evidence;
- proof that an equal-name ANJ clip is still not auto-promoted to a JACS walk
  sequence.

The previous divergent Stage 14 branch was preserved as
`scratch/zerocomico-stage14-legacy`; the active Stage 14 is rebased on the
latest green Stage 13 line.

CI syntax-checks Stages 6 through 14 with `-Werror`, builds the real ScummVM
`libcommon`, runs the Stage 14 parser/profile test and packages all current
experimental modules.
