# Nexus combat item evidence and implementation status

**Updated:** 2026-10-29

Older versions of this page described a DM1 weapon set, armor formula,
inventory capacity, and flask system as Nexus behavior. Those descriptions
were extrapolations and are withdrawn. See
[`NEXUS_STALE_CLAIM_AUDIT.md`](NEXUS_STALE_CLAIM_AUDIT.md).

## Current evidence

- `nexus_v1_combat.c` contains host-side combat calculations. Their presence
  does not bind them to the Japanese retail executable's item dispatch or
  combat state.
- The shared item encyclopedia is presentation data, not an authenticated
  Nexus item table.
- The retail spell-table byte match and Magic-window entry capture documented
  in [`nexus_spells.md`](nexus_spells.md) do not establish item use, weapon
  damage, armor reduction, or potion effects.
- Nexus production keeps unsupported spell operations fail-closed. No
  DM1-derived item action is admitted as retail behavior.

## Unresolved retail behavior

The admitted evidence does not establish which items are used in combat, how
the Saturn executable selects an item or target, how weapon and armor values
are loaded, how damage is computed, or which state writes and animations
follow. It also does not establish whether Nexus has the DM1 flask-based
potion crafting flow. DM1 and DM2 manuals or implementations may inform a
comparison, but cannot fill these Nexus gaps.

## Promotion evidence required

For each action, retain an authentic pre-action state, capture the exact
controller input and bounded SH-2 reads/writes, identify the retail source
record and consumer, then compare post-action memory and rendered output.
Until those joins exist, keep the Firestaff route inert and do not show
synthetic combat item effects.
