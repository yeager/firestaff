# Nexus VDP2 row-state replay

Ymir's ordered VDP2 event bundle lets Firestaff inspect the VRAM and CRAM state
that existed when each scanline began rendering. Use this verifier on an
authentic Nexus capture produced by the patched Yeager Ymir fork:

```sh
python3 scripts/verify_nexus_ymir_vdp2_row_state.py "$YMIR_CAPTURE/dumps" \
  --watch-vram 0x5ca00:8
```

The verifier requires `row_state_reconstruction_complete=1`, complete start
and end snapshots, a drop-free event trace, and full unfiltered VDP2 memory
coverage. It replays big-endian VRAM and CRAM writes in strict sequence order,
prints hashes at every scanline marker, emits requested byte ranges, and checks
the replayed state against Ymir's export-time memory dumps. Repeat
`--watch-vram ADDRESS:LENGTH` or `--watch-cram ADDRESS:LENGTH` to inspect other
authentic map, character, or palette ranges.

The event sequence marks the beginning of a row render in Ymir's synchronous
software renderer. It does not identify a write's exact dot position within a
scanline, and it does not claim timing parity for hardware renderers. VDP2
register writes are not part of this event stream; pair it with the existing
register and pixel-fetch captures when interpreting a row.

This tool validates the raw memory timeline. It does not paint a fixed row clip
into the MAPD compositor. Update the compositor only after an authentic event
bundle demonstrates the row-specific source state and the resulting composed
pixels can be compared with the matching retail capture.
