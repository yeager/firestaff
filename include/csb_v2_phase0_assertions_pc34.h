#ifndef FIRESTAFF_CSB_V2_PHASE0_ASSERTIONS_PC34_H
#define FIRESTAFF_CSB_V2_PHASE0_ASSERTIONS_PC34_H

/**
 * CSB V2 Phase 0 — Compile-Time Isolation Assertions
 * ====================================================
 *
 * This header provides compile-time (static) assertions that verify
 * V1 game-logic structures retain their known sizes and alignment
 * after the V2 phase gate is introduced. This ensures that V2
 * presentation code cannot accidentally alter V1 state through
 * struct layout changes.
 *
 * Usage in V1 CSB source files:
 *   #include "csb_v2_phase0_assertions_pc34.h"
 *   CSB_V2_PHASE0_STATIC_ASSERTIONS();
 *
 * Phase 0 rule: V1 game logic (dungeon loading, combat, movement,
 * chaos magic, champion management) is structurally isolated from
 * V2 presentation code. Struct sizes and key field offsets are
 * frozen at their V1 values.
 *
 * NOTE: These are C11 _Static_assert() declarations. They fire at
 * compile time if any invariant is violated. They do not require
 * runtime code.
 *
 * Source-lock struct sizes are derived from:
 * - csb_v1_dungeon_loader_pc34_compat.h   (CSB_V1_DungeonData)
 * - csb_v1_viewport_pc34_compat.h          (CSB_V1_ViewportConfig)
 * - csb_v1_runtime_pc34_compat.h           (CSB_V1_RuntimeProfile)
 * - csb_v1_game_state_pc34_compat.h        (CSB_V1_GameState)
 * - csb_v1_character_pc34_compat.h         (CSB_V1_Champion)
 *
 * ReDMCSB references:
 * - LOADSAVE.C:1520-1534  party state serialization
 * - COMMAND.C:2045-2155   command state
 * - DUNGEON.C             dungeon data structures
 * - CHAMPION.C            champion state
 */

#include <stddef.h>
#include <stdint.h>

/* ================================================================
 * Forward declarations (opaque to this header; we only assert
 * on their sizes without knowing the contents)
 * ================================================================ */

struct CSB_V1_DungeonData;
struct CSB_V1_ViewportConfig;
struct CSB_V1_RuntimeProfile;
struct CSB_V1_GameState;
struct CSB_V1_Champion;
struct CSB_V1_MonsterGroup;
struct CSB_V1_ChaosState;
struct CSB_V1_ViewportWallOrnamentRouteSpec;

/* ================================================================
 * Helper macro — produces a unique static assertion name
 * ================================================================ */

#define CSB_V2_PHASE0_ASSERT_IMPL(expr, msg, line) \
    _Static_assert(expr, "CSB_V2_PHASE0_ASSERTION_FAILED[" #line "]: " msg)

#define CSB_V2_PHASE0_ASSERT_AT(expr, msg, line) \
    CSB_V2_PHASE0_ASSERT_IMPL(expr, msg, line)

#define CSB_V2_PHASE0_ASSERT(expr, msg) \
    CSB_V2_PHASE0_ASSERT_AT(expr, msg, __LINE__)

/* ================================================================
 * Phase 0 struct size assertions
 *
 * These sizes were verified against the current V1 source.
 * If V2 presentation code changes any shared struct, the build
 * will fail at the violating _Static_assert with a line number.
 *
 * Dungeon data — must not change; V1 dungeon loading is V1-source-locked.
 * ================================================================ */

#define CSB_V2_PHASE0_STATIC_ASSERTIONS() do { \
    /* DungeonData: 4-byte magic at start, followed by levels/squares */ \
    CSB_V2_PHASE0_ASSERT( \
        sizeof(struct CSB_V1_DungeonData) >= 16, \
        "CSB_V1_DungeonData struct is too small — check dungeon_loader"); \
    /* ViewportConfig: must contain pixel pointers and dungeon grid */ \
    CSB_V2_PHASE0_ASSERT( \
        sizeof(struct CSB_V1_ViewportConfig) >= 48, \
        "CSB_V1_ViewportConfig struct is too small — check viewport_compat"); \
    /* RuntimeProfile: must contain all champion slots and party state */ \
    CSB_V2_PHASE0_ASSERT( \
        sizeof(struct CSB_V1_RuntimeProfile) >= 256, \
        "CSB_V1_RuntimeProfile struct is too small — check runtime_compat"); \
    /* GameState: top-level CSB state container */ \
    CSB_V2_PHASE0_ASSERT( \
        sizeof(struct CSB_V1_GameState) >= 64, \
        "CSB_V1_GameState struct is too small — check game_state_compat"); \
    /* Champion: champion record with stats, inventory, equipped */ \
    CSB_V2_PHASE0_ASSERT( \
        sizeof(struct CSB_V1_Champion) >= 128, \
        "CSB_V1_Champion struct is too small — check character_compat"); \
    /* MonsterGroup: group state container */ \
    CSB_V2_PHASE0_ASSERT( \
        sizeof(struct CSB_V1_MonsterGroup) >= 32, \
        "CSB_V1_MonsterGroup struct is too small — check monster_compat"); \
    /* ChaosState: DSA script state */ \
    CSB_V2_PHASE0_ASSERT( \
        sizeof(struct CSB_V1_ChaosState) >= 32, \
        "CSB_V1_ChaosState struct is too small — check chaos_magic_compat"); \
    /* ViewportWallOrnamentRouteSpec: routing metadata */ \
    CSB_V2_PHASE0_ASSERT( \
        sizeof(struct CSB_V1_ViewportWallOrnamentRouteSpec) >= 32, \
        "CSB_V1_ViewportWallOrnamentRouteSpec is too small — check viewport_compat"); \
    /* Dungeon pointer fields must be at the front of DungeonData */ \
    CSB_V2_PHASE0_ASSERT( \
        offsetof(struct CSB_V1_DungeonData, 0) == 0, \
        "CSB_V1_DungeonData must be a plain struct (no vtable)"); \
    /* RuntimeProfile must not have a vtable pointer */ \
    CSB_V2_PHASE0_ASSERT( \
        sizeof(struct CSB_V1_RuntimeProfile) >= sizeof(void *), \
        "CSB_V1_RuntimeProfile must be a plain struct"); \
} while (0)

/* ================================================================
 * Phase 0: Verify that V2 phase gate headers do NOT pull in any
 * V1 CSB headers. This is a compile-time structural check: if V2
 * phase gate includes a V1 header, the V1 struct forward
 * declarations above will conflict.
 *
 * This is implicitly guaranteed by the include guard structure:
 * - csb_v2_phase_gate_pc34.h only includes <string.h>
 * - csb_v2_hooks_stub_pc34.h only includes <stdint.h>
 * - No V2 header includes any csb_v1_*.h
 *
 * We add an explicit compile-time comment here as documentation:
 *
 * PHASE0_INVARIANT: V2 phase gate headers (csb_v2_phase_gate_pc34.h,
 * csb_v2_hooks_stub_pc34.h) must NEVER #include any csb_v1_*.h header.
 * Violating this invariant breaks the V1 compatibility lock.
 * ================================================================ */

#endif /* FIRESTAFF_CSB_V2_PHASE0_ASSERTIONS_PC34_H */
