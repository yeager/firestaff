/* DM1 V1 Creature Viewport Rendering — source-locked from ReDMCSB
 * GROUP.C G0217: creature data table, creature type properties
 * DUNVIEW.C: creature sprite drawing at viewport depth/side positions
 * OBJECT.C: creature → object thing mapping
 * PROJEXPL.C: damage flash overlay on creature sprites */

#include "dm1_v1_creature_viewport_pc34_compat.h"
#include <string.h>

void DM1_V1_CreatureViewport_InitPc34Compat(DM1_V1_CreatureViewportStatePc34* state) {
    if (!state) return;
    memset(state, 0, sizeof(DM1_V1_CreatureViewportStatePc34));
}

/* Initialize sprite table with DM1 creature graphics info
 * Based on GROUP.C creature property tables */
void DM1_V1_CreatureViewport_SetupSpriteTablePc34Compat(DM1_V1_CreatureViewportStatePc34* state) {
    if (!state) return;

    /* ReDMCSB GROUP.C G0217_as_Graphic559_CreatureInfo.
     * gfx_index = first GRAPHICS.DAT bitmap, w/h = display pixels. */
    static const struct { uint16_t gfx; uint16_t frames; uint16_t w; uint16_t h; bool mirror; } defaults[] = {
        /* DM1_V1_CREATURE_VIEWPORT_MUMMY_PC34 */            { 225, 4, 32, 48, true },
        /* DM1_V1_CREATURE_VIEWPORT_SCREAMER_PC34 */         { 229, 2, 24, 24, false },
        /* DM1_V1_CREATURE_VIEWPORT_ROCK_PILE_PC34 */        { 231, 2, 32, 24, false },
        /* DM1_V1_CREATURE_VIEWPORT_GIANT_SCORPION_PC34 */   { 233, 4, 48, 32, true },
        /* DM1_V1_CREATURE_VIEWPORT_TROLIN_PC34 */           { 237, 4, 32, 48, true },
        /* DM1_V1_CREATURE_VIEWPORT_MAGENTA_WORM_PC34 */     { 241, 4, 24, 32, true },
        /* DM1_V1_CREATURE_VIEWPORT_PAIN_RAT_PC34 */         { 245, 4, 16, 16, true },
        /* DM1_V1_CREATURE_VIEWPORT_SKELETON_PC34 */         { 249, 4, 32, 48, true },
        /* DM1_V1_CREATURE_VIEWPORT_GIANT_WASP_PC34 */       { 253, 4, 24, 24, true },
        /* DM1_V1_CREATURE_VIEWPORT_STONE_GOLEM_PC34 */      { 257, 4, 40, 56, true },
        /* DM1_V1_CREATURE_VIEWPORT_GHOST_PC34 */            { 261, 2, 32, 48, false },
        /* DM1_V1_CREATURE_VIEWPORT_COUATL_PC34 */           { 263, 4, 40, 40, true },
        /* DM1_V1_CREATURE_VIEWPORT_WATER_ELEMENTAL_PC34 */  { 267, 4, 32, 48, true },
        /* DM1_V1_CREATURE_VIEWPORT_OITU_PC34 */             { 271, 4, 32, 48, true },
        /* DM1_V1_CREATURE_VIEWPORT_DEMON_PC34 */            { 275, 4, 48, 56, true },
        /* DM1_V1_CREATURE_VIEWPORT_LORD_CHAOS_PC34 */       { 279, 4, 56, 64, true },
        /* DM1_V1_CREATURE_VIEWPORT_RED_DRAGON_PC34 */       { 283, 4, 64, 48, true },
        /* DM1_V1_CREATURE_VIEWPORT_KNIGHT_PC34 */           { 287, 4, 32, 48, true },
        /* DM1_V1_CREATURE_VIEWPORT_SWAMP_SLIME_PC34 */      { 291, 2, 32, 24, false },
        /* DM1_V1_CREATURE_VIEWPORT_ANIMATED_ARMOR_PC34 */   { 293, 4, 32, 48, true },
        /* DM1_V1_CREATURE_VIEWPORT_BLACK_FLAME_PC34 */      { 297, 2, 24, 32, false }
    };

    for (int i = 0; i < DM1_V1_CREATURE_VIEWPORT_TYPE_COUNT_PC34 && i < (int)(sizeof(defaults)/sizeof(defaults[0])); i++) {
        state->sprite_info[i].gfx_index = defaults[i].gfx;
        state->sprite_info[i].frame_count = defaults[i].frames;
        state->sprite_info[i].base_width = defaults[i].w;
        state->sprite_info[i].base_height = defaults[i].h;
        state->sprite_info[i].mirror_walk = defaults[i].mirror;
    }
}

uint16_t DM1_V1_CreatureViewport_AddCreaturePc34Compat(DM1_V1_CreatureViewportStatePc34* state, DM1_V1_CreatureViewportTypePc34 type,
                              int16_t x, int16_t y, uint8_t facing,
                              int16_t hp) {
    if (!state || state->creature_count >= DM1_V1_CREATURE_VIEWPORT_MAX_CREATURES_PC34)
        return UINT16_MAX;

    uint16_t idx = state->creature_count++;
    DM1_V1_CreatureViewportCreaturePc34* c = &state->creatures[idx];
    c->type = type;
    c->map_x = x;
    c->map_y = y;
    c->facing = facing;
    c->cell = 0;
    c->hit_points = hp;
    c->max_hit_points = hp;
    c->anim_frame = 0;
    c->anim_timer = 0;
    c->flash_timer = 0;
    c->alive = true;
    c->visible = false;
    return idx;
}

