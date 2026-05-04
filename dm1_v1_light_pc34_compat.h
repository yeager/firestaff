#ifndef FIRESTAFF_DM1_V1_LIGHT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_LIGHT_PC34_COMPAT_H
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
/* Based on ReDMCSB CHAMPION.C F0303 light calc, DUNVIEW.C darkness mask.
 * Light falls off with viewport depth; darkness = 255 - light. */
enum { DM1_LIGHT_SOURCE_TORCH = 0, DM1_LIGHT_SOURCE_SPELL, DM1_LIGHT_SOURCE_ITEM, DM1_LIGHT_SOURCE_COUNT };
typedef struct { int type; int intensity; int radius; int duration; int decayRate; } M11_LightSource;
#define M11_LIGHT_MAX_SOURCES 8
typedef struct {
    M11_LightSource sources[M11_LIGHT_MAX_SOURCES];
    int sourceCount; int ambientLight; int partyLightRadius; int darknessLevel; int torchBurnRate;
} M11_LightState;
void m11_light_init(M11_LightState* s);
void m11_light_add_source(M11_LightState* s, int type, int intensity, int radius, int duration);
void m11_light_remove_source(M11_LightState* s, int index);
void m11_light_tick(M11_LightState* s, int tickMs);
int  m11_light_get_level_at_depth(const M11_LightState* s, int viewDepth);
int  m11_light_get_darkness_mask(const M11_LightState* s, int viewDepth);
void m11_light_apply_torch(M11_LightState* s, int torchPower);
#ifdef __cplusplus
}
#endif
#endif
