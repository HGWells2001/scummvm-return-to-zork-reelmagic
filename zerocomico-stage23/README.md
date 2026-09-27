# Zero Comico / ScummVM - Stage 23 mainline

Stage 23 mainline applies the Stage 22 transition graph to the live Stage 7
gameplay state without losing the full green Stage 6–22 history.

Branch: `scratch/zerocomico-stage23-mainline`.

## Transactional Room activation

`GameplayRoomTransitionHost` implements
`RoomTransitionRuntimeHost::activateResolvedRoom()`.

For a resolved target Room it:

1. verifies that the Room exists in the active MainPlace registry;
2. cross-checks Stage 22 resource evidence against the Stage 7 descriptor;
3. builds the target navigation path;
4. reads the target BSP;
5. parses and validates it into a temporary `BspNavigation`;
6. commits `activeRoom` + `navigation` only after all checks pass.

If the target BSP is absent, malformed, or the room resources disagree, the
old Room and old navigation graph remain active.

## State deliberately preserved

The gameplay Object registry is MainPlace-wide, so a Room switch does not
reload or clear `puzzle.isc` Objects. The same principle applies to higher
runtime layers such as gameplay variables and dialogue state.

This removes an important source of accidental puzzle resets.

## Current runtime chain

```
picked Portal
  -> Stage 21 exact topology evidence
  -> Stage 22 resolved transition
  -> Stage 23 transactional target Room
  -> target BSP/path graph
  -> active Room commit
```

Camera/object-scene asset loading is still a renderer-side transaction and
will be layered on top of this state transition rather than guessed here.

## Validation

The focused executable test verifies:

- Room1_1 -> Room1_2 activation;
- target BSP replacement;
- camera/map descriptor propagation;
- MainPlace-wide Objects survive the switch;
- missing target BSP rolls back;
- malformed target BSP rolls back;
- resource mismatch rolls back;
- unknown target Room is rejected.

CI syntax-checks Stages 6 through 23 mainline with `-Werror`, builds
ScummVM's real `libcommon`, executes the Room transition host test and
packages the clean module chain.
