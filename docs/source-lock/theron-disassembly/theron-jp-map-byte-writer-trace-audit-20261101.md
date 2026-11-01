# JP map-byte writer-trace audit (2026-11-01)

## Finding

The accepted state-only UP/control report observes `$BC` at physical
`$0E8B14`, while the authenticated JP Track 02 projection contains `$B4` at
the corresponding raw offset `$B10E4`. Its statement that the isolated CD-RAM
target-write trace recorded no write to this address is limited by the
instrumenter's address allowlist: the original allowlist did not include bank
`$74`, offset `$0B14` (`$0E8B14`). Therefore, the zero-row result cannot be
used to conclude that the byte was not written during the replay.

The earlier observations remain valid: both autoloaded state-only captures
read `$BC`, and neither capture provides cold loader provenance. The byte's
origin and meaning are still unknown.

## Follow-up instrumentation

An isolated Mednafen 1.32.1 PCE Fast build was rebuilt to add this exact
address to the bounded CD-RAM target-write filter while retaining the existing
targets. The resulting binary SHA-256 is
`688f12959c49aa51d36ced85f4bd8223e87dfce070e1a20bf36f23c822b95c67`.
This only establishes that the next capture can report writes to the target;
it is not itself a capture and provides no writer, loader, or gameplay
evidence. No repository runtime code was changed for this instrumenter rebuild.

## Required next capture

Use an authentic cold JP route with this build and retain the shared event
sequence, writer PC, physical destination, MPR context, old/new byte, and
source-read provenance where available. If the route does not reach the
game's loader, report the capture as blocked and make no claim about the
writer. To explain the saved-state discrepancy, join an observed write to the
subsequent `$0E8B14` read and continue through active-map selection, the
pose-bound tile consumer, and atomic video state. The state-only reports remain
useful for read context, but their previous zero-write count is not exhaustive.

Related evidence: [state-only map-index context capture](theron-jp-state-only-map-index-context-capture-20261101.md),
[capture readiness](../../theron/THERON_CAPTURE_READINESS.md), and
[Theron TODO](../../../TODO-theron.md).
