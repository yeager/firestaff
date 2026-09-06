# CSB V1 — Items and Spells: Source Audit

Corrected 2026-09-06 against local ReDMCSB WIP20210206,
`Toolchains/Common/Source/MENU.C` and `DEFS.H`.

## Zokathra is shared object creation, not a fireball

The earlier audit incorrectly described Zo Kath Ra as a CSB-only fireball
variant. `MENU.C:76` defines it as `0x006B6E76`, required skill 0,
Wizard skill, attributes `0x3C73`. `DEFS.H:1755–1757` decodes this as
kind 3 (other), type 7 (Zokathra), action-disabled duration 15 ticks.
This is not a kinetic-energy descriptor.

`MENU.C:F0412:1994–2027` allocates a junk record and assigns
`C51_JUNK_ZOKATHRA`. It places the object in the ready hand if empty,
otherwise the action hand if empty, otherwise on the party square through
the movement owner. If allocation fails, the effect branch exits but the
normal success XP/disable-action tail still runs. This code applies to both
DM1 and CSB. Zokathra's object identity is not evidence of a CSB-exclusive
spell or new item.

The unused Firestaff helper that returned 50 for CSB and 0 for DM1, and its
self-confirming test, have been removed. That removal is not implementation
or verification of CSB's live cast transaction. Genuine DM1 materialization
tests remain separate.

## Spell tables are edition-specific

| Source family | Record count | Evidence |
|---|---:|---|
| Atari CSB S20/S21 | 25 | `MENU.C:15–18`, `DEFS.H:3224` |
| Amiga CSB A31/A33/A35 and FM Towns F31 | 29 | `MENU.C:50–84`, `DEFS.H:3227` |

The later table adds four magic-map spells: Oh Gor Dain, Oh Gor Ros,
Oh Gor Ku and Zo Ir Neta (`MENU.C:80–83`). Their source effect owner
requires a magic map in the action hand (`MENU.C:1873–1920`). A universal
"25 spells plus a new Zokathra" model is incorrect.

CSBWin's separately decoded 25-entry graphic `0x230` table is not proof of
original Atari/Amiga/FM Towns table admission. Original-media identity,
table ownership and live effect execution must be verified independently.

## Remaining scope

A complete edition-specific item census and native CSB cast-route parity
remain open. The previous blanket assertion that all CSB editions add no
item types was not established by comparing a partial shared icon range.
Do not use this source correction as completion evidence for inventory,
magic-map functionality, spell effects or an original-media playthrough.
