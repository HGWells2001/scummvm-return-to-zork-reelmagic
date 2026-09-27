# Zero Comico / ScummVM - Stage 13

Stage 13 begins reverse engineering the JapoTek Animation Control System
(JACS) SequenceTable without pretending the retail `.seq` grammar is already
known.

Branch: `scratch/zerocomico-stage13`.

## Ground truth used

Strings recovered from `japotek3d.dll` prove that the engine owns a
`SequenceTable` and emits these diagnostics/tokens:

- `e3dBody->BuildTables(): Error Building SequenceTable`
- `RunPathSequence`
- `fromseq`
- `start`
- `endseq`
- `end_seq`
- `blending`
- `table`
- transition classes reported as `0>1`, `1>1`, `1>0`

The same DLL explicitly reports missing animation references in:

- a 0→1 **start** transition;
- a 1→1 **sequence** transition;
- a 1→0 **sequence/stop** transition.

The public retail corpus also proves that
`Mpx/bodies/Giovanni/Giovanni.seq` begins with the comment:

```
//JapoTek Animation Control System (JACS)
```

and contains comments such as `//stop2`, `//stopcl,stopcr` and
`//sit,opla`.

## Forensic parser

`SequenceTableForensicParser` is deliberately line-oriented.

It records:

- physical source line number;
- raw line;
- proven directive kind;
- payload after the directive;
- literal transition marker, when the source itself contains `0>1`,
  `1>1` or `1>0`;
- all unrecognized non-comment lines as opaque records.

It also detects the JACS banner and supports exact identifier lookup across
both known and opaque lines.

Crucially, it does **not** infer that an opaque identifier is automatically
an ANJ clip, a stop state, a blend table or a walk sequence.

## Why this matters

Stage 11 already accepts an injected Idle/Walk -> ANJ-clip mapping.
Stage 13 provides the safe front end for the retail `Giovanni.seq`.

Once the actual file bytes can be extracted again, the next step is:

1. feed the decoded retail SequenceTable to this parser;
2. census every opaque line shape;
3. correlate referenced names with the already decoded Giovanni ANJ clips;
4. promote only proven line forms into semantic transition records;
5. use those records to populate Stage 11 motion mappings.

That avoids hardcoding `Walk`, `Standby` or similar guesses.

## Validation

The focused executable self-test checks:

- CRLF parsing;
- blank-line-preserving physical line numbers;
- JACS banner detection;
- all six proven JACS directive tokens;
- all three literal transition classes;
- inline comments;
- quoted `//` text;
- exact identifier matching;
- empty input rejection.

CI syntax-checks Stages 6 through 13 against current ScummVM headers using
`-Werror`, builds the real ScummVM `libcommon`, executes the Stage 13
forensic parser test and packages all experimental modules.
