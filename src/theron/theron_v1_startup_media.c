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

static void theron_v1_startup_media_capture_runtime_identity(
    const uint8_t *hucard_rom,
    size_t hucard_rom_size,
    const char *md5_hex,
    Theron_RuntimeMediaIdentity *identity) {
    Theron_Track02BankSignal signal;

    if (!identity) return;
    memset(identity, 0, sizeof(*identity));
    if (theron_v1_track02_find_bank_signal(hucard_rom, hucard_rom_size,
                                            md5_hex, &signal) !=
        THERON_TRACK02_SIGNAL_OK || signal.anchor_count == 0u ||
        signal.stride == 0u) {
        return;
    }
    identity->track02_variant = (int)signal.variant;
    identity->bank_anchor_index = 0u;
    identity->bank_descriptor_offset = signal.descriptor_offsets[0];
    identity->bank_first_value = signal.first_value;
    identity->bank_last_value = signal.last_value;
    identity->bank_stride = signal.stride;
    identity->audio_frame_ready = signal.audio_bank_id_recognized[0] ? 1 : 0;
    identity->audio_bank_id = signal.audio_bank_id[0];
    identity->audio_bank_id_offset = signal.audio_bank_id_offsets[0];
    identity->audio_bank_prefix_offset = signal.audio_bank_prefix_offsets[0];
    identity->checksum = signal.audio_bank_id[0] ^
        (uint32_t)signal.descriptor_offsets[0] ^
        ((uint32_t)signal.first_value << 16) ^ signal.last_value;
    identity->ready = 1;
}

