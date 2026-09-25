#!/usr/bin/env python3
"""Small executable contract distilled from MADERM.EXE + FMPDRV.EXE 1.11."""

def made_state(raw: int) -> int:
    # Exact order used by MADERM: paused bit wins, then RTZ playing mask.
    if raw & 0x01:
        return 2
    if raw & 0x14:
        return 1
    return 0

def z_order(layer_arg: int) -> int:
    return 2 if layer_arg == 2 else 4

def initial_play_subfunc(play_mode: int) -> int:
    # MADERM sends 4 for its looping mode 2, otherwise pause-on-complete 1.
    return 0x0004 if play_mode == 2 else 0x0001

def resume_subfunc(looping: bool) -> int:
    # External 106 adds the 0x0300 high flags to the same low mode nibble.
    return 0x0304 if looping else 0x0301

assert made_state(0x00) == 0
assert made_state(0x01) == 2
assert made_state(0x02) == 0
assert made_state(0x04) == 1
assert made_state(0x10) == 1
assert made_state(0x14) == 1
assert made_state(0x15) == 2  # paused test is evaluated first

assert z_order(2) == 2
assert z_order(0) == 4
assert z_order(1) == 4
assert z_order(3) == 4

assert initial_play_subfunc(1) == 0x0001
assert initial_play_subfunc(2) == 0x0004
assert initial_play_subfunc(3) == 0x0001
assert resume_subfunc(False) == 0x0301
assert resume_subfunc(True) == 0x0304

ALPHA_INDEX = 0
MAGIC_KEY = 0x40044041
assert ALPHA_INDEX == 0
assert MAGIC_KEY == 0x40044041
print('MADERM_FMP_CONTRACT_OK')
