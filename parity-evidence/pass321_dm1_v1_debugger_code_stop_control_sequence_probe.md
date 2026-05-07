# Pass321 — DM1 V1 debugger strict code-stop/control-sequencing probe

Status: `BLOCKED_PASS321_MISSING_DEBUGGER_CODE_STOP_CONTROL_PRIMITIVE`

## Findings

- Source audit confirms `F0128_DUNGEONVIEW_Draw_CPSF` calls `F0097_DUNGEONVIEW_DrawViewport` in `DUNVIEW.C`, and `F0097` reaches `VIDRV_09_BlitViewPort` in `DRAWVIEW.C`.
- Pass318 parser bug confirmed: it accepted `Breakpoint list:` as a stop.
- Pass320 strict filter correctly rejected BP/BPLIST echoes, but did not regain a real F0128 stop.
- Pass321 strict parser only accepts unprompted code-stop lines and ignores setup/BPLIST forms.

## Decision

tmux-pane DOSBox-debug control still lacks a reliable actual code-stop line distinct from BP/BPLIST setup echoes. Need a raw debugger event stream or a paused/running-state primitive before promoting runtime F0128/F0097 sequencing.

Manifest: `parity-evidence/verification/pass321_dm1_v1_debugger_code_stop_control_sequence_probe/manifest.json`
