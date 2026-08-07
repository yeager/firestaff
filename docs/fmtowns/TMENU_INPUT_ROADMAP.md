# DM1 FM Towns TMENU mouse/input capture — roadmap

## Status

**Not implemented.** The FM Towns launcher `TMENU.EXP` (a separate
Phar Lap P3 binary from EDM.EXP) owns the TownsOS event queue and
hit-test logic. Its input handling has not been reverse-engineered
against real hardware traces.

## What is known

From EDM.EXP disassembly (`parity-evidence/dm1_fmtowns_menu_p3_disassembly.md`):

  * `MOUSE_ON` @ 0xdd18 (32 bytes) — reference-counted show; 51 callers.
  * `MOUSE_OFF` @ 0xdd38 (32 bytes) — reference-counted hide.
  * `MSE_STATE` @ 0x25848 — word, mouse hide depth counter.
  * `PICKING_CHARACTER` @ 0x29418 — word, active character during pick.
  * `PARTY_RESTING` @ 0x2941a — word, rest gate.

These are all shipped via `dm1_v1_fmtowns_menu_bss.h`.

## What is unknown

The TownsOS event queue (source of mouse coordinates and button
events) is exposed to game code through TBIOS INT calls that this
project has not decoded. Without a decode, four things are missing:

  1. Mouse coordinate poll — TBIOS call number and packet layout.
  2. Button state poll — same TBIOS category, different subcode.
  3. Hit-test dispatch loop — how DYNAMENU rows convert mouse (x,y)
     into an action index.
  4. Keyboard poll — TMENU-owned keymap; not the same as EDM.EXP.

## What must NOT be done

Do NOT synthesize mouse events, hit-test rectangles or key mappings.
FM Towns input is fail-closed until the real TBIOS decode lands.
The visible menu drawn today by `M11_GameView_RenderDm1FmtownsMenu`
is a **display-only** path; interaction remains blocked.

## What can be done incrementally

Bounded steps that do not require synthesis:

  1. Disassemble TMENU.EXP's Phar Lap P3 header (same format as
     EDM.EXP, offset 0x200). Enumerate its export/symbol table if
     present.
  2. Locate the TMENU input-poll site via cross-reference to a
     known TBIOS INT vector or a shared-memory event flag.
  3. Byte-fingerprint the TBIOS wrapper functions against known
     TownsOS Phar Lap SDK signatures if available.
  4. Publish the entry points as `dm1_v1_fmtowns_tmenu_input.h`
     with vaddrs only, no synthesised behaviour.
  5. Layer a source-locked poll function on top once the byte-level
     I/O contract is verified against a real hardware trace.

## Entry-point candidates (unverified)

The `TMENU.INF` file parsed by the startup receipt names two
launch records: `\\JDM.EXP` and `\\EDM.EXP`. TMENU therefore reads
its own runtime state, dispatches to one of the two game binaries,
and hands mouse/keyboard focus. The exact handoff mechanism (shared
memory? command-line? TBIOS state?) is the first thing a decode
pass should establish.

## Blocking action

None from Firestaff's side. This roadmap document is the deliverable
for the "TMENU mouse/input capture" line-item; concrete decode is
future work that requires either a hardware trace or a documented
TownsOS SDK reference.
