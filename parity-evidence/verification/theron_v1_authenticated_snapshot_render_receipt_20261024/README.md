# Theron V1 authenticated snapshot render receipt

Three TRV2 Firestaff `--debug --boot-probe` loops used the authentic US Track
02 BIN (MD5 `f23601102138f87c33025877767ebf76`) and the allowlisted authentic
`selection.trace` VDC capture. The CLI accepted the complete VRAM, VCE,
HuC6270-state, SAT, and VDC-I/O bundle and reported
`THERON AUTHENTICATED ATOMIC VDC CAPTURE` on every run.

Each boot probe reached the Track 02 dungeon runtime: dungeon 1, level 0,
32x27, entrance (4,0), with four loaded dungeon levels, 291 source objects,
four verified headers, 455 non-zero source tiles, and two champions at map 0,
position (1,0). `gameTick` remained zero and the input/movement pipeline had no
activity.

All three real Firestaff source and presented BMPs were identical, with SHA-256
`cdcb9d4f4247d72f6fa9f28dd28571153c718984fae8dd581553729d3317df47`, size
320x200, and 35,452 non-black pixels. The images and raw run logs remain in
TRV2 `/dev/shm`; no screenshot or original game-media bytes are committed.

The TRV2 source snapshot has no Git metadata. SHA-256 values for the four
relevant CLI, capture-loader, viewport, and M11 presenter source files match
this checkout and are recorded in `manifest.json`; that does not prove the
entire executable's source commit. The same TRV2 PCE Fast close-time capture
also retained an 8 KiB main-RAM snapshot (SHA-256
`909e9fae11955465b6c665665650b50aa8f6844cf5bd9d05ee7905fadd0955c3`). Its
capture summary reports raw bytes `current_level_2031=02`,
`party_direction_203f=01`, `party_x_2040=02`, and `party_y_2041=03` beside the
VDC snapshot boundary. These fields have not been joined to the live world
through a source-backed consumer trace. The same capture retained 65,536
main-RAM consumer reads but none directly read those four candidate offsets;
its transition receipt is `missing`. The original three-loop Firestaff probe
reported level 0 and party `(1,0,dir0)`. A follow-up three-loop `--debug` route
reached party `(2,3,dir1)`, three champions, and `gameTick=17`; each presented
frame retained the same snapshot-render SHA-256. The x/y/direction raw
candidate bytes therefore match that route, but Firestaff still loaded level 0
and `$2031=02` has no source-backed world meaning. This is only a candidate
pose match, not a same-level or same-world join. No original-emulator
screenshot was paired with the Firestaff frame. The receipt proves authentic
capture admission and non-black screen-space rendering only. It does not prove
gameplay or visual parity.

Reproduction on TRV2, repeated three times with separate output directories:

```sh
capture=/home/trv2/firestaff-theron-evidence/capture/selection.trace
app=/home/trv2/work/firestaff-current-main-20261024/.build-debug/firestaff
out=/dev/shm/theron-auth-capture-runtime-loop1
mkdir -p "$out"
SDL_VIDEODRIVER=dummy FIRESTAFF_AUTOTEST=1 \
FIRESTAFF_AUTOTEST_SCREENSHOT_DIR="$out/source" \
FIRESTAFF_AUTOTEST_PRESENTED_SCREENSHOT_DIR="$out/presented" \
FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$out/runtime.json" \
  "$app" --debug --game theron \
    --data-dir /home/trv2/.firestaff/data/theron/TQUS02.bin \
    --theron-vram-snapshot "$capture.vram" \
    --theron-vce-snapshot "$capture.vce" \
    --theron-vdc-state "$capture.vdc-state" \
    --theron-vdc-sat "$capture.sat" \
    --theron-vdc-io "$capture.vdc-io" \
    --boot-probe --boot-probe-frames 0 \
    --script enter,enter,down,down,down,down,down,down,enter,down,enter \
    --duration 0 --width 320 --height 224
```

The candidate-pose follow-up was repeated three times with this route script
and `--boot-probe --boot-probe-frames 2 --boot-probe-expect-party 2,3,1`:
`down,down,down,down,enter,enter,enter,down,down,down,down,down,down,enter,down,enter,up,right,down,left,up,up,right`.

It produced party `(2,3,dir1)`, three champions, `gameTick=17`, and the same
presented-frame SHA-256 on each run. Firestaff still loaded level 0; this is a
pose-candidate match only.
