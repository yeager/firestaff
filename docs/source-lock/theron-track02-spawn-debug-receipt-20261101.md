# Theron Track 02 spawn-table debug receipt

## Purpose

The native `--debug` path is being extended to print the authenticated Track 02
spawn-table source metadata already retained by the Theron world. The receipt
is intended to make the static source projection inspectable during a native
launch; it does not implement or authorize creature spawning.

## Planned receipt fields

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

The diagnostic's modified engine and decoder translation units compiled
successfully in an isolated TRV2 Debug configuration. The full application has
not been linked, and no native `--debug` output has been observed. Treat the
output format above as an implementation target, not as verified runtime
evidence. Before relying on it, link the full application on TRV2 and inspect
one authentic US or JP launch log. Confirm that the printed MD5 and offsets match the
[authenticated ISO projection report](theron-track02-spawn-record-iso-projection-20261101.md),
and that a non-Theron launch emits no Theron receipt. Keep the output bounded
to one receipt per launch.

The independent state-only reference replay remains negative for runtime
spawn semantics: it consumed five scripted controller events, recorded zero
spawn-consumer/register entries and zero RNG samples, and showed no dungeon
transition. See the [capture readiness report](../theron/THERON_CAPTURE_READINESS.md)
and the [authentic JP state-only comparison](../../parity-evidence/verification/theron_v1_jp_state_movement_input_compare_20261031/README.md).
