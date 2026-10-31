# Theron US PCE mouse-held command capture — 2026-10-31

## Scope

This report records three repeated guest-input captures from one authentic US
gameplay save state. It establishes input delivery and a candidate command
consumer observation. Save-state replay bypasses the original disc transfer,
so the capture has no authenticated runtime media-to-RAM provenance and does
not establish original-game semantics or native parity.

## Source and artifacts

- Track 02: authentic US raw MODE1/2352, MD5
  `f23601102138f87c33025877767ebf76`.
- System Card: canonical image verified by the capture wrapper.
- Save state: authentic post-Ak-Tu-Ba state copied from TRV2,
  `trv2-authentic-post-akutuba.mc0`, MD5
  `f17f377df210b4a3ae904a13fb85a7f0`. The local MCS state had the same MD5.
- Standard PCE module: accepted and loaded this save state. PCE Fast rejected
  this state serialization schema; that rejection is module-specific.
- Captures and binary were retained outside the repository under
  `/Volumes/Extern-disk/.codex-scratch/theron-render-reference-pce-20261031/`.
  No ROM, save state, screenshot, binary, or raw trace is included in Git.
- Neutral mouse trace SHA-256:
  `be8321fc344a9d301f30f5081d062ca7ec5fb4bde6b49313bc5e3bcf6142df54`.
- Neutral close RAM SHA-256:
  `c8e8015ccd30d63e86ce75256d2916904e0d2c6b015dc6df10654eda5ee1def4`.
- Held-input close RAM SHA-256 (same for all three repeats):
  `1f473af0036625c050e84a74e359727ce6124d0bc96b24f1a572a171f1f38462`.
- Held-input command-consumer sidecar SHA-256 (same for all three repeats):
  `91d61d6b77fab939bfdac1c92c4428158b7f10c545872cc9e4f20b3822820380`.
- Neutral close VRAM, VCE, SAT SHA-256:
  `242cc371a9f9bda19bf51cc35abde7a009d64ff3841d3c04897d2d9edc73e690`,
  `2263486eb8c1e2cdc822a655b4a7da01f4f9a27c73987a8f19bb822e454ada45`,
  `c546ac140d8bd70165a4fa6dc45d8fa511b4238911976a7553e9e213fab2439d`.

The original state file was not modified. The emulator binary was an
instrumented diagnostic build and is not part of Firestaff's shipped runtime.

## Procedure

The same state, media, System Card, module and capture settings were used for
one neutral replay and three held-input replays. The held-input script was
`0,0,0*30;0,0,1*60;0,0,0*120`: the raw mouse button bit `0x01` was asserted for
60 updates. Do not interpret this raw bit as a named game button. A separate
one-update click and motion-only probe were also captured. The motion probe
used `0,0,0*30;5,0,0*20;0,0,0*120` and recorded the hardware delta latch
`x=-10`.

## Observations

- The wrapper reports the authentic save state loaded; its RAM has dungeon
  bank bytes `$20DA=01`, `$20DB=00`.
- Each held-input replay reports 60 scripted updates and 60 guest mouse-device
  reads. The held-input close RAM and command-consumer stream are byte-identical
  across the three repeats.
- The command-consumer stream contains 473 reads per held-input capture,
  including logical `$2905=$50` twice at instrumented PC `$D34D`. This is a
  candidate queue/consumer observation. No semantic meaning is assigned to
  `$2905`, the value, or the consuming routine from this capture alone.
- Compared with neutral, held-input close RAM differs in 24 bytes; all three
  held-input replays produce the same changed snapshot. The known candidate
  position bytes `$2031`, `$203F`, `$2040`, `$2041`, and pointers `$2E3B` and
  `$2E7B` are unchanged.
- Close VRAM, VCE, SAT and VDC snapshots match the neutral capture. The one-
  update click has the same close-time RAM and display snapshots as neutral.
- The motion probe records hardware delta `x=-10`, but produces no change to
  the candidate position/pointer bytes or close-time display snapshots.
- Every replay validation reports `transition=missing`. The state autoload
  bypasses CD-origin receipts; the capture has no authenticated CD-RAM receipt
  and no non-System-Card CD reads.

## Evidence boundary

These artifacts show that the raw mouse input reached guest hardware and that
a candidate command consumer ran. They do not bind bit `0x01` to Button I/II,
prove party movement or item use, identify a target, authenticate the runtime
map against the disc transfer, establish a game-owned pose/map join, or verify
the renderer. Unchanged candidate position and display snapshots specifically
leave movement and visual response unproven. No gameplay or parity claim should
be inferred from the 24 opaque RAM differences.

The next useful evidence must preserve one cold, same-session chain from
authentic disc reads through game RAM and input dispatch to an identified
movement/action result and game-owned VDC output. Firestaff's production route
remains native C/SDL; Mednafen is a reference instrument only.
