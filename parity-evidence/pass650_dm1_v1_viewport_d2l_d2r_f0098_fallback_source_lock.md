# Pass650 DM1 V1 D2L/D2R F0098 fallback source lock

Status: passed

DM1 V1 D2L/D2R non-wall F0098 floor/ceiling fallback order is source-locked against ReDMCSB.

Primary evidence:
- PASS DUNVIEW.C:2962-3002: F0098 owns floor/ceiling refresh and clears the dirty flag.
- PASS DUNVIEW.C:8337-8610: F0128 gates F0098 on the dirty flag, performs F0099 flip work, dispatches D2L/D2R, then presents with F0097.
- PASS DUNVIEW.C:6900-7049: D2L non-wall branch reaches floor ornament, ceiling pit, F0115, and optional field without taking the C00 wall/F0107 return.
- PASS DUNVIEW.C:7051-7224: D2R non-wall branch mirrors the D2L fallback order without the C00 wall/F0107 return path.
- PASS DEFS.H:2582-4223: DEFS.H binds D2L/D2R view-square, floor-view, cell-order, and floor/ceiling/zone ids.

Local gates:
- PASS src/dm1/dm1_v1_viewport_d2l_d2r_f0098_fallback_pc34_compat.h
- PASS src/dm1/dm1_v1_viewport_d2l_d2r_f0098_fallback_pc34_compat.c
- PASS tests/test_dm1_v1_viewport_d2l_d2r_f0098_fallback_pc34_compat.c
- PASS CMakeLists.txt

Verification:
- /Users/bosse/.openclaw/workspace-main/build/test_dm1_v1_viewport_d2l_d2r_f0098_fallback_pc34_compat: rc=0

Non-claims:
- Does not duplicate D2L/D2R wall table gates.
- Does not duplicate D2L2/D2R2 wall source-lock gates.
- Does not duplicate D0C F0098 row-ownership coverage.
- Does not claim original DOS pixel parity or capture-backed closure.
- Does not change renderer behavior.
