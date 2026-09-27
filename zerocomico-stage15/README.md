# Zero Comico / ScummVM - Stage 15

Stage 15 adds the retail speech-resource layer without guessing how dialogue
text is enumerated.

Branch: `scratch/zerocomico-stage15`.

## Ground truth from the executable

The retail executable contains the runtime / speech-enumeration format string:

```
%s/speech/%s/%s%04d.mp3
```

and nearby strings:

```
SpeechEnumeration
//SpeechTracer File Map!MainPlace: %s ... %s // %s
_%04d%s
error reading speech sample: %s
```

The same MP3 path format appears twice in the executable, covering both the
enumeration/tracing and runtime-reading paths.

## Ground truth from the disc

The ISO tree follows that format exactly. Examples include:

```
Speech/MP1/Aldo0000.mp3
Speech/MP1/giovanni0000.mp3
Speech/MP1/giovanni0005.mp3
Speech/MP1/Operaio0002.mp3
Speech/Mp4/Giacomo0001.mp3
```

Mp1 contains exactly:

- Aldo: 1 file
- Giovanni: 6 files
- Operaio: 3 files

The disc also proves that path/name casing is inconsistent: `MP1` vs
`Mp2`, `Aldo` vs `aldo`, `Giacomo` vs `giacomo`.

## SpeechResourceCatalog

The Stage 15 catalog parses any retail speech path ending in:

```
<speaker><four decimal digits>.mp3
```

and stores:

- normalized MainPlace;
- original speaker stem;
- numeric index;
- original resource path.

Lookup is case-insensitive while the original path is preserved for opening.

`buildSpeechResourcePath()` implements the executable's proven
`%s%04d.mp3` naming rule.

## MainPlayer alias

`dialog.isc` may use `MainPlayer`, while the disc stores voice files under
the concrete character name.

`resolveSpeechSpeakerStem()` therefore accepts the active MainPlayer name
from the engine and substitutes it only when the dialogue speaker is literally
`MainPlayer`.

It does not attempt to infer which protagonist is active from subtitle colour
or other heuristics.

## Deliberate boundary: line -> index

Stage 15 does **not** increment a per-speaker counter merely because a
`dialog.isc` line was encountered.

The reason is empirical: the game contains 698 spoken text lines but far fewer
retail speech MP3 files. Some speakers / lines are text-only.

Therefore:

`resolveDialogueSpeechResource()` requires an **explicit retail speech
index**. Given that index, the path is resolved exactly against the catalog.

The remaining reverse-engineering question is the rule or SpeechTracer mapping
that assigns that index to selected dialogue lines. Keeping this boundary
prevents one unvoiced line from shifting every following sample.

## Validation

The Stage 15 focused self-test uses actual retail filenames from Mp1 and checks:

- full-prefix path parsing;
- exact four-digit suffix handling;
- rejection of `.sfk` and malformed names;
- MainPlace normalization;
- case-insensitive speaker lookup;
- deduplication across casing variants;
- the real Mp1 1/6/3 speech counts;
- explicit Giovanni index resolution;
- `MainPlayer -> Giovanni` aliasing only when provided.

CI syntax-checks Stages 6 through 15 with `-Werror`, builds ScummVM's real
`libcommon`, executes the speech catalog test and packages all modules.
