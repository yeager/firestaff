# Pass 1113 — DM2 v1 Database Allocation (c_dballoc.cpp)

## Source

skproject `SKULLWIN/c_dballoc.cpp` — 35 functions across three classes:
`c_cpx_linklist`, `c_ulp`, `c_dballochandler`.

## Ported functions

### CPX linked list (c_cpx_linklist)

| skproject function | Firestaff function | Status |
|---|---|---|
| DM2_ALLOC_CPX_LINK_NODE (SKW_3e74_0d32) | dm2_v1_dballoc_cpx_link_node | Proven |
| DM2_ALLOC_CPX_UNLINK_NODE (SKW_3e74_0c8c) | dm2_v1_dballoc_cpx_unlink_node | Proven |
| DM2_ALLOC_CPX1 | dm2_v1_dballoc_cpx1 | Proven |

### Preserved GFX table

| skproject function | Firestaff function | Status |
|---|---|---|
| DM2_FIND_IN_PRESERVED_GFX (SKW_3e74_5420) | dm2_v1_dballoc_find_in_preserved_gfx | Proven |
| DM2_ADD_TO_PRESERVED_GFX (SKW_3e74_54a1) | dm2_v1_dballoc_add_to_preserved_gfx | Proven |

### W-table and pool descriptors

| skproject function | Firestaff function | Status |
|---|---|---|
| DM2_ALLOC_CPX_GET_WTABLE_ENTRY (SKW_3e74_0c62) | dm2_v1_dballoc_get_wtable_entry | Proven |
| DM2_SET_PPPW_ENTRY (R_2D7EC) | dm2_v1_dballoc_set_pppw_entry | Proven |

### CPX heap allocator

| skproject function | Firestaff function | Status |
|---|---|---|
| DM2_GUARANTEE_FREE_CPXHEAP_SIZE | dm2_v1_dballoc_guarantee_free_cpxheap_size | Proven |
| DM2_ALLOC_CPXHEAP_CREATE_POINTER (SKW_ALLOC_LOWER_CPXHEAP) | dm2_v1_dballoc_cpxheap_create_pointer | Proven |
| DM2_ALLOC_CPXHEAP_CREATE_INDEX (R_2E4B9) | dm2_v1_dballoc_cpxheap_create_index | Proven |
| DM2_INIT_GFX_TABLE (SKW_3e74_2b30) | dm2_v1_dballoc_init_gfx_table | Proven |

### ID generator

| skproject function | Firestaff function | Status |
|---|---|---|
| DM2_ALLOC_GENERATE_ID (DM2_dballoc_3e74_53ea) | dm2_v1_dballoc_generate_id | Proven |

## Not ported (require game state integration)

The following functions depend on runtime game state (dungeon data, timer system,
graphics loading, save/load) and are deferred to future integration passes:

- DM2_SETUP_DB_ALLOCATION, DM2_ALLOC_CPX_SETUP, DM2_dballoc_38c8_0109,
  DM2_dballoc_38c8_0224 — startup/setup requiring pool memory subsystem
- DM2_ALLOCATION1 through DM2_ALLOCATION11 — LRU cache management requiring
  full gfx_table and w_table state
- DM2_ALLOC_CPXHEAP_MEM, DM2_ALLOCATION9 — combined pointer+index allocation
- DM2_GET_BMP, DM2_ALLOCATE_GFX16, DM2_ALLOCATE_GFX256 — graphics access
- DM2_ALLOC_NEW_CREATURE — creature allocation requiring record system
- DM2_DEALLOC_BIGPOOL_STRUCT_BEFORE — deallocation requiring pool system
- c_ulp methods — GDAT raw data pointer/length table
- s_hex6::splitlong — trivial byte-split helper

## Test results

29 tests, 29 passed, 0 failed.

## Files

- Header: `include/dm2_v1_dballoc_pc34_compat.h`
- Source: `src/dm2/dm2_v1_dballoc_pc34_compat.c`
- Test: `tests/test_dm2_v1_dballoc_pc34_compat.c`
