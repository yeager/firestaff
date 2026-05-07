# Pass236 — DM1 PC34 FIRES symbol bridge audit

Status: `BLOCKED_NO_PROMOTED_CSIP`

## Result

No existing FIRES.MAP/FIRES.SYM/listing/object bridge was found in focused N2-local roots. ReDMCSB MKII.BAT can in principle emit FIRES.MAP, but the bounded DOSBox-X build did not produce a usable map.

## Searched paths

- `<redmcsb-source>/Reference/ReDMCSB/I34E`
- `<redmcsb-source>/ReDMCSB_WIP20210206/Reference/ReDMCSB/I34E`
- `<redmcsb-source>/Reference/Original/I34E`
- `<redmcsb-source>/ReDMCSB_WIP20210206/Reference/Original/I34E`
- `<redmcsb-source>/ReDMCSB_WIP20210206/Toolchains/IBM PC/Source`
- `<redmcsb-source>/ReDMCSB_WIP20210206/Toolchains/Common/Source`
- `<firestaff-original-games>/_canonical/dm1`
- `<firestaff-original-games>/_extracted/dm-pc34/DungeonMasterPC34`

## Public symbol files found

- None in the focused N2-local original/ReDMCSB roots.

## ReDMCSB build-script bridge facts

- `MKII.BAT`: `<redmcsb-source>/ReDMCSB_WIP20210206/Toolchains/IBM PC/Source/MKII.BAT`
- `I34E.LNK`: `<redmcsb-source>/ReDMCSB_WIP20210206/Toolchains/IBM PC/Source/I34E.LNK`
- `MKII.BAT` has an explicit `TLINK.EXE ... ,\BUILD\I34E\FIRES.EXE,\BUILD\I34E\FIRES.MAP` line for EXEID 74.
- `I34E.LNK` order includes `MOVESENS.OBJ`, `CLIKMENU.OBJ`, `COMMAND.OBJ`, then `DUNVIEW.OBJ`.

## DOSBox build attempt

- Return code: `0`
- `DUNVIEW.OBJ` size: `0`
- `FIRES.MAP` size: `0`
- Error excerpt: `<firestaff-repo>/parity-evidence/verification/pass236_dm1_v1_fires_symbol_bridge_audit/mkii_log_error_excerpt.txt`
  - `Error dunview.c 4791: ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ`
  - `Error dunview.c 4791: ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ`
  - `Error dunview.c 4791: ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ`
  - `Error dunview.c 4827: ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ`

## Promotion decision

CS:IP candidates are still blocked. The build-script TLINK line is actionable evidence for the next unblock path, but it is not a verified public-symbol/runtime bridge until FIRES.MAP is actually produced and matched to FIRES.EXENEW or debugger-observed seam hits.
