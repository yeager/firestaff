
#ifndef FIRESTAFF_DM2_V2_OUTDOOR_ENHANCED_H
#define FIRESTAFF_DM2_V2_OUTDOOR_ENHANCED_H
#include <stdint.h>

/* DM2 V2.2 Enhanced Outdoor — particle weather, dynamic sky, tree sway.
 * V1: static sky gradient + weather flag.
 * V2.2: animated rain particles, cloud movement, tree sway animation. */

typedef struct {
    float cloud_offset;
    float rain_intensity;
    float tree_sway_phase;
    float wind_strength;
    int lightning_flash;
    float lightning_timer;
} DM2_V2_OutdoorFX;

void dm2_v2_outdoor_fx_init(DM2_V2_OutdoorFX *fx);
void dm2_v2_outdoor_fx_tick(DM2_V2_OutdoorFX *fx, float dt, int weather);
void dm2_v2_outdoor_fx_trigger_lightning(DM2_V2_OutdoorFX *fx);
const char *dm2_v2_outdoor_source_evidence(void);
#endif

