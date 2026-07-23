#ifndef CSB_V1_COMMAND_INPUT_GEOMETRY_PC34_COMPAT_H
#define CSB_V1_COMMAND_INPUT_GEOMETRY_PC34_COMPAT_H

/*
 * Resolves a CSB gameplay pointer event against the original shared
 * COMMAND.C G0448 movement surface.  The geometry itself remains owned by
 * dm1_v1_mouse_routes_pc34_compat: this adapter never invents click zones.
 */

#include "csb_v1_input_command_bridge_pc34_compat.h"
#include "menu_startup_m12.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int matched;
    int command;
    int zone_id;
    M12_MenuInput input;
} CSB_V1_CommandInputGeometryResultPc34Compat;

/* Returns 1 only for a left-click in a source G0448 C001..C006 movement
 * zone.  Other source table entries (C080 viewport click and C083 right
 * click) intentionally remain unclaimed for their own command owners. */
int CSB_V1_CommandInputGeometryFromPointerPc34Compat(
    int screen_x,
    int screen_y,
    int button_mask,
    CSB_V1_CommandInputGeometryResultPc34Compat* out_result);

/* Resolve one real G0448 movement-arrow click and commit it through the
 * same authenticated keyboard/command queue used by F0361/F0380.  This is
 * deliberately limited to C001..C006: viewport and inventory clicks retain
 * their original COMMAND.C owners.  Returns the bridge dispatch result:
 * 1 for one consumed command, 0 for an unmatched/rejected source click, and
 * -1 for an invalid runtime dispatch. */
int CSB_V1_CommandInputGeometryProcessPointerPc34Compat(
    CSB_V1_RuntimeProfile* profile,
    int screen_x,
    int screen_y,
    int button_mask,
    int disabled_movement_ticks,
    int projectile_disabled_movement_ticks,
    int last_projectile_disabled_movement_direction,
    CSB_V1_CommandInputGeometryResultPc34Compat* out_geometry,
    CSB_V1_InputCommandBridgeResult* out_bridge);

const char* CSB_V1_CommandInputGeometrySourceEvidencePc34Compat(void);

#ifdef __cplusplus
}
#endif

#endif
