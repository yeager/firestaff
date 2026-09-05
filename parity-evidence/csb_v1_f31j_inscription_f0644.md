# CSB FM Towns F31J inscription pipeline

## Source ownership

ReDMCSB `DUNGEON.C` F0168 performs a second decoding pass for Japanese media.
After the ordinary five-bit dungeon alphabet is expanded, pairs in `A..P`
restore one original byte; a leading `A` preserves the following literal byte.
`DUNVIEW.C` F0107 then calls `TEXT.C` F0646 to choose at most three printable
substrings. F0646 measures Shift-JIS pairs as 16 pixels, ANK bytes as 8 pixels,
treats `0x1b` and `0x7c` as zero-width controls, and observes the strict
`accumulated + next >= available` boundary.

Firestaff implements those bounded transformations in
`csb_v1_f31j_unpack_f0168_text` and
`csb_v1_f31j_f0646_printable_substring`. Invalid nibble pairs, truncated
Shift-JIS and undersized output buffers fail closed.

## Real-media discrimination

`test_csb_v1_fmtowns_archive_launch_real` opens the supplied retail
`Dungeon-Master-Chaos-Strikes-Back-Expansion-Set-1_FM-Towns_JA-EN.zip` without
extracting it to disk and scans the selected dungeon's visible C02 records.

| Session | Visible C02 strings | Bytes >= `0x80` |
|---|---:|---:|
| F31E / `CDATA` | 41 | 0 |
| F31J / `CJDATA` | 46 | 557 |

This proves that F31J keeps its own byte stream in the same authenticated
language session and does not use an English M648 fallback.

## Remaining pixel boundary

`TEXT2.C` F0644 delegates Japanese glyph rasterization to F0952, which on FM
Towns calls the EGB system-font service. The retail game ZIP contains the CD
image and subchannel metadata, but no FM Towns font ROM. Firestaff therefore
keeps F31J inscription glyph presentation fail-closed instead of inventing
pixels, borrowing M648, or claiming pixel parity. Closing that boundary needs
an authentic system-font material strategy consistent with Firestaff's
no-BIOS runtime requirement.
