# Pass 1095 — DM2 rectangle operations (skrect.cpp)

## Source

skproject/SKWINSPX/src/v5/skrect.{h,cpp}

## What was ported

Rectangle primitives used by the DM2 viewport renderer, HUD layout,
and blit clipping system.

### Functions ported

| skproject function | Firestaff function |
|---|---|
| c_rect::init | dm2_v1_rect_init |
| c_rect::set (DM2_SET_SRECT) | dm2_v1_rect_set |
| c_rect::set_origin (DM2_SET_ORIGIN_RECT) | dm2_v1_rect_set_origin |
| c_rect::inflate (DM2_INFLATE_RECT) | dm2_v1_rect_inflate |
| c_rect::unify (DM2_UNION_RECT) | dm2_v1_rect_intersect |
| c_rect::calc_centered_rect_in_rect | dm2_v1_rect_center_in |
| c_rect::pt_in_rect (DM2_PT_IN_RECT) | dm2_v1_rect_contains |
| c_tmprects::init | dm2_v1_tmprects_init |
| c_tmprects::alloc_tmprect (DM2_ALLOC_TEMP_RECT) | dm2_v1_tmprects_alloc |
| c_tmprects::alloc_origin_tmprect | dm2_v1_tmprects_alloc_origin |

### Data structures ported

- `c_rect` → `DM2_V1_Rect`
- `c_tmprects` → `DM2_V1_TempRects` (4-entry ring buffer)
- `NUM_TMPRECTS = 4`

### Key design decisions

- `unify` renamed to `intersect` (it computes intersection, not union)
- Returns int (0/1) instead of pointer for C idiom clarity
- Global rects (dm2rect1-5) not ported — those are runtime state

## Tests

14 tests: init, set, set_origin, inflate, point-in-rect (inside/outside),
intersection (full overlap, partial clip, no overlap, right clip),
centering, temp rect ring buffer wrap-around, origin alloc, viewport rect.

## Files

- `include/dm2_v1_rect_pc34_compat.h`
- `src/dm2/dm2_v1_rect_pc34_compat.c`
- `tests/test_dm2_v1_rect_pc34_compat.c`
