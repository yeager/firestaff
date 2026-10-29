# Nexus magic evidence and implementation status

**Updated:** 2026-10-29

This is a short entry point to the current Nexus magic evidence. Older text
that described a DM1 rune grid, spell list, cost formula, flask recipes,
champion mana pools, or effects as Nexus behavior has been withdrawn. Shared
Firestaff UI declarations and diagnostic C code do not prove retail semantics.

## Verified retail facts

- The Japanese retail `DM.BIN` has 32 big-endian halfwords at offset `0x38368`
  matching bytes in `src/nexus/nexus_v1_magic.c`. The match establishes bytes,
  not the complete table meaning or any cast effect.
- Follow-up static analysis places `FUN_0603a472` on a four-index ITEM.IBS
  action-eligibility path. Its helper reads champion-row `+0x18` as one input
  to an eligibility threshold, while related selectors update a distinct
  `+0x35..+0x38` state block. Available caller and capture evidence does not
  connect that path to the Magic composer at `+0x3d/+0x3e` or to casting. Do
  not interpret its four indices as the composer's four rune entries.
- The Japanese retail manual documents Control Pad Y opening the Magic window;
  an authentic Ymir `--debug` capture verifies that entry action from the
  retained gameplay checkpoint.
- Authentic paired-input captures and decompilation confirm the retail
  composer stores four action slots at champion row `+0x3d/+0x3e..+0x41` using
  `0xDA + 6*cursor + action`. All six cursor-0 numeric choices and costs
  (1–6 mana) have been captured; their rune names and the Nexus menu's
  coordinate-to-index table are not yet identified. A four-action sequence
  fills the composer buffer, but any final validator and cast path remain
  unverified.

See [`nexus_spells.md`](nexus_spells.md) for evidence hashes, capture details,
and the bounded interpretation.

## Runtime status

Firestaff production links `nexus_v1_magic_runtime_noop.c`. Spell lookup,
mana-cost, cast, and damage APIs fail closed. The desktop route does not yet
open or render the retail Magic window. The confirmed retail composer behavior
is not yet implemented in Firestaff; final sequence validation, target
selection, effect dispatch, and save changes also remain unverified.

Keep the diagnostic source-study file out of gameplay. Do not substitute DM1
tables or synthetic item/spell data for missing retail joins.
