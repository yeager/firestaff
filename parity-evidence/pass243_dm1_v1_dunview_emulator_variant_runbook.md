# Pass243 — DM1 V1 DUNVIEW emulator variant runbook

Status: `BLOCKED_DUNVIEW_TCPP101_DOSBOX_VARIANTS_EXHAUSTED`

## Result

No tested N2-local DOSBox variant produced `DUNVIEW.OBJ`; therefore the authentic `FIRES.MAP` route is still blocked before TLINK.

## Source anchors

- `MKII.BAT` TCPP101 PATH line: `10`.
- `MKII.BAT` authentic DUNVIEW compile line: `473`.
- `MKII.BAT` FIRES.MAP TLINK line: `510`.
- First repeated TCC error source line anchor in `DUNVIEW.C`: `4791`.

## Tested variants

- `dosbox-x normal 386`: rc `124`, `DUNVIEW.OBJ` `0` bytes, log `278528` bytes — timeout with zero-byte/missing DUNVIEW.OBJ; DOSBox-X INT6 flood observed when stdout was not suppressed.
- `dosbox-x normal 386_prefetch`: rc `124`, `DUNVIEW.OBJ` `0` bytes, log `278528` bytes — timeout with zero-byte/missing DUNVIEW.OBJ; no improvement over normal 386.
- `dosbox-x simple 386`: rc `0`, `DUNVIEW.OBJ` `0` bytes, log `280735` bytes — compiler returned to DOSBox but emitted errors and no DUNVIEW.OBJ.
- `dosbox-x dynamic auto`: rc `0`, `DUNVIEW.OBJ` `0` bytes, log `280735` bytes — same compiler failure pattern as simple 386; no DUNVIEW.OBJ.
- `classic dosbox normal 386`: rc `124`, `DUNVIEW.OBJ` `0` bytes, log `1187` bytes — classic DOSBox did not complete the DUNVIEW compile inside the bounded window; no DUNVIEW.OBJ.

## Next single action

Run the same HARDDISK/SOURCE tree in a non-DOSBox DOS environment (real DOS, 86Box/PCem, or QEMU+FreeDOS/MS-DOS with Turbo C++ 1.01 mounted) and preserve only DUNVIEW.OBJ/FIRES.MAP metadata if produced.

Evidence manifest: `parity-evidence/verification/pass243_dm1_v1_dunview_emulator_variant_runbook/manifest.json`.
