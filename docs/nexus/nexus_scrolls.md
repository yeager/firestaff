# Nexus scroll and consumable evidence status

**Updated:** 2026-10-29

This page replaces earlier DM1/DM2-derived claims about scrolls, flasks,
potion recipes, and consumable effects. A shared Firestaff item category or
encyclopedia entry is not proof that an item exists or can be used in Nexus.

## What the current sources establish

- Firestaff has shared item-category and encyclopedia structures, including
  placeholder entries. They are UI data and do not authenticate the retail
  Nexus item roster.
- `nexus_v1_magic.c` contains diagnostic/source-study data. The production
  target instead links fail-closed operations in
  `nexus_v1_magic_runtime_noop.c`.
- Retail evidence verifies Y opening the Magic window and a byte match for a
  table in Japanese retail `DM.BIN`. It does not prove scroll casting, flask
  crafting, fountain filling, potion use, or effects. See
  [`nexus_spells.md`](nexus_spells.md).

## Unknown behavior

Retail scroll and potion records, inventory identity, item-use dispatch,
charges, recipes, flask lifecycle, fountain interaction, state changes, and
rendered effects remain unjoined to an authentic Saturn capture. DM1/DM2
behavior may be recorded as a comparison, but it must not fill these Nexus
gaps.

Keep unsupported actions inert. For each proposed item action, capture a
reproducible authentic pre-action state, exact input, bounded SH-2 reads and
writes, the source record and consumer, and the post-action state and visible
result. Do not use synthetic game data where authentic retail data is missing.
