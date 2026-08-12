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

The implementation consequence is strict: Firestaff may accept a Mac gameplay
pointer only after resolving a rectangle published by the authenticated Mac
viewport path. The currently recovered owner is the source `c_rwbb` wall-button
target (`b_0b == 4`): it is dispatched through the live DB3/DB14 wall-action
chain for both English editions. Other target kinds remain fail-closed until
their original Mac action owner is recovered; no PC or FM Towns coordinates
are substituted.
