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

This proves that these bytes were observed at the instrumented CD data port
and match the authentic media span. The separate CPU-write trace binds the
same 4,096 bytes to the System Card's mapped buffer. In that capture, the
executed first-stage routine at $2B39 reads and accepts the authentic System
Card signature at $3020, then sampled first-stage metadata and header reads
lead to a $4000 handoff and an observed instruction-byte window at second-
stage PC $4002 (physical PC $100002). See [the buffer-write evidence](theron-jp-system-card-cd-buffer-write-20261009.md)
and rerun all real-media checks with
scripts/verify_theron_jp_syscard_bootstrap_trace.py.

The data-port and CPU-write traces stop at 4,096 rows, their instrumentation
cap; they cannot establish that no later reads or writes occurred. The
consumer trace only samples reads, so it does not bind the $4002 code window
to a later authentic Track 02 span. The transition report still has
transition=missing, no game-owned state change, and no observed
non-System-Card controller poll. Thus this is evidence of first-stage
signature acceptance and a sampled second-stage handoff, not proof of a
complete game launch, dungeon loading, or gameplay. Continue by source-binding
the second-stage code and proving a game-owned state transition against
authentic media.

Static source correlation finds the sampled eight-byte window uniquely at
raw Track 02 file offset 2,716,578: LBA 4521, raw-sector offset 18, user offset
2. H6280 disassembly there starts with TII $2000,$2001,$000F and
TII $2000,$2700,$0080. This is a static source candidate, not a dynamic
CD-to-RAM receipt.
