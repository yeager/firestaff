/* Opt-in host-gate smoke. Normal mode requires explicit media paths and
 * remains fail-closed at the presently unbound palette descriptor boundary. */
#include "asset_status_m12.h"
#include "theron_v1_boot.h"
#include "theron_v1_track02.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define SYSCARD3_MD5 "ff1a674273fe3540ccef576376407d1d"
#define SYSCARD3_BYTES 0x40200u

static int known_file(const char *path, const char *a, const char *b,
                      size_t exact_size, int raw)
{
    struct stat st;
    char md5[33];
    if (!path || !path[0] || stat(path, &st) != 0 || st.st_size <= 0 ||
        (exact_size && (size_t)st.st_size != exact_size) ||
        (raw && ((size_t)st.st_size % THERON_TRACK02_RAW_SECTOR_BYTES) != 0u) ||
        !m12_file_md5_hex(path, md5)) return 0;
    return strcmp(md5, a) == 0 || (b && strcmp(md5, b) == 0);
}

static void fixture_media(Theron_StartupMediaStateReceipt *m)
{
    unsigned int bits = THERON_TRACK02_STARTUP_BITMAP_ROUTE_TITLE |
        THERON_TRACK02_STARTUP_BITMAP_ROUTE_STAGE |
        THERON_TRACK02_STARTUP_BITMAP_ROUTE_SOUL_ROOM |
        THERON_TRACK02_STARTUP_BITMAP_ROUTE_FORCEFIELD;
    size_t i;
    memset(m, 0, sizeof(*m));
    m->track02_variant = THERON_TRACK02_VARIANT_US_BIN;
    snprintf(m->track02_md5, sizeof(m->track02_md5), "%s", THERON_TRACK02_MD5_US_BIN);
    m->startup_media_ready = m->startup_bitmap_atlas_ready = 1;
    m->startup_bitmap_decode_status = THERON_TRACK02_SIGNAL_OK;
    m->startup_bitmap_route_mask = m->startup_bitmap_atlas_route_mask = bits;
    m->startup_bitmap_raw_route_mask = bits;
    m->startup_bitmap_title_route_ready = 1;
    m->startup_bitmap_stage_route_ready = 1;
    m->startup_bitmap_soul_room_route_ready = 1;
    m->startup_bitmap_forcefield_route_ready = 1;
    m->startup_bitmap_sample_count = 48;
    m->startup_bitmap_nonzero_pixel_count = 48u;
    m->startup_bitmap_checksum = 1u;
    m->startup_bitmap_atlas_route_count = 4;
    m->startup_bitmap_atlas_tile_count = 48u;
    m->startup_bitmap_atlas_nonzero_pixel_count = 48u;
    m->startup_bitmap_raw_route_count = 4;
    m->startup_bitmap_raw_atlas_tile_count = 48u;
    m->startup_bitmap_wide_route_mask = bits;
    m->startup_bitmap_wide_route_count = 4;
    m->startup_bitmap_wide_atlas_tile_count = 48u;
    m->startup_bitmap_title_sample_count = 12;
    m->startup_bitmap_stage_sample_count = 12;
    m->startup_bitmap_soul_room_sample_count = 12;
    m->startup_bitmap_forcefield_sample_count = 12;
    m->startup_bitmap_title_nonzero_pixel_count = 1u;
    m->startup_bitmap_stage_nonzero_pixel_count = 1u;
    m->startup_bitmap_soul_room_nonzero_pixel_count = 1u;
    m->startup_bitmap_forcefield_nonzero_pixel_count = 1u;
    m->startup_bitmap_title_checksum = 1u;
    m->startup_bitmap_stage_checksum = 2u;
    m->startup_bitmap_soul_room_checksum = 3u;
    m->startup_bitmap_forcefield_checksum = 4u;
    m->startup_bitmap_title_atlas_tile_count = 12u;
    m->startup_bitmap_stage_atlas_tile_count = 12u;
    m->startup_bitmap_soul_room_atlas_tile_count = 12u;
    m->startup_bitmap_forcefield_atlas_tile_count = 12u;
    m->startup_bitmap_title_atlas_width = 96u;
    m->startup_bitmap_stage_atlas_width = 96u;
    m->startup_bitmap_soul_room_atlas_width = 96u;
    m->startup_bitmap_forcefield_atlas_width = 96u;
    m->startup_bitmap_atlas_checksum = 1u;
    m->startup_bitmap_atlas.route_count = 4u;
    m->startup_bitmap_atlas.route_mask = bits;
    for (i = 0u; i < 4u; ++i) {
        Theron_Track02StartupBitmapAtlasRoute *r = &m->startup_bitmap_atlas.routes[i];
        r->route_bit = 1u << i; r->tile_count = 12u; r->width = 96u; r->height = 8u;
        r->nonzero_pixel_count = 1u; r->checksum = (uint32_t)(i + 1u);
    }
}

static int selftest(void)
{
    Theron_StartupMediaStateReceipt media;
    Theron_V1_BootStartupRawMediaGraphicsReceipt raw;

    fixture_media(&media);
    /* The receipt is int-flag-only now (the df88dbda4 clobber dropped the
     * status strings); the flags carry the same contract. */
    if (!theron_v1_boot_startup_raw_media_graphics_receipt_from_verified_media(
            &media, 1, 0, &raw) ||
        !raw.valid || raw.palette_descriptor_relation_verified) return 0;
    if (!theron_v1_boot_startup_raw_media_graphics_receipt_from_verified_media(
            &media, 1, 1, &raw) ||
        !raw.valid || !raw.palette_descriptor_relation_verified) return 0;
    return 1;
}

int main(int argc, char **argv)
{
    const char *raw = getenv("THERON_RAW_TRACK02");
    const char *card = getenv("THERON_SYSTEM_CARD");
    if (argc == 2 && strcmp(argv[1], "--selftest") == 0) return selftest() ? 0 : 1;
    if (argc != 1) return 2;
    if (!raw || !raw[0] || !card || !card[0]) {
        printf("status=skip reason=explicit_raw_track02_and_system_card_required fallback=not_run\n"); return 0;
    }
    if (!known_file(raw, THERON_TRACK02_MD5_JP_BIN, THERON_TRACK02_MD5_US_BIN, 0u, 1) ||
        !known_file(card, SYSCARD3_MD5, NULL, SYSCARD3_BYTES, 0)) {
        printf("status=blocked reason=raw_media_or_system_card_unverified fallback=not_run\n"); return 1;
    }
    printf("status=blocked reason=TRACK02_PALETTE_DESCRIPTOR_UNPROVEN fallback=not_run\n");
    return 0;
}
