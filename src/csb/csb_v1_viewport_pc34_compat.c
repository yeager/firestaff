
#include "csb_v1_viewport_pc34_compat.h"
#include <string.h>

/* pass603: CSB V1 viewport
 *
 * CSBWin/Viewport.cpp: main rendering (7290 lines)
 * CSBWin/Graphics.cpp: asset loading/cache (3186 lines)
 * CSBWin/CSBCode.cpp: CustomBackgrounds (line 26)
 * ReDMCSB DUNVIEW.C: F0128_DUNGEONVIEW_Draw_CPSF (shared core)
 */

void csb_v1_viewport_init(CSB_V1_ViewportConfig *cfg) {
    if (!cfg) return;
    memset(cfg, 0, sizeof(*cfg));
    cfg->ambient_color = 0xFF000000; /* black ambient */
}

void csb_v1_viewport_set_wall_set(CSB_V1_ViewportConfig *cfg, int set) {
    if (cfg) cfg->wall_set_index = set;
}

void csb_v1_viewport_set_custom_background(CSB_V1_ViewportConfig *cfg, int bg_id) {
    if (cfg) cfg->custom_background = bg_id;
}

const char *csb_v1_viewport_source_evidence(void) {
    return
        "CSBWin/Viewport.cpp: 7290 lines viewport rendering\n"
        "CSBWin/Graphics.cpp: 3186 lines asset cache\n"
        "CSBWin/CSBCode.cpp:26 CustomBackgrounds\n"
        "CSBWin/CSBCode.cpp:9196 _DisplayChaosStrikesBack (prison door)\n"
        "ReDMCSB DUNVIEW.C F0128: shared viewport draw core\n";
}

