/* Nexus V1 production action/world-state boundary.
 *
 * The action timer, door, trap and projectile implementations are DM1-shaped
 * state machines. No Saturn command queue, SDDRVS transition, actor trigger
 * or projectile DMA/writeback consumer is captured, so production keeps the
 * ABI available but rejects every inferred mutation.
 */

#include "nexus_v1_action_timer.h"
#include "nexus_v1_doors.h"
#include "nexus_v1_traps.h"
#include "nexus_v1_projectiles.h"

void nexus_v1_action_timers_init(Nexus_ActionTimers *timers) { (void)timers; }
void nexus_v1_action_timers_tick(Nexus_ActionTimers *timers) { (void)timers; }
int nexus_v1_action_is_ready(const Nexus_ActionTimers *timers, int party_slot)
{ (void)timers; (void)party_slot; return 0; }
void nexus_v1_action_start_cooldown(Nexus_ActionTimers *timers, int party_slot,
                                    int base_ticks,
                                    const Nexus_V1_Champion *champion)
{ (void)timers; (void)party_slot; (void)base_ticks; (void)champion; }
int nexus_v1_action_remaining(const Nexus_ActionTimers *timers, int party_slot)
{ (void)timers; (void)party_slot; return 0; }
int nexus_v1_action_fraction(const Nexus_ActionTimers *timers, int party_slot)
{ (void)timers; (void)party_slot; return 0; }

void nexus_v1_door_manager_init(Nexus_DoorManager *mgr) { (void)mgr; }
int nexus_v1_door_register(Nexus_DoorManager *mgr, int map_x, int map_y,
                           int map_index, int type, int key_id,
                           int bash_resistance, int flags)
{ (void)mgr; (void)map_x; (void)map_y; (void)map_index; (void)type;
  (void)key_id; (void)bash_resistance; (void)flags; return -1; }
int nexus_v1_door_find(const Nexus_DoorManager *mgr, int map_x, int map_y,
                       int map_index)
{ (void)mgr; (void)map_x; (void)map_y; (void)map_index; return -1; }
int nexus_v1_door_try_open(Nexus_DoorManager *mgr, int door_idx,
                           int held_key_id, int bash_strength)
{ (void)mgr; (void)door_idx; (void)held_key_id; (void)bash_strength;
  return NEXUS_DOOR_RESULT_NO_KEY; }
int nexus_v1_door_close(Nexus_DoorManager *mgr, int door_idx)
{ (void)mgr; (void)door_idx; return -1; }
int nexus_v1_door_toggle(Nexus_DoorManager *mgr, int door_idx)
{ (void)mgr; (void)door_idx; return -1; }
void nexus_v1_door_tick(Nexus_DoorManager *mgr) { (void)mgr; }
int nexus_v1_door_is_passable(const Nexus_DoorManager *mgr, int door_idx)
{ (void)mgr; (void)door_idx; return 0; }
float nexus_v1_door_anim_progress(const Nexus_DoorManager *mgr, int door_idx)
{ (void)mgr; (void)door_idx; return 0.0f; }

void nexus_v1_trap_manager_init(Nexus_V1_TrapManager *mgr) { (void)mgr; }
int nexus_v1_trap_add(Nexus_V1_TrapManager *mgr, const Nexus_V1_Trap *trap)
{ (void)mgr; (void)trap; return 0; }
const Nexus_V1_Trap *nexus_v1_trap_find(const Nexus_V1_TrapManager *mgr,
                                        int level, int x, int y)
{ (void)mgr; (void)level; (void)x; (void)y; return NULL; }
Nexus_V1_TrapResult nexus_v1_trap_trigger(Nexus_V1_TrapManager *mgr,
                                          Nexus_V1_Champion *champion,
                                          int level, int x, int y)
{ Nexus_V1_TrapResult result = {0};
  (void)mgr; (void)champion; (void)level; (void)x; (void)y; return result; }
void nexus_v1_trap_tick(Nexus_V1_TrapManager *mgr) { (void)mgr; }

void nexus_v1_projectiles_init(Nexus_ProjectileManager *mgr) { (void)mgr; }
int nexus_v1_projectile_spawn(Nexus_ProjectileManager *mgr,
                              enum Nexus_ProjectileType type, int x, int y,
                              int dir, int damage, int speed_ticks,
                              int source_champion)
{ (void)mgr; (void)type; (void)x; (void)y; (void)dir; (void)damage;
  (void)speed_ticks; (void)source_champion; return -1; }
int nexus_v1_projectiles_tick(Nexus_ProjectileManager *mgr,
                              const uint8_t squares[NEXUS_MAX_MAP_SIZE]
                                                    [NEXUS_MAX_MAP_SIZE],
                              Nexus_ProjectileHit *out_hits, int max_hits)
{ (void)mgr; (void)squares; (void)out_hits; (void)max_hits; return 0; }
int nexus_v1_projectile_count(const Nexus_ProjectileManager *mgr)
{ (void)mgr; return 0; }
