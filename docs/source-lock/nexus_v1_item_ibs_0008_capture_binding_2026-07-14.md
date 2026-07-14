# Nexus ITEM.IBS 0008 Capture Binding (2026-07-14)

`ITEM.IBS` descriptor `0008` already has bounded packed-4bpp bytes and a
local BGR555 palette. Those source bytes alone do not establish which VDP1
colour mode, source address, or nibble order the original game used.

`nexus_v1_item_ibs_bind_0008_vdp1_capture()` is the only authorization input
to the existing expansion helper. It requires one candidate packet to match
the complete hash-verified `ITEM.IBS` source, the selected descriptor
metadata, the exact packed span, the 32-byte palette state, VDP1 state and
command fingerprints, declared texture-source extent, and texture observation
before the command observation. A changed source byte, descriptor, palette,
state, command, or sequence rejects the whole receipt.

No retail Saturn capture packet is tracked yet. Therefore this adds no real
decode, palette interpretation, VDP1 draw, placement, or fallback visual. The
retail corpus remains blocked until a genuine original-Saturn packet is
imported and independently establishes the command colour-mode semantics.

Verification: `test_nexus_v1_dgn_geometry_readiness`.
