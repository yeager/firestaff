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
