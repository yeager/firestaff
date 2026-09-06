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
Its runtime setter contract is tested separately; host panel-close behavior
still needs a complete source-owner audit.
