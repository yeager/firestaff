# Theron JP cold-start CD data-port evidence

## Capture identity

An emulator-created cold-start run used authentic JP Rev. 1 CUE Track 02 (raw
BIN MD5 `b7afb338ad31be1025b53f9aff12d73a`) and System Card 3.0 (MD5
`ff1a674273fe3540ccef576376407d1d`). The instrumented PCE Fast Mednafen binary
was MD5 `8a43dfb6ae6155d5562d1aa04a5bd761`. From a fresh private Mednafen home,
the input replay applied `run@480:300` (a five-second RUN hold after eight
seconds). The private trace is under
`/home/trv2/firestaff-theron-evidence/capture/theron-jp-cold-boot-media-trace-20261009/`.

## Source-bound data-port bytes

The emulator trace records 4,096 sequential reads at CD data port `$1808`.
Each row says `source_valid=1`, with no unbound rows. The rows cover JP Track
02 raw LBA 3590, user offsets 0-2047, and LBA 3591, user offsets 0-2047.
Track 02 begins at LBA 3590 and INDEX 01 file offset 526,848; raw sector size
is 2,352 bytes and MODE1 user data begins at offset 16. The read bytes match
the corresponding offsets in the hash-authenticated BIN in all three
independent verification loops. The observed byte-stream SHA-256 is
`5acf4b878cf4f67cf9b7e8f7aee609f7f52d6c7d10c20ba4ab420a855c33c0d6`.

Reproduce the media/trace comparison with:

```sh
python3 scripts/verify_theron_jp_cd_data_port_trace.py \
  /home/trv2/firestaff-theron-evidence/capture/theron-jp-cold-boot-media-trace-20261009/live.trace.cd-data-port-read \
  "/home/trv2/.firestaff/data/theron/Dungeon Master - Theron's Quest (Japan) (Rev 1) (Track 02).bin"
```

## Boundary and next proof

This proves that those bytes were observed at the instrumented CD data port
and match the authentic media span. It does not bind them to a game-code
consumer or a RAM destination. The same capture reports zero CD IRQ callbacks,
zero SCSI read commands, zero authenticated CD-to-RAM receipts, and no
non-System-Card controller poll. The scripted input was observed only at the
System Card poll. Therefore this run does not prove game launch, dungeon
loading, or any Track 02 gameplay semantics. Continue by binding the data-port
reads to a retail transfer/consumer and an observed game-owned state change.
