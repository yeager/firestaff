#include "csb_v1_atari_f0115_explosion_plan.h"

#include <string.h>

enum {
    MEDIA020_M614_FIRST_EXPLOSION = 348,
    MEDIA020_M636_FIRST_D0C_PATTERN = 351,
    MEDIA020_M600_D3C = 0,
    MEDIA020_M609_D0C = 9,
    MEDIA020_M611_D0R = 11,
    MEDIA020_M613_FIRST_PROJECTILE = 316,
    CSB_G20E_C558_SIZE = 4720,
    CSB_G20E_G0210_OFFSET = 1558,
    G0210_RECORD_SIZE = 6,
    G0210_LIGHTNING_ASPECT_INDEX = 3,
    G0210_LIGHTNING_NATIVE_RELATIVE_INDEX = 9,
    G0210_LIGHTNING_DERIVED_INDEX = 54,
    G0210_LIGHTNING_BYTE_WIDTH = 32,
    G0210_LIGHTNING_HEIGHT = 15,
    C000_EXPLOSION_FIREBALL = 0,
    C002_EXPLOSION_LIGHTNING_BOLT = 2,
    C006_EXPLOSION_POISON_BOLT = 6,
    C007_EXPLOSION_POISON_CLOUD = 7,
    C040_EXPLOSION_SMOKE = 40,
    C050_EXPLOSION_FLUXCAGE = 50,
    C100_EXPLOSION_REBIRTH_STEP1 = 100,
    C101_EXPLOSION_REBIRTH_STEP2 = 101,
    C0_EXPLOSION_ASPECT_FIRE = 0,
    C1_EXPLOSION_ASPECT_SPELL = 1,
    C2_EXPLOSION_ASPECT_POISON = 2,
    C3_EXPLOSION_ASPECT_SMOKE = 3
};

/* ReDMCSB DUNVIEW.C MEDIA009 G0211 (DUNVIEW.C:1319) dimensions. */
static const uint8_t s_g0211_byte_width[4] = {80u, 64u, 80u, 80u};
static const uint8_t s_g0211_height[4] = {111u, 97u, 91u, 91u};

/* ReDMCSB DUNVIEW.C MEDIA358 G0216, depth order D4..D0. */
static const uint8_t s_g0216_depth_scale[5] = {10u, 16u, 23u, 32u, 32u};

/* G0225 and G0226 are source-owned signed pixel coordinates. Index order is
 * D4C/L/R, D3C/L/R, D2C/L/R, D1C/L/R, D0C/L/R. */
static const int16_t s_g0225_centered[15][2] = {
    {111,47},{57,47},{167,47},{111,50},{45,50},{179,50},
    {111,53},{20,53},{205,53},{111,57},{-30,57},{253,57},
    {111,60},{-53,60},{276,60}
};
static const int16_t s_g0226_cell[15][2][2] = {
    {{100,47},{122,47}},{{52,47},{76,47}},{{148,47},{172,47}},
    {{95,50},{127,50}},{{31,50},{63,50}},{{159,50},{191,50}},
    {{92,53},{131,53}},{{-3,53},{46,53}},{{177,53},{226,53}},
    {{83,57},{141,57}},{{-54,57},{18,57}},{{207,57},{277,57}},
    {{0,0},{0,0}},{{-73,60},{-33,60}},{{256,60},{296,60}}
};

/* ReDMCSB DUNVIEW.C G0227/G0228, D3C through D1C. */
static const int16_t s_g0227_rebirth_step2[7][3] = {
    {113,57,12},{24,57,12},{195,57,12},{111,63,16},
    {12,63,16},{213,63,16},{112,76,24}
};

/* ReDMCSB DUNVIEW.C G0228, D3C through D1C. */
static const int16_t s_g0228_rebirth_step1[7][3] = {
    {112,53,15},{24,53,15},{194,53,15},{112,59,20},
    {15,59,20},{208,59,20},{112,70,32}
};

static int explosion_aspect(int type)
{
    if (type == C000_EXPLOSION_FIREBALL ||
        type == C002_EXPLOSION_LIGHTNING_BOLT) {
        return C0_EXPLOSION_ASPECT_FIRE;
    }
    if (type == C006_EXPLOSION_POISON_BOLT ||
        type == C007_EXPLOSION_POISON_CLOUD) {
        return C2_EXPLOSION_ASPECT_POISON;
    }
    if (type == C040_EXPLOSION_SMOKE) return C3_EXPLOSION_ASPECT_SMOKE;
    return C1_EXPLOSION_ASPECT_SPELL;
}

