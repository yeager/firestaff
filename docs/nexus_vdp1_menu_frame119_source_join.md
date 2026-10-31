# Nexus Japanese menu frame 119 source joins

The authentic Japanese Ymir frame-119 capture verifies a bounded NBG0 and
VDP1 menu composition. This state is separate from the title-prompt frame
13162 and gameplay frame 239.

The frame has `TVMD=0x8000`, `BGON=0x0001`, `CHCTLA=0x0013`, `BMPNA=0`,
`MPOFN=0`, `CRAOFA=0`, and `PRINA=0x0004`: NBG0 is the only enabled VDP2
background and is a 512×256 8bpp bitmap at VRAM byte offset zero. The
nontransparent NBG0 pixels match 74,052 doubled screenshot samples within
eight RGB levels. Captured VDP1 framebuffer pixels account for the visible
lower layer where the NBG0 index is zero.

Five late VDP1 menu commands, trace rows 171–175, are bound to the captured
command words and texture bytes. Each texture is an exact match to a unique
Japanese retail `TITLE.BIN` DGT2/PP record; each source palette matches its
active CRAM bank. Command 172 uses the captured horizontal-flip bit. The five
draws match their screenshot pixels within eight RGB levels. Combined with
captured VDP1 framebuffer and NBG0, the bounded reconstruction matches all
286,720 doubled screenshot samples within eight levels (maximum error 8).

The C capture-only compositor has also matched the independent native 320×224
RGB reference byte-for-byte (215,040 bytes). Four additional authentic
frame-119 captures from control-pad and mouse scenarios pass the same full
screenshot comparison and C/reference comparison. These repeated captures
verify this menu state only; they do not prove mouse hitbox behavior, a general
VDP renderer, or production renderer permission. The full evidence and exact
input hashes are retained in
`parity-evidence/verification/nexus_v1_ymir_menu_cpu_writer_trace_20261030/`.

`MENU.BPK` is not established as the source for these five visible commands:
the command texture joins point to `TITLE.BIN`. The active NBG0 bitmap is not
fully source-owned either. A direct same-coordinate `LOGOBG.DG2` join was
tested and rejected, while a visible 921-byte NBG0 span from `TITLE.BIN` TITL
record 2 was found. That span match does not identify the rest of the bitmap.
