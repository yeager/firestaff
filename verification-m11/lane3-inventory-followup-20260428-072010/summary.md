# Lane 3 inventory follow-up

UTC: 2026-04-28T07:20Z
Branch: sync/n2-dm1-v1-20260428
Head before evidence commit: f532cc4

## Scope
Verified current DM1 V1 inventory/UI/slots/icons evidence on N2. Focused on source-backed normal inventory dynamic slot drawing, source slot-box mapping, leader-hand pickup, action/inventory icon palette split, and GRAPHICS.DAT slot-box loader path.

## Commands

```sh
./run_firestaff_memory_graphics_dat_slots_probe.sh 2>&1 | tee "verification-m11/lane3-inventory-followup-20260428-072010/slots_probe.log"
./run_firestaff_m11_game_view_probe.sh "verification-m11/lane3-inventory-followup-20260428-072010" 2>&1 | tee "verification-m11/lane3-inventory-followup-20260428-072010/game_view_probe.full.log"
```

## Results

- slots probe: PASS (`ok`)
- M11 game-view probe: PASS (`# summary: 578/578 invariants passed`)
- inventory-specific covered gates include INV_GV_353-363 and INV_GV_309B/300-309.

## Conclusion

No small source-faithful inventory fix was identified in this bounded follow-up. Current lane evidence passes with assets available (`assetsAvailable=1 cyanCellsDrawn=2`). Remaining documented larger gap is compatibility-model expansion for original backpack slots beyond Firestaff modeled `C527`.

## Additional gate note

- `ctest --test-dir build --output-on-failure`: FAIL/blocked by stale partial build tree; `m11_game_view`, `m11_launcher_smoke`, and `m11_ingame_capture_smoke` passed, but four configured probe executables were missing (`firestaff_m11_phase_a_probe`, `firestaff_m11_audio_probe`, `firestaff_m11_viewport_state_probe`, `firestaff_m11_capture_route_state_probe`). This is a build-tree completeness issue, not an inventory probe failure.
