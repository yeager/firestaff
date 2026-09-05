# CSB V1 visible wall-inscription ownership

ReDMCSB `DUNGEON.C` F0168 decodes a visible C02 TextString with text type 0.
Atari ST and FM Towns store `Visible` in bit 0 and the 13-bit text-word offset
in bits 3..15. Amiga A31/A35 reverses that compiler bitfield: the offset is in
bits 0..12 and `Visible` is bit 15. The inscription decoder emits line marker
0x80 and terminator 0x81; the readable presentation text is derived only from
those decoded source bytes.

`csb_v1_runtime_decode_visible_inscription_text_pc34` now owns this boundary
for the selected CSB dungeon. It rejects invisible/malformed C02 records and
never consults M11's DM1 `world.things`. The same platform-specific offset fix
also applies to the existing native C07 scroll path.

The focused data-free boundary test pins both legitimate compiler layouts and
the invisible fail-closed case. The authentic Atari launcher regression proves
that the selected packed media still reaches the native runtime without disk
extraction.

`csb_v1_inscription_presentation_plan` now locks the source material admission:
Atari S20/S21/F20E use MEDIA020 M648 graphic 120 and the fixed G0203 line
geometry `{48,59,75,86}`; Amiga A31/A35 and English FM Towns F31E use
MEDIA720 M648 graphic 258 and F0635 zone geometry. Every admitted route uses
8x8 cells and transparent colour C10. FM Towns F31J deliberately fails closed:
F0107 uses the separate F0644 Japanese renderer and its selected-media font,
so borrowing English M648 would be a false parity claim.

`csb_v1_visible_front_inscription_receipt` now reproduces F0172 ownership for
the live square in front of the party: it requires a native wall, walks only
that selected CSB square's ordered C02/C03 prefix, identifies the facing cell,
and preserves BUG0_76's last-visible-C02 publication. M11 consumes that receipt
inside the candidate-page transaction. The MEDIA020 Atari path installs real
graphic 120 through the selected source loader, applies
`fs_po_gettext_in_domain("csb", ...)` only after authentic C02 decode, and
draws the result with the source 8x8/C10/G0203 geometry. A missing font,
malformed chain, unsupported translation glyph, or absent inscription cannot
fall through to DM1 state; retail decoded English remains the translation
fallback.

MEDIA720 Amiga and FM Towns English now read raw graphic 696 directly from the
already-memory-resident selected container (Amiga DMCSB2 item span or F31 raw
item copy), validate its F0639 ranges, and resolve C1000..C1003 through the
strict F0635 type-7/parent-4 contract. Their live candidate-page draw therefore
uses selected M648 pixels and selected layout anchors rather than Atari or PC
rectangles. F31J remains separately closed on F0644 Japanese font material.

The focused tests prove decode, publication, material selection and F0639/
F0635 geometry, and the launcher proves that the composed path remains bootable.
They do not yet place an authentic inscription-bearing party state in front of
the renderer and assert the resulting framebuffer delta. Accordingly this is
implemented source wiring, not an original-capture pixel-parity claim. The same
missing fixture/capture applies to Atari graphic 120.

Distant and side inscriptions now stay inside the ordinary F0107 ornament
transaction. A view-wall-aware F0172 receipt selects the C02 whose Thing cell
owns the requested right, front, or left face; the opposite face fails closed.
It returns global M615 (ornament zero, ordinal one) to the existing ornament
resolver instead of drawing a later overlay. For one through three decoded
lines, the shared F0791 path consumes the exact MEDIA720 G0190 increment and
G0204 value (`5/8/13`, `7/13/20`, `5/12/19`, `10/17/27`, or `11/22/33`). The
receipt also records that G2154 is the source raster width, matching the
`MASK0x4000_SHIFT_UNREADABLE_INSCRIPTION_AND_OPEN_VERTICAL_DOOR` contract.
Four-line inscriptions retain the complete plaque zone. D1C remains owned by
the readable M648 path and is deliberately not double-blitted as M615.

Verification:

- `m11_csb_leader_hand_no_dm1_fallback`
- `csb_v1_inscription_presentation`
- `csb_v1_m11_launcher_handoff_boundary`
