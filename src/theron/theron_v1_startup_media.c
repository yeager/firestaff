#include "theron_v1_startup_media.h"

#include <stdio.h>
#include <string.h>

void theron_v1_startup_media_init(Theron_StartupMedia *media) {
    if (!media) {
        return;
    }
    memset(media, 0, sizeof(*media));
    media->track02_variant = (int)THERON_TRACK02_VARIANT_UNKNOWN;
    media->startup_roster_name_status =
        (int)THERON_TRACK02_SIGNAL_BAD_INPUT;
    media->startup_text_prompt_status =
        (int)THERON_TRACK02_SIGNAL_BAD_INPUT;
}

void theron_v1_startup_media_state_receipt_init(
    Theron_StartupMediaStateReceipt *receipt) {
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
    receipt->track02_variant = (int)THERON_TRACK02_VARIANT_UNKNOWN;
    receipt->startup_roster_name_status =
        (int)THERON_TRACK02_SIGNAL_BAD_INPUT;
    receipt->startup_text_prompt_status =
        (int)THERON_TRACK02_SIGNAL_BAD_INPUT;
}

static void theron_v1_startup_media_capture_roster(
    const uint8_t *hucard_rom,
    size_t hucard_rom_size,
    const char *md5_hex,
    Theron_StartupMedia *media) {
    Theron_Track02StartupRosterNameCatalog catalog;
    Theron_Track02SignalStatus status;
    size_t i;

    memset(&catalog, 0, sizeof(catalog));
    status = theron_v1_track02_catalog_startup_roster_names(hucard_rom,
                                                            hucard_rom_size,
                                                            md5_hex,
                                                            &catalog);
    media->startup_roster_name_status = (int)status;
    if (status != THERON_TRACK02_SIGNAL_OK) {
        return;
    }

    for (i = 0u;
         i < catalog.name_count &&
         i < THERON_STARTUP_MEDIA_ROSTER_CAPACITY;
         ++i) {
        snprintf(media->startup_roster_names[i],
                 sizeof(media->startup_roster_names[i]),
                 "%s",
                 catalog.names[i].name);
        snprintf(media->startup_roster_titles[i],
                 sizeof(media->startup_roster_titles[i]),
                 "%s",
                 catalog.names[i].title);
    }
    media->startup_roster_name_count = (int)i;
}

static void theron_v1_startup_media_capture_text(
    const uint8_t *hucard_rom,
    size_t hucard_rom_size,
    const char *md5_hex,
    Theron_StartupMedia *media) {
    Theron_Track02StartupTextMarkerCatalog catalog;
    Theron_Track02SignalStatus status;
    Theron_Track02StartupTextMarker marker;
    size_t text_len = 0u;

    memset(&catalog, 0, sizeof(catalog));
    status = theron_v1_track02_catalog_startup_text_markers(hucard_rom,
                                                            hucard_rom_size,
                                                            md5_hex,
                                                            &catalog);
    media->startup_text_prompt_status = (int)status;
    if (status != THERON_TRACK02_SIGNAL_OK) {
        return;
    }

    status = theron_v1_track02_copy_startup_text_marker(
        hucard_rom,
        hucard_rom_size,
        md5_hex,
        THERON_TRACK02_STARTUP_TEXT_US_RESURRECT_THERON_PROMPT,
        0u,
        media->startup_text_prompt,
        sizeof(media->startup_text_prompt),
        &text_len,
        &marker);
    if (status == THERON_TRACK02_SIGNAL_OK && text_len > 0u &&
        marker.kind == THERON_TRACK02_STARTUP_TEXT_US_RESURRECT_THERON_PROMPT) {
        media->startup_text_prompt_count = (int)catalog.marker_count;
    }
}

