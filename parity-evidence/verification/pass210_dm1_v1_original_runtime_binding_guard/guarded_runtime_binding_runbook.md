# Pass210 guarded runtime-binding runbook

Use this only after a live DOS debugger or map build provides real runtime inputs.

## Required order
1. Record PSP segment and compute program load segment as `PSP+0x10`.
2. Break at the stock FIRES LZEXE loader entry only to find the decompressor handoff; do not treat that loader entry as game code.
3. Record the post-LZEXE transfer CS:IP or produce a verified decompressed FIRES dump/map.
4. Bind ReDMCSB map/source symbols to runtime segment:offsets using the recorded load/relocation rule.
5. Only then set/check the four seam hits in `trace_binding_contract.json`.

## Loader-only fact currently known
- Stock compressed loader entry: `1665:000e` relative to DOS load-image base; this is **not** a breakpoint for `command_accepted` etc.

## Guardrail
If the trace has only compressed-file offsets or the compressed loader CS:IP, classify it as `blocked/runtime-base-and-symbol-map-unavailable`.
