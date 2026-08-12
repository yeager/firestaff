# DM2 Macintosh editor and input-source preservation note

`/Users/bosse/Downloads/Dungeon_Master_II_Editor.sit` is an authentic
Macintosh StuffIt archive containing **DMII Editor**, a small utility for the
classic Mac version of Dungeon Master II.

The included `Read.Me` identifies it as a live-memory editor. It searches for
the running DM2 process and can change hero attributes, health, stamina and
mana. It does not edit DM2 save files and it is not a game-data container.

## Firestaff use

The archive is preservation and reverse-engineering evidence only. Firestaff
must not launch it, depend on it, or use its modified memory as a gameplay or
save fixture. Its useful scope is limited to future comparison of Macintosh
hero-object layout and runtime memory ownership against the original game.

The file remains outside the repository because it is user-supplied original
software. It is explicitly excluded from the DM2 Mac implementation and from
all runtime tests. Any future memory-layout claim must be checked against
authentic DM2 media and disassembly; the editor alone is not sufficient
evidence.

## Source-owned Mac pointer path

The authentic retail application resource fork is the authority for the
Macintosh gameplay pointer owner. The retained fork is the 5,046,234-byte
resource fork from the large English retail image; its SHA-256 is
`c2555d9c30bbcc17582e32782c7c46690ca16e7888d5da3d1984398b806d4b79`.

Its `CODE(3)` resource is 29,456 bytes with SHA-256
`b2992838bfc7ec1080cc39e5914679e49fa2cb25ea1e06f66ed30e6a01c6f4d7`. The
source path at `CODE(3)+0x00c2` reads the Mac global mouse position, applies
the source window offset and scale, and queries the physical button. The
path at `CODE(3)+0x0170` walks the linked list rooted at the Mac event owner,
tests each source rectangle, and returns the owning action index. The event
loop at `CODE(3)+0x0358` consumes that result and dispatches the object's
action byte; it is not a PC `GRAPHICS.DAT` rectangle table.

The same retail resource fork contains `CODE(11)` (23,908 bytes, SHA-256
`b52ac092bd888cc8821e1bb7cd8073014c38c7d67d114c5b8aa1451ef168cdb5`). Its
disassembly provides two additional ownership anchors. The routine at
`CODE(11)+0x0214` allocates four 0x12-byte dynamic records and fills them from
the source champion/control tables. The routine at `CODE(11)+0x0308` resolves
an event through the authenticated DB/ObjectID links and walks the source
record chain before changing the event/control state. The shared builder at
`CODE(11)+0x3186` derives the control rectangle from the live record, source
viewport state, and Mac color/control tables before publishing it; its
`CODE(11)+0x3474` call site is the materialization boundary. These are source
builders/resolvers, not a reusable screen-coordinate table.

The current Firestaff boundary therefore treats keyboard/gamepad confirmation
as an alternate device for the already authenticated ObjectID transaction,
while native Mac pointer/drag input remains closed until these dynamic records
are materialized and dispatched with their original event owner. No PC or FM
Towns rectangle is substituted.

The implementation consequence is strict: Firestaff may accept a Mac gameplay
pointer only after resolving a rectangle published by the authenticated Mac
viewport path. The currently recovered owner is the source `c_rwbb` wall-button
target (`b_0b == 4`): it is dispatched through the live DB3/DB14 wall-action
chain for both English editions. Other target kinds remain fail-closed until
their original Mac action owner is recovered; no PC or FM Towns coordinates
are substituted.

## Separate small-demo application evidence

The local Downloads corpus also contains the original `DungeonMasterII_demo.hqx`
installer. Its StuffIt payload has a real demo application with a 484,815-byte
data fork and a 1,889,960-byte resource fork. The resource fork contains a
23,908-byte `CODE(11)` resource with SHA-256
`c238573c9fe7e99d18d8c85ec3dfa2014ba8cbecd905e72088492f73aed9f311`; its
`CODE(11)+0x0214` control-builder entry is the same source routine as the
retail edition but uses the demo's own A5-global offsets. The demo's authentic
`Dungeon.dat` and `Graphics.dat` are 6,535 and 3,110,116 bytes respectively.

This installer is preservation/disassembly evidence, not a replacement for
the authoritative `.firestaff/data/dm2` CD ZIP. The verified small CD image
contains the real `DMFiles` game data but no application fork, so Firestaff does
not silently copy the Downloads application into production or claim native
small-edition dynamic pointer ownership without an in-scope packed source.
