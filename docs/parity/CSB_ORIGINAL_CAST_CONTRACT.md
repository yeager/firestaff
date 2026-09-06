# Original CSB cast implementation contract

Source audit: 2026-09-06. This is open implementation work, not a parity
receipt. Rendering runes or passing table-parser tests does not execute a spell.

## Source ownership

ReDMCSB MENU.C:40-84 defines G0487 for Amiga/FM Towns with 29 records,
including four magic-map spells. Atari S20/S21 uses loaded Graphic560
globals (MENU.C:15-18), with 25 records (DEFS.H:3224-3227). Bind the
selected edition's authenticated data before lookup; do not substitute
CSBWin's fixed 0x4e8 menu payload or a DM1 table.

The existing CSBWin spell parser has no original-platform admission caller.
The runtime DSA cast callback handles only an explicit abort-without-message
packet. Neither constitutes original CSB spell execution. Optional CSBWin
filter semantics are not prerequisites for original Atari/Amiga/FM Towns.

## FM Towns original-table admission

The supplied JP/EN ZIP's first 4,500 MODE1/2352 sectors were read into RAM
and their ISO9660 payload inspected without extraction. Each executable has
one G0487 signature match. Both 232-byte tables are identical little-endian
records: u32 Symbols, u8 BaseRequiredSkillLevel, u8 SkillIndex, u16 Attributes.

| Program | Bytes | Raw table offset | SHA-256 of program |
| --- | ---: | --- | --- |
| CHTWE.EXP | 283936 | 0x2a07c | `7edbdd6f088eaa7a82afd8a78c5ae32c0b3be7e023d261419277ea588c183e0d` |
| CHTWJ.EXP | 284416 | 0x2a254 | `c2add7b7c152e1b7be50b407828807b7005bbd36e2b4b96dbc1dd51703d86da6` |

Table SHA-256: `97227c2ee947a7c61e29cf5c1ef94a7a4be584815b19f259fd091036bf622e9e`;
FNV-1a: `9fd916c2`. The existing whole-executable verified handoff now reads
and checks this exact span, retaining all 29 typed records. No compiled spell
definitions or CSBWin parser supply values. Original-media tests compare all
fields against independently read bytes, check Zokathra/map descriptors,
and reject a RAM-only mutation of a source spell byte. Admission is a
prerequisite for, not an implementation of, the cast transaction below.

## Original mastery query prerequisite

The public runtime F0303 query now implements the independent ignore-temporary
XP/object flags, hidden/base averaging, the uncapped 500-XP threshold loop,
Atari signed versus later unsigned 32-bit experience, resting override and
source action-hand/neck modifiers (CHAMPION.C:732-824). Unresolved equipped
objects fail the query rather than silently losing their potential bonus.
Imported states without full skill experience retain a legacy fallback;
that fallback and end-to-end resting-state publication are not parity claims.

Bounded RAM tests cover flags, thresholds and signedness. Original F31 EN/JP
media tests locate existing Sceptre of Lyf, Gem of Ages, Ekkhard Cross,
Moonstone and Pendant Feral records, temporarily equip each in RAM, verify
its specific bonus and restore the full champion. No object records are
created. These are original-object checks, not natural-route emulator traces;
the two Firestaff variants were not found by this corpus scan.
Complete original F0304 XP/level-up mutation and F0412 casting remain open.

## RNG and level-up integration audit

BASE.C:193-194 initializes F31 G0349 to 31459. Fresh verified F31 boot now
binds this value instead of inheriting the generic PC/I34 zero initializer;
later source save restore still owns the saved stream. Both original-media
title tests assert this boundary. This does not establish all intervening
original title/gameplay draws or emulator timing.

DEFS.H:1-4 and BASE.C:1688-1775 require one LCG advance per draw, with
the middle 16 bits used for modulo/one-bit/two-bit operations. G0361's
last-creature-attack time starts at -200 (PROJEXPL.C:5) and is assigned at
the beginning of GROUP.C:1691's attack, before hit/target decisions. The
current CSB field is only imported/exported, not updated by a live attack
owner. Do not treat its zero default as authentic combat context.

F0304:896-989 consumes minor, major, vitality and antifire draws first,
then mana/antimagic for magic classes, then health/stamina. F31/Atari use
antimagic modulo 3; PC/I34 and late Amiga use two bits. The shared live
PC34 level-up helper now follows that order, class health multipliers and
random stamina increase; 24 independent unit cases check exact maxima and
final RNG. It is not a substitute for CSB's edition-aware transaction or
G2016 recently-upgraded flags and localized source UI publication.

## Required complete transaction

1. F0408 (MENU.C:1651-1661) invokes F0412 for G0514's source champion.
2. F0412 (:1810-2041) validates the champion, performs F0409 lookup, applies
   source RNG/practice XP, commits the effect, grants success XP and schedules
   action-disable timing. Rune-entry mana has already been paid.
3. F0408 clears Symbols and SymbolStep for all results except result 3,
   which preserves the input when an empty flask or magic map is required.
4. Publish updated source state and repaint controls; distinguish an actual
   source failure from an unavailable implementation before mutating state.

Descriptor fields are kind low four bits, type next six bits and disable
ticks high six bits (DEFS.H:1755-1757). Required effects include potion/load
mutation, champion-facing projectiles, map events 80-83, light/darkness event
70, invisibility 71, thieves-eye 73, shield 74, footprints 79, Zokathra object
allocation and the fire-shield helper. Existing projectile/timeline/vitals
APIs are building blocks, not a complete cast owner.

## Zokathra correction

MENU.C:1994-2027 allocates C51_JUNK_ZOKATHRA, preferring the ready hand,
then action hand, then the party square. This applies to both DM1 and CSB.
The old unused model claiming a CSB 50-energy fireball and a DM1 no-op is
contradicted by the source and must not be reused as an implementation or
completion gate. Genuine DM1 object-allocation regressions remain relevant.

## Verification still required

- Authenticate edition-specific tables and field ordering against original media.
- Exercise actual source champion casts for every effect family and failure.
- Verify RNG order, XP, mana, cooldown and object/timeline ownership together.
- Compare representative original-emulator traces, not only local fixtures.
- Keep extended CSBWin spell/filter support separately scoped.

F0394's -1 no-caster selector is a distinct operation: CASTER.C:17,75-86
clears the spell surface without changing champion incantations or mana.
The runtime setter and host panel close/reopen now preserve champion data.
F31 EN/JP original-media tests verify the black closed panel, retained paid
runes, unchanged champion/mana/RNG and public-pointer reopening across all
three modes. This does not establish the F0408/F0412 cast transaction above.