void DM1_V1_CreatureViewport_SetPartyPosPc34Compat(DM1_V1_CreatureViewportStatePc34* state, int16_t x, int16_t y,
                           uint8_t facing) {
    if (!state) return;
    state->party_x = x;
    state->party_y = y;
    state->party_facing = facing;
}

/* DUNVIEW.C pattern: determine which creatures are visible from party's
 * viewpoint, up to 3 tiles deep, and assign screen positions */
void DM1_V1_CreatureViewport_UpdateVisibilityPc34Compat(DM1_V1_CreatureViewportStatePc34* state) {
    if (!state) return;
    state->visible_count = 0;

    /* Direction vectors for facing */
    static const int16_t dx[] = { 0, 1, 0, -1 };
    static const int16_t dy[] = { -1, 0, 1, 0 };
    /* Perpendicular (left) vectors for side placement */
    static const int16_t lx[] = { -1, 0, 1, 0 };
    static const int16_t ly[] = { 0, -1, 0, 1 };

    int16_t fdx = dx[state->party_facing & 3];
    int16_t fdy = dy[state->party_facing & 3];
    int16_t flx = lx[state->party_facing & 3];
    int16_t fly = ly[state->party_facing & 3];

    /* Depth scale factors from ReDMCSB DUNVIEW.C G0163 wall frame heights.
     * D0=136px (100%), D1=111px (82%), D2=71px (52%), D3=51px (38%).
     * Screen positions from G0163 frame [blitX,blitY] center values. */
    static const uint8_t scale_pct[] = { 100, 82, 52, 38 };
    /* Screen X/Y base positions from ReDMCSB G0163 center frame coords */
    static const int16_t cx[] = { 112, 112, 112, 112 };
    static const int16_t cy[] = { 33, 41, 52, 57 };

    for (uint16_t i = 0; i < state->creature_count; i++) {
        DM1_V1_CreatureViewportCreaturePc34* c = &state->creatures[i];
        c->visible = false;
        if (!c->alive) continue;

        /* Calculate relative position */
        int16_t rel_x = c->map_x - state->party_x;
        int16_t rel_y = c->map_y - state->party_y;

        /* Project onto forward/lateral axes */
        int16_t depth_val = rel_x * fdx + rel_y * fdy;
        int16_t side_val = rel_x * flx + rel_y * fly;

        /* Only visible if in front (depth 1-3) and within side range */
        if (depth_val < 1 || depth_val > 3) continue;
        if (side_val < -1 || side_val > 1) continue;

        if (state->visible_count >= DM1_V1_CREATURE_VIEWPORT_MAX_VISIBLE_PC34) break;

        c->visible = true;
        DM1_V1_CreatureViewportEntryPc34* ve = &state->visible[state->visible_count++];
        ve->creature = c;
        ve->depth = (uint8_t)(depth_val - 1); /* 0-based */
        ve->side = (uint8_t)(side_val + 1);   /* 0=left, 1=center, 2=right */

        /* Scale sprite dimensions */
        const DM1_V1_CreatureViewportSpriteInfoPc34* si = &state->sprite_info[c->type];
        uint8_t sc = scale_pct[ve->depth];
        ve->draw_w = (int16_t)(si->base_width * sc / 100);
        ve->draw_h = (int16_t)(si->base_height * sc / 100);

        /* Screen position */
        ve->screen_x = cx[ve->depth] + side_val * (int16_t)(48 * sc / 100) - ve->draw_w / 2;
        ve->screen_y = cy[ve->depth];
        ve->flipped = (side_val < 0 && si->mirror_walk);
    }
}

void DM1_V1_CreatureViewport_AnimateFramePc34Compat(DM1_V1_CreatureViewportStatePc34* state) {
    if (!state) return;
    for (uint16_t i = 0; i < state->creature_count; i++) {
        DM1_V1_CreatureViewportCreaturePc34* c = &state->creatures[i];
        if (!c->alive) continue;

        /* Advance animation timer */
        c->anim_timer++;
        if (c->anim_timer >= 6) { /* ~6 frames per anim step */
            c->anim_timer = 0;
            const DM1_V1_CreatureViewportSpriteInfoPc34* si = &state->sprite_info[c->type];
            c->anim_frame = (uint8_t)((c->anim_frame + 1) % si->frame_count);
        }

        /* Decay damage flash */
        if (c->flash_timer > 0) {
            c->flash_timer--;
        }
    }
}

void DM1_V1_CreatureViewport_DamagePc34Compat(DM1_V1_CreatureViewportStatePc34* state, uint16_t index, int16_t damage) {
    if (!state || index >= state->creature_count) return;
    DM1_V1_CreatureViewportCreaturePc34* c = &state->creatures[index];
    if (!c->alive) return;

    c->hit_points -= damage;
    c->flash_timer = DM1_V1_CREATURE_VIEWPORT_FLASH_DURATION_PC34;
    if (c->hit_points <= 0) {
        c->hit_points = 0;
        c->alive = false;
    }
}

bool DM1_V1_CreatureViewport_IsAlivePc34Compat(const DM1_V1_CreatureViewportStatePc34* state, uint16_t index) {
    if (!state || index >= state->creature_count) return false;
    return state->creatures[index].alive;
}

uint8_t DM1_V1_CreatureViewport_GetVisibleCountPc34Compat(const DM1_V1_CreatureViewportStatePc34* state) {
    if (!state) return 0;
    return state->visible_count;
}
