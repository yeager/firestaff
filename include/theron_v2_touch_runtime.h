#ifndef FIRESTAFF_THERON_V2_TOUCH_RUNTIME_H
#define FIRESTAFF_THERON_V2_TOUCH_RUNTIME_H

#include "dm1_v1_input_command_queue_pc34_compat.h"
#include "theron_v2_phase_gate_pc34.h"
#include "theron_v2_touch_controller_affordance.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Theron V2 Phase 6 touch/controller runtime bridge.
 *
 * Translates Theron V2 affordances into the shared DM1-family V1 command
 * queue entry shape.  V2 presentation must be active; V1 faithful mode keeps
 * the existing mouse/touch/click matrix as the sole input path.
 */

#define THERON_V2_TOUCH_FRAMEBUFFER_W 256
#define THERON_V2_TOUCH_FRAMEBUFFER_H 224

void theron_v2_touch_runtime_init(void);
void theron_v2_touch_runtime_shutdown(void);
void theron_v2_touch_runtime_set_gate_config(const THERON_V2_PhaseGateConfig *config);

int theron_v2_touch_runtime_translate_affordance(
    Theron_V2_TouchControllerAffordance affordance,
    int x,
    int y,
    struct Dm1V1QueuedCommandPc34Compat *out);

/* Returns 1 for framebuffer points owned by V2 HUD chrome instead of the
 * dungeon gesture surface.  Controller affordances bypass this point gate. */
int theron_v2_touch_runtime_point_in_hud_chrome(int x, int y);

int theron_v2_touch_runtime_is_active(void);
void theron_v2_touch_runtime_force_active_for_test(int active);
int theron_v2_touch_runtime_translation_count(void);
const char *theron_v2_touch_runtime_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_THERON_V2_TOUCH_RUNTIME_H */
