# Theron JP System Card CD buffer-write evidence

## Capture identity

This separate 30-second cold-start capture used the authentic JP Rev. 1 CUE and
MODE1/2352 Track 02 (MD5 `b7afb338ad31be1025b53f9aff12d73a`) with authentic
System Card 3.0. The instrumented PCE Fast Mednafen executable was built in an
isolated reference copy and had MD5 `f7af75d3b614b0f1be056049b10572dd`. Its
reader-PC and CPU-store instrumentation is not part of Firestaff's runtime.
The private capture is under
`/home/trv2/firestaff-theron-evidence/capture/theron-cd-reader-buffer-write-20261009/`.
The run applied scripted input `run@480:300`.

## Source-to-buffer receipt

The 4,096 traced reads of CD data port `$1808` each came from an authenticated
JP Track 02 source span. For every row, the data-port read occurred at logical
PC `$EA99` and physical PC `$000A99`. The next instruction stored the same
accumulator byte at logical addresses `$2800-$37FF`, mapped by MPR `$F8` to
physical `$1F0800-$1F17FF`. All 4,096 write values match both the corresponding
data-port read and the accumulator value at writer PC `$EA9C` / physical
`$000A9C`. No discontinuities or unbound source rows were observed.

Reproduce the two real-media checks from the repository root:

```sh
python3 scripts/verify_theron_jp_cd_data_port_trace.py \
  /home/trv2/firestaff-theron-evidence/capture/theron-cd-reader-buffer-write-20261009/live.trace.cd-data-port-read \
  "/home/trv2/.firestaff/data/theron/Dungeon Master - Theron's Quest (Japan) (Rev 1) (Track 02).bin"

python3 scripts/verify_theron_jp_syscard_buffer_write_trace.py \
  /home/trv2/firestaff-theron-evidence/capture/theron-cd-reader-buffer-write-20261009/live.trace.syscard-buffer-write \
  /home/trv2/firestaff-theron-evidence/capture/theron-cd-reader-buffer-write-20261009/live.trace.cd-data-port-read \
  "/home/trv2/.firestaff/data/theron/Dungeon Master - Theron's Quest (Japan) (Rev 1) (Track 02).bin"
```

The buffer verifier reruns the authentic-media comparison, then independently
checks sequence, logical/physical mapping, MPR, instruction PCs, accumulator,
and byte-for-byte equality across the CD-port and RAM-write traces.

## Boundary

This is a System Card CPU copy into its mapped address space, not a CD DMA
receipt or proof of a game-owned load. The transition report has zero CD IRQ
callbacks, SCSI reads, authenticated CD-to-RAM receipts, non-System-Card input
polls, and game-main-RAM loader dispatches. The scripted RUN reached only one
System Card poll; `transition=missing`. Therefore the capture proves neither
game launch nor dungeon loading or gameplay. The next proof still needs a
retail consumer of this buffer, a game-owned state transition, and an
authenticated gameplay state.
