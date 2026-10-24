# Theron V1 real-data debug render receipt

Three Firestaff `--debug --boot-probe` runs used the authentic US Track 02
image (MD5 `f23601102138f87c33025877767ebf76`) and the same Ak-Tu-Ba route.
Each run loaded dungeon 1, level 0 (32x27, entrance (4,0)), reported four
loaded dungeon levels, 291 source objects, four verified source headers, 455
non-zero source tiles, and a two-champion party at map 0, position (1,0).
The runtime probe reported `THERON RUNTIME (TRACK 02 DUNGEON)`, but `gameTick`
was zero and the input/movement pipeline had no activity.

All three runs emitted identical 320x200 source and presented BMPs with SHA-256
`a70034eeabbfab4da3e6e2846a111fcf41c08c21ab17bf0228cd23255be7074f`.
The retained BMP is a real Firestaff runtime capture and contains zero
non-black pixels. The debug output states that the later original graphics
bank is not captured and fallback visuals remain disabled. No authentic VDC/VCE
snapshot pair was supplied, and no original-emulator frame was paired with this
Firestaff frame.

The Firestaff executable SHA-256 was
`28bad3c4b41c08cc112b52ddd2e3a3e345cb330de52306bb3fc4693a5f1811af`.
Its exported build tree had no Git metadata, so the exact source commit cannot
be identified from this binary. The executable, original game data, emulator,
and raw run logs remain on TRV2 or in private task scratch; no original game
media or BIOS bytes are included here.

This historical no-snapshot receipt proves an authentic Track 02 dungeon-load
boundary and confirms that this particular invocation, which supplied no
source-backed VDC/VCE snapshot, rendered a black viewport. The separate
authenticated snapshot render receipt in
`parity-evidence/verification/theron_v1_authenticated_snapshot_render_receipt_20261024/`
records a later non-black frame from the authenticated capture route. Neither
receipt proves movement, gameplay, or visual parity. Do not promote
`presented-black.bmp` as a gameplay screenshot.

Reproduction command on TRV2, repeated three times:

```sh
FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$out/runtime.json" \
FIRESTAFF_AUTOTEST_SCREENSHOT_DIR="$out/source" \
FIRESTAFF_AUTOTEST_PRESENTED_SCREENSHOT_DIR="$out/presented" \
FIRESTAFF_FAIL_IF_NO_LAUNCH=1 SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
  "$app" --debug --game theron --width 320 --height 200 \
  --data-dir "$root" --theron-native us --boot-probe \
  --boot-probe-frames 0 \
  --script enter,enter,down,down,down,down,down,down,enter,down,enter \
  --duration 0
```

The original US media remains under the user's `.firestaff/data/theron`
directory on TRV2. This reproduction does not copy game data into the repo.
