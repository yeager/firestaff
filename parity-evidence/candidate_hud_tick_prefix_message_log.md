# Candidate HUD pass — V1 message-log tick prefix removal

Scope: HUD/UI text chrome only.

## Source-backed rationale

`V1_BLOCKERS.md` §12 tracks the `Tn:` prefix on message-log lines as a Firestaff convention from the Pass 35 text-vs-graphics audit, not DM1 PC 3.4 UI.

## Change

`m11_log_event(...)` now strips a leading `T<digits>: ` prefix before pushing text into `M11_MessageLog`. This keeps existing call sites intact while preventing the synthetic tick counter from reaching V1-visible/debug message-log storage.

## Gate

`INV_GV_302A` asserts the boot log entry no longer carries the old `T0: ` prefix.
