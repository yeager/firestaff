# Theron's Quest — FIFO-origin capture receipt

The Theron capture build now applies
`scripts/mednafen_1.32.1_theron_fifo_origin_main_ram_consumer_v2.patch` after
the existing Mednafen 1.32.1 trace patches. The patch is byte-provenance only:

- each authenticated MODE1/2352 or MODE2/2352 user-data write into the PCE CD
  data FIFO queues its raw source LBA, sector offset and FIFO sequence;
- the FIFO read associated with the PCE data bus retains that tuple;
- the existing CD-to-main-RAM receipt includes the tuple and the physical/logical
  writer coordinates;
- Firestaff accepts the receipt only when its source LBA belongs to a recorded
  SCSI READ range and the byte fields are bounded.

The patch compiles against the source tree after the existing Firestaff trace
patches and passes `patch --dry-run` in that order. This is a capture
capability, not a semantic result. The already captured
`/tmp/tq-vram.trace.cd` was produced before this extension and therefore has no
FIFO-origin rows. A fresh capture is required before any Track 02 byte can be
promoted to a `$2600` consumer, object/level record, tile/material, palette,
HUD, or viewport meaning.

## Fresh authentic capture result (2026-08-06)

The freshly rebuilt instrumented binary was run against authenticated US Track
02 with real SDL 2.32.8 (not `sdl2-compat`). The System Card MD5 was
`ff1a674273fe3540ccef576376407d1d` and the Track 02 BIN MD5 was
`f23601102138f87c33025877767ebf76`. The replay receipt records 161 raw sector
spans, 51 SCSI READ commands, 25 CD IRQ callbacks, and 4,096 bounded main-RAM
consumer reads. It still records zero game-owned `$e009` CD data reads and
zero `$2600` handoff receipts. Its two FIFO-origin rows are from the CD
routine at `reader_pc=eb33`/`writer_pc=eb37`, with destinations `$21e7`/`$21e9`;
they are not game-owned consumer evidence and remain rejected for semantic
publication.

A second 70-second replay with held keys produced the same six `$e009`
windows, zero data reads, 159 raw sector spans, and 4,096 consumer reads. A
real Cocoa/Quartz run additionally proved eight host key events and
`host_input_order=followed_by_pce_input_poll`, but still produced no game-owned
CD data read. These negative results are retained as capture receipts; no
level, object, bitmap, palette, HUD, or viewport meaning is inferred.
