#include "csb_v1_atari_f0115_projectile_plan.h"

#include <string.h>

enum {
    G0210_RECORD_SIZE = 6,
    G0218_SET_COUNT = 3,
    G0218_SQUARE_COUNT = 10,
    G0218_CELL_COUNT = 5,
    M614_GRAPHIC_FIRST_EXPLOSION = 348,
    F0115_TYPE_MASK = 0x0003,
    F0115_SIDE_MASK = 0x0010,
    F0115_SCALE_WITH_ENERGY_MASK = 0x0100,
    F0115_TYPE_HAS_BACK_AND_ROTATION = 0,
    F0115_TYPE_HAS_BACK_NO_ROTATION = 1,
    F0115_TYPE_NO_BACK_AND_ROTATION = 2,
    F0115_TYPE_NO_BACK_NO_ROTATION = 3,
    F0115_VIEW_CELL_FRONT_LEFT = 0,
    F0115_VIEW_CELL_FRONT_RIGHT = 1,
    F0115_VIEW_CELL_BACK_RIGHT = 2,
    F0115_VIEW_CELL_BACK_LEFT = 3,
    F0115_VIEW_LANE_CENTER = 0,
    F0115_VIEW_LANE_LEFT = 1,
    F0115_VIEW_LANE_RIGHT = 2,
    F0115_PROJECTILE_CENTER_Y = 47,
    F0115_ASPECT_ORDINAL_FIREBALL = 11,
    F0115_ASPECT_ORDINAL_SLIME = 13,
    F0115_ASPECT_ORDINAL_LIGHTNING = 4,
    F0115_ASPECT_ORDINAL_DEFAULT = 12,
    F0115_ASPECT_ORDINAL_POISON = 14
};

static const uint8_t s_g0215_projectile_scales[7] = {
    13u, 16u, 19u, 22u, 25u, 28u, 32u
};

static uint16_t read_be16(const uint8_t *bytes)
{
    return (uint16_t)(((uint16_t)bytes[0] << 8) | bytes[1]);
}

static unsigned int scaled_dimension(unsigned int dimension,
                                     unsigned int scale32)
{
    return (dimension * scale32 + (scale32 >> 1)) >> 5;
}

/* The fixed offsets are the G20E/MEDIA020 layout in the retained authentic
 * Atari CSB C558. G0210 ends exactly where G0209 begins (1642); G0218's set 0
 * supplies projectile X/Y, and G0215 differs from later MEDIA720 builds.
 * The caller admits the media identity before passing this buffer. */
static int validate_c558(const uint8_t *c558, size_t size)
{
    unsigned int index;
    if (!c558 || size != CSB_V1_ATARI_F0115_PROJECTILE_C558_SIZE ||
        memcmp(c558 + CSB_V1_ATARI_F0115_PROJECTILE_G0215_OFFSET,
               s_g0215_projectile_scales,
               sizeof(s_g0215_projectile_scales)) != 0) {
        return 0;
    }
    for (index = 0u; index < CSB_V1_ATARI_F0115_PROJECTILE_ASPECT_COUNT;
         ++index) {
        const uint8_t *aspect = c558 +
            CSB_V1_ATARI_F0115_PROJECTILE_G0210_OFFSET +
            index * G0210_RECORD_SIZE;
        const unsigned int native_index = aspect[0];
        const unsigned int derived_index = aspect[1];
        const unsigned int byte_width = aspect[2];
        const unsigned int height = aspect[3];
        const unsigned int info = read_be16(aspect + 4u);
        if (native_index >= M614_GRAPHIC_FIRST_EXPLOSION -
                            CSB_V1_ATARI_F0115_PROJECTILE_GFX_BASE ||
            derived_index > 156u || byte_width == 0u || height == 0u ||
            (info & F0115_TYPE_MASK) > F0115_TYPE_NO_BACK_NO_ROTATION) {
            return 0;
        }
    }
    for (index = 0u;
         index < G0218_SET_COUNT * G0218_SQUARE_COUNT * G0218_CELL_COUNT;
         ++index) {
        const size_t offset = CSB_V1_ATARI_F0115_PROJECTILE_G0218_OFFSET +
                              (size_t)index * 2u;
        if (c558[offset] >= 224u || c558[offset + 1u] > 136u) return 0;
    }
    return 1;
}

