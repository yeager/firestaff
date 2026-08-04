# Nexus V1 Water/Fire Crossing Gate — DM.BIN Disassembly Evidence

## Summary

Disassembled the SH-2 binary DM.BIN (555,144 bytes, base 0x06010000) to find the
movement-path crossing gate logic for water (type 21) and fire (type 22) squares.

## Water (type 21) — No Crossing Gate

All four CMP/EQ #21,R0 (opcode 0x8815) sites in DM.BIN are rendering or dispatch
paths, not movement passability checks:

| Address      | File offset | Purpose                              |
|-------------|-------------|--------------------------------------|
| 0x0601CE40  | 0x00CE40    | Rendering: tile index selection (R4=42 vs 41) |
| 0x06020E3A  | 0x010E3A    | Flag-OR on struct init, routes to handler      |
| 0x0602CBAA  | 0x01CBAA    | Square setup dispatcher               |
| 0x0602CD52  | 0x01CD52    | Square setup dispatcher (leaf)        |

**Conclusion**: Water squares have no explicit crossing gate in the movement path.
They are always passable. Damage effects may be applied on entry but movement is
not blocked.

## Fire (type 22) — Conditional Gate at 0x0603C386

Three CMP/EQ #22,R0 (opcode 0x8816) sites found. The crossing gate is at
0x0603C386 (file offset 0x02C386):

```
0x0603C384: MOV.L @(4,R15),R0    ; load square type from stack
0x0603C386: CMP/EQ #22,R0        ; is it fire?
0x0603C388: BF 0x0603C398        ; if NOT fire, skip
0x0603C38A: MOV.W @(310,R9),R0   ; load attribute word at offset 310
0x0603C38C: EXTU.W R0,R0         ; zero-extend to 32 bits
0x0603C38E: TST #1,R0            ; test bit 0 (fire protection flag)
0x0603C390: BF 0x0603C398        ; if bit 0 IS set → passable (skip gate)
0x0603C392: SHAR R14             ; R14 >>= 1 (halve damage/counter)
0x0603C394: TST R14,R14          ; is R14 now zero?
0x0603C396: BT 0x0603C44C        ; if zero → branch to handler
```

**Semantics**: Bit 0 of a 16-bit attribute word at offset 310 from a state
pointer (R9) acts as the fire protection flag. When set, the party can cross
fire squares. When clear, the square blocks movement (or applies damage until
a counter is exhausted).

Other two fire sites are rendering configuration (0x0601C28C) and event flag
writing (0x0603C0C4).

## Implementation

- `nexus_square_is_passable()`: only wall (type 0) is unconditionally impassable.
  Water is always passable. Fire passability is conditional.
- `nexus_mechanics_tick()`: fire squares block when `fire_shield_ticks <= 0`.
  Water squares pass through unconditionally.
- `Nexus_MechanicsState.fire_shield_ticks`: decremented each tick. Set by
  fire shield spell effect (NEXUS_SPELL_EFFECT_FIRE_SHIELD).

## Source-locks

- DM.BIN CMP/EQ #21 at 0x0601CE40, 0x06020E3A, 0x0602CBAA, 0x0602CD52
- DM.BIN CMP/EQ #22 fire gate at 0x0603C386
- src/nexus/nexus_v1_movement.c: nexus_square_is_passable()
- src/nexus/nexus_v1_mechanics.c: fire crossing gate in tick
- include/nexus_v1_mechanics.h: fire_shield_ticks field
