#ifndef FIRESTAFF_DM1_V2_TOOLTIP_PC34_H
#define FIRESTAFF_DM1_V2_TOOLTIP_PC34_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    char text[128];
    int x;
    int y;
    bool visible;
    float timer;
    float fade_alpha;
} M11_V2_Tooltip;

void v2_tooltip_init(void);
void v2_tooltip_show(const char* text, int x, int y);
void v2_tooltip_hide(void);
void v2_tooltip_update(float dt);
void v2_tooltip_render(uint8_t* fb, int w, int h);
bool v2_tooltip_is_visible(void);

/* Compatibility-only.  PC34 source text owns its glyphs and placement; this
 * API retains no host tooltip, timer, font or framebuffer overlay. */
unsigned int v2_tooltip_source_lock_ok(void);
const char* v2_tooltip_get_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif
