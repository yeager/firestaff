# Theron JP PCE Fast TII control-flow capture — 2026-10-31

## Finding

Three identical state-only replays from the authentic JP Akutuba F5 emulator
state captured the same 128-record PCE Fast instruction window ending at frame
469, logical PC `$C1FC`, MPR6 `$6E` (physical PC `$0DC1FC`). The final record is
the target instruction itself; the preceding 127 records show a
conditional branch at `$C1FA` falling through to the `$C1FC` opcode, whose
bytes begin `73 B4 20 40`; the existing write observer records the two-byte
transfer `$20B4->$2040/$2041`. The preceding path includes a call to `$C14E`
and its return before the branch sequence. This adds a repeatable dynamic
instruction-context receipt; it does not identify the routine's gameplay
purpose or prove the transfer is a player step.

The 128 `tii_instruction_context_record` rows have SHA-256
`9924db9f54762133a9847ed10ed52f547bec85a5c810890c898a1c0f6530f2d5` in all
three runs. The complete movement-context sidecar has SHA-256
`3667514b329a209f1ba50a8ce5fad4daf274fadfd0cc0c4ddfcae07b2de7b448` in all
three runs. Raw traces remain on TRV2; only hashes and the bounded summary are
recorded here.

The relevant consecutive instruction records are:

```text
12522970  $C1D1  F0 03       BEQ +3 (taken)
12522971  $C1D6  20 4E C1    JSR $C14E
12522972  $C14E  08          PHP
12522973  $C14F  73 BA 20 36 TII opcode
12522974  $C156  28          PLP
12522975  $C157  60          RTS
12522976  $C1D9  7F B4 03    BBR7 $B4,+3 (taken)
12522977  $C1DF  A5 29       LDA $29
12522978  $C1E1  85 B0       STA $B0
12522979  $C1E3  85 AF       STA $AF
12522980  $C1E5  A5 AF       LDA $AF
12522981  $C1E7  C5 3E       CMP $3E
12522982  $C1E9  D0 05       BNE +5 (falls through)
12522983  $C1EB  FF B6 02    BBS7 $B6,+2 (falls through)
12522984  $C1EE  E6 AA       INC $AA
12522985  $C1F0  A5 BA       LDA $BA
12522986  $C1F2  C9 FF       CMP #$FF
12522987  $C1F4  D0 04       BNE +4 (falls through)
12522988  $C1F6  A5 BB       LDA $BB
12522989  $C1F8  C9 FF       CMP #$FF
12522990  $C1FA  D0 1C       BNE +$1C (falls through)
12522991  $C1FC  73 B4 20 40 target TII opcode and first operands
```

The branch mnemonics and fall-through observations describe the captured
opcode stream. The meaning of the tested RAM fields is not source-bound by
this capture. In particular, it does not identify the active dungeon map,
associate the bytes with a map object, establish collision/passability, or
show a rendered response. The same `$C1FC` instruction remains associated in
the source-locked Track 02 analysis with teleporter destination copying, so
the movement-versus-teleporter question remains open.

## Provenance and reproduction

- Game: authentic Japanese Rev. 1 Track 02 media; CUE SHA-256
  `7bbbe8b077f35b2e9b05c68848f8cb86ea9aa74021a076e882eee8841bc56545`.
- System Card 3.0 SHA-256
  `df4f75feebb95e53dfef72dea0787df743e3474ba046ff5a4cb88e34dda93ff1`.
- Autoload state: authentic JP Akutuba F5 emulator state (not BRAM), SHA-256
  `2cc9938b96640a74db1a5b706113564b5d578d5011daf5f85c588ef1c98d70ee`.
- Instrumented Mednafen 1.32.1 PCE Fast binary SHA-256
  `223cb2e079e0bee75c60f54a055c280c38fda4e928238cada5d017bb7c2181b7`.
- Three replays used the same bounded state-only input plan:
  `ii@61:120,right@62:120,ii@190:120,up@191:120,ii@330:120,right@331:120,ii@460:120,up@461:120`.
- The observer runs only in the isolated reference emulator. No raw input,
  game data, or rendered screenshot is promoted as parity evidence.

All runs report the state-only boundary: save-state loading bypasses the cold
CD handoff and authenticated CD-to-RAM receipts. This is instruction-flow
evidence from an original-game state replay, not a cold-start, map, collision,
visual, or native Firestaff parity result.
