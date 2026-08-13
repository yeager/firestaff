# Theron scripted replay transport boundary

Date: 2026-08-13

This is a local-source-bound capture receipt. The raw disc images, sidecars,
and instrumented Mednafen build remain outside the repository on the external
disk; this note records only the reproducible boundary and hashes needed to
review the result.

## Inputs

- US Track 02: `TQUS02.bin`, MD5 `f23601102138f87c33025877767ebf76`
- System Card: MD5 `ff1a674273fe3540ccef576376407d1d`
- Replay plan: `run@1:1,run@480:30,i@900:30`
- Capture mode: scripted PCE input, dummy SDL video, instrumented Mednafen
- Binary MD5: `66872820f4b1e4f0932a79699dc3f6b2`

## Authenticated transport receipt

The transition sidecar reports:

- 240 raw sector spans and 25 CD IRQ callbacks
- 256 byte-exact origin-RAM receipts and 256 authenticated CD-RAM receipts
- 32 game-owned `$E009` dispatches
- 11 main-RAM loader TII transfers
- 65 KiB VRAM and 1 KiB VCE snapshots

The transition receipt test passes with `semantics blocked`.

## Main-RAM consumer boundary

The parser-only consumer test passes with:

```text
reads=65536 first_physical=1f01fe last_physical=1f00ff
target_2600=present target_reads=512 target_nonzero=0
target_readers=1 target_init=512 target_runtime=0
target_c3a0=0 target_c3a0_nonzero=0 target_c3a0_readers=0
semantic_publication=blocked
```

All `$2600-$27FF` target reads are zero-valued reads from `$CB22`. The replay
therefore proves transport and initialization only. It does not prove a
post-CD level/object consumer, square-to-tile mapping, HUD publication, or
T700/T900 semantics. Those remain gated until a source-owned HuC6280 RAM
instruction window and the corresponding runtime consumer are captured.
