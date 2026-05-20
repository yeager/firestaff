#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
checks = {
    'include/memory_timeline_pc34_compat.h': [
        'TIMELINE_EVENT_MOVE_GROUP_SILENT',
        'TIMELINE_EVENT_MOVE_GROUP_AUDIBLE',
    ],
    'src/memory/memory_tick_orchestrator_pc34_compat.c': [
        'ReDMCSB MOVESENS.C:F0265:169-192',
        'MOVESENS.C:F0267:830-844',
        'ReDMCSB GROUP.C:F0185:543-545',
        'ReDMCSB TIMELINE.C:F0252:1527-1535',
        'lines 1565-1567 retry',
        'TIMELINE_EVENT_MOVE_GROUP_SILENT',
        'TIMELINE_EVENT_MOVE_GROUP_AUDIBLE',
        'orch_schedule_deferred_group_move_compat',
        'orch_handle_deferred_group_move_event_compat',
    ],
    'tests/test_m10_c006_generator_reenable_dispatch_pc34_compat.c': [
        'blocked C006 generator keeps initialized group slot for event60',
        'blocked C006 generator schedules silent event60 insertion retry',
        'event60 retry links deferred group when destination clears',
        'event60 retry schedules source C37 wander after insertion',
    ],
}

missing = []
for rel, needles in checks.items():
    text = (ROOT / rel).read_text()
    for needle in needles:
        if needle not in text:
            missing.append(f'{rel}: missing {needle!r}')

if missing:
    raise SystemExit('\n'.join(missing))

print('C006_EVENT60_61_DEFER_SOURCE_LOCK_OK')
