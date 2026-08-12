# CSBWin attacking-character ordinal capture contract

## Purpose

This contract records the exact evidence needed before Firestaff may publish
CSBWin's C035 attacking-hand border. It prevents a PC3.4 action-menu value,
an inventory item, or a saved character record from being mistaken for the
CSBWin transient state.

## Source behaviour

`CSBWin/Data.h` declares `AttackingCharacterOrdinal` as a signed 16-bit
runtime field at the legacy `20246` location. It is not a character-save field.

`CSBWin/Character.cpp::PrepareAttack(chIdx)` establishes the only admission
sequence relevant to the border:

1. reject disabled or dead characters;
2. resolve the fighting-hand object's attack class, rejecting a class of zero;
3. set `AttackingCharacterOrdinal = chIdx + 1`;
4. calculate legal attacks, mark `CHARFLAG_weaponAttack`, and call
   `DrawCharacterState(chIdx)`.

`DisplayBackpackItem` selects graphic C035 only for possession index `1` when
`chIdx + 1 == AttackingCharacterOrdinal`. `ExecuteAttack` and the recorded
input cancellation path clear the ordinal again. The ordinal is therefore
one-based, bounded by the live party, and frame-transient.

## Required capture packet

A promotable Atari ST/CSBWin capture must contain one same-frame packet with:

- source build and media identity;
- before/after `AttackingCharacterOrdinal` values;
- selected character index and possession index;
- action-hand object identity and resolved attack class;
- the `PrepareAttack` admission result;
- a 320x200 original framebuffer, including the C035 18x18 crop;
- the matching Firestaff framebuffer and a crop comparison at the exact
  C232/character-state destination.

The packet must also capture one clear transition, either `ExecuteAttack` or
the source cancellation route. A static save, a synthetic M11 state mutation,
or an action-menu selection without this source packet is insufficient.

## Firestaff boundary

`CSB_V1_RuntimeM11MirrorReceipt_PC34` currently carries party/view and leader
hand state only. Until it carries a verified source ordinal from such a packet,
the Atari C017 compositor must leave C035 undrawn. This is intentional
fail-closed behaviour, not a fallback to C033/C034 or PC3.4's acting-champion
state.
