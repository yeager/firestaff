
#include "nexus_v2_atmosphere.h"
#include <string.h>
#include <math.h>

/* Level-specific atmosphere presets */
static const struct { float fr, fg, fb; float tr, tg, tb; } g_level_atm[16] = {
    {0.05f,0.05f,0.08f, 1.0f,1.0f,0.95f},   /* L0: Hall — warm */
    {0.03f,0.03f,0.06f, 0.9f,0.95f,1.0f},   /* L1: blue dungeon */
    {0.04f,0.02f,0.02f, 1.0f,0.9f,0.85f},   /* L2: red/warm */
    {0.02f,0.04f,0.02f, 0.85f,1.0f,0.85f},  /* L3: green moss */
    {0.05f,0.05f,0.05f, 0.95f,0.95f,0.95f}, /* L4: neutral */
    {0.06f,0.04f,0.02f, 1.0f,0.85f,0.7f},   /* L5: lava/warm */
    {0.02f,0.03f,0.06f, 0.8f,0.85f,1.0f},   /* L6: icy blue */
    {0.04f,0.04f,0.04f, 0.9f,0.9f,0.9f},    /* L7-15: neutral variants */
    {0.04f,0.04f,0.04f, 0.9f,0.9f,0.9f},
    {0.04f,0.04f,0.04f, 0.9f,0.9f,0.9f},
    {0.04f,0.04f,0.04f, 0.9f,0.9f,0.9f},
    {0.04f,0.04f,0.04f, 0.9f,0.9f,0.9f},
    {0.04f,0.04f,0.04f, 0.9f,0.9f,0.9f},
    {0.04f,0.04f,0.04f, 0.9f,0.9f,0.9f},
    {0.04f,0.04f,0.04f, 0.9f,0.9f,0.9f},
    {0.04f,0.04f,0.04f, 0.9f,0.9f,0.9f},
};

void nexus_v2_atmosphere_init(Nexus_V2_Atmosphere *atm, int level_index) {
    if (!atm) return;
    if (level_index < 0 || level_index > 15) level_index = 0;
    atm->fog_start = 1.5f;
    atm->fog_end = 4.0f;
    atm->fog_r = g_level_atm[level_index].fr;
    atm->fog_g = g_level_atm[level_index].fg;
    atm->fog_b = g_level_atm[level_index].fb;
    atm->ao_strength = 0.3f;
    atm->tint_r = g_level_atm[level_index].tr;
    atm->tint_g = g_level_atm[level_index].tg;
    atm->tint_b = g_level_atm[level_index].tb;
}

void nexus_v2_apply_fog(uint32_t *rgba, int w, int h, const Nexus_V2_Atmosphere *atm) {
    int x, y;
    if (!rgba || !atm) return;
    for (y = 0; y < h; y++) {
        /* Estimate depth from screen Y (top=far, bottom=near) */
        float depth = 1.0f + 3.0f * ((float)y / h);
        float fog_factor = (depth - atm->fog_start) / (atm->fog_end - atm->fog_start);
        if (fog_factor < 0) fog_factor = 0;
        if (fog_factor > 1) fog_factor = 1;

        for (x = 0; x < w; x++) {
            uint32_t c = rgba[y*w+x];
            float cr = (float)((c>>16)&0xFF)/255.0f;
            float cg = (float)((c>>8)&0xFF)/255.0f;
            float cb = (float)(c&0xFF)/255.0f;

            /* Tint */
            cr *= atm->tint_r;
            cg *= atm->tint_g;
            cb *= atm->tint_b;

            /* Fog blend */
            cr = cr*(1-fog_factor) + atm->fog_r*fog_factor;
            cg = cg*(1-fog_factor) + atm->fog_g*fog_factor;
            cb = cb*(1-fog_factor) + atm->fog_b*fog_factor;

            int rr = (int)(cr*255); if(rr>255) rr=255;
            int gg = (int)(cg*255); if(gg>255) gg=255;
            int bb = (int)(cb*255); if(bb>255) bb=255;
            rgba[y*w+x] = 0xFF000000|(rr<<16)|(gg<<8)|bb;
        }
    }
}

/* SSAO approximation: darken pixels near edges (where depth changes) */
void nexus_v2_apply_ao(uint32_t *rgba, int w, int h, float strength) {
    int x, y;
    if (!rgba || strength <= 0) return;
    for (y = 1; y < h-1; y++) {
        for (x = 1; x < w-1; x++) {
            uint32_t c = rgba[y*w+x];
            uint32_t t = rgba[(y-1)*w+x];
            uint32_t b2 = rgba[(y+1)*w+x];
            uint32_t l = rgba[y*w+x-1];
            uint32_t r = rgba[y*w+x+1];

            /* Detect edges by color difference */
            int diff = 0;
            diff += abs((int)((c>>16)&0xFF) - (int)((t>>16)&0xFF));
            diff += abs((int)((c>>16)&0xFF) - (int)((b2>>16)&0xFF));
            diff += abs((int)((c>>16)&0xFF) - (int)((l>>16)&0xFF));
            diff += abs((int)((c>>16)&0xFF) - (int)((r>>16)&0xFF));

            if (diff > 40) {
                float darken = 1.0f - strength * 0.3f;
                int cr = (int)(((c>>16)&0xFF) * darken);
                int cg = (int)(((c>>8)&0xFF) * darken);
                int cb = (int)((c&0xFF) * darken);
                rgba[y*w+x] = 0xFF000000|(cr<<16)|(cg<<8)|cb;
            }
        }
    }
}

