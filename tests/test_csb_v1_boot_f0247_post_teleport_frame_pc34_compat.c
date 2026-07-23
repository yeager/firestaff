/* Real CSB boot-profile C14 ownership -> F0128/F0115 frame route. */
#include "csb_v1_boot.h"

#include <stdio.h>
#include <string.h>

typedef struct { int calls; } Probe;

static void put_le16(unsigned char *p, int off, unsigned short v)
{ p[off] = (unsigned char)v; p[off + 1] = (unsigned char)(v >> 8); }

static int draw_real_c14(void *user,
                         const CSB_V1_ViewportRuntimeProjectileSpriteBlit *blit,
                         uint8_t *pixels, int stride)
{
    Probe *probe = (Probe *)user;
    if (!probe || !blit || !pixels || stride < 320 || blit->graphic_index != 487 ||
        !blit->uses_f0791_blit || blit->transparent_color != 10) return 0;
    ++probe->calls;
    pixels[33 * stride + 112] = 0x5a;
    return 1;
}

static void make_real_pc34_c14(CSB_V1_DungeonData *dungeon, unsigned char raw[224])
{
    int i;
    memset(dungeon, 0, sizeof(*dungeon)); memset(raw, 0, 224u);
    dungeon->level_count = 2; dungeon->level_widths[0] = dungeon->level_widths[1] = 2;
    dungeon->level_heights[0] = dungeon->level_heights[1] = 2;
    dungeon->level_offsets[0] = 0; dungeon->level_offsets[1] = 4;
    dungeon->square_bytes = 1; dungeon->raw_data = raw; dungeon->raw_size = 224;
    dungeon->square_first_thing_base = 96; dungeon->square_first_thing_count = 2;
    dungeon->thing_data_bases[3] = 160; dungeon->thing_type_counts[3] = 1;
    dungeon->thing_data_bases[14] = 184; dungeon->thing_type_counts[14] = 1;
    for (i = 0; i < 8; ++i) raw[i] = (unsigned char)(1u << 5);
    raw[4] = (unsigned char)((1u << 5) | 0x10u);
    put_le16(raw, 80, 1u); put_le16(raw, 98, (unsigned short)(3u << 10));
    put_le16(raw, 160, (unsigned short)(14u << 10));
    put_le16(raw, 184, 0xfffeu); put_le16(raw, 186, 0x1407u);
    raw[188] = 200u; raw[189] = 45u; put_le16(raw, 190, 17u);
}

int main(void)
{
    CSB_V1_BootProfile profile;
    CSB_V1_DungeonData dungeon;
    CSB_V1_ViewportRuntimeDrawerBinding binding;
    CSB_V1_ViewportRuntimeDrawCounts counts;
    CSB_V1_F0219ProjectileImpactMaterialHandoffPc34 handoff;
    Probe probe;
    unsigned char raw[224];
    uint8_t frame[320 * 200];

    memset(&profile, 0, sizeof(profile)); memset(&binding, 0, sizeof(binding));
    memset(&counts, 0, sizeof(counts)); memset(&probe, 0, sizeof(probe));
    memset(frame, 0, sizeof(frame)); make_real_pc34_c14(&dungeon, raw);
    profile.runtime.dungeon_handle = &dungeon; profile.runtime.current_level = 1;
    profile.runtime.party_x = 0; profile.runtime.party_y = 0; profile.runtime.party_dir = 0;
    if (!csb_v1_boot_admit_post_teleport_projectile_impact_pc34(
            &profile, 1, 0, 0, (uint16_t)(14u << 10),
            CSB_V1_F0115_PROJECTILE_ORDINAL_M715,
            CSB_V1_F0115_PROJECTILE_SIDE_RIGHT,
            CSB_V1_F0115_PROJECTILE_COORDINATE_SET_MIDDLE, &handoff) ||
        !handoff.valid || profile.post_teleport_projectile_handoff_count != 1u) {
        fputs("boot route rejected source-owned C14\n", stderr); return 1;
    }
    binding.projectile_sprite_drawer = draw_real_c14;
    binding.projectile_sprite_user = &probe;
    if (!csb_v1_boot_render_viewport_frame_pc34(&profile, frame, 320, 200,
                                                 &binding, &counts) ||
        probe.calls != 1 || frame[33 * 320 + 112] != 0x5a ||
        counts.projectile_marker_drawn_count != 0) {
        fputs("boot startup frame did not consume C14 as a real F0115 blit\n", stderr);
        return 1;
    }
    puts("ok: source C14 reaches CSB boot F0128/F0115 without marker fallback");
    return 0;
}
