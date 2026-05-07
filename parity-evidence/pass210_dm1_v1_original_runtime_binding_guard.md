# Pass210 — DM1 V1 original runtime binding guard

Classification: `blocked/runtime-base-and-symbol-map-unavailable`
Exact remaining blocker: No decompressed stock FIRES runtime image base, post-LZEXE transfer CS:IP, or TLINK FIRES.MAP is present on N2; compressed loader CS:IP remains loader-only evidence.

## What was investigated
- Stock FIRES: `/home/trv2/.openclaw/data/firestaff-original-games/DM/_canonical/dm1/DungeonMasterPC34/FIRES` size `94779` sha256 `ebf84045c3edbce7690b826eadbea2e278fbb4c0a3cc19a470552586f37712eb`.
- LZEXE signature at relocation table: `LZ91`.
- Compressed loader entry: `1665:000e`; interpretation: `LZEXE loader entry only; not a decompressed FIRES runtime text address`.
- ReDMCSB `*.MAP` artifacts found: `0`.
- Candidate FIRES runtime dumps found: `1`.
- FIRES-like original binaries inventoried: `7`.
- Unpack/link tools found: `4`.
- Inventory conclusion: `only packed/original FIRES-like binaries and tooling are present; no TLINK .MAP or verified decompressed runtime dump was found`.

## Source seams re-audited
- PASS `command_accepted` — `COMMAND.C:2095-2127,2150-2156,2322-2324`; function(s): `F0380_COMMAND_ProcessQueue_CPSC`
- PASS `movement_applied` — `CLIKMENU.C:256-270,317-329,345-347`; function(s): `F0366_COMMAND_ProcessTypes3To6_MoveParty`
- PASS `party_coordinates_committed` — `MOVESENS.C:438-444,493-496,573-578`; function(s): `F0267_MOVE_GetMoveResult_CPSCE`
- PASS `viewport_present` — `GAMELOOP.C:83-91,164-168,215-219`; function(s): `F0128_DUNGEONVIEW_Draw_CPSF / F0380_COMMAND_ProcessQueue_CPSC cadence`
- PASS `viewport_present_blit` — `DRAWVIEW.C:849-858`; function(s): `F0097_DUNGEONVIEW_DrawViewport`

## Binding rule
A debugger hit can be promoted only when the trace includes PSP/load segment, post-LZEXE transfer or map/decompressed-image evidence, symbol segment:offset, and observed hit CS:IP/context for each seam. Static compressed offsets and the MZ loader entry are explicitly rejected.

## Artifacts
- Manifest: `/home/trv2/work/firestaff/parity-evidence/verification/pass210_dm1_v1_original_runtime_binding_guard/manifest.json`
- Trace contract: `/home/trv2/work/firestaff/parity-evidence/verification/pass210_dm1_v1_original_runtime_binding_guard/trace_binding_contract.json`
- Runtime trace template: `/home/trv2/work/firestaff/parity-evidence/verification/pass210_dm1_v1_original_runtime_binding_guard/runtime_trace_template.json`
- Guarded runbook: `/home/trv2/work/firestaff/parity-evidence/verification/pass210_dm1_v1_original_runtime_binding_guard/guarded_runtime_binding_runbook.md`

## Non-claims
- does not claim a loaded/decompressed FIRES runtime image base
- does not claim any source symbol is bound to stock runtime CS:IP
- does not claim debugger breakpoints were hit
- does not promote compressed LZEXE loader offsets as runtime text addresses
