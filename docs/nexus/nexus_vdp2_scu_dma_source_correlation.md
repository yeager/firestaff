# Nexus VDP2 SCU DMA source correlation

## Capture

This authentic `--debug` Ymir 0.4.2-dev run used the Japanese retail Nexus disc and the user-provided Japanese Saturn IPL 1.01 (SHA-256 `dcfef4b99605f872b6c3b6d05c045385cdea3d1b702906a0ed930df7bcb7deac`). Raw disc and IPL files are excluded from the evidence package and Git.

The full SCU descriptor history contains 6,489 transfers with zero drops. The payload trace covered bus interval `[0x05e5c000, 0x05e5e000)`, recorded 14,336 16-bit writes, and dropped zero rows. Four complete direct channel-0 transfers wrote 7,168 bytes each to `0x05e5c000`:

| Transfer | Source | Bytes | Payload SHA-256 |
| --- | --- | ---: | --- |
| 6031 | `0x060b803c` | 7,168 | `4492b51b4e398584d2940eecaf6514b47fcfab66aaedbd6256c5682175434873` |
| 6188 | `0x060b9c40` | 7,168 | `7bccac1ffb6299bdc8a73d894ea6db43557be446ab4a5b0f077842c2a75d1d3a` |
| 6305 | `0x060bb844` | 7,168 | `08d2612dca43df0271f24f22fd126750db4a1aab38d0bc9ba067c62e121a9382` |
| 6462 | `0x060bd448` | 7,168 | `b8aff2b4ec785e880910cbe5e5a202dca35965c0d3bb0406a97e46ad79aef2f6` |

The analyzer rebuilds each destination stream from the recorded bus values and verifies byte-for-byte equality with the corresponding contiguous bytes in captured high WRAM. Transfer 6462 is the last descriptor in the complete history overlapping this interval. Its payload also matches 7,168 bytes of the final VDP2 VRAM snapshot at file offset `0x5c000`, with the same SHA-256.

The same cold boot captured 113 NBG0 live-row samples at VDP1 frame 7486 (rows 4–116): H/V scroll `0x1f79/0x0bcd`, increments `0xcd/0xcd`, priority 4, and color calculation enabled at ratio 15. The bounded VDP2 VRAM/CRAM write trace recorded zero writes in frame range `[7486, 7487)` and zero dropped rows. Its final VRAM snapshot is at frame 7663, not frame 7486.

## Reproduction

Run Ymir with the same retail disc and a private profile containing the hash-pinned IPL at `roms/ipl/saturn-jp-bios.bin`:

```sh
xvfb-run -a env SDL_AUDIODRIVER=dummy /usr/local/bin/ymir-sdl3 \
  --profile /dev/shm/nexus-vdp2-scu-payload \
  --disc "/home/trv2/.firestaff/data/nexus/Dungeon Master Nexus (Japan).cue" \
  --debug --capture-boot-frames 8400 --dump-memory --capture-screenshot --capture-no-present \
  --capture-pad-input 700:right:2 --capture-pad-input 720:right:2 \
  --capture-pad-input 740:right:2 --capture-pad-input 760:right:2 \
  --capture-pad-input 780:right:2 --capture-pad-input 800:right:2 \
  --capture-pad-input 830:a:2 --capture-pad-input 7000:start:2 \
  --trace-vdp2-writes --trace-vdp2-writes-max-rows 2000000 \
  --trace-vdp2-writes-frame-range 7486:7487 \
  --trace-scu-dma-payload-begin 0x05e5c000 \
  --trace-scu-dma-payload-end 0x05e5e000 \
  --trace-scu-dma-payload-max-rows 200000 \
  --trace-vdp2-render-state /dev/shm/nexus-vdp2-scu-payload/dumps/nbg0-rows.csv \
  --trace-vdp2-render-state-frame-range 7486:7487 \
  --trace-vdp2-render-state-row-range 4:117
```

To verify the recorded source, payload, and final VRAM bytes, run:

```sh
python3 scripts/analyze_nexus_scu_dma_payload.py /path/to/capture/dumps
```

## Limits

The SH-2 VDP2-write trace dropped 1,660,912 master rows. The DMA/payload/final-snapshot match therefore does not prove SCU DMA was the exclusive final writer or establish exact write timing relative to CPU writes. The final VDP2 register snapshot (frame 7663) has CHCTLA `0x0013`, PNCN1 `0`, PLSZ `0`, MPOFN `0`, and MPABN1/MPCDN1 `0x1717`. Ymir's `CalcPageBaseAddress` in `vdp2_defs.hpp` maps NBG1 Plane A/B to `(0x17 & 0x3f) << 14 = 0x5c000`, the captured DMA destination. The SCU descriptors have no frame IDs, so this final-register mapping does not bind each transfer to a specific frame or pixel fetch. The NBG0 row state is independently corroborated; this is not evidence of an NBG1 fetch or visible pixel contribution, complete VDP1+VDP2 composition, arbitrary-frame or original-Saturn parity, or renderer admission.

Raw evidence, run log, screenshot, and checksums are retained outside Git under `/Volumes/Extern-disk/firestaff-evidence/nexus/nexus-vdp2-scu-dma-payload-20261101/`.