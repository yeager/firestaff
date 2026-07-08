#include "theron_v1_startup_media.h"

#include <stdio.h>
#include <string.h>

void theron_v1_startup_media_init(Theron_StartupMedia *media) {
    if (!media) {
        return;
    }
    memset(media, 0, sizeof(*media));
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

    theron_v1_startup_media_capture_roster(hucard_rom,
                                           hucard_rom_size,
                                           md5_hex,
                                           out_media);
    theron_v1_startup_media_capture_text(hucard_rom,
                                         hucard_rom_size,
                                         md5_hex,
                                         out_media);
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
