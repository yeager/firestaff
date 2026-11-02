# Theron Track 02 spawn-table debug receipt

## Purpose

The native `--debug` path prints the authenticated Track 02 spawn-table source
metadata already retained by the Theron world. The receipt makes the static
source projection inspectable during a native launch; it does not implement
or authorize creature spawning.

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

The modified engine and decoder translation units compiled successfully in an
isolated TRV2 Debug configuration. The full application also linked in the
Linux, macOS, and Windows CMake CI builds for PR #243 (run
[41284640562](https://github.com/yeager/firestaff/actions/runs/41284640562));
the localization, warning, repository-policy, and cross-platform determinism
checks passed.

On 2026-11-01, a TRV2 native Debug build from `3deebb02f` ran headless in
three consecutive loops against the authentic JP Rev. 1 Track 02 file. Each
direct-launch boot probe used:

```text
--debug --game theron --theron-native jp
--data-dir /home/trv2/.firestaff/data
--boot-probe --boot-probe-frames 0
--script enter,enter,enter,action,tab --duration 0
```

SDL's dummy video and audio drivers were enabled. Each probe entered
`theron-runtime`, loaded four source dungeon levels and reported the expected
Track 02 MD5 `b7afb338ad31be1025b53f9aff12d73a`. Each emitted one table
header, eight pointer rows, and five zone rows. A separate Python extraction
read the same authentic BIN directly on TRV2, checked its MD5 and MODE1/2352
framing, then independently projected the listed user-data offsets through
each sector's 16-byte header. Every loop's 14 emitted lines, including raw
fields and static links, matched exactly. The observed pointer offset was
`0x273818`; the five zone offsets were `0x273858`, `0x2738d7`, `0x273902`,
`0x273929`, and `0x273950`, consistent with the
[authenticated ISO projection report](theron-track02-spawn-record-iso-projection-20261101.md).

A headless authentic DM1 FM Towns direct-launch boot probe completed as a
non-Theron control and emitted no `theron-source` lines. Thus the bounded
receipt and its game boundary have native runtime evidence. The JP probe
reported `runtimeTick=0`; these checks do not establish original runtime
pointer selection, spawn/RNG behavior, creature creation, gameplay parity, or
visible rendering. No game media or emulator was used by Firestaff at runtime,
and no game media was copied into the repository.

The independent state-only reference replay remains negative for runtime
spawn semantics: it consumed five scripted controller events, recorded zero
spawn-consumer/register entries and zero RNG samples, and showed no dungeon
transition. See the [capture readiness report](../theron/THERON_CAPTURE_READINESS.md)
and the [authentic JP state-only comparison](../../parity-evidence/verification/theron_v1_jp_state_movement_input_compare_20261031/README.md).
