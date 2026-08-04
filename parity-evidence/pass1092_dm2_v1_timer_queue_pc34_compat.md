# Pass 1092 — DM2 timer priority queue (sktimer.cpp)

## Source

skproject/SKWINSPX/src/v5/sktimer.{h,cpp}

## What was ported

Binary min-heap priority queue for DM2 timer events. This is the core
timer infrastructure that all timed game events depend on: creature AI,
spell effects, light decay, door mechanics, cloud processing, etc.

### Functions ported

| skproject function | Firestaff function |
|---|---|
| c_timerdata::init | dm2_v1_timer_queue_init |
| c_tim::init | dm2_v1_timer_entry_init |
| DM2_cmp_timers | cmp_timers (static) |
| DM2_timer_3a15_0486 | sift (static) |
| DM2_REARRANGE_TIMERLIST | rearrange_freelist (static) |
| DM2_SORT_TIMERS | dm2_v1_timer_sort |
| DM2_GET_TIMER_NEW_INDEX | dm2_v1_timer_get_heap_index |
| DM2_DELETE_TIMER | dm2_v1_timer_delete |
| DM2_QUEUE_TIMER | dm2_v1_timer_queue |
| DM2_GET_AND_DELETE_NEXT_TIMER | dm2_v1_timer_get_and_delete_next |
| DM2_IS_TIMER_TO_PROCEED | dm2_v1_timer_is_due |
| DM2_timer_3a15_05f7 | dm2_v1_timer_reheapify |

### Data structures ported

- `c_tim` (12 bytes) → `DM2_V1_TimerEntry`
- `c_timerdata` → `DM2_V1_TimerQueue`

### Key design decisions

- Timer entry uses flat struct (no C++ union/method overhead)
- Inline accessor functions for packed l_00 field (map << 24 | ticks)
- Queue takes caller-owned arrays (no internal allocation)
- Added deferred-sift flush to get_and_delete_next for safety

## Tests

16 tests covering: init, queue/reject, min-heap ordering, is_due,
delete from middle, tiebreak by type and actor, map packing, inc_data,
sort from unsorted state, reheapify after modification, full capacity,
slot reuse after delete, heap index lookup.

## Files

- `include/dm2_v1_timer_queue_pc34_compat.h`
- `src/dm2/dm2_v1_timer_queue_pc34_compat.c`
- `tests/test_dm2_v1_timer_queue_pc34_compat.c`
