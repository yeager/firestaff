# Theron's Quest US roster/text-consumer audit

**Date:** 2026-08-06
**Status:** US champion roster remains unavailable; no host labels promoted

## Real-media result

The authenticated US Track 02 BIN is
`TQUS02.bin` (`f23601102138f87c33025877767ebf76`). Its startup text marker is
the byte-exact prompt `GO AWAY AND RESURRECT THERON` at raw offset `0xa0722`.
The real-media catalog does not find the eight-name ASCII cluster used by the
JP release, so the US roster receipt remains `NOT_FOUND` with zero names.

The authenticated US Track 19 ISO is `TQUS19.iso`
(`51b40a17b92a30339957ba564aa0015c`, 5,984,256 bytes). Its existing source
readers validate the real item-name table, item-property table, 15 level-label
entries, opaque record window, and startup-level envelope. Those tables are
real Track 19 data, but they are not a champion-roster consumer.

## Disassembly boundary

The static bank-$1f consumer at `$243e` and the stage-2 resource handler are
verified against both US and JP Track 19 media. The static image contains no
post-CD-read `$2600` RAM consumer; the live-capture path that could identify
the missing text owner is therefore not present in the checked-in static
image. No executing text consumer, glyph destination, or source span joins a
US champion name/title payload.

Evidence:

- `docs/source-lock/theron-disassembly/theron-us-bank1f-consumer.asm`
- `docs/source-lock/theron-disassembly/theron-us-stage2-huc6280.asm`
- `docs/source-lock/tqr_v1_track02_consumer_disassembly_2026-08-05.md`
- `tests/test_theron_v1_bank1f_consumer_receipt.c`
- `tests/test_theron_v1_huc6280_disassembly.c`
- `probes/theron/firestaff_theron_v1_track02_bank_probe.c`

The production rule remains: use the real prompt and real Track 19 tables
where their byte readers prove them; keep US champion names/titles unavailable
until a real encoded payload is joined to its original text consumer.