int csb_v1_atari_f0115_resolve_f0142_selector(
    uint16_t source_slot_thing,
    int weapon_aspect_ordinal,
    int object_aspect_index,
    int *out_selector)
{
    unsigned int thing_type;
    int selector;

    if (!out_selector || source_slot_thing == 0xfffeu ||
        source_slot_thing == 0xffffu) {
        return 0;
    }
    thing_type = (source_slot_thing >> 10) & 0x0fu;
    if (thing_type == 15u) {
        /* ReDMCSB DUNGEON.C F0142's special C15 selectors. Ordinals are the
         * source M000_INDEX_TO_ORDINAL values, hence the 1-based negatives. */
        switch (source_slot_thing) {
            case 0xff80u: selector = -F0115_ASPECT_ORDINAL_FIREBALL; break;
            case 0xff81u: selector = -F0115_ASPECT_ORDINAL_SLIME; break;
            case 0xff82u: selector = -F0115_ASPECT_ORDINAL_LIGHTNING; break;
            case 0xff86u:
            case 0xff87u: selector = -F0115_ASPECT_ORDINAL_POISON; break;
            default: selector = -F0115_ASPECT_ORDINAL_DEFAULT; break;
        }
    } else if (thing_type == 5u && weapon_aspect_ordinal != 0) {
        if (weapon_aspect_ordinal < 0 ||
            weapon_aspect_ordinal >
                CSB_V1_ATARI_F0115_PROJECTILE_ASPECT_COUNT) return 0;
        selector = -weapon_aspect_ordinal;
    } else {
        /* F0142 delegates this branch to F0141/G0237. The caller must provide
         * that authentic G559 result; this function does not guess from slot
         * bits or from the projectile's damage subtype. */
        if (object_aspect_index < 0 || object_aspect_index >= 85 ||
            weapon_aspect_ordinal < 0 ||
            weapon_aspect_ordinal >
                CSB_V1_ATARI_F0115_PROJECTILE_ASPECT_COUNT) return 0;
        selector = object_aspect_index;
    }
    *out_selector = selector;
    return 1;
}

