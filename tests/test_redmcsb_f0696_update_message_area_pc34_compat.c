#include "redmcsb_f0696_update_message_area_pc34_compat.h"

#include <assert.h>
#include <stdint.h>

typedef struct {
    unsigned int step;
    unsigned int enabled_at;
    unsigned int write_page_at;
    unsigned int zone_at;
    unsigned int viewport_at;
    unsigned int scroll_at;
    unsigned int blit_at;
    unsigned int disabled_at;
    int16_t scroll_lines;
    RedmcsbF0696BoxPc34Compat viewport;
    RedmcsbF0696BoxPc34Compat scroll_box;
    RedmcsbF0696BoxPc34Compat blit_box;
    const uint8_t *blit_bitmap;
    int16_t source_width;
    int16_t destination_width;
} Capture;

static void enable(void *context) { Capture *c = context; c->enabled_at = ++c->step; }
static void disable(void *context) { Capture *c = context; c->disabled_at = ++c->step; }
static void zone(void *context, uint16_t zone_id, RedmcsbF0696BoxPc34Compat *box)
{
    (void)zone_id;
    Capture *c = context;
    assert(zone_id == 15U);
    c->zone_at = ++c->step;
    box->left = 0; box->top = 154; box->right = 319; box->bottom = 180;
}
static void write_page(void *context, int16_t work, int16_t write)
{
    (void)write;
    (void)work;
    Capture *c = context;
    assert(work == 2 && write == 1);
    c->write_page_at = ++c->step;
}
static void viewport(void *context, int16_t work, const RedmcsbF0696BoxPc34Compat *box)
{
    (void)work;
    Capture *c = context;
    assert(work == 2);
    c->viewport = *box;
    c->viewport_at = ++c->step;
}
static void scroll(void *context, int16_t work, int16_t x, int16_t y,
                   int16_t lines, const RedmcsbF0696BoxPc34Compat *box)
{
    (void)y;
    (void)x;
    (void)work;
    Capture *c = context;
    assert(work == 2 && x == 0 && y == 0);
    c->scroll_lines = lines;
    c->scroll_box = *box;
    c->scroll_at = ++c->step;
}
static void blit(void *context, const uint8_t *bitmap,
                 const RedmcsbF0696BoxPc34Compat *box, int16_t sx, int16_t sy,
                 int16_t sw, int16_t dw, int16_t transparent)
{
    (void)transparent;
    (void)sy;
    (void)sx;
    Capture *c = context;
    assert(sx == 0 && sy == 0 && transparent == -1);
    c->blit_bitmap = bitmap;
    c->blit_box = *box;
    c->source_width = sw;
    c->destination_width = dw;
    c->blit_at = ++c->step;
}

int main(void)
{
    static const uint8_t new_row[] = { 1U, 2U, 3U };
    (void)new_row;
    Capture capture = { 0 };
    const RedmcsbF0696VideoOpsPc34Compat ops = {
        enable, disable, zone, write_page, viewport, scroll, blit, &capture
    };
    (void)ops;
    const RedmcsbF0696StatePc34Compat state = { 2, 1, 7, 320, 320, -1 };
    (void)state;

    assert(redmcsb_f0696_update_message_area_pc34_compat(&ops, &state, new_row) == 1);
    assert(capture.enabled_at == 1U && capture.write_page_at == 2U);
    assert(capture.zone_at == 3U && capture.viewport_at == 4U);
    assert(capture.scroll_at == 5U && capture.blit_at == 6U && capture.disabled_at == 7U);
    assert(capture.scroll_lines == -7);
    assert(capture.viewport.top == 154 && capture.scroll_box.bottom == 180);
    assert(capture.blit_box.top == 174 && capture.blit_box.bottom == 180);
    assert(capture.blit_bitmap == new_row);
    assert(capture.source_width == 320 && capture.destination_width == 320);
    return 0;
}
