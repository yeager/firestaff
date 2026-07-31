/* Nexus V2 HUD production boundary.
 *
 * The original Saturn HUD surface/palette ownership is not yet authenticated
 * from the supplied data.  Keep M11's lifecycle ABI available, but do not
 * copy synthetic compass, font, icon, bar, or counter pixels into the V1
 * framebuffer.  The old bitmap fixture remains probe-only. */

#include "nexus_v2_hud_runtime.h"

void nexus_v2_hud_runtime_init(void) {}
void nexus_v2_hud_runtime_shutdown(void) {}
void nexus_v2_hud_runtime_set_gate_config(
    const NEXUS_V2_PhaseGateConfig *config) { (void)config; }
void nexus_v2_hud_runtime_set_party_gold(int gold_pieces) { (void)gold_pieces; }
void nexus_v2_hud_runtime_set_direction(int dir) { (void)dir; }
void nexus_v2_hud_runtime_set_level(int cur, int max) {
    (void)cur; (void)max;
}
void nexus_v2_hud_runtime_set_champion(int champ_idx, int hp_pct,
    int stamina_pct, int mana_pct, bool leader, bool spell_ready) {
    (void)champ_idx; (void)hp_pct; (void)stamina_pct; (void)mana_pct;
    (void)leader; (void)spell_ready;
}
void nexus_v2_hud_runtime_set_action_active(Nexus_V2_ActionIcon icon) {
    (void)icon;
}
void nexus_v2_hud_runtime_trigger_hit_flash(void) {}
void nexus_v2_hud_runtime_set_opacity(uint8_t val) { (void)val; }
void nexus_v2_hud_runtime_render(uint8_t *fb, int w, int h_res) {
    (void)fb; (void)w; (void)h_res;
}
int nexus_v2_hud_runtime_is_active(void) { return 0; }
void nexus_v2_hud_runtime_force_active_for_test(int active) { (void)active; }
const char *nexus_v2_hud_runtime_source_evidence(void) {
    return "Nexus V2 HUD production route blocked: Saturn HUD surface and palette ownership unverified";
}
