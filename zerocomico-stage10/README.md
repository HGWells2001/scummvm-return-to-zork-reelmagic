# Zero Comico / ScummVM - Stage 10

Stage 10 connects the Stage 8 gameplay VM's `start_dialog` /
`wait_last_dialog` path to a real parser and runtime for the retail
`dialog.isc` files.

Branch: `scratch/zerocomico-stage10`.

## Why text first

The retail data contains far more spoken dialogue lines than speech MP3 files
(for example Mp1 has 123 dialogue lines but only 10 files under
`Speech/MP1`). Therefore Stage 10 deliberately does **not** invent a
one-line-one-MP3 enumeration rule.

The text, speaker table, colours and typing speed are present directly in
`dialog.isc`, so those are implemented first and exactly.

## Dialogue document

`DialogueDocumentParser` reads:

- `speaker NAME KEY R G B SPEED`;
- named `Dialog` blocks;
- direct one-letter speaker lines such as `G "text"`;
- quoted option text inside `BEGIN ... END` choice sections.

Each speech line is resolved from its one-letter key back to the declared
speaker.

## Dialogue runtime

`DialogueRuntime` presents one speech node at a time and remains active
until all sequential lines have been advanced.

Choice text is exposed to the presentation host, but the branch after a
selection is intentionally **not guessed**. After a choice selection the
runtime enters `kDialogueChoiceBranchUnresolved` and remains active. This
keeps `wait_last_dialog` blocked instead of silently choosing the wrong
retail branch.

That is sufficient for ordinary linear conversations such as the first
Pacman interactions while making the remaining choice-control work explicit.

## MainPlace integration

`DialogueMainPlaceLoader` resolves:

```
MpN/gameplay/dialog.isc
```

through the existing Stage 7 decoded-text resource host and loads a
`GameplayDialogueService`.

An engine host can now implement the Stage 8 callbacks as:

```
startDialog(speaker, name) -> dialogueService.startDialog(speaker, name)
isDialogPlaying()          -> dialogueService.isDialogPlaying()
```

so `wait_last_dialog` yields until the text conversation really finishes.

## Audio boundary

Speech playback remains optional until the retail mapping from dialogue
events to the much smaller set of MP3 files is proven. The service API does
not bake in a speculative filename counter.

## Stage 9 status

Stage 9 remains green and already contains:

- JFX1/LZHUF;
- JGF5 BGRA;
- P3D F000/F001/F002/F003;
- recursive ANJ/F044;
- TCB tracks;
- Giovanni ActorModel;
- actor texture loading;
- classic-mesh render catalog;
- per-actor animation runtime.

The unresolved `.seq` SequenceTable remains separate from Stage 10; its
grammar is not guessed here.

## Validation

Stage 10 CI syntax-checks Stages 6 through 10 against current ScummVM headers
using `-Wall -Wextra -Werror` and packages the complete experimental module
set. The focused self-test covers speaker parsing, sequential speech,
choice exposure/conservative blocking, MainPlace path resolution and the
service API used by Stage 8.
