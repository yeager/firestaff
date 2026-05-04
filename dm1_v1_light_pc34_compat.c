/* DM1 V1 Light/Darkness — ReDMCSB CHAMPION.C F0303, DUNVIEW.C darkness.
 * Generated via Q3.6, verified and fixed by Opus. */
#include "dm1_v1_light_pc34_compat.h"
#include <string.h>

void m11_light_init(M11_LightState* s) {
    if (!s) return;
    memset(s, 0, sizeof(*s));
    s->darknessLevel = 255;
}

void m11_light_add_source(M11_LightState* s, int type, int intensity, int radius, int duration) {
    if (!s || s->sourceCount >= M11_LIGHT_MAX_SOURCES) return;
    M11_LightSource* src = &s->sources[s->sourceCount++];
    src->type = type; src->intensity = intensity;
    src->radius = radius; src->duration = duration; src->decayRate = 1;
}

void m11_light_remove_source(M11_LightState* s, int index) {
    if (!s || index < 0 || index >= s->sourceCount) return;
    for (int i = index; i < s->sourceCount - 1; i++)
        s->sources[i] = s->sources[i + 1];
    s->sourceCount--;
}

void m11_light_tick(M11_LightState* s, int tickMs) {
    if (!s) return;
    int maxIntensity = 0;
    for (int i = 0; i < s->sourceCount; ) {
        M11_LightSource* src = &s->sources[i];
        if (src->duration > tickMs) { src->duration -= tickMs; }
        else { m11_light_remove_source(s, i); continue; }
        if (src->intensity > src->decayRate) src->intensity -= src->decayRate;
        else src->intensity = 0;
        if (src->intensity > maxIntensity) maxIntensity = src->intensity;
        i++;
    }
    s->partyLightRadius = maxIntensity;
    int dark = 255 - s->partyLightRadius - s->ambientLight;
    if (dark < 0) dark = 0;
    if (dark > 255) dark = 255;
    s->darknessLevel = dark;
}

int m11_light_get_level_at_depth(const M11_LightState* s, int viewDepth) {
    /* ReDMCSB: light falls off with depth, ~64 units per depth level */
    if (!s) return 0;
    int level = s->partyLightRadius - viewDepth * 64;
    return level > 0 ? level : 0;
}

int m11_light_get_darkness_mask(const M11_LightState* s, int viewDepth) {
    return 255 - m11_light_get_level_at_depth(s, viewDepth);
}

void m11_light_apply_torch(M11_LightState* s, int torchPower) {
    /* ReDMCSB: torch adds light source with duration proportional to power */
    if (!s) return;
    m11_light_add_source(s, DM1_LIGHT_SOURCE_TORCH, torchPower, torchPower, 2000 * torchPower);
}
