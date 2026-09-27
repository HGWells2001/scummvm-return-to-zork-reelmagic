# Zero Comico / ScummVM - Stage 17

Stage 17 adds a safe extension point for gameplay opcodes that affect the
world rather than local puzzle state.

Branch: `scratch/zerocomico-stage17`.

## VM external-opcode hook

The Stage 8 Object handler VM now accepts an optional
`ObjectHandlerExternalOpcodeHost`.

Execution order is:

1. built-in puzzle/control-flow opcodes;
2. Stage 6 menu/scene opcodes;
3. optional external gameplay/world opcode host;
4. otherwise block on the unknown opcode.

Unsupported commands are therefore still visible. The VM does not skip them
and the program counter remains on the blocked instruction.

Existing Stage 8 callers remain source compatible because the new host
parameter defaults to null.

## Gameplay host routing

`GameplayRuntimeHost` now implements the external-opcode interface with a
default “unhandled” response.

`GameplayRuntime` supplies its real host when starting an Object handler, so
world commands inside retail `operate:` / `examine:` bodies can reach the
engine layer.

`IntegratedGameplayHost` can forward those commands to a separately bound
service. This keeps actor rendering, dialogue and world manipulation
independent.

## Proven world opcodes

`WorldOpcodeService` currently implements only:

```
SetCharPos_Vector <character> <helper>
portals_on
portals_off
```

For `SetCharPos_Vector`, Stage 17 resolves the helper against the Stage 16
`ShapeScriptDocument` and rejects anything that is not explicitly declared
as:

```
ge_Shape <helper> Position
```

Only after that validation does it call the renderer/world adapter.

The adapter still owns the final numeric Position-shape interpretation because
the public corpus does not yet prove that layout.

## Deliberately blocked commands

The retail executable proves the existence of additional world commands,
including:

- `SetCharRotation_Vector`
- `SetY_Vector`
- `SetEntityPos_Vector`
- `setplace`
- `csetplace`
- `setmap`
- `SetCameraMode`
- `setcamera`
- `setfocus`

Stage 17 does not assign argument signatures to these without a retail usage
example. They remain blocked by the VM.

This is preferable to silently moving a character, changing room or selecting
a camera with the wrong argument order.

## Validation

The executable self-test verifies:

- Position-shape validation;
- rejection of a Portal as a Position;
- `portals_off` / `portals_on`;
- VM dispatch through the new external hook;
- exact program-counter blocking on an unsupported `setplace`;
- repeated updates remain blocked on the same opcode;
- bad argument counts return `kObjectHandlerVmBadArguments`.

CI syntax-checks Stages 6 through 17 using `-Werror`, builds ScummVM's real
`libcommon`, runs the Stage 17 VM/world test and packages all experimental
modules.
