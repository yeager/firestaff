/* Nexus V1 production rest/status boundary.
 *
 * The DM1-shaped regeneration and poison/status state machines remain source
 * studies. The supplied Saturn corpus does not bind their action dispatch,
 * cadence, champion writeback or HUD/VDP consumer. Keep the production ABI
 * linkable while every operation stays state-preserving until that owner is
 * captured.
 */

#include "nexus_v1_rest.h"
#include "nexus_v1_status.h"

void nexus_v1_status_init(Nexus_StatusEffects *se) { (void)se; }
void nexus_v1_status_apply(Nexus_StatusEffects *se, int effect,
                           int duration, int strength)
{ (void)se; (void)effect; (void)duration; (void)strength; }
void nexus_v1_status_remove(Nexus_StatusEffects *se, int effect)
{ (void)se; (void)effect; }
int nexus_v1_status_is_active(const Nexus_StatusEffects *se, int effect)
{ (void)se; (void)effect; return 0; }
int nexus_v1_status_strength(const Nexus_StatusEffects *se, int effect)
{ (void)se; (void)effect; return 0; }
int nexus_v1_status_tick(Nexus_StatusEffects *se)
{ (void)se; return 0; }
int nexus_v1_status_poison_damage(const Nexus_StatusEffects *se)
{ (void)se; return 0; }
int nexus_v1_status_defense_bonus(const Nexus_StatusEffects *se)
{ (void)se; return 0; }

void nexus_v1_xp_init(Nexus_Experience *xp) { (void)xp; }
void nexus_v1_xp_add(Nexus_Experience *xp, int xp_class, int amount)
{ (void)xp; (void)xp_class; (void)amount; }
int nexus_v1_xp_check_levelup(Nexus_Experience *xp, int xp_class)
{ (void)xp; (void)xp_class; return 0; }
int nexus_v1_xp_threshold(int level)
{ (void)level; return 0; }

void nexus_v1_rest_init(Nexus_RestState *rs) { (void)rs; }
void nexus_v1_rest_start(Nexus_RestState *rs) { (void)rs; }
void nexus_v1_rest_stop(Nexus_RestState *rs) { (void)rs; }
int nexus_v1_rest_tick(Nexus_RestState *rs, Nexus_V1_ChampionPool *pool)
{ (void)rs; (void)pool; return 0; }
void nexus_v1_rest_interrupt(Nexus_RestState *rs) { (void)rs; }
int nexus_v1_rest_is_resting(const Nexus_RestState *rs)
{ (void)rs; return 0; }
int nexus_v1_rest_all_full(const Nexus_V1_ChampionPool *pool)
{ (void)pool; return 0; }
