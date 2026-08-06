#ifndef FIRESTAFF_DM1_V1_COMBAT_LOG_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_COMBAT_LOG_PC34_COMPAT_H

/*
 * dm1_v1_combat_log_pc34_compat — scrolling combat / event log.
 *
 * Ring-buffer log of combat-related events.  Hooks fire whenever the
 * combat path applies damage, a creature attacks, or a spell is cast.
 * Rendering is a half-transparent (dithered) overlay along the bottom
 * of the framebuffer, gated by M11_QolRuntime_GetCombatLogEnabled().
 */

#include <stdint.h>
#include "m11_game_view.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    DM1_V1_COMBAT_LOG_TYPE_INFO = 0,
    DM1_V1_COMBAT_LOG_TYPE_CHAMP_HIT,
    DM1_V1_COMBAT_LOG_TYPE_CREATURE_HIT,
    DM1_V1_COMBAT_LOG_TYPE_SPELL,
    DM1_V1_COMBAT_LOG_TYPE_MISS
} DM1_V1_CombatLogTypePc34;

typedef struct {
    char     text[128];
    uint32_t gameTick;
    uint8_t  type;
} DM1_V1_CombatLogEntryPc34;

/* Clear the log (called on new game / level reset). */
void DM1_CombatLog_Reset(void);

/* Push one entry (printf-style). Honours combatLogMaxLines. */
void DM1_CombatLog_Pushf(uint32_t gameTick,
                         DM1_V1_CombatLogTypePc34 type,
                         const char* fmt, ...);

/* Convenience hooks used by the combat / magic paths. */
void DM1_CombatLog_OnChampionHit(uint32_t gameTick,
                                 const char* championName,
                                 const char* creatureName,
                                 int damage);
void DM1_CombatLog_OnCreatureAttack(uint32_t gameTick,
                                    const char* creatureName,
                                    const char* championName);
void DM1_CombatLog_OnSpellCast(uint32_t gameTick,
                               const char* championName,
                               const char* spellName);

/* Source gate: authenticated DM1 routes never use the diagnostic fallback
 * font when the original GRAPHICS.DAT font is unavailable. */
static inline int DM1_CombatLog_SourceAllowsFallbackFont(
    M11_GameSourceKind sourceKind) {
    return sourceKind != M11_GAME_SOURCE_BUILTIN_CATALOG &&
           sourceKind != M11_GAME_SOURCE_CUSTOM_DUNGEON &&
           sourceKind != M11_GAME_SOURCE_DIRECT_DUNGEON;
}

/* The log text itself is Firestaff diagnostic text, not a DM1 PC34 surface.
 * Keep it out of authenticated DM1 sessions even when a persisted QoL
 * setting enables the overlay; source TEXT.C owns the real message lane. */
static inline int DM1_CombatLog_SourceAllowsDiagnosticOverlay(
    M11_GameSourceKind sourceKind) {
    return sourceKind != M11_GAME_SOURCE_BUILTIN_CATALOG &&
           sourceKind != M11_GAME_SOURCE_CUSTOM_DUNGEON &&
           sourceKind != M11_GAME_SOURCE_DIRECT_DUNGEON;
}

/* Render the overlay (no-op when disabled). */
void DM1_CombatLog_Render(M11_GameViewState* gameView,
                          unsigned char* framebuffer,
                          int fbWidth,
                          int fbHeight);

/* Compatibility aliases for older M11 call sites. */
typedef DM1_V1_CombatLogTypePc34 M11_CombatLogType;
typedef DM1_V1_CombatLogEntryPc34 M11_CombatLogEntry;
#define M11_COMBAT_LOG_TYPE_INFO DM1_V1_COMBAT_LOG_TYPE_INFO
#define M11_COMBAT_LOG_TYPE_CHAMP_HIT DM1_V1_COMBAT_LOG_TYPE_CHAMP_HIT
#define M11_COMBAT_LOG_TYPE_CREATURE_HIT DM1_V1_COMBAT_LOG_TYPE_CREATURE_HIT
#define M11_COMBAT_LOG_TYPE_SPELL DM1_V1_COMBAT_LOG_TYPE_SPELL
#define M11_COMBAT_LOG_TYPE_MISS DM1_V1_COMBAT_LOG_TYPE_MISS

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_COMBAT_LOG_PC34_COMPAT_H */