int csb_v1_atari_f0115_plan_explosion(
    const CSB_V1_AtariF0115ExplosionInput *input,
    CSB_V1_AtariF0115ExplosionPlan *out_plan)
{
    int aspect;
    int index;
    int depth;
    int scale;
    int cell_index;

    if (!input || !out_plan || input->view_square < MEDIA020_M600_D3C ||
        input->view_square > MEDIA020_M611_D0R || input->cell < 0 ||
        input->cell > 3 || input->party_direction < 0 ||
        input->party_direction > 3 || input->attack < 0 ||
        input->attack > 255) {
        return CSB_V1_ATARI_F0115_EXPLOSION_INVALID;
    }
    memset(out_plan, 0, sizeof(*out_plan));
    out_plan->view_square = input->view_square;
    out_plan->explosion_coordinate_index = input->view_square + 3;
    out_plan->transparent_color = 10; /* MEDIA020 C10_COLOR_FLESH. */
    if (input->explosion_type == C100_EXPLOSION_REBIRTH_STEP1) {
        const uint8_t *aspect;
        int cell;
        int scale;
        if (input->view_square > 6 || input->last_view_cell < 0 ||
            input->last_view_cell > 3) {
            out_plan->kind = CSB_V1_ATARI_F0115_EXPLOSION_NOT_DRAWN;
            return out_plan->kind;
        }
        if (!input->c558 || input->c558_size != CSB_G20E_C558_SIZE) {
            return CSB_V1_ATARI_F0115_EXPLOSION_INVALID;
        }
        cell = (input->last_view_cell + input->party_direction) & 3;
        if (input->cell != cell) {
            out_plan->kind = CSB_V1_ATARI_F0115_EXPLOSION_NOT_DRAWN;
            return out_plan->kind;
        }
        aspect = input->c558 + CSB_G20E_G0210_OFFSET +
            G0210_LIGHTNING_ASPECT_INDEX * G0210_RECORD_SIZE;
        /* G20E G0210 lightning: native bitmap relative index 9, 32-byte
         * width, 15-pixel height, and the authentic rotation/back flags. */
        if (aspect[0] != G0210_LIGHTNING_NATIVE_RELATIVE_INDEX ||
            aspect[1] != G0210_LIGHTNING_DERIVED_INDEX ||
            aspect[2] != G0210_LIGHTNING_BYTE_WIDTH ||
            aspect[3] != G0210_LIGHTNING_HEIGHT ||
            aspect[4] != 0x01u || aspect[5] != 0x12u) {
            return CSB_V1_ATARI_F0115_EXPLOSION_INVALID;
        }
        scale = s_g0228_rebirth_step1[input->view_square][2];
        out_plan->aspect_index = G0210_LIGHTNING_ASPECT_INDEX;
        out_plan->graphic_index = MEDIA020_M613_FIRST_PROJECTILE +
            G0210_LIGHTNING_NATIVE_RELATIVE_INDEX + 1;
        out_plan->source_byte_width = aspect[2];
        out_plan->source_height = aspect[3];
        out_plan->center_x = s_g0228_rebirth_step1[input->view_square][0];
        out_plan->center_y = s_g0228_rebirth_step1[input->view_square][1];
        out_plan->scale_32 = scale;
        out_plan->draw_byte_width =
            (out_plan->source_byte_width * scale + (scale >> 1)) >> 5;
        out_plan->draw_height =
            (out_plan->source_height * scale + (scale >> 1)) >> 5;
        out_plan->uses_projectile_bitmap = 1;
        out_plan->flip_horizontal = input->flip_horizontal != 0;
        out_plan->flip_vertical = input->flip_vertical != 0;
        out_plan->kind = CSB_V1_ATARI_F0115_EXPLOSION_SPRITE;
        return out_plan->kind;
    }
    if (input->explosion_type == C101_EXPLOSION_REBIRTH_STEP2) {
        int scale;
        int cell;
        if (input->view_square > 6 || input->last_view_cell < 0 ||
            input->last_view_cell > 3) {
            out_plan->kind = CSB_V1_ATARI_F0115_EXPLOSION_NOT_DRAWN;
            return out_plan->kind;
        }
        cell = (input->last_view_cell + input->party_direction) & 3;
        if (input->cell != cell) {
            out_plan->kind = CSB_V1_ATARI_F0115_EXPLOSION_NOT_DRAWN;
            return out_plan->kind;
        }
        scale = s_g0227_rebirth_step2[input->view_square][2];
        out_plan->aspect_index = C0_EXPLOSION_ASPECT_FIRE;
        out_plan->graphic_index = MEDIA020_M614_FIRST_EXPLOSION;
        out_plan->source_byte_width = s_g0211_byte_width[C0_EXPLOSION_ASPECT_FIRE];
        out_plan->source_height = s_g0211_height[C0_EXPLOSION_ASPECT_FIRE];
        out_plan->center_x = s_g0227_rebirth_step2[input->view_square][0];
        out_plan->center_y = s_g0227_rebirth_step2[input->view_square][1];
        out_plan->scale_32 = scale;
        out_plan->draw_byte_width =
            (out_plan->source_byte_width * scale + (scale >> 1)) >> 5;
        out_plan->draw_height =
            (out_plan->source_height * scale + (scale >> 1)) >> 5;
        out_plan->flip_horizontal = input->flip_horizontal != 0;
        out_plan->flip_vertical = input->flip_vertical != 0;
        out_plan->kind = CSB_V1_ATARI_F0115_EXPLOSION_SPRITE;
        return out_plan->kind;
    }
    if (input->explosion_type == C050_EXPLOSION_FLUXCAGE) {
        /* DUNVIEW.C:6006-6013 only retains C050 from D3L (explosion
         * index C04) toward the party; D3C is a source-invisible lane. */
        if (input->view_square < 1) {
            out_plan->kind = CSB_V1_ATARI_F0115_EXPLOSION_NOT_DRAWN;
            return out_plan->kind;
        }
        out_plan->kind = CSB_V1_ATARI_F0115_EXPLOSION_FLUXCAGE;
        return out_plan->kind;
    }
    aspect = explosion_aspect(input->explosion_type);
    if (aspect < 0) return CSB_V1_ATARI_F0115_EXPLOSION_NOT_DRAWN;
    out_plan->aspect_index = aspect;
    out_plan->smoke_palette = aspect == C3_EXPLOSION_ASPECT_SMOKE;
    index = out_plan->explosion_coordinate_index;
    if (input->view_square == MEDIA020_M609_D0C) {
        if (out_plan->smoke_palette) aspect = C2_EXPLOSION_ASPECT_POISON;
        out_plan->kind = CSB_V1_ATARI_F0115_EXPLOSION_D0C_PATTERN;
        out_plan->graphic_index = MEDIA020_M636_FIRST_D0C_PATTERN +
            aspect * 3 + (input->attack >> 5 > 3 ? 2 :
                          input->attack >> 5 > 0 ? 1 : 0);
        out_plan->d0c_pattern_size = input->attack >> 5;
        if (out_plan->d0c_pattern_size > 3) {
            out_plan->d0c_pattern_size = 3;
        }
        return out_plan->kind;
    }
    cell_index = input->centered ? -1 :
        ((input->cell == input->party_direction ||
          input->cell == ((input->party_direction + 3) & 3)) ? 0 : 1);
    out_plan->center_x = cell_index < 0 ? s_g0225_centered[index][0]
                                        : s_g0226_cell[index][cell_index][0];
    out_plan->center_y = cell_index < 0 ? s_g0225_centered[index][1]
                                        : s_g0226_cell[index][cell_index][1];
    out_plan->flip_horizontal = input->flip_horizontal != 0;
    out_plan->flip_vertical = input->flip_vertical != 0;
    if (index < 3 || index >= 15) {
        return CSB_V1_ATARI_F0115_EXPLOSION_INVALID;
    }
    depth = index / 3;
    scale = ((input->attack + 1 > 48 ? input->attack + 1 : 48) *
             s_g0216_depth_scale[depth]) >> 8;
    if (scale < 4) scale = 4;
    scale &= 0xfffe;
    out_plan->scale_32 = scale;
    out_plan->source_byte_width = s_g0211_byte_width[aspect];
    out_plan->source_height = s_g0211_height[aspect];
    out_plan->draw_byte_width =
        (out_plan->source_byte_width * scale + (scale >> 1)) >> 5;
    out_plan->draw_height =
        (out_plan->source_height * scale + (scale >> 1)) >> 5;
    out_plan->graphic_index = MEDIA020_M614_FIRST_EXPLOSION +
        (aspect > C2_EXPLOSION_ASPECT_POISON
             ? C2_EXPLOSION_ASPECT_POISON : aspect);
    out_plan->kind = CSB_V1_ATARI_F0115_EXPLOSION_SPRITE;
    return out_plan->kind;
}
