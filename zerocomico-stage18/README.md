# Zero Comico / ScummVM - Stage 18

Stage 18 adds actual speech playback infrastructure while preserving the
explicit-index boundary established by Stage 15.

Branch: `scratch/zerocomico-stage18`.

## No invented dialogue enumeration

The API deliberately requires:

```
MainPlace + dialogue speaker + active MainPlayer + explicit retail speech index
```

before a sound can be played.

Stage 18 does not increment a per-speaker counter from subtitle order and
does not infer missing indices. If Stage 15 cannot resolve an exact retail
sample, no stream is opened.

## SpeechPlayer

`SpeechPlayer` separates resource access from decoding/playback:

```
DialogueSpeechResource
  -> SpeechStreamHost
  -> SeekableReadStream
  -> SpeechPlaybackBackend
```

The playback backend always takes ownership of the input stream, including
failure paths. This matches ScummVM's MP3 decoder ownership behaviour and
avoids a double-delete if decoder creation fails.

Starting a new sample stops the previous speech handle first.

## Native ScummVM backend

`ScummVMSpeechBackend` uses the current ScummVM APIs:

```
Audio::makeMP3Stream(stream, DisposeAfterUse::YES)
Audio::Mixer::kSpeechSoundType
Audio::Mixer::playStream()
Audio::SoundHandle
```

When ScummVM is built without `USE_MAD`, playback fails explicitly with a
useful error and consumes the input stream. It never pretends an MP3 started.

## Stage 15 dialogue integration

`SpeechPlayerDialoguePlaybackHost` implements the Stage 15
`DialogueSpeechPlaybackHost` interface.

The full runtime path is therefore:

```
DialogueRuntime node
  -> explicit DialogueSpeechIndexProvider
  -> SpeechResourceCatalog
  -> SpeechAwareDialoguePresentation
  -> resolved retail path
  -> SpeechPlayerDialoguePlaybackHost
  -> SpeechPlayer
  -> ScummVM mixer backend
```

The adapter receives only a path that Stage 15 already resolved from an
explicit retail index. It performs no enumeration of its own.

## ExplicitDialogueSpeech

This is the bridge from Stage 15's proven filename/index resolver to actual
playback.

Example:

```
play("Mp1", "MainPlayer", "Giovanni", 5)
  -> Speech/MP1/giovanni0005.mp3
  -> MP3 decoder
  -> kSpeechSoundType
```

The unresolved problem remains **which dialogue events carry which explicit
indices**. That belongs to SpeechTracer reverse engineering and is intentionally
not folded into the player.

## Validation

The focused executable test uses a fake stream host/backend and verifies:

- exact retail index resolution;
- MainPlayer -> Giovanni aliasing;
- missing indices never open a stream;
- resource-open failures;
- backend failure ownership;
- replacement playback stops the previous sample;
- explicit stop clears the current resource;
- Stage 10/15/18 end-to-end dialogue-to-MP3 routing;
- speech stops when the text node finishes.

The previous divergent Stage 18 branch is preserved as
`scratch/zerocomico-stage18-legacy`; the active branch is based on the
latest green Stage 17 line.

All Stage 6–18 sources are syntax-checked with `-Werror`. The native
ScummVM MP3 adapter is therefore checked against current mixer/decoder APIs,
while the focused lifecycle test remains independent of an actual MP3 fixture.
