# Theron's Quest Track 02 Palette-Offset Receipt

The focused palette test now reads the authenticated raw MODE1/2352 files from
the standard Firestaff data root when they are present:

| Variant | MD5 | Raw palette candidate |
| --- | --- | ---: |
| US BIN | `f23601102138f87c33025877767ebf76` | `0x2A06A0` |
| JP BIN | `b7afb338ad31be1025b53f9aff12d73a` | `0x29FD70` |

Both windows are 32-byte HuC6260-shaped little-endian words and pass the
existing strict 9-bit palette decoder. The US and JP offsets are not
interchangeable: the US offset is a zero/text tail in the authenticated JP
BIN. The runtime startup-media binder now selects the regional offset instead
of silently applying the US offset to JP media.

This is a real-byte format receipt only. No capture binds either window to a
VCE register write, VDC destination, tile bank, or rendered viewport, so
`semantic_binding_verified` and `promotion_allowed` remain false. The focused
test therefore verifies authentic palette bytes without enabling production
palette or viewport rendering.
