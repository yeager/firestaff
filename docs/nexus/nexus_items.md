# Nexus item system evidence and implementation status

**Updated:** 2026-10-29

This page records what the current sources and retained retail evidence prove.
Older DM1/DM2 item tables and Firestaff's shared encyclopedia are not evidence
of Nexus item names, inventory behavior, or actions. See
[`NEXUS_STALE_CLAIM_AUDIT.md`](NEXUS_STALE_CLAIM_AUDIT.md) and
[`NEXUS_STRICT_FIDELITY_INVENTORY.md`](NEXUS_STRICT_FIDELITY_INVENTORY.md).

## What is established

- `FS_ItemEntry` and `firestaff_item_encyclopedia.c` are shared Firestaff UI
  data. Their representative names, weights, attack, and defense values do
  not establish the Japanese retail Nexus roster or Saturn behavior.
- The current `Nexus_V1_Champion` host structure contains `inventory[30]` and
  a separate equipment-slot array. These are implementation storage choices;
  they do not establish a retail 30-slot inventory or a retail equipment
  layout.
- `nexus_v1_magic.c` is diagnostic/source-study code. Production links the
  fail-closed `nexus_v1_magic_runtime_noop.c`; its item and spell APIs do not
  implement retail actions.
- Authentic item and resource bytes are useful receipts only until their
  runtime consumer, state mutation, and visible result are joined to an
  authenticated Saturn capture.

## Not established

The following remain unknown from the admitted evidence: the complete retail
item roster and item identifiers; inventory capacity and slot semantics;
equipment rules; pickup, drop, use, consume, and equip dispatch; weight and
encumbrance rules; flask and fountain behavior; and item effects. DM1/DM2
behavior must not be substituted for these gaps.

## Production rule

Keep unsupported Nexus item actions inert. Promote an action only after a
retail byte/source record is joined to the Saturn dispatch, destination state
write, and observable result. Do not populate gameplay from synthetic
encyclopedia entries when authentic data is missing.
