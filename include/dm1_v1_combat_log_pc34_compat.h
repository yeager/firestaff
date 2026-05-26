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
    M11_COMBAT_LOG_TYPE_INFO = 0,
    M11_COMBAT_LOG_TYPE_CHAMP_HIT,
    M11_COMBAT_LOG_TYPE_CREATURE_HIT,
    M11_COMBAT_LOG_TYPE_SPELL,
    M11_COMBAT_LOG_TYPE_MISS
} M11_CombatLogType;

typedef struct {
    char     text[128];
    uint32_t gameTick;
    uint8_t  type;
} M11_CombatLogEntry;

/* Clear the log (called on new game / level reset). */
void DM1_CombatLog_Reset(void);

/* Push one entry (printf-style). Honours combatLogMaxLines. */
void DM1_CombatLog_Pushf(uint32_t gameTick,
                         M11_CombatLogType type,
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

/* Render the overlay (no-op when disabled). */
void DM1_CombatLog_Render(M11_GameViewState* gameView,
                          unsigned char* framebuffer,
                          int fbWidth,
                          int fbHeight);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_COMBAT_LOG_PC34_COMPAT_H */