static void theron_v1_startup_media_record_bitmap_route(
    Theron_StartupMedia *media,
    const Theron_Track02StartupBitmapSample *sample) {
    int *ready = NULL;
    int *sample_count = NULL;
    size_t *nonzero = NULL;
    uint32_t *checksum = NULL;

    if (!media || !sample) {
        return;
    }
    switch (sample->route_bit) {
    case THERON_TRACK02_STARTUP_BITMAP_ROUTE_TITLE:
        ready = &media->startup_bitmap_title_route_ready;
        sample_count = &media->startup_bitmap_title_sample_count;
        nonzero = &media->startup_bitmap_title_nonzero_pixel_count;
        checksum = &media->startup_bitmap_title_checksum;
        break;
    case THERON_TRACK02_STARTUP_BITMAP_ROUTE_STAGE:
        ready = &media->startup_bitmap_stage_route_ready;
        sample_count = &media->startup_bitmap_stage_sample_count;
        nonzero = &media->startup_bitmap_stage_nonzero_pixel_count;
        checksum = &media->startup_bitmap_stage_checksum;
        break;
    case THERON_TRACK02_STARTUP_BITMAP_ROUTE_SOUL_ROOM:
        ready = &media->startup_bitmap_soul_room_route_ready;
        sample_count = &media->startup_bitmap_soul_room_sample_count;
        nonzero = &media->startup_bitmap_soul_room_nonzero_pixel_count;
        checksum = &media->startup_bitmap_soul_room_checksum;
        break;
    case THERON_TRACK02_STARTUP_BITMAP_ROUTE_FORCEFIELD:
        ready = &media->startup_bitmap_forcefield_route_ready;
        sample_count = &media->startup_bitmap_forcefield_sample_count;
        nonzero = &media->startup_bitmap_forcefield_nonzero_pixel_count;
        checksum = &media->startup_bitmap_forcefield_checksum;
        break;
    default:
        return;
    }

    *ready = 1;
    ++(*sample_count);
    *nonzero += sample->nonzero_pixel_count;
    *checksum ^= sample->checksum;
}

static void theron_v1_startup_media_capture_bitmaps(
    const uint8_t *hucard_rom,
    size_t hucard_rom_size,
    const char *md5_hex,
    Theron_StartupMedia *media) {
    Theron_Track02StartupBitmapCatalog catalog;
    Theron_Track02StartupBitmapAtlas atlas;
    Theron_Track02SignalStatus status;
    size_t i;

    memset(&catalog, 0, sizeof(catalog));
    status = theron_v1_track02_catalog_startup_bitmap_samples(hucard_rom,
                                                              hucard_rom_size,
                                                              md5_hex,
                                                              &catalog);
    media->startup_bitmap_decode_status = (int)status;
    if (status != THERON_TRACK02_SIGNAL_OK) {
        return;
    }

    media->startup_bitmap_sample_count = (int)catalog.sample_count;
    media->startup_bitmap_route_mask = catalog.route_mask;
    for (i = 0u; i < catalog.sample_count; ++i) {
        media->startup_bitmap_nonzero_pixel_count +=
            catalog.samples[i].nonzero_pixel_count;
        media->startup_bitmap_checksum ^=
            catalog.samples[i].checksum +
            (uint32_t)(catalog.samples[i].route_bit * 16777619u);
        theron_v1_startup_media_record_bitmap_route(media,
                                                    &catalog.samples[i]);
    }

    status = theron_v1_track02_build_startup_bitmap_atlas(&catalog, &atlas);
    if (status == THERON_TRACK02_SIGNAL_OK &&
        atlas.route_count > 0u &&
        atlas.total_tile_count > 0u &&
        atlas.total_nonzero_pixel_count > 0u &&
        atlas.checksum != 0u) {
        media->startup_bitmap_atlas_ready = 1;
        media->startup_bitmap_atlas_route_count = (int)atlas.route_count;
        media->startup_bitmap_atlas_route_mask = atlas.route_mask;
        media->startup_bitmap_atlas_tile_count = atlas.total_tile_count;
        media->startup_bitmap_atlas_nonzero_pixel_count =
            atlas.total_nonzero_pixel_count;
        media->startup_bitmap_atlas_checksum = atlas.checksum;
        media->startup_bitmap_atlas = atlas;
    }
}

