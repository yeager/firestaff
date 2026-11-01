# Theron Track 02 spawn-table debug receipt

## Purpose

The native `--debug` path prints the authenticated Track 02
spawn-table source metadata already retained by the Theron world. The receipt
is intended to make the static source projection inspectable during a native
launch; it does not implement or authorize creature spawning.

## Receipt fields

For an authenticated US or JP Track 02 launch, the diagnostic prints the
regional identity, Track 02 MD5, source layout (`MODE1/2048` ISO projection or
`MODE1/2352` raw BIN), pointer-table user-data offset, and the fixed counts of
eight pointer records and five regular-spawn zone records. Each pointer row
prints its source offset, four decoded words, and the static zone index resolved
from `spawn_data_offset` when that offset matches an authenticated zone. Each
zone row prints its source offset, two raw words, and four raw bytes.

All decoded values are labeled as raw or static source links. No category,
creature identity, RNG behavior, selection rule, placement, or runtime consumer
is inferred. Records 5–7 may have no regular-zone link; that absence is not a
spawn failure or a semantic classification.

## Verification state

The receipt implementation is in `m11_debug_theron_spawn_source()` and emits
once per authenticated Theron launch from the native `--debug` game-event
path. The full application has linked successfully in the PR's Linux CMake
build. The native runtime receipt has since been captured and checked in three
JP and three US loops, as recorded below. The values and bounded emission are
verified. Non-Theron log isolation remains untested. Confirm that its MD5 and
offsets match the
[authenticated ISO projection report](theron-track02-spawn-record-iso-projection-20261101.md),
and that a non-Theron launch emits no Theron receipt. Source inspection gates
the receipt on an authenticated US/JP Track 02 world and one launch serial.

The independent state-only reference replay remains negative for runtime
spawn semantics: it consumed five scripted controller events, recorded zero
spawn-consumer/register entries and zero RNG samples, and showed no dungeon
transition. See the [capture readiness report](../theron/THERON_CAPTURE_READINESS.md)
and the [authentic JP state-only comparison](../../parity-evidence/verification/theron_v1_jp_state_movement_input_compare_20261031/README.md).

## Native receipt verification (2026-11-01)

A fresh macOS Debug build of the native `firestaff` target was launched with
authentic raw JP and US Track 02 data. For each region, three scripted native
startup loops advanced through title, stage select and Soul Room into the
loaded runtime. All six logs contain exactly one authenticated spawn-table
header, eight pointer rows and five zone rows, and each runtime summary reports
`level-loaded=1`. The reported regional MD5, raw user-data offsets, all pointer
words, all zone words/bytes, and static link indices were compared against
the complete MODE1/2048 user-data projection of the independently MD5-verified
MODE1/2352 BIN; every field matched across all six runs.

The logs and a per-run SHA-256 manifest are retained outside Git at
`/Volumes/Extern-disk/.codex-scratch/theron-native-spawn-receipt-20261101/`;
the manifest SHA-256 is
`46c418756efbc8400fc9296ead3d6c2790203d21f0c885b6a4db9afc3d200dff`. This
verifies the native diagnostic's source projection and bounded emission only.
It does not establish original runtime pointer selection, spawn semantics,
RNG, placement, AI, combat, renderer parity, or non-Theron log isolation.
