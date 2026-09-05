# DM1 F0446 event-local delay test alignment

Status: **source-aligned test repair**

ReDMCSB `ENDGAME.C` F0446 calls F0445 once for each ordered victory
TextString, then immediately calls `F0022_MAIN_Delay(780)`. After every text
message has completed, it calls `F0022_MAIN_Delay(600)` before handing off to
F0444. Consequently, Firestaff's presentation replay must not expose the next
F0445 event while either message-local 780-tick wait is active.

`m11_action_stamina_runtime_source_lock` previously drained the remaining 44
F0445 events with exactly 44 idle calls. That assumption became stale when the
runtime implemented the source-owned event-local waits: the test stopped at
event 44, and its seven later presentation/restart failures were cascading
consequences of that incomplete drain.

The test now drains replay events while respecting both 780-tick waits, proves
that 600 ticks remain after the second message, and then drains the final wait.
It also expects missing F0444 graphics/material to fail closed. Dedicated
F0444 material and final-presentation tests retain the positive admission
coverage; the reduced synthetic fixture must not manufacture restart or quit
controls.

Verified tests:

- `m11_action_stamina_runtime_source_lock`
- `dm1_v1_f0444_f0445_f0446_endgame_material_pc34_compat`
- `dm1_v1_endgame_presentation_pc34_compat`
- `m11_dm1_endgame_final_presentation_receipt_pc34`
- `m11_dm1_endgame_delay_order_pc34`