void theron_v1_startup_media_capture_track02(
    const uint8_t *hucard_rom,
    size_t hucard_rom_size,
    const char *md5_hex,
    Theron_StartupMedia *out_media) {
    theron_v1_startup_media_init(out_media);
    if (!out_media || !hucard_rom || hucard_rom_size == 0u ||
        !md5_hex || md5_hex[0] == '\0') {
        return;
    }

    out_media->track02_variant =
        (int)theron_v1_track02_variant_for_md5(md5_hex);
    snprintf(out_media->track02_md5,
             sizeof(out_media->track02_md5),
             "%s",
             md5_hex);
    out_media->track02_size = hucard_rom_size;
    theron_v1_startup_media_capture_roster(hucard_rom,
                                           hucard_rom_size,
                                           md5_hex,
                                           out_media);
    theron_v1_startup_media_capture_text(hucard_rom,
                                         hucard_rom_size,
                                         md5_hex,
                                         out_media);
    theron_v1_startup_media_capture_bitmaps(hucard_rom,
                                            hucard_rom_size,
                                            md5_hex,
                                            out_media);
    out_media->startup_media_ready =
        out_media->track02_variant != THERON_TRACK02_VARIANT_UNKNOWN &&
        (out_media->startup_bitmap_decode_status == THERON_TRACK02_SIGNAL_OK ||
         out_media->startup_roster_name_status == THERON_TRACK02_SIGNAL_OK ||
         out_media->startup_text_prompt_status == THERON_TRACK02_SIGNAL_OK);
}

void theron_v1_startup_media_capture_track02_state_receipt(
    const uint8_t *hucard_rom,
    size_t hucard_rom_size,
    const char *md5_hex,
    Theron_StartupMediaStateReceipt *out_receipt) {
    Theron_StartupMedia media;
    size_t i;

    theron_v1_startup_media_state_receipt_init(out_receipt);
    if (!out_receipt) {
        return;
    }

    theron_v1_startup_media_capture_track02(hucard_rom,
                                            hucard_rom_size,
                                            md5_hex,
                                            &media);
    out_receipt->track02_variant = media.track02_variant;
    snprintf(out_receipt->track02_md5,
             sizeof(out_receipt->track02_md5),
             "%s",
             media.track02_md5);
    out_receipt->track02_size = media.track02_size;
    out_receipt->startup_media_ready = media.startup_media_ready;
    out_receipt->startup_bitmap_decode_status =
        media.startup_bitmap_decode_status;
    out_receipt->startup_bitmap_sample_count =
        media.startup_bitmap_sample_count;
    out_receipt->startup_bitmap_route_mask =
        media.startup_bitmap_route_mask;
    out_receipt->startup_bitmap_nonzero_pixel_count =
        media.startup_bitmap_nonzero_pixel_count;
    out_receipt->startup_bitmap_checksum =
        media.startup_bitmap_checksum;
    out_receipt->startup_bitmap_title_route_ready =
        media.startup_bitmap_title_route_ready;
    out_receipt->startup_bitmap_stage_route_ready =
        media.startup_bitmap_stage_route_ready;
    out_receipt->startup_bitmap_soul_room_route_ready =
        media.startup_bitmap_soul_room_route_ready;
    out_receipt->startup_bitmap_forcefield_route_ready =
        media.startup_bitmap_forcefield_route_ready;
    out_receipt->startup_bitmap_atlas_ready =
        media.startup_bitmap_atlas_ready;
    out_receipt->startup_bitmap_atlas_route_count =
        media.startup_bitmap_atlas_route_count;
    out_receipt->startup_bitmap_atlas_route_mask =
        media.startup_bitmap_atlas_route_mask;
    out_receipt->startup_bitmap_atlas_tile_count =
        media.startup_bitmap_atlas_tile_count;
    out_receipt->startup_bitmap_atlas_nonzero_pixel_count =
        media.startup_bitmap_atlas_nonzero_pixel_count;
    out_receipt->startup_bitmap_atlas_checksum =
        media.startup_bitmap_atlas_checksum;
    out_receipt->startup_bitmap_atlas = media.startup_bitmap_atlas;
    out_receipt->startup_bitmap_title_sample_count =
        media.startup_bitmap_title_sample_count;
    out_receipt->startup_bitmap_stage_sample_count =
        media.startup_bitmap_stage_sample_count;
    out_receipt->startup_bitmap_soul_room_sample_count =
        media.startup_bitmap_soul_room_sample_count;
    out_receipt->startup_bitmap_forcefield_sample_count =
        media.startup_bitmap_forcefield_sample_count;
    out_receipt->startup_bitmap_title_nonzero_pixel_count =
        media.startup_bitmap_title_nonzero_pixel_count;
    out_receipt->startup_bitmap_stage_nonzero_pixel_count =
        media.startup_bitmap_stage_nonzero_pixel_count;
    out_receipt->startup_bitmap_soul_room_nonzero_pixel_count =
        media.startup_bitmap_soul_room_nonzero_pixel_count;
    out_receipt->startup_bitmap_forcefield_nonzero_pixel_count =
        media.startup_bitmap_forcefield_nonzero_pixel_count;
    out_receipt->startup_bitmap_title_checksum =
        media.startup_bitmap_title_checksum;
    out_receipt->startup_bitmap_stage_checksum =
        media.startup_bitmap_stage_checksum;
    out_receipt->startup_bitmap_soul_room_checksum =
        media.startup_bitmap_soul_room_checksum;
    out_receipt->startup_bitmap_forcefield_checksum =
        media.startup_bitmap_forcefield_checksum;
    out_receipt->startup_roster_name_status =
        media.startup_roster_name_status;
    out_receipt->startup_text_prompt_status =
        media.startup_text_prompt_status;
    out_receipt->startup_text_prompt_count =
        media.startup_text_prompt_count;
    snprintf(out_receipt->startup_text_prompt,
             sizeof(out_receipt->startup_text_prompt),
             "%s",
             media.startup_text_prompt);
    if (media.startup_roster_name_status != THERON_TRACK02_SIGNAL_OK) {
        return;
    }

    for (i = 0u;
         i < (size_t)media.startup_roster_name_count &&
         i < THERON_STARTUP_MEDIA_ROSTER_CAPACITY;
         ++i) {
        snprintf(out_receipt->startup_roster_names[i],
                 sizeof(out_receipt->startup_roster_names[i]),
                 "%s",
                 media.startup_roster_names[i]);
        snprintf(out_receipt->startup_roster_titles[i],
                 sizeof(out_receipt->startup_roster_titles[i]),
                 "%s",
                 media.startup_roster_titles[i]);
    }
    out_receipt->startup_roster_name_count = (int)i;
}

