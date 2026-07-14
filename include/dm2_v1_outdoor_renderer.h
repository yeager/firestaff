
#ifndef FIRESTAFF_DM2_V1_OUTDOOR_RENDERER_H
#define FIRESTAFF_DM2_V1_OUTDOOR_RENDERER_H
#include <stdint.h>

/* DM2 Outdoor Renderer — source-material admission boundary.
 *
 * Outdoor sky and ground pixels belong to ENVIRONMENT GDAT images.  This
 * legacy facade cannot select or palette-bind those images, so it must not
 * manufacture a time-of-day colour as a substitute.  Callers that need an
 * outdoor frame use the GDAT weather/viewport receipts instead.
 * Source: skproject SKWIN/c_bkgrnd.cpp ENVIRONMENT_DRAW_DISTANT_ELEMENT. */

typedef struct {
    int sky_texture;
    int ground_texture;
    int tree_density;
    int building_count;
    int weather; /* 0=clear, 1=rain, 2=fog, 3=storm */
    float time_of_day; /* 0.0-1.0, affects sky color */
} DM2_V1_OutdoorConfig;

void dm2_v1_outdoor_init(DM2_V1_OutdoorConfig *cfg);
void dm2_v1_outdoor_set_weather(DM2_V1_OutdoorConfig *cfg, int weather);
void dm2_v1_outdoor_set_time(DM2_V1_OutdoorConfig *cfg, float time);
/* Returns DM2_V1_OUTDOOR_SOURCE_COLOR_UNAVAILABLE until a source-owned
 * ENVIRONMENT image/palette/destination receipt is consumed by the viewport.
 */
#define DM2_V1_OUTDOOR_SOURCE_COLOR_UNAVAILABLE 0u
uint32_t dm2_v1_outdoor_sky_color(const DM2_V1_OutdoorConfig *cfg);
const char *dm2_v1_outdoor_source_evidence(void);
#endif
