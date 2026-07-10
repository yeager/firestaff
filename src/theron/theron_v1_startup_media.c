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
    size_t min_wide_tiles = 12u;
    uint16_t min_wide_width = 96u;

    if (!media || !route) {
        return;
    }
    switch (route->route_bit) {
    case THERON_TRACK02_STARTUP_BITMAP_ROUTE_TITLE:
        tile_count = &media->startup_bitmap_title_atlas_tile_count;
        width = &media->startup_bitmap_title_atlas_width;
        break;
    case THERON_TRACK02_STARTUP_BITMAP_ROUTE_STAGE:
        tile_count = &media->startup_bitmap_stage_atlas_tile_count;
        width = &media->startup_bitmap_stage_atlas_width;
        break;
    case THERON_TRACK02_STARTUP_BITMAP_ROUTE_SOUL_ROOM:
        tile_count = &media->startup_bitmap_soul_room_atlas_tile_count;
        width = &media->startup_bitmap_soul_room_atlas_width;
        break;
    case THERON_TRACK02_STARTUP_BITMAP_ROUTE_FORCEFIELD:
        tile_count = &media->startup_bitmap_forcefield_atlas_tile_count;
        width = &media->startup_bitmap_forcefield_atlas_width;
        break;
    default:
        return;
    }
    *tile_count = route->tile_count;
    *width = route->width;
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

int theron_v1_startup_media_bind_runtime_receipt(
    Theron_V1_World *world,
    const Theron_StartupMediaStateReceipt *receipt) {

    const Theron_Track02StartupBitmapAtlasRoute *title = NULL;
    const Theron_Track02StartupBitmapAtlasRoute *stage = NULL;
    const Theron_Track02StartupBitmapAtlasRoute *soul_room = NULL;
    const Theron_Track02StartupBitmapAtlasRoute *forcefield = NULL;
    size_t i;

    if (!world || !receipt ||
        !theron_v1_startup_media_state_receipt_has_complete_bitmap_routes(
            receipt)) {
        return 0;
    }
    for (i = 0u; i < receipt->startup_bitmap_atlas.route_count; ++i) {
        const Theron_Track02StartupBitmapAtlasRoute *route =
            &receipt->startup_bitmap_atlas.routes[i];
        if (route->route_bit == THERON_TRACK02_STARTUP_BITMAP_ROUTE_TITLE) {
            title = route;
        } else if (route->route_bit == THERON_TRACK02_STARTUP_BITMAP_ROUTE_STAGE) {
            stage = route;
        } else if (route->route_bit ==
                   THERON_TRACK02_STARTUP_BITMAP_ROUTE_SOUL_ROOM) {
            soul_room = route;
        } else if (route->route_bit ==
                   THERON_TRACK02_STARTUP_BITMAP_ROUTE_FORCEFIELD) {
            forcefield = route;
        }
    }
    theron_v1_world_runtime_media_clear(world);
    if (!title || !stage || !soul_room || !forcefield ||
        !theron_v1_world_runtime_media_set_surface(
            world, THERON_RUNTIME_MEDIA_SURFACE_TITLE, title->route_bit,
            title->width, title->height, title->tile_count,
            title->nonzero_pixel_count, title->checksum, title->pixels,
            (size_t)title->width * (size_t)title->height) ||
        !theron_v1_world_runtime_media_set_surface(
            world, THERON_RUNTIME_MEDIA_SURFACE_STAGE, stage->route_bit,
            stage->width, stage->height, stage->tile_count,
            stage->nonzero_pixel_count, stage->checksum, stage->pixels,
            (size_t)stage->width * (size_t)stage->height) ||
        !theron_v1_world_runtime_media_set_surface(
            world, THERON_RUNTIME_MEDIA_SURFACE_SOUL_ROOM,
            soul_room->route_bit, soul_room->width, soul_room->height,
            soul_room->tile_count, soul_room->nonzero_pixel_count,
            soul_room->checksum, soul_room->pixels,
            (size_t)soul_room->width * (size_t)soul_room->height) ||
        !theron_v1_world_runtime_media_set_surface(
            world, THERON_RUNTIME_MEDIA_SURFACE_FORCEFIELD,
            forcefield->route_bit, forcefield->width, forcefield->height,
            forcefield->tile_count, forcefield->nonzero_pixel_count,
            forcefield->checksum, forcefield->pixels,
            (size_t)forcefield->width * (size_t)forcefield->height) ||
        !theron_v1_world_runtime_media_set_identity(
            world, &receipt->runtime_media_identity)) {
        theron_v1_world_runtime_media_clear(world);
        return 0;
    }
    return 1;
}
