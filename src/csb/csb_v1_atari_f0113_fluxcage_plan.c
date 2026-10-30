#include "csb_v1_atari_f0113_fluxcage_plan.h"

#include <string.h>

enum {
    MEDIA020_NO_MASK = 0xff,
    MEDIA020_MASK_FLIP_HORIZONTAL = 0x80,
    MEDIA020_MASK_INDEX = 0x7f,
    MEDIA020_TRANSPARENT_COLOR = 0x7f
};

/* ReDMCSB DUNVIEW.C MEDIA020 G0188, DUNVIEW.C:718-731, D3C..D0R. */
static const uint8_t s_g0188_field_aspects
    [CSB_V1_ATARI_F0113_VIEW_SQUARE_COUNT][8] = {
    {0, 63, 0x8a, 0xff,  0,   0,  0, 64}, /* D3C */
    {0, 63, 0x0a, 0x80, 48,  51, 11, 64}, /* D3L */
    {0, 63, 0x0a, 0x00, 48,  51,  0, 64}, /* D3R */
    {0, 60, 0x8a, 0xff,  0,   0,  0, 64}, /* D2C */
    {0, 63, 0x0a, 0x81, 40,  71,  5, 64}, /* D2L */
    {0, 63, 0x0a, 0x01, 40,  71,  0, 64}, /* D2R */
    {0, 61, 0x8a, 0xff,  0,   0,  0, 64}, /* D1C */
    {0, 63, 0x0a, 0x82, 32, 111,  0, 64}, /* D1L */
    {0, 63, 0x0a, 0x02, 32, 111,  0, 64}, /* D1R */
    {0, 59, 0x8a, 0xff,  0,   0,  0, 64}, /* D0C */
    {0, 63, 0x0a, 0x83, 16, 136,  0, 64}, /* D0L */
    {0, 63, 0x0a, 0x03, 16, 136,  0, 64}  /* D0R */
};

/* ReDMCSB DUNVIEW.C MEDIA020 G0163, DUNVIEW.C:581-594. */
static const uint8_t s_g0163_frame_walls
    [CSB_V1_ATARI_F0113_VIEW_SQUARE_COUNT][8] = {
    { 74,149,25, 75, 64, 51, 18,0}, /* D3C */
    {  0, 83,25, 75, 64, 51, 32,0}, /* D3L */
    {139,223,25, 75, 64, 51,  0,0}, /* D3R */
    { 60,163,20, 90, 72, 71, 16,0}, /* D2C */
    {  0, 74,20, 90, 72, 71, 61,0}, /* D2L */
    {149,223,20, 90, 72, 71,  0,0}, /* D2R */
    { 32,191, 9,119,128,111, 48,0}, /* D1C */
    {  0, 63, 9,119,128,111,192,0}, /* D1L */
    {160,223, 9,119,128,111,  0,0}, /* D1R */
    {  0,223, 0,135,  0,  0,  0,0}, /* D0C */
    {  0, 31, 0,135, 16,136,  0,0}, /* D0L */
    {192,223, 0,135, 16,136,  0,0}  /* D0R */
};

int csb_v1_atari_f0113_plan_fluxcage(
    const CSB_V1_AtariF0113FluxcageInput *input,
    CSB_V1_AtariF0113FluxcagePlan *out_plan)
{
    const uint8_t *aspect;
    const uint8_t *frame;
    unsigned int mask;
    int square;

    if (!input || !out_plan || input->view_square < 0 ||
        input->view_square >= CSB_V1_ATARI_F0113_VIEW_SQUARE_COUNT ||
        input->door_front_view_pass < 0 || input->door_front_view_pass > 2 ||
        (input->suppress_during_endgame != 0 &&
         input->suppress_during_endgame != 1)) {
        return CSB_V1_ATARI_F0113_FLUXCAGE_INVALID;
    }
    memset(out_plan, 0, sizeof(*out_plan));
    out_plan->kind = CSB_V1_ATARI_F0113_FLUXCAGE_NOT_DRAWN;
    out_plan->view_square = input->view_square;

    /* F0115 MEDIA020 only retains C050 from D3L (M601) toward the party. */
    if (input->explosion_type != CSB_V1_ATARI_F0113_C050 ||
        input->view_square == 0 || input->door_front_view_pass == 1 ||
        input->suppress_during_endgame) {
        return out_plan->kind;
    }

    square = input->view_square;
    aspect = s_g0188_field_aspects[square];
    frame = s_g0163_frame_walls[square];
    memcpy(out_plan->field_aspect, aspect, sizeof(out_plan->field_aspect));
    memcpy(out_plan->destination_frame, frame,
           sizeof(out_plan->destination_frame));
    /* DUNVIEW.C:6211 turns the teleporter bitmap into the C050 bitmap. */
    out_plan->field_aspect[0]++;
    out_plan->graphic_index =
        CSB_V1_ATARI_F0113_TELEPORTER_GRAPHIC + out_plan->field_aspect[0];
    /* F0133 arguments are `lastUnit = random(2) + G0188[1]`,
     * `firstUnit = random(32)`. The last unit is an exclusive restart
     * sentinel; F0133 resets to unit 0 when it reaches that index. */
    out_plan->base_last_unit = (int)aspect[1];
    out_plan->last_unit_random_range = 2;
    out_plan->first_unit_random_range = 32;
    out_plan->source_byte_width = (int)aspect[4];
    out_plan->source_height = (int)aspect[5];
    out_plan->source_pixel_width = (int)aspect[4] * 2;
    out_plan->source_x = (int)aspect[6];
    out_plan->transparent_color = (int)(aspect[2] & MEDIA020_TRANSPARENT_COLOR);
    mask = aspect[3] & MEDIA020_MASK_INDEX;
    if (aspect[3] == MEDIA020_NO_MASK) {
        out_plan->mask_flip_horizontal = 0;
        out_plan->mask_index = -1;
        out_plan->mask_graphic_index = -1;
    } else {
        out_plan->mask_flip_horizontal =
            (aspect[3] & MEDIA020_MASK_FLIP_HORIZONTAL) != 0u;
        out_plan->mask_index = (int)mask;
        out_plan->mask_graphic_index =
            CSB_V1_ATARI_F0113_FIELD_MASK_BASE + (int)mask;
    }
    out_plan->kind = CSB_V1_ATARI_F0113_FLUXCAGE_DRAW_FIELD;
    return out_plan->kind;
}
