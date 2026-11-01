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
checks passed. These checks verify compilation and integration, but no native
`--debug` launch log containing the new receipt has been inspected yet. Treat
the fields above as the implemented output contract, not as verified
source-receipt values. Before relying on runtime values, inspect one authentic
US or JP native launch log and compare its MD5 and offsets with the
[authenticated ISO projection report](theron-track02-spawn-record-iso-projection-20261101.md).
Also confirm that a non-Theron launch emits no Theron receipt. Output is
bounded to one receipt per launch.

The independent state-only reference replay remains negative for runtime
spawn semantics: it consumed five scripted controller events, recorded zero
spawn-consumer/register entries and zero RNG samples, and showed no dungeon
transition. See the [capture readiness report](../theron/THERON_CAPTURE_READINESS.md)
and the [authentic JP state-only comparison](../../parity-evidence/verification/theron_v1_jp_state_movement_input_compare_20261031/README.md).
