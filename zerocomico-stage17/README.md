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

Unsupported commands remain visible and the VM program counter stays parked
on the same instruction.

`GameplayRuntimeHost` implements the external interface with an unhandled
default, while `IntegratedGameplayHost` can forward it to a separately
bound world service.

## Retail-grounded world opcode

The public retail grammar counts **78 uses** of:

```
SetCharPos_Vector
```

and Mp1 specifically proves:

```
SetCharPos_Vector Pacman r12_Start_Pacman
ge_Shape r12_Start_Pacman Position
```

`WorldOpcodeService` therefore implements exactly:

```
SetCharPos_Vector <character> <helper>
```

The helper must exist in the Stage 16 `ShapeScriptDocument` and must be a
`Position` shape before control reaches the renderer/world adapter.

## Commands intentionally not promoted

Although additional world-related strings exist in the executable, the public
retail grammar used here does not establish the same argument contract for
commands such as:

- `portals_on` / `portals_off`;
- `setplace` / `csetplace`;
- `setmap`;
- `SetCameraMode`;
- `setcamera`.

They stay unhandled. The VM blocks on them rather than silently guessing
argument order or state changes.

## Remaining boundary

A Position shape is now resolved by name, but its numeric coordinate/orientation
body is still opaque. The world adapter must not invent coordinates.

## Validation

The executable test verifies:

- the actual Mp1 Pacman helper name;
- Position-shape validation;
- rejection of a Portal;
- VM dispatch through the external hook;
- blocking on unsupported `setplace`;
- repeated updates stay on the same instruction;
- malformed `SetCharPos_Vector` arguments stop at the same program counter.

The old divergent branch is preserved as
`scratch/zerocomico-stage17-legacy`; the active branch is based on the
latest green Stage 16 line.