int csb_v1_atari_f0115_plan_projectile(
    const CSB_V1_AtariF0115ProjectileInput *input,
    CSB_V1_AtariF0115ProjectilePlan *out_plan)
{
    const uint8_t *aspect;
    const uint8_t *coordinate;
    uint16_t info;
    int aspect_index;
    int depth;
    int scale_index;
    int scale32;
    int scale_with_energy;
    int view_lane;
    int map_parity;
    int projectile_axis;
    int party_axis;
    int delta = 0;
    int flip_horizontal = 0;
    int flip_vertical = 0;
    unsigned int byte_width;
    unsigned int height;

    if (!input || !out_plan || !validate_c558(input->c558,
                                               input->c558_size) ||
        input->view_square < 0 || input->view_square >= G0218_SQUARE_COUNT ||
        input->view_cell < 0 || input->view_cell >= G0218_CELL_COUNT - 1 ||
        input->projectile_direction < 0 || input->projectile_direction > 3 ||
        input->party_direction < 0 || input->party_direction > 3 ||
        input->kinetic_energy < 0 || input->kinetic_energy > 255) {
        return CSB_V1_ATARI_F0115_PROJECTILE_INVALID;
    }
    memset(out_plan, 0, sizeof(*out_plan));
    out_plan->f0142_selector = input->f0142_selector;
    out_plan->source_view_square = input->view_square;
    out_plan->source_view_cell = input->view_cell;
    out_plan->derived_bitmap_index = -1;
    out_plan->palette_change_index =
        CSB_V1_ATARI_F0115_PROJECTILE_NO_PALETTE;
    out_plan->destination_center_y = F0115_PROJECTILE_CENTER_Y;

    /* ReDMCSB DUNVIEW.C F0115:5647 admits a projectile only if the X byte
     * in G0218 set 0 is nonzero. Zero is an authentic hidden-cell marker. */
    coordinate = input->c558 + CSB_V1_ATARI_F0115_PROJECTILE_G0218_OFFSET +
        ((size_t)input->view_square * G0218_CELL_COUNT +
         (size_t)input->view_cell) * 2u;
    out_plan->source_coordinate_x = coordinate[0];
    out_plan->source_coordinate_y = coordinate[1];
    if (coordinate[0] == 0u) {
        return CSB_V1_ATARI_F0115_PROJECTILE_NOT_VISIBLE;
    }
    if (input->f0142_selector >= 0) {
        if (input->f0142_selector >= 85) {
            return CSB_V1_ATARI_F0115_PROJECTILE_INVALID;
        }
        out_plan->uses_object_aspect = 1;
        out_plan->object_aspect_index = input->f0142_selector;
        return CSB_V1_ATARI_F0115_PROJECTILE_DRAW_OBJECT;
    }

    /* F0142 returns -M000_INDEX_TO_ORDINAL(aspect index). */
    aspect_index = -input->f0142_selector - 1;
    if (aspect_index < 0 ||
        aspect_index >= CSB_V1_ATARI_F0115_PROJECTILE_ASPECT_COUNT) {
        return CSB_V1_ATARI_F0115_PROJECTILE_INVALID;
    }
    aspect = input->c558 + CSB_V1_ATARI_F0115_PROJECTILE_G0210_OFFSET +
             (size_t)aspect_index * G0210_RECORD_SIZE;
    info = read_be16(aspect + 4u);
    depth = input->view_square / 3;
    scale_index = (depth << 1) + (input->view_cell >> 1);
    if (scale_index < 0 || scale_index >= 7) {
        /* D0C has only the first two occupied cells in authentic G0218. */
        return CSB_V1_ATARI_F0115_PROJECTILE_INVALID;
    }
    scale_with_energy = (info & F0115_SCALE_WITH_ENERGY_MASK) != 0u;
    if ((!scale_with_energy || input->kinetic_energy == 255) &&
        input->view_square == 9) {
        scale32 = 0;
    } else {
        const int energy_factor =
            input->kinetic_energy + 1 > 96 ? input->kinetic_energy + 1 : 96;
        scale32 = s_g0215_projectile_scales[scale_index];
        if (scale_with_energy) scale32 = (scale32 * energy_factor) >> 8;
    }

    projectile_axis = input->projectile_direction & 1;
    party_axis = input->party_direction & 1;
    map_parity = (input->map_x + input->map_y) & 1;
    view_lane = input->view_square % 3 == 1 ? F0115_VIEW_LANE_LEFT :
                input->view_square % 3 == 2 ? F0115_VIEW_LANE_RIGHT :
                                              F0115_VIEW_LANE_CENTER;
    out_plan->aspect_index = aspect_index;
    out_plan->aspect_type = info & F0115_TYPE_MASK;
    out_plan->graphic_info = info;
    out_plan->source_scale_index = scale_index;
    out_plan->source_scale_32 = scale32;
    out_plan->source_byte_width = aspect[2];
    out_plan->source_height = aspect[3];

    if (out_plan->aspect_type == F0115_TYPE_NO_BACK_NO_ROTATION) {
        delta = 0;
    } else if (projectile_axis != party_axis) {
        delta = out_plan->aspect_type == F0115_TYPE_NO_BACK_AND_ROTATION
            ? 1 : 2;
        if (out_plan->aspect_type == F0115_TYPE_HAS_BACK_AND_ROTATION) {
            flip_vertical = map_parity;
            flip_horizontal = input->view_cell == F0115_VIEW_CELL_FRONT_LEFT ||
                              input->view_cell == F0115_VIEW_CELL_BACK_LEFT;
            if (!flip_vertical) flip_horizontal = !flip_horizontal;
        }
    } else {
        if (out_plan->aspect_type >= F0115_TYPE_NO_BACK_AND_ROTATION ||
            (out_plan->aspect_type == F0115_TYPE_HAS_BACK_NO_ROTATION &&
             input->projectile_direction != input->party_direction) ||
            (out_plan->aspect_type == F0115_TYPE_HAS_BACK_AND_ROTATION &&
             map_parity)) {
            delta = 0;
        } else {
            delta = 1;
        }
        flip_vertical =
            out_plan->aspect_type == F0115_TYPE_HAS_BACK_AND_ROTATION &&
            input->view_cell < F0115_VIEW_CELL_BACK_RIGHT;
        flip_horizontal = (info & F0115_SIDE_MASK) != 0u &&
            !((view_lane == F0115_VIEW_LANE_RIGHT) ||
              (view_lane == F0115_VIEW_LANE_CENTER &&
               (input->view_cell == F0115_VIEW_CELL_FRONT_RIGHT ||
                input->view_cell == F0115_VIEW_CELL_BACK_RIGHT)));
    }
    out_plan->bitmap_delta = delta;
    out_plan->native_graphic_index =
        CSB_V1_ATARI_F0115_PROJECTILE_GFX_BASE + aspect[0] + delta;
    if (out_plan->native_graphic_index <
            CSB_V1_ATARI_F0115_PROJECTILE_GFX_BASE ||
        out_plan->native_graphic_index >= M614_GRAPHIC_FIRST_EXPLOSION) {
        return CSB_V1_ATARI_F0115_PROJECTILE_INVALID;
    }
    out_plan->scale_with_kinetic_energy = scale_with_energy;
    out_plan->flip_horizontal = flip_horizontal;
    out_plan->flip_vertical = flip_vertical;
    if (scale32 == 0) {
        byte_width = aspect[2];
        height = aspect[3];
    } else {
        byte_width = scaled_dimension(aspect[2], (unsigned int)scale32);
        height = scaled_dimension(aspect[3], (unsigned int)scale32);
        out_plan->palette_change_index = (uint8_t)(scale_index >> 1);
    }
    if (byte_width == 0u || height == 0u) {
        return CSB_V1_ATARI_F0115_PROJECTILE_INVALID;
    }
    out_plan->draw_byte_width = (int)byte_width;
    out_plan->draw_width_pixels = (int)(byte_width << 1);
    out_plan->draw_height = (int)height;
    if (!scale_with_energy && scale32 != 0) {
        out_plan->derived_bitmap_index =
            CSB_V1_ATARI_F0115_PROJECTILE_DERIVED_BASE + aspect[1] +
            delta * 6 + scale_index;
    }
    return CSB_V1_ATARI_F0115_PROJECTILE_DRAW_PROJECTILE;
}
