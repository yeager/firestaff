# pass764 DM1 V1 mirror candidate scroll-pickup party-rotate regression

Status: contract-only DM1 V1 runtime regression.

Scope: `test_dm1_v1_mirror_candidate_scroll_pickup_with_party_rotate_in_progress_pc34_compat` proves that a scroll-pickup routed through the panel path while `COMMAND.C F0380` is actively dispatching a party turn is ignored. The live C040 candidate index, chain, and redraw tuple are preserved. The party rotation completes before a second pickup attempt is honored.

Non-overlap: this is the party-rotation-in-progress branch, not candidate-internal rotation, and not the already-covered select/click/cancel/deadzone/cancel-reselect, lower-arrow, mid-close, reopen, pickup/drop, capacity, encumbrance, save-load, teleporter, or prior mirror-candidate race family.

ReDMCSB anchors:

- `PANEL.C F0344:1895-1944 + F0345:1946-1999` - panel click/cell-highlight route used by the scroll pickup attempt.
- `CHAMPION.C F0297:243-268` - leader-hand put.
- `CHAMPION.C F0298:270-298` - leader-hand remove/empty state.
- `CHAMPION.C F0302:662-713` - occupied slot and C30 chest-slot pickup dispatch.
- `COMMAND.C F0359:1985-1990` - M568/C040 pending panel click dispatch.
- `COMMAND.C F0361:1709-1813` - C001/C002 queue write.
- `COMMAND.C F0380:2045-2156` - queue dispatch, pending-click unlock, and C001/C002 turn routing.
- `MOUSE.C F0077:97-126 + F0078:128-168` - original wheel/click queue bracket named by the pass.
- `REVIVE.C F0280:124-132` - live candidate/C040 publish gate.
- `REVIVE.C F0282:744-806` - candidate clear/decision path, asserted not to fire.
- `CHAMDRAW.C F0291:498-560` - slot draw and C30 source.
- `CHAMDRAW.C F0292:703-735` - champion-state redraw.
- `CHAMDRAW.C F0296:1185-1252` - changed-object redraw and chest sweep.
- `DEFS.H:277 C040; 810 C30; 3906-3913 C537..C544; 5694 G0299; 5700 G0305; 873/876 M516_CHAMPIONS`.

Verification:

- Build target: `cmake --build build --target test_dm1_v1_mirror_candidate_scroll_pickup_with_party_rotate_in_progress_pc34_compat --parallel`
- Runtime: `./build/test_dm1_v1_mirror_candidate_scroll_pickup_with_party_rotate_in_progress_pc34_compat`
- CTest: `ctest --test-dir build -R 'dm1_v1_mirror_candidate_scroll_pickup_with_party_rotate_in_progress_pc34_compat' --output-on-failure`
- Whitespace: `git diff --check`

No original assets or `GRAPHICS.DAT` are required; the test is a deterministic contract-only regression.