int theron_v1_startup_media_state_receipt_has_complete_bitmap_routes(
    const Theron_StartupMediaStateReceipt *receipt) {
    const unsigned int required_mask =
        THERON_TRACK02_STARTUP_BITMAP_ROUTE_TITLE |
        THERON_TRACK02_STARTUP_BITMAP_ROUTE_STAGE |
        THERON_TRACK02_STARTUP_BITMAP_ROUTE_SOUL_ROOM |
        THERON_TRACK02_STARTUP_BITMAP_ROUTE_FORCEFIELD;

    if (!receipt) {
        return 0;
    }
    if (!receipt->startup_media_ready ||
        receipt->startup_bitmap_decode_status != THERON_TRACK02_SIGNAL_OK ||
        receipt->startup_bitmap_sample_count < 8 ||
        !receipt->startup_bitmap_atlas_ready ||
        receipt->startup_bitmap_atlas_route_count < 4 ||
        (receipt->startup_bitmap_atlas_route_mask & required_mask) !=
            required_mask ||
        receipt->startup_bitmap_atlas_tile_count < 8u ||
        receipt->startup_bitmap_atlas_nonzero_pixel_count == 0u ||
        receipt->startup_bitmap_atlas_checksum == 0u ||
        (receipt->startup_bitmap_route_mask & required_mask) !=
            required_mask ||
        receipt->startup_bitmap_nonzero_pixel_count == 0u ||
        receipt->startup_bitmap_checksum == 0u) {
        return 0;
    }

    return receipt->startup_bitmap_title_route_ready &&
           receipt->startup_bitmap_stage_route_ready &&
           receipt->startup_bitmap_soul_room_route_ready &&
           receipt->startup_bitmap_forcefield_route_ready &&
           receipt->startup_bitmap_title_sample_count >= 3 &&
           receipt->startup_bitmap_stage_sample_count >= 3 &&
           receipt->startup_bitmap_soul_room_sample_count >= 1 &&
           receipt->startup_bitmap_forcefield_sample_count >= 1 &&
           receipt->startup_bitmap_title_nonzero_pixel_count > 0u &&
           receipt->startup_bitmap_stage_nonzero_pixel_count > 0u &&
           receipt->startup_bitmap_soul_room_nonzero_pixel_count > 0u &&
           receipt->startup_bitmap_forcefield_nonzero_pixel_count > 0u &&
           receipt->startup_bitmap_title_checksum != 0u &&
           receipt->startup_bitmap_stage_checksum != 0u &&
           receipt->startup_bitmap_soul_room_checksum != 0u &&
           receipt->startup_bitmap_forcefield_checksum != 0u;
}
