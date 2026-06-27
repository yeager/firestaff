# Pass1072 - DM1 V1 keyboard-buffer live provenance readiness

Status: BLOCKED_ORIGINAL_I34E_KEYBOARD_BUFFER_LIVE_DEBUGGER_OBSERVATION_MISSING

This gate fingerprints the pass513 deterministic transcript and keeps the original keyboard-buffer evidence row honest: source-filled rows are readiness evidence, not live M527/M528/F0361/F0380 debugger observations.

## Transcript

- path: `verification-screens/pass513-dm1-v1-promoted-transcript/promoted_transcript.json`
- sha256: `124284703846ba26355235b6eacd24621c3d5234fbfe1088ea1ea3d68afd8396`
- row count: `3`
- live debugger rows: `0`
- capture hashes OK: `True`
- explicit non-live boundary: `True`

## Host Prerequisites

- dosbox-debug: available
- Xvfb: available
- xdotool: available

## Decision

The pass513 transcript is source-filled and reproducible, but it has zero debugger-observed original PC/I34E keyboard-buffer rows. The B1 keyboard-buffer evidence row remains PARTIAL until a live dosbox-debug run records M527/M528, F0361, F0380, F0128, and F0097 for the same route.

## Non-claims

- no original DOSBox/FIRES runtime was launched by this verifier
- no source-filled transcript row is treated as live debugger observation
- no original-vs-Firestaff pixel parity is promoted
- no gameplay, renderer, or input behavior is changed
- no push, tag, package, or release action

Manifest: `parity-evidence/verification/pass1072_dm1_v1_keyboard_buffer_live_provenance_readiness/manifest.json`
