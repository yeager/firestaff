/* DM1 V1 Creature Viewport Rendering — source-locked from ReDMCSB
 * GROUP.C G0217: creature data table, creature type properties
 * DUNVIEW.C: creature sprite drawing at viewport depth/side positions
 * OBJECT.C: creature → object thing mapping
 * PROJEXPL.C: damage flash overlay on creature sprites */

#include "dm1_v1_creature_viewport_pc34_compat.h"
#include <string.h>

void m11_cr_init(M11_CR_State* state) {
    if (!state) return;
    memset(state, 0, sizeof(M11_CR_State));
}

/* Initialize sprite table with DM1 creature graphics info
 * Based on GROUP.C creature property tables */
void m11_cr_setup_sprite_table(M11_CR_State* state) {
    if (!state) return;

    /* Default sprite info — gfx_index values are GRAPHICS.DAT bitmap indices
     * Actual values depend on the specific GRAPHICS.DAT version */
    static const struct { uint16_t gfx; uint16_t frames; uint16_t w; uint16_t h; bool mirror; } defaults[] = {
        /* M11_CR_MUMMY */            { 225, 4, 32, 48, true },
        /* M11_CR_SCREAMER */         { 229, 2, 24, 24, false },
        /* M11_CR_ROCK_PILE */        { 231, 2, 32, 24, false },
        /* M11_CR_GIANT_SCORPION */   { 233, 4, 48, 32, true },
        /* M11_CR_TROLIN */           { 237, 4, 32, 48, true },
        /* M11_CR_MAGENTA_WORM */     { 241, 4, 24, 32, true },
        /* M11_CR_PAIN_RAT */         { 245, 4, 16, 16, true },
        /* M11_CR_SKELETON */         { 249, 4, 32, 48, true },
        /* M11_CR_GIANT_WASP */       { 253, 4, 24, 24, true },
        /* M11_CR_STONE_GOLEM */      { 257, 4, 40, 56, true },
        /* M11_CR_GHOST */            { 261, 2, 32, 48, false },
        /* M11_CR_COUATL */           { 263, 4, 40, 40, true },
        /* M11_CR_WATER_ELEMENTAL */  { 267, 4, 32, 48, true },
        /* M11_CR_OITU */             { 271, 4, 32, 48, true },
        /* M11_CR_DEMON */            { 275, 4, 48, 56, true },
        /* M11_CR_LORD_CHAOS */       { 279, 4, 56, 64, true },
        /* M11_CR_RED_DRAGON */       { 283, 4, 64, 48, true },
        /* M11_CR_KNIGHT */           { 287, 4, 32, 48, true },
        /* M11_CR_SWAMP_SLIME */      { 291, 2, 32, 24, false },
        /* M11_CR_ANIMATED_ARMOR */   { 293, 4, 32, 48, true },
        /* M11_CR_BLACK_FLAME */      { 297, 2, 24, 32, false }
    };

    for (int i = 0; i < M11_CR_TYPE_COUNT && i < (int)(sizeof(defaults)/sizeof(defaults[0])); i++) {
        state->sprite_info[i].gfx_index = defaults[i].gfx;
        state->sprite_info[i].frame_count = defaults[i].frames;
        state->sprite_info[i].base_width = defaults[i].w;
        state->sprite_info[i].base_height = defaults[i].h;
        state->sprite_info[i].mirror_walk = defaults[i].mirror;
    }
}

uint16_t m11_cr_add_creature(M11_CR_State* state, M11_CR_CreatureType type,
                              int16_t x, int16_t y, uint8_t facing,
                              int16_t hp) {
    if (!state || state->creature_count >= DM1_CR_MAX_CREATURES)
        return UINT16_MAX;

    uint16_t idx = state->creature_count++;
    M11_CR_Creature* c = &state->creatures[idx];
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

void m11_cr_set_party_pos(M11_CR_State* state, int16_t x, int16_t y,
                           uint8_t facing) {
    if (!state) return;
    state->party_x = x;
    state->party_y = y;
    state->party_facing = facing;
}

/* DUNVIEW.C pattern: determine which creatures are visible from party's
 * viewpoint, up to 3 tiles deep, and assign screen positions */
void m11_cr_update_visibility(M11_CR_State* state) {
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

    /* Depth scale factors: 100%, 66%, 44%, 28% */
    static const uint8_t scale_pct[] = { 100, 66, 44, 28 };
    /* Screen X base positions for center at each depth */
    static const int16_t cx[] = { 96, 104, 108, 112 };
    static const int16_t cy[] = { 40, 52, 60, 66 };

    for (uint16_t i = 0; i < state->creature_count; i++) {
        M11_CR_Creature* c = &state->creatures[i];
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

        if (state->visible_count >= DM1_CR_MAX_VISIBLE) break;

        c->visible = true;
        M11_CR_ViewportEntry* ve = &state->visible[state->visible_count++];
        ve->creature = c;
        ve->depth = (uint8_t)(depth_val - 1); /* 0-based */
        ve->side = (uint8_t)(side_val + 1);   /* 0=left, 1=center, 2=right */

        /* Scale sprite dimensions */
        const M11_CR_SpriteInfo* si = &state->sprite_info[c->type];
        uint8_t sc = scale_pct[ve->depth];
        ve->draw_w = (int16_t)(si->base_width * sc / 100);
        ve->draw_h = (int16_t)(si->base_height * sc / 100);

        /* Screen position */
        ve->screen_x = cx[ve->depth] + side_val * (int16_t)(48 * sc / 100) - ve->draw_w / 2;
        ve->screen_y = cy[ve->depth];
        ve->flipped = (side_val < 0 && si->mirror_walk);
    }
}

void m11_cr_animate_frame(M11_CR_State* state) {
    if (!state) return;
    for (uint16_t i = 0; i < state->creature_count; i++) {
        M11_CR_Creature* c = &state->creatures[i];
        if (!c->alive) continue;

        /* Advance animation timer */
        c->anim_timer++;
        if (c->anim_timer >= 6) { /* ~6 frames per anim step */
            c->anim_timer = 0;
            const M11_CR_SpriteInfo* si = &state->sprite_info[c->type];
            c->anim_frame = (uint8_t)((c->anim_frame + 1) % si->frame_count);
        }

        /* Decay damage flash */
        if (c->flash_timer > 0) {
            c->flash_timer--;
        }
    }
}

void m11_cr_damage(M11_CR_State* state, uint16_t index, int16_t damage) {
    if (!state || index >= state->creature_count) return;
    M11_CR_Creature* c = &state->creatures[index];
    if (!c->alive) return;

    c->hit_points -= damage;
    c->flash_timer = DM1_CR_FLASH_DURATION;
    if (c->hit_points <= 0) {
        c->hit_points = 0;
        c->alive = false;
    }
}

bool m11_cr_is_alive(const M11_CR_State* state, uint16_t index) {
    if (!state || index >= state->creature_count) return false;
    return state->creatures[index].alive;
}

uint8_t m11_cr_get_visible_count(const M11_CR_State* state) {
    if (!state) return 0;
    return state->visible_count;
}
