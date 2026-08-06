/* Nexus V1 production light boundary.
 *
 * The simple torch/FUL/ambient light state machine is a DM1-shaped host
 * compatibility model. The Saturn light command, timer/state writeback and
 * HUD/VDP consumer are not captured, so production keeps the ABI linkable
 * while rejecting every inferred mutation.
 */

#include "nexus_v1_light.h"

void nexus_v1_light_init(Nexus_LightState *ls) { (void)ls; }
void nexus_v1_light_tick(Nexus_LightState *ls) { (void)ls; }
void nexus_v1_light_set(Nexus_LightState *ls, int level)
{ (void)ls; (void)level; }
void nexus_v1_light_add(Nexus_LightState *ls, int amount)
{ (void)ls; (void)amount; }
void nexus_v1_light_torch_on(Nexus_LightState *ls, int burn_ticks)
{ (void)ls; (void)burn_ticks; }
void nexus_v1_light_torch_off(Nexus_LightState *ls) { (void)ls; }
void nexus_v1_light_ful_spell(Nexus_LightState *ls, int power,
                             int duration_ticks)
{ (void)ls; (void)power; (void)duration_ticks; }
int nexus_v1_light_get(const Nexus_LightState *ls)
{ (void)ls; return 0; }
