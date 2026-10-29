# Nexus potion and flask evidence and implementation status

**Updated:** 2026-10-29

Earlier versions of this page presented DM1 potion recipes, mana formulas,
fountain behavior, and two-step flask consumption as Nexus features. These
claims were based on shared or diagnostic Firestaff code and are withdrawn.
They are not verified against Nexus retail behavior.

## Verified evidence

The Japanese retail `DM.BIN` contains a 32-halfword table at file offset
`0x38368` that matches bytes copied into `src/nexus/nexus_v1_magic.c`. This
establishes the byte match only. It does not identify all entries, their
meaning, potion recipes, mana costs, or effects. Static analysis also found a
runtime champion-row byte used in the threshold for accepting or rejecting a
rune in a four-entry sequence; the field meaning and dynamic path remain
unknown. The Japanese retail manual documents Y opening the Magic window, and
the retained authentic Ymir capture verifies that entry action. See
[`nexus_spells.md`](nexus_spells.md) for hashes and capture references.

## Firestaff implementation boundary

`nexus_v1_magic.c` is source-study code, not an admitted production mechanic.
The production target links `nexus_v1_magic_runtime_noop.c`, which fails
closed. Retail potion creation, flask consumption, fountain filling, potion
consumption, stat changes, and resulting visuals have not been joined to
authenticated Saturn input and state traces. Do not enable DM1 potion tables
or infer Nexus behavior from shared encyclopedia entries.

## Evidence needed to implement potions

Starting from an authentic gameplay save, capture flask acquisition, fountain
interaction, each candidate rune sequence, casting attempt, and potion use as
separate actions. Preserve pre/post saves, bounded SH-2 reads and writes,
source-record joins, and screenshots. Identify the exact inventory and
champion fields and verify the visible effect before implementing the route.
Unknown actions remain inert; missing retail data must not be replaced with
synthetic game data.
