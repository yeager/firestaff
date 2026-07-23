/* Live F0219 runtime receipt -> boot profile -> F0128/F0115 C14 blit. */
#include "csb_v1_boot.h"

#include <stdio.h>
#include <string.h>

typedef struct { int calls; } Probe;

static void put16(unsigned char *p, int offset, unsigned short value)
{
    p[offset] = (unsigned char)value;
    p[offset + 1] = (unsigned char)(value >> 8);
}

static int draw(void *user, const CSB_V1_ViewportRuntimeProjectileSpriteBlit *blit,
                uint8_t *pixels, int stride)
{
    Probe *probe = user;
    if (!probe || !blit || !pixels || stride < 320 ||
        blit->graphic_index != 455 || !blit->uses_f0791_blit ||
        blit->transparent_color != 10) {
        return 0;
    }
    ++probe->calls;
    pixels[33 * stride + 112] = 0x5a;
    return 1;
}

static void fixture(CSB_V1_DungeonData *dungeon, unsigned char raw[224])
{
    int i;
    memset(dungeon, 0, sizeof(*dungeon));
    memset(raw, 0, 224);
    dungeon->level_count = 2;
    dungeon->level_widths[0] = dungeon->level_widths[1] = 2;
    dungeon->level_heights[0] = dungeon->level_heights[1] = 2;
    dungeon->level_offsets[1] = 4;
    dungeon->square_bytes = 1;
    dungeon->raw_data = raw;
    dungeon->raw_size = 224;
    dungeon->square_first_thing_base = 96;
    dungeon->square_first_thing_count = 2;
    dungeon->thing_data_bases[3] = 160;
    dungeon->thing_type_counts[3] = 1;
    dungeon->thing_data_bases[14] = 184;
    dungeon->thing_type_counts[14] = 1;
    for (i = 0; i < 8; ++i) raw[i] = (unsigned char)(1u << 5);
    raw[4] = (unsigned char)((1u << 5) | 0x10u);
    put16(raw, 80, 1);
    put16(raw, 98, (unsigned short)(3u << 10));
    put16(raw, 160, (unsigned short)(14u << 10));
    put16(raw, 184, 0xfffeu);
    put16(raw, 186, 0x1407u);
    raw[188] = 200;
    raw[189] = 45;
    put16(raw, 190, 17);
}

int main(void)
{
    CSB_V1_BootProfile boot;
    CSB_V1_DungeonData dungeon;
    CSB_V1_ViewportRuntimeDrawerBinding binding;
    CSB_V1_ViewportRuntimeDrawCounts counts;
    Probe probe;
    unsigned char raw[224];
    uint8_t frame[320 * 200];
    struct ProjectileInstance_Compat *projectile;
    CSB_V1_RuntimePostTeleportProjectileReceiptPc34 *receipt;

    memset(&boot, 0, sizeof(boot));
    memset(&binding, 0, sizeof(binding));
    memset(&counts, 0, sizeof(counts));
    memset(&probe, 0, sizeof(probe));
    memset(frame, 0, sizeof(frame));
    fixture(&dungeon, raw);

    boot.runtime.dungeon_handle = &dungeon;
    boot.runtime.current_level = 1;
    boot.runtime.party_x = 0;
    boot.runtime.party_y = 0;
    boot.runtime.party_dir = 0;
    projectile = &boot.runtime.projectiles.entries[0];
    projectile->slotIndex = 0;
    projectile->reserved3 = 1;
    projectile->mapIndex = 1;
    projectile->mapX = 0;
    projectile->mapY = 0;
    projectile->cell = 0;
    projectile->direction = 0;
    projectile->projectileSubtype = PROJECTILE_SUBTYPE_KINETIC_ARROW;
    receipt = &boot.runtime.post_teleport_projectiles[0];
    receipt->valid = 1;
    receipt->projectile_slot = 0;
    receipt->map_index = 1;
    receipt->map_x = 0;
    receipt->map_y = 0;
    receipt->cell = 0;
    boot.runtime.post_teleport_projectile_count = 1;

    binding.projectile_sprite_drawer = draw;
    binding.projectile_sprite_user = &probe;
    if (!csb_v1_boot_render_viewport_frame_pc34(
            &boot, frame, 320, 200, &binding, &counts) ||
        probe.calls != 1 || frame[33 * 320 + 112] != 0x5a ||
        counts.post_teleport_projectile_handoff_drawn_count != 1 ||
        counts.projectile_marker_drawn_count != 0) {
        fprintf(stderr,
                "live F0219 receipt did not draw real F0115 material "
                "(calls=%d handoff=%d marker=%d frame=%u)\n",
                probe.calls, counts.post_teleport_projectile_handoff_drawn_count,
                counts.projectile_marker_drawn_count, frame[33 * 320 + 112]);
        return 1;
    }
    puts("ok: live F0219 receipt reaches boot F0128/F0115 without marker");
    return 0;
}
