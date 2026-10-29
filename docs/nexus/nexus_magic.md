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
- Static analysis found a champion-row byte used in a threshold for accepting
  or rejecting the next rune in a four-entry sequence. Its semantic name and
  dynamic input path are unknown.
- The Japanese retail manual documents Control Pad Y opening the Magic window;
  an authentic Ymir `--debug` capture verifies that entry action from the
  retained gameplay checkpoint.

See [`nexus_spells.md`](nexus_spells.md) for evidence hashes, capture details,
and the bounded interpretation.

## Runtime status

Firestaff production links `nexus_v1_magic_runtime_noop.c`. Spell lookup,
mana-cost, cast, and damage APIs fail closed. The desktop route does not yet
open or render the retail Magic window. Rune selection, sequence validation,
mana mutation, target selection, effect dispatch, and save changes are not
implemented as admitted Nexus behavior.

Keep the diagnostic source-study file out of gameplay. Do not substitute DM1
tables or synthetic item/spell data for missing retail joins.