void theron_v1_startup_media_capture_track02_identity(
    const uint8_t *hucard_rom,
    size_t hucard_rom_size,
    const char *md5_hex,
    Theron_RuntimeMediaIdentity *out_identity) {
    theron_v1_startup_media_capture_runtime_identity(
        hucard_rom, hucard_rom_size, md5_hex, out_identity);
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

static void theron_v1_startup_media_record_atlas_route(
    Theron_StartupMedia *media,
    const Theron_Track02StartupBitmapAtlasRoute *route) {
    size_t *tile_count = NULL;
    uint16_t *width = NULL;
    size_t *first_raw_offset = NULL;
    size_t *last_raw_offset = NULL;
    size_t *first_user_data_offset = NULL;
    size_t *last_user_data_offset = NULL;
    size_t min_wide_tiles = 12u;
    uint16_t min_wide_width = 96u;

    if (!media || !route) {
        return;
    }
    switch (route->route_bit) {
    case THERON_TRACK02_STARTUP_BITMAP_ROUTE_TITLE:
        tile_count = &media->startup_bitmap_title_atlas_tile_count;
        width = &media->startup_bitmap_title_atlas_width;
        first_raw_offset = &media->startup_bitmap_title_first_raw_offset;
        last_raw_offset = &media->startup_bitmap_title_last_raw_offset;
        first_user_data_offset =
            &media->startup_bitmap_title_first_user_data_offset;
        last_user_data_offset =
            &media->startup_bitmap_title_last_user_data_offset;
        break;
    case THERON_TRACK02_STARTUP_BITMAP_ROUTE_STAGE:
        tile_count = &media->startup_bitmap_stage_atlas_tile_count;
        width = &media->startup_bitmap_stage_atlas_width;
        first_raw_offset = &media->startup_bitmap_stage_first_raw_offset;
        last_raw_offset = &media->startup_bitmap_stage_last_raw_offset;
        first_user_data_offset =
            &media->startup_bitmap_stage_first_user_data_offset;
        last_user_data_offset =
            &media->startup_bitmap_stage_last_user_data_offset;
        break;
    case THERON_TRACK02_STARTUP_BITMAP_ROUTE_SOUL_ROOM:
        tile_count = &media->startup_bitmap_soul_room_atlas_tile_count;
        width = &media->startup_bitmap_soul_room_atlas_width;
        first_raw_offset =
            &media->startup_bitmap_soul_room_first_raw_offset;
        last_raw_offset =
            &media->startup_bitmap_soul_room_last_raw_offset;
        first_user_data_offset =
            &media->startup_bitmap_soul_room_first_user_data_offset;
        last_user_data_offset =
            &media->startup_bitmap_soul_room_last_user_data_offset;
        break;
    case THERON_TRACK02_STARTUP_BITMAP_ROUTE_FORCEFIELD:
        tile_count = &media->startup_bitmap_forcefield_atlas_tile_count;
        width = &media->startup_bitmap_forcefield_atlas_width;
        first_raw_offset =
            &media->startup_bitmap_forcefield_first_raw_offset;
        last_raw_offset =
            &media->startup_bitmap_forcefield_last_raw_offset;
        first_user_data_offset =
            &media->startup_bitmap_forcefield_first_user_data_offset;
        last_user_data_offset =
            &media->startup_bitmap_forcefield_last_user_data_offset;
        break;
    default:
        return;
    }
    *tile_count = route->tile_count;
    *width = route->width;
    if (route->tile_count > 0u) {
        *first_raw_offset = route->first_raw_offset;
        *last_raw_offset = route->last_raw_offset;
        *first_user_data_offset = route->first_user_data_offset;
        *last_user_data_offset =
            route->user_data_offsets[route->tile_count - 1u];
    }
    if (route->tile_count >= min_wide_tiles &&
        route->width >= min_wide_width &&
        route->nonzero_pixel_count > 0u &&
        (media->startup_bitmap_wide_route_mask & route->route_bit) == 0u) {
        media->startup_bitmap_wide_route_mask |= route->route_bit;
        ++media->startup_bitmap_wide_route_count;
        media->startup_bitmap_wide_atlas_tile_count += route->tile_count;
    }
    if ((media->track02_variant == (int)THERON_TRACK02_VARIANT_JP_BIN ||
         media->track02_variant == (int)THERON_TRACK02_VARIANT_US_BIN) &&
        (media->startup_bitmap_raw_route_mask & route->route_bit) == 0u) {
        media->startup_bitmap_raw_route_mask |= route->route_bit;
        ++media->startup_bitmap_raw_route_count;
        media->startup_bitmap_raw_atlas_tile_count += route->tile_count;
    }
    if ((media->track02_variant == (int)THERON_TRACK02_VARIANT_JP_REV1_ISO ||
         media->track02_variant == (int)THERON_TRACK02_VARIANT_US_ISO) &&
        (media->startup_bitmap_iso_route_mask & route->route_bit) == 0u) {
        media->startup_bitmap_iso_route_mask |= route->route_bit;
        ++media->startup_bitmap_iso_route_count;
        media->startup_bitmap_iso_atlas_tile_count += route->tile_count;
    }
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
    /* The catalog route bits identify bounded byte candidates selected by the
     * Track 02 layout scan. They are not a game-owned title/stage/Soul
     * Room/forcefield semantic binding; the original VDC/VCE consumer is
     * still absent from the captured evidence. Keep the indexed samples for
     * diagnostics, while palette/render admission remains separately gated. */
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

    status = theron_v1_track02_build_startup_bitmap_atlas_wide(&catalog,
                                                               &atlas);
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
        for (i = 0u; i < atlas.route_count; ++i) {
            theron_v1_startup_media_record_atlas_route(media,
                                                       &atlas.routes[i]);
        }
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
    theron_v1_startup_media_capture_runtime_identity(
        hucard_rom, hucard_rom_size, md5_hex, &out_media->runtime_media_identity);
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
    {
        Theron_Track02FontTileReceipt font_receipt;
        memset(&font_receipt, 0, sizeof(font_receipt));
        if (theron_v1_track02_extract_font_tiles(
                hucard_rom, hucard_rom_size, md5_hex,
                &font_receipt) == THERON_TRACK02_SIGNAL_OK &&
            font_receipt.valid &&
            font_receipt.nonblank_tile_count > 0u) {
            out_media->startup_font_tiles_ready = 1;
            out_media->startup_font_tile_receipt = font_receipt;
        }
    }
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
    out_receipt->runtime_media_identity = media.runtime_media_identity;
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
    out_receipt->startup_bitmap_title_atlas_tile_count =
        media.startup_bitmap_title_atlas_tile_count;
    out_receipt->startup_bitmap_stage_atlas_tile_count =
        media.startup_bitmap_stage_atlas_tile_count;
    out_receipt->startup_bitmap_soul_room_atlas_tile_count =
        media.startup_bitmap_soul_room_atlas_tile_count;
    out_receipt->startup_bitmap_forcefield_atlas_tile_count =
        media.startup_bitmap_forcefield_atlas_tile_count;
    out_receipt->startup_bitmap_title_atlas_width =
        media.startup_bitmap_title_atlas_width;
    out_receipt->startup_bitmap_stage_atlas_width =
        media.startup_bitmap_stage_atlas_width;
    out_receipt->startup_bitmap_soul_room_atlas_width =
        media.startup_bitmap_soul_room_atlas_width;
    out_receipt->startup_bitmap_forcefield_atlas_width =
        media.startup_bitmap_forcefield_atlas_width;
    out_receipt->startup_bitmap_title_first_raw_offset =
        media.startup_bitmap_title_first_raw_offset;
    out_receipt->startup_bitmap_title_last_raw_offset =
        media.startup_bitmap_title_last_raw_offset;
    out_receipt->startup_bitmap_title_first_user_data_offset =
        media.startup_bitmap_title_first_user_data_offset;
    out_receipt->startup_bitmap_title_last_user_data_offset =
        media.startup_bitmap_title_last_user_data_offset;
    out_receipt->startup_bitmap_stage_first_raw_offset =
        media.startup_bitmap_stage_first_raw_offset;
    out_receipt->startup_bitmap_stage_last_raw_offset =
        media.startup_bitmap_stage_last_raw_offset;
    out_receipt->startup_bitmap_stage_first_user_data_offset =
        media.startup_bitmap_stage_first_user_data_offset;
    out_receipt->startup_bitmap_stage_last_user_data_offset =
        media.startup_bitmap_stage_last_user_data_offset;
    out_receipt->startup_bitmap_soul_room_first_raw_offset =
        media.startup_bitmap_soul_room_first_raw_offset;
    out_receipt->startup_bitmap_soul_room_last_raw_offset =
        media.startup_bitmap_soul_room_last_raw_offset;
    out_receipt->startup_bitmap_soul_room_first_user_data_offset =
        media.startup_bitmap_soul_room_first_user_data_offset;
    out_receipt->startup_bitmap_soul_room_last_user_data_offset =
        media.startup_bitmap_soul_room_last_user_data_offset;
    out_receipt->startup_bitmap_forcefield_first_raw_offset =
        media.startup_bitmap_forcefield_first_raw_offset;
    out_receipt->startup_bitmap_forcefield_last_raw_offset =
        media.startup_bitmap_forcefield_last_raw_offset;
    out_receipt->startup_bitmap_forcefield_first_user_data_offset =
        media.startup_bitmap_forcefield_first_user_data_offset;
    out_receipt->startup_bitmap_forcefield_last_user_data_offset =
        media.startup_bitmap_forcefield_last_user_data_offset;
    out_receipt->startup_bitmap_wide_route_mask =
        media.startup_bitmap_wide_route_mask;
    out_receipt->startup_bitmap_wide_route_count =
        media.startup_bitmap_wide_route_count;
    out_receipt->startup_bitmap_wide_atlas_tile_count =
        media.startup_bitmap_wide_atlas_tile_count;
    out_receipt->startup_bitmap_raw_route_mask =
        media.startup_bitmap_raw_route_mask;
    out_receipt->startup_bitmap_raw_route_count =
        media.startup_bitmap_raw_route_count;
    out_receipt->startup_bitmap_raw_atlas_tile_count =
        media.startup_bitmap_raw_atlas_tile_count;
    out_receipt->startup_bitmap_iso_route_mask =
        media.startup_bitmap_iso_route_mask;
    out_receipt->startup_bitmap_iso_route_count =
        media.startup_bitmap_iso_route_count;
    out_receipt->startup_bitmap_iso_atlas_tile_count =
        media.startup_bitmap_iso_atlas_tile_count;
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
    out_receipt->startup_font_tiles_ready =
        media.startup_font_tiles_ready;
    if (media.startup_font_tiles_ready) {
        out_receipt->startup_font_tile_receipt =
            media.startup_font_tile_receipt;
    }
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
        receipt->startup_bitmap_sample_count < 48 ||
        !receipt->startup_bitmap_atlas_ready ||
        receipt->startup_bitmap_atlas_route_count < 4 ||
        (receipt->startup_bitmap_atlas_route_mask & required_mask) !=
            required_mask ||
        receipt->startup_bitmap_atlas_tile_count < 48u ||
        receipt->startup_bitmap_atlas_nonzero_pixel_count == 0u ||
        receipt->startup_bitmap_atlas_checksum == 0u ||
        receipt->startup_bitmap_wide_route_count < 4 ||
        (receipt->startup_bitmap_wide_route_mask & required_mask) !=
            required_mask ||
        receipt->startup_bitmap_wide_atlas_tile_count < 48u ||
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
           receipt->startup_bitmap_title_sample_count >= 12 &&
           receipt->startup_bitmap_stage_sample_count >= 12 &&
           receipt->startup_bitmap_soul_room_sample_count >= 12 &&
           receipt->startup_bitmap_forcefield_sample_count >= 12 &&
           receipt->startup_bitmap_title_nonzero_pixel_count > 0u &&
           receipt->startup_bitmap_stage_nonzero_pixel_count > 0u &&
           receipt->startup_bitmap_soul_room_nonzero_pixel_count > 0u &&
           receipt->startup_bitmap_forcefield_nonzero_pixel_count > 0u &&
           receipt->startup_bitmap_title_checksum != 0u &&
           receipt->startup_bitmap_stage_checksum != 0u &&
           receipt->startup_bitmap_soul_room_checksum != 0u &&
           receipt->startup_bitmap_forcefield_checksum != 0u &&
           receipt->startup_bitmap_title_atlas_tile_count >= 12u &&
           receipt->startup_bitmap_stage_atlas_tile_count >= 12u &&
           receipt->startup_bitmap_soul_room_atlas_tile_count >= 12u &&
           receipt->startup_bitmap_forcefield_atlas_tile_count >= 12u &&
           receipt->startup_bitmap_title_atlas_width >= 96u &&
           receipt->startup_bitmap_stage_atlas_width >= 96u &&
           receipt->startup_bitmap_soul_room_atlas_width >= 96u &&
           receipt->startup_bitmap_forcefield_atlas_width >= 96u;
}

int theron_v1_startup_media_consume_raw_bitmap_route(
    const Theron_StartupMediaStateReceipt *receipt,
    unsigned int route_bit,
    Theron_StartupRawBitmapRouteReceipt *out_receipt) {
    const Theron_Track02StartupBitmapAtlasRoute *route = NULL;
    Theron_Track02Variant variant;
    size_t i;

    if (out_receipt) {
        memset(out_receipt, 0, sizeof(*out_receipt));
    }
    if (!receipt || !out_receipt || route_bit == 0u ||
        !theron_v1_startup_media_state_receipt_has_complete_bitmap_routes(
            receipt)) {
        return 0;
    }
    variant = theron_v1_track02_variant_for_md5(receipt->track02_md5);
    if (variant == THERON_TRACK02_VARIANT_UNKNOWN ||
        receipt->track02_variant != (int)variant ||
        receipt->startup_bitmap_atlas.variant != variant ||
        (receipt->startup_bitmap_raw_route_mask & route_bit) == 0u) {
        return 0;
    }
    for (i = 0u; i < receipt->startup_bitmap_atlas.route_count; ++i) {
        const Theron_Track02StartupBitmapAtlasRoute *candidate =
            &receipt->startup_bitmap_atlas.routes[i];
        if (candidate->route_bit == route_bit) {
            route = candidate;
            break;
        }
    }
    if (!route || route->tile_count == 0u || route->width == 0u ||
        route->width > THERON_TRACK02_STARTUP_BITMAP_ATLAS_MAX_WIDTH ||
        route->height == 0u ||
        route->height > THERON_TRACK02_STARTUP_BITMAP_ATLAS_MAX_HEIGHT ||
        route->nonzero_pixel_count == 0u || route->checksum == 0u ||
        route->first_raw_offset == 0u || route->first_user_data_offset == 0u) {
        return 0;
    }

    out_receipt->variant = variant;
    snprintf(out_receipt->track02_md5, sizeof(out_receipt->track02_md5),
             "%s", receipt->track02_md5);
    out_receipt->route_bit = route->route_bit;
    out_receipt->tile_count = route->tile_count;
    out_receipt->width = route->width;
    out_receipt->height = route->height;
    out_receipt->first_raw_offset = route->first_raw_offset;
    out_receipt->last_raw_offset = route->last_raw_offset;
    out_receipt->first_user_data_offset = route->first_user_data_offset;
    out_receipt->nonzero_pixel_count = route->nonzero_pixel_count;
    out_receipt->checksum = route->checksum;
    memcpy(out_receipt->pixels, route->pixels, sizeof(out_receipt->pixels));
    out_receipt->raw_source_verified = 1;
    /* No observed Track 02 palette write identifies a palette window for
     * these routes.  Do not colorize the real indices with a fallback. */
    out_receipt->palette_binding_verified = 0;
    out_receipt->rgba_output_allowed = 0;
    out_receipt->valid = 1;
    return 1;
}

int theron_v1_startup_media_bind_runtime_palette(
    Theron_V1_World *world,
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex) {

    enum {
        PALETTE_RAW_OFFSET_BIN = 0x2a06a0u,
        PALETTE_RAW_OFFSET_JP_BIN = 0x29fd70u,
        PALETTE_RAW_OFFSET_ISO = 0x249800u
    };
    Theron_Track02PaletteWindowEvidence evidence;
    Theron_Track02Variant variant;
    Theron_Track02SignalStatus status;
    size_t offset;
    size_t i;

    if (!world || !track02_data || !md5_hex || track02_size == 0u) {
        return 0;
    }
    variant = theron_v1_track02_variant_for_md5(md5_hex);
    if (variant == THERON_TRACK02_VARIANT_US_BIN) {
        offset = PALETTE_RAW_OFFSET_BIN;
    } else if (variant == THERON_TRACK02_VARIANT_JP_BIN) {
        /* The authenticated JP BIN has a distinct aligned palette candidate;
         * reusing the US offset lands in a zero/text tail. */
        offset = PALETTE_RAW_OFFSET_JP_BIN;
    } else if (variant == THERON_TRACK02_VARIANT_US_ISO) {
        offset = PALETTE_RAW_OFFSET_ISO;
    } else {
        return 0;
    }
    status = theron_v1_track02_inspect_4bpp_palette_window(
        track02_data, track02_size, md5_hex, offset, &evidence);
    if (status != THERON_TRACK02_SIGNAL_OK || !evidence.format_valid ||
        !evidence.palette.valid) {
        return 0;
    }
    for (i = 0u; i < 16u; ++i) {
        world->runtime_media.startup_palette_rgb8[i][0] =
            evidence.palette.entries[i].red;
        world->runtime_media.startup_palette_rgb8[i][1] =
            evidence.palette.entries[i].green;
        world->runtime_media.startup_palette_rgb8[i][2] =
            evidence.palette.entries[i].blue;
    }
    world->runtime_media.startup_palette_valid = 1;
    return 1;
}

int theron_v1_startup_media_bind_runtime_receipt(
    Theron_V1_World *world,
    const Theron_StartupMediaStateReceipt *receipt) {

    const Theron_Track02StartupBitmapAtlasRoute *routes[4] = {NULL};
    const unsigned int route_bits[4] = {
        THERON_TRACK02_STARTUP_BITMAP_ROUTE_TITLE,
        THERON_TRACK02_STARTUP_BITMAP_ROUTE_STAGE,
        THERON_TRACK02_STARTUP_BITMAP_ROUTE_SOUL_ROOM,
        THERON_TRACK02_STARTUP_BITMAP_ROUTE_FORCEFIELD
    };
    Theron_Track02Variant variant;
    size_t i;
    size_t route_index;

    if (!world || !receipt ||
        !theron_v1_startup_media_state_receipt_has_complete_bitmap_routes(
            receipt)) {
        return 0;
    }
    variant = theron_v1_track02_variant_for_md5(receipt->track02_md5);
    if (variant == THERON_TRACK02_VARIANT_UNKNOWN ||
        receipt->track02_variant != (int)variant ||
        receipt->startup_bitmap_atlas.variant != variant) {
        return 0;
    }
    /* A runtime receipt carries the source-captured indexed atlas forward
     * without decoding or inventing a new image. Raw-route consumption is a
     * stricter rendering API and additionally requires the original byte
     * offsets and MD5; Continue has no raw Track 02 buffer to re-read. */
    for (i = 0u; i < receipt->startup_bitmap_atlas.route_count; ++i) {
        const Theron_Track02StartupBitmapAtlasRoute *candidate =
            &receipt->startup_bitmap_atlas.routes[i];
        for (route_index = 0u; route_index < 4u; ++route_index) {
            if (candidate->route_bit == route_bits[route_index]) {
                routes[route_index] = candidate;
                break;
            }
        }
    }
    for (route_index = 0u; route_index < 4u; ++route_index) {
        const Theron_Track02StartupBitmapAtlasRoute *route =
            routes[route_index];
        if (!route || route->tile_count == 0u || route->width == 0u ||
            route->width > THERON_TRACK02_STARTUP_BITMAP_ATLAS_MAX_WIDTH ||
            route->height == 0u ||
            route->height > THERON_TRACK02_STARTUP_BITMAP_ATLAS_MAX_HEIGHT ||
            route->nonzero_pixel_count == 0u || route->checksum == 0u) {
            return 0;
        }
        if (route->first_raw_offset == 0u ||
            route->first_raw_offset > route->last_raw_offset ||
            route->first_user_data_offset == 0u) {
            return 0;
        }
    }
    theron_v1_world_runtime_media_clear(world);
    if (!theron_v1_world_runtime_media_set_surface(
            world, THERON_RUNTIME_MEDIA_SURFACE_TITLE, receipt->track02_md5,
            routes[0]->route_bit, routes[0]->width, routes[0]->height,
            routes[0]->first_raw_offset, routes[0]->last_raw_offset,
            routes[0]->first_user_data_offset, routes[0]->tile_count,
            routes[0]->nonzero_pixel_count, routes[0]->checksum,
            routes[0]->pixels,
            (size_t)routes[0]->width * (size_t)routes[0]->height) ||
        !theron_v1_world_runtime_media_set_surface(
            world, THERON_RUNTIME_MEDIA_SURFACE_STAGE, receipt->track02_md5,
            routes[1]->route_bit, routes[1]->width, routes[1]->height,
            routes[1]->first_raw_offset, routes[1]->last_raw_offset,
            routes[1]->first_user_data_offset, routes[1]->tile_count,
            routes[1]->nonzero_pixel_count, routes[1]->checksum,
            routes[1]->pixels,
            (size_t)routes[1]->width * (size_t)routes[1]->height) ||
        !theron_v1_world_runtime_media_set_surface(
            world, THERON_RUNTIME_MEDIA_SURFACE_SOUL_ROOM,
            receipt->track02_md5, routes[2]->route_bit, routes[2]->width,
            routes[2]->height, routes[2]->first_raw_offset,
            routes[2]->last_raw_offset, routes[2]->first_user_data_offset,
            routes[2]->tile_count, routes[2]->nonzero_pixel_count,
            routes[2]->checksum, routes[2]->pixels,
            (size_t)routes[2]->width * (size_t)routes[2]->height) ||
        !theron_v1_world_runtime_media_set_surface(
            world, THERON_RUNTIME_MEDIA_SURFACE_FORCEFIELD,
            receipt->track02_md5, routes[3]->route_bit, routes[3]->width,
            routes[3]->height, routes[3]->first_raw_offset,
            routes[3]->last_raw_offset, routes[3]->first_user_data_offset,
            routes[3]->tile_count, routes[3]->nonzero_pixel_count,
            routes[3]->checksum, routes[3]->pixels,
            (size_t)routes[3]->width * (size_t)routes[3]->height) ||
        !theron_v1_world_runtime_media_set_identity(
            world, &receipt->runtime_media_identity)) {
        theron_v1_world_runtime_media_clear(world);
        return 0;
    }
    return 1;
}
