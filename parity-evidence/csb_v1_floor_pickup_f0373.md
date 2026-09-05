# CSB V1 floor pickup ownership (ReDMCSB F0373)

Firestaff previously returned from the CSB-specific C080 viewport dispatcher
before the generic floor-pile branch. Consequently, a visible CSB floor
object could not enter the leader hand on Atari ST, Amiga, or FM Towns.

The corrected route keeps the mutation in the CSB runtime. It resolves the
party or front square from view cells C00..C03, walks the real dungeon Thing
chain, retains the last rendered floor object in the requested cell (the
`G0292_aT_PileTopObject` overwrite rule in `DUNVIEW.C` F0115), unlinks that
exact record, and writes it to the runtime leader hand. M11 only performs
source hit testing and consumes the runtime result.

The front-square group guard also remains source-owned. F0175 locates the
linked C04, F0144 joins `GROUP.Type` to the authentic G0243 `CreatureInfo`
table, and F0264 tests `MASK0x0020_LEVITATION`. For a non-levitating group,
F0176 resolves its packed or active cells and blocks only when a creature
occupies `(PartyDirection + ViewCell) & 3`. The focused regression holds the
same group/object chain constant: a Screamer blocks the target cell, whereas
the levitating Wizard Eye permits pickup. No host levitation flag or DM1
world mirror is consulted.

The party-map case passes through the shared F0145/F0147 effective-group
owner. Raw C04 byte 5 is strictly `ActiveGroupIndex` there; Cells and
Directions come from that indexed, valid `ACTIVE_GROUP`. The regression uses
raw index 2 while slot 2 contains the distinct centered-cell value `0xff`, so
the former byte-5-as-cells interpretation cannot satisfy the test. Off-party
maps retain the raw C04 Cells/Direction layout.

The presentation receipt now resolves the moved Thing through the same
runtime-owned `OBJECT.C` F0031/F0033 name path used by the leader hand.  The
selected edition's decoded M564 table (or the F31J Shift-JIS table) therefore
provides the displayed object name.  The former host-invented `CSB FLOOR
OBJECT` label has been removed; localization is applied only to that authentic
name at the final CSB gettext boundary.

Source anchors: `CLIKVIEW.C` F0373 lines 78-128 and F0377 lines 410-440;
`DUNVIEW.C` F0115 assignment to `G0292_aT_PileTopObject`.

The immediately following C017 inventory transaction also consumes a fresh
CSB party receipt before F0302 reads the destination slot. This prevents an
open M11 presentation mirror from overwriting a newer runtime possession.
