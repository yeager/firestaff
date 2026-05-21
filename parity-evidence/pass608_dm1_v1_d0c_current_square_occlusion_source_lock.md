# Pass608 DM1 V1 D0C current-square occlusion source lock

Status: passed

Claim: D0C current-square foreground blockers, common F0115 content handoff, and teleporter-field overlay order are source-locked for DM1 V1 PC34/I34E.

Primary evidence:
- PASS DUNVIEW.C:8534-8542: F0128 draws D0L and D0R before the current-square D0C pass.
- PASS DUNVIEW.C:8184-8240: D0C door-side foreground frame work completes and breaks before the common F0115 current-square content pass.
- PASS DUNVIEW.C:8241-8308: D0C stairs-front foreground breaks; pit falls through to ceiling, F0115 with C0x0021, then teleporter field in C715.
- PASS DUNVIEW.C:4561-4582: F0115 draws objects, creatures, and projectiles per ordered cell, then explosions after ordered cells.
- PASS DEFS.H:2596-2647: PC34 MEDIA720 view-square and cell ordinals make D0C id 0 and define the back-left/back-right cell names used by C0x0021.
- PASS DEFS.H:4055-4219: PC34/I34E zone ids bind D0C field, frame, thieves-eye, stairs, floor pit, and ceiling pit draw destinations.

Verification:
- /home/trv2/work/firestaff-worktrees/n2-viewport-wall-contract-20260521-0629/build/test_dm1_v1_viewport_3d_pc34_compat: rc=0

Non-claims:
- Does not duplicate pass517 D3/D2 side-field occlusion.
- Does not duplicate pass518 D1 side-field occlusion.
- Does not duplicate pass583 D0 side-field occlusion.
- Does not claim original DOS pixel parity or capture-backed closure.
- Does not change renderer behavior.
- No DANNESBURK use.
