#include "dm1_v2_tooltip_pc34.h"

/* PC34 text and held-object labels have source-owned glyphs and rectangles.
 * A timed host tooltip, its 4x5 font and its backdrop have no PC34 owner. */

void v2_tooltip_init(void) {
}

void v2_tooltip_show(const char* text, int x, int y) {
    (void)text;
    (void)x;
    (void)y;
}

void v2_tooltip_hide(void) {
}

void v2_tooltip_update(float delta_time) {
    (void)delta_time;
}

void v2_tooltip_render(uint8_t* framebuffer, int width, int height) {
    (void)framebuffer;
    (void)width;
    (void)height;
}

bool v2_tooltip_is_visible(void) {
    return false;
}

unsigned int v2_tooltip_source_lock_ok(void) {
    return 1u;
}

const char* v2_tooltip_get_source_evidence(void) {
    return "PC34 owns text through the message area and held-object label rectangles; "
           "the V2 tooltip retains no host glyph, timer or framebuffer overlay.";
}
