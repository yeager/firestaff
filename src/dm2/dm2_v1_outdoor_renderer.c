
#include "dm2_v1_outdoor_renderer.h"
#include <string.h>

/* DM2 outdoor renderer source-material boundary.
 *
 * skproject realizes ENVIRONMENT art through QUERY_TEMP_PICST followed by
 * DRAW_TEMP_PICST.  A weather/time state alone does not identify a GDAT image,
 * its local palette, or its destination rectangle. */

void dm2_v1_outdoor_init(DM2_V1_OutdoorConfig *cfg) {
    if (!cfg) return;
    memset(cfg, 0, sizeof(*cfg));
    /* No default sky/time state: ENVIRONMENT owns both its selected image
     * and the temporal state that selects it. This legacy no-draw facade
     * must not seed an apparently playable noon scene. */
}

void dm2_v1_outdoor_set_weather(DM2_V1_OutdoorConfig *cfg, int weather) {
    if (cfg) cfg->weather = weather;
}

void dm2_v1_outdoor_set_time(DM2_V1_OutdoorConfig *cfg, float time) {
    if (cfg) cfg->time_of_day = time < 0 ? 0 : (time > 1 ? 1 : time);
}

uint32_t dm2_v1_outdoor_sky_color(const DM2_V1_OutdoorConfig *cfg) {
    (void)cfg;
    /* Do not draw a gradient that is absent from the selected GDAT material. */
    return DM2_V1_OUTDOOR_SOURCE_COLOR_UNAVAILABLE;
}

const char *dm2_v1_outdoor_source_evidence(void) {
    return "skproject SKWIN/c_bkgrnd.cpp: ENVIRONMENT_DRAW_DISTANT_ELEMENT\n"
           "skproject SKWIN/skgdtqdb.cpp: QUERY_TEMP_PICST/DRAW_TEMP_PICST\n"
           "Policy: no synthetic outdoor sky colour without GDAT receipt\n";
}
