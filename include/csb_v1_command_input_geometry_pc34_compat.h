#ifndef CSB_V1_COMMAND_INPUT_GEOMETRY_PC34_COMPAT_H
#define CSB_V1_COMMAND_INPUT_GEOMETRY_PC34_COMPAT_H

/*
 * Resolves a CSB gameplay pointer event against the original shared
 * COMMAND.C G0448 movement surface.  The geometry itself remains owned by
 * dm1_v1_mouse_routes_pc34_compat: this adapter never invents click zones.
 */

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

const char* CSB_V1_CommandInputGeometrySourceEvidencePc34Compat(void);

#ifdef __cplusplus
}
#endif

#endif
