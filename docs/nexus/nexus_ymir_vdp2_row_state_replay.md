# Nexus VDP2 row-state replay

Ymir's ordered VDP2 event bundle lets Firestaff inspect the VRAM and CRAM state
that existed when each scanline began rendering. Use this verifier on an
authentic Nexus capture produced by the patched Yeager Ymir fork:

```sh
python3 scripts/verify_nexus_ymir_vdp2_row_state.py "$YMIR_CAPTURE/dumps" \
  --row-details --watch-vram 0x5ca00:8
```

The verifier requires `row_state_reconstruction_complete=1`, complete start
and end snapshots, a drop-free event trace, and full unfiltered VDP2 memory
coverage. It replays big-endian VRAM and CRAM writes in strict sequence order,
checks the replayed state against Ymir's export-time memory dumps, and prints
capture totals and final memory hashes. Add `--row-details` to print hashes and
requested byte ranges at every scanline marker; full startup traces can produce
millions of detail lines. Combine it with repeatable `--detail-frame FRAME`
and `--detail-row ROW` selectors to inspect only selected authentic rows. Repeat
`--watch-vram ADDRESS:LENGTH` or `--watch-cram ADDRESS:LENGTH` to inspect other
authentic map, character, or palette ranges. Watch ranges are printed only with
`--row-details`.

## Authentic title-menu capture validation (2026-11-02)

The verifier was run on a real Japanese Nexus cold boot captured by the patched
Yeager Ymir fork. The run used the hash-verified Japanese CUE
(`878b3bc223bb56e99626dbaa2946d17fb349a2a67a10cd4423742d8ab30b8699`) and
Japanese IPL (`dcfef4b99605f872b6c3b6d05c045385cdea3d1b702906a0ed930df7bcb7deac`),
with no emulator save state or game save. Its screenshot shows the authentic
three-choice title menu. The complete external evidence package is at
`/Volumes/Extern-disk/firestaff-evidence/nexus/nexus-ymir-vdp2-row-state-capture-20261102/`.

The capture exported 2,195,061 VRAM/CRAM writes and 2,042,879 render events,
with full VDP2 memory coverage, no frame filter, and zero dropped events. The
verifier replayed the entire ordered stream and matched both export-time
snapshots. Replayed VRAM SHA-256 is
`de1a77427dc62457eb4b59fa0424fd25e9768ce229505445311929d3bdae405c`; replayed
CRAM SHA-256 is
`db282593790dec4a2bf8f76293c0458a1a140adc3351650bd56a417f0359e0de`.

For VDP1 frame 8037, the verifier emitted 448 row details. The first 64 bytes
at VRAM `0x5c000` were unchanged across those row boundaries; at rows 83 and 84,
`0x5ca00` remained `0x00001000`. This validates raw memory reconstruction for
the captured menu state, not retail source ownership or production composition.
Keep the production renderer gate closed until the retail source joins and
composed pixels are independently verified.

The event sequence marks the beginning of a row render in Ymir's synchronous
software renderer. It does not identify a write's exact dot position within a
scanline, and it does not claim timing parity for hardware renderers. VDP2
register writes are not part of this event stream; pair it with the existing
register and pixel-fetch captures when interpreting a row.

This tool validates the raw memory timeline. It does not paint a fixed row clip
into the MAPD compositor. Update the compositor only after an authentic event
bundle demonstrates the row-specific source state and the resulting composed
pixels can be compared with the matching retail capture.
