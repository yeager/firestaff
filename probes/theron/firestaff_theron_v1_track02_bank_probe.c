/*
 * firestaff_theron_v1_track02_bank_probe.c
 *
 * Theron's Quest V1 -- Track 02 bank evidence probe.
 *
 * This probe does not prove a dungeon map grid or promote runtime loading.
 * It regression-locks hash-gated JP/US Track 02 bank-stride anchors and keeps
 * the JP Rev 1 zero-filled ISO outcome explicit.
 */

#include "asset_status_m12.h"
#include "theron_v1_track02.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#if defined(_WIN32)
#define PATH_SEP "\\"
#else
#define PATH_SEP "/"
#endif

static int g_fail = 0;
static int g_skip = 0;

static const uint8_t g_descriptor[18] = {
    0x20, 0x00, 0x20, 0x04, 0x20, 0x08, 0x20, 0x0c, 0x20, 0x10,
    0x20, 0x14, 0x20, 0x18, 0x20, 0x1c, 0x20, 0x20
};

static const uint8_t g_post_boundary_span[44] = {
    0xbe, 0x80, 0xfe, 0x80, 0x34, 0x81, 0x76, 0x81,
    0xd0, 0x81, 0x2a, 0x80, 0x2b, 0x80, 0x38, 0x80,
    0x45, 0x80, 0x52, 0x80, 0x5f, 0x80, 0x6c, 0x80,
    0x79, 0x80, 0x86, 0x80, 0xa0, 0x80, 0xa5, 0x80,
    0xaa, 0x80, 0xaf, 0x80, 0xb4, 0x80, 0xb9, 0x80,
    0x93, 0x80, 0x00, 0x3f
};

/* 12-byte audio-bank sentinel: `00 ff*10 00`.  Immediately precedes the
 * 4-byte LE audio-bank id word (and therefore the 44-byte post-boundary
 * span) at every audio-bank anchor in raw US/JP Track 02 BINs. */
static const uint8_t g_audio_bank_prefix[12] = {
    0x00,
    0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff,
    0x00
};

static const size_t g_us_bin_descriptor_offsets[THERON_TRACK02_MAX_BANK_ANCHORS] = {
    0x70be06u, 0x70e2c6u, 0x710904u
};

static const size_t g_jp_bin_descriptor_offsets[THERON_TRACK02_MAX_BANK_ANCHORS] = {
    0x70b4d6u, 0x70d996u, 0x70ffd4u
};

static const size_t g_us_bin_span_offsets[THERON_TRACK02_MAX_BANK_ANCHORS] = {
    0x2d53e0u, 0x47d040u, 0x712840u
};

static const size_t g_jp_bin_span_offsets[THERON_TRACK02_MAX_BANK_ANCHORS] = {
    0x2d4ab0u, 0x47c710u, 0x711f10u
};

/* Real per-anchor audio-bank id words, observed locally in the
 * hash-verified raw US/JP Track 02 BINs (see source_evidence()).
 * Each value is a 4-byte little-endian word at offset
 * (span_offsets[i] - 4) in the matching raw BIN.
 *
 * Receipt boundary: these values prove byte-level audio-bank marker
 * presence only.  They do not prove ADPCM decode, CD-DA decode, or runtime
 * playback. */
static const uint32_t g_us_audio_bank_ids[THERON_TRACK02_MAX_BANK_ANCHORS] = {
    0x01725800u, 0x01600801u, 0x01122401u
};

static const uint32_t g_jp_audio_bank_ids[THERON_TRACK02_MAX_BANK_ANCHORS] = {
    0x01530301u, 0x01411301u, 0x01682801u
};

static void check_int(const char *label, int got, int want) {
    if (got != want) {
        printf("FAIL %s: got %d want %d\n", label, got, want);
        ++g_fail;
    }
}

static void check_size(const char *label, size_t got, size_t want) {
    if (got != want) {
        printf("FAIL %s: got %zu want %zu\n", label, got, want);
        ++g_fail;
    }
}

static void check_u16(const char *label, uint16_t got, uint16_t want) {
    if (got != want) {
        printf("FAIL %s: got 0x%04x want 0x%04x\n",
               label, (unsigned)got, (unsigned)want);
        ++g_fail;
    }
}

static void check_u32(const char *label, uint32_t got, uint32_t want) {
    if (got != want) {
        printf("FAIL %s: got 0x%08x want 0x%08x\n",
               label, (unsigned)got, (unsigned)want);
        ++g_fail;
    }
}

static void check_str_eq(const char *got, const char *want, const char *label) {
    if (!got || !want || strcmp(got, want) != 0) {
        printf("FAIL %s: got \"%s\" want \"%s\"\n",
               label,
               got ? got : "(null)",
               want ? want : "(null)");
        ++g_fail;
    }
}

static void check_bytes(const char *label,
                        const uint8_t *got,
                        const uint8_t *want,
                        size_t count) {
    if (!got || !want || memcmp(got, want, count) != 0) {
        printf("FAIL %s: byte span mismatch (%zu bytes)\n", label, count);
        ++g_fail;
    }
}

static int file_exists(const char *path) {
    struct stat st;
    return path && path[0] && stat(path, &st) == 0 && st.st_size > 0;
}

static int read_file(const char *path, uint8_t **out_data, size_t *out_size) {
    FILE *fp;
    long size;
    uint8_t *data;

    if (!path || !out_data || !out_size) return 0;
    fp = fopen(path, "rb");
    if (!fp) return 0;
    if (fseek(fp, 0, SEEK_END) != 0) {
        fclose(fp);
        return 0;
    }
    size = ftell(fp);
    if (size <= 0) {
        fclose(fp);
        return 0;
    }
    if (fseek(fp, 0, SEEK_SET) != 0) {
        fclose(fp);
        return 0;
    }
    data = (uint8_t *)malloc((size_t)size);
    if (!data) {
        fclose(fp);
        return 0;
    }
    if (fread(data, 1, (size_t)size, fp) != (size_t)size) {
        free(data);
        fclose(fp);
        return 0;
    }
    fclose(fp);
    *out_data = data;
    *out_size = (size_t)size;
    return 1;
}

static void check_no_reported_offset(const char *label,
                                     const Theron_Track02BankSignal *signal) {
    check_size(label, signal->descriptor_offset, 0u);
    check_size(label, signal->descriptor_size, 0u);
    check_size(label, signal->post_descriptor_zero_offset, 0u);
    check_size(label, signal->post_descriptor_zero_bytes, 0u);
    check_size(label, signal->next_nonzero_offset, 0u);
    check_size(label, signal->boundary_prefix_size, 0u);
    check_size(label, signal->post_boundary_span_size, 0u);
}

static void default_data_path(const char *relative_name, char out_path[512]) {
    const char *home = getenv("HOME");
    if (!home || !home[0]) home = ".";
    snprintf(out_path, 512, "%s%s.firestaff%sdata%s%s",
             home, PATH_SEP, PATH_SEP, PATH_SEP, relative_name);
}

static void check_iso_signal(const Theron_Track02BankSignal *signal) {
    check_int("variant", signal->variant, THERON_TRACK02_VARIANT_US_ISO);
    check_size("anchor count", signal->anchor_count, 1u);
    check_size("descriptor offset", signal->descriptor_offset, 0x1584u);
    check_size("descriptor offsets[0]", signal->descriptor_offsets[0], 0x1584u);
    check_size("descriptor size", signal->descriptor_size, 18u);
    check_size("descriptor occurrence count", signal->occurrence_count, 1u);
    check_size("descriptor value count", signal->value_count, 9u);
    check_u16("first descriptor value", signal->first_value, 0x0020u);
    check_u16("last descriptor value", signal->last_value, 0x2020u);
    check_u16("descriptor stride", signal->stride, 0x0400u);
    check_size("post descriptor zero offset", signal->post_descriptor_zero_offset, 0x1596u);
    check_size("post descriptor zero bytes", signal->post_descriptor_zero_bytes, 0x1a6au);
    check_size("next nonzero offset", signal->next_nonzero_offset, 0x3000u);
    check_size("boundary prefix size", signal->boundary_prefix_size, 16u);
    check_size("boundary prefix occurrence count",
               signal->boundary_prefix_occurrence_count,
               1u);
    check_size("post-boundary span size", signal->post_boundary_span_size, 44u);
    check_size("post-boundary span offsets[0]",
               signal->post_boundary_span_offsets[0],
               0x3000u);
    check_size("post-boundary span occurrence count",
               signal->post_boundary_span_occurrence_count,
               1u);
    check_u16("post-boundary span first word",
              signal->post_boundary_span_first_word,
              0x80beu);
    check_u16("post-boundary span last word",
              signal->post_boundary_span_last_word,
              0x3f00u);
}

static void check_raw_signal(const char *label,
                             const Theron_Track02BankSignal *signal,
                             Theron_Track02Variant variant,
                             const size_t descriptor_offsets[THERON_TRACK02_MAX_BANK_ANCHORS],
                             const size_t span_offsets[THERON_TRACK02_MAX_BANK_ANCHORS],
                             const uint32_t audio_bank_ids[THERON_TRACK02_MAX_BANK_ANCHORS],
                             size_t first_descriptor_sector,
                             size_t first_span_sector) {
    size_t i;

    check_int("raw variant", signal->variant, variant);
    check_size("raw anchor count",
               signal->anchor_count,
               THERON_TRACK02_MAX_BANK_ANCHORS);
    check_size("raw descriptor size", signal->descriptor_size, 18u);
    check_size("raw descriptor occurrence count",
               signal->occurrence_count,
               THERON_TRACK02_MAX_BANK_ANCHORS);
    check_size("raw descriptor value count", signal->value_count, 9u);
    check_u16("raw first descriptor value", signal->first_value, 0x0020u);
    check_u16("raw last descriptor value", signal->last_value, 0x2020u);
    check_u16("raw descriptor stride", signal->stride, 0x0400u);
    check_size("raw post-boundary span occurrence count",
               signal->post_boundary_span_occurrence_count,
               THERON_TRACK02_MAX_BANK_ANCHORS);
    check_size("raw boundary prefix occurrence count",
               signal->boundary_prefix_occurrence_count,
               THERON_TRACK02_MAX_BANK_ANCHORS);
    check_size("raw sector bytes", signal->raw_sector_bytes, 2352u);
    check_size("raw sector user-data offset",
               signal->raw_sector_user_data_offset,
               0x10u);

    for (i = 0; i < THERON_TRACK02_MAX_BANK_ANCHORS; ++i) {
        char name[96];
        snprintf(name, sizeof(name), "%s descriptor offset[%zu]", label, i);
        check_size(name, signal->descriptor_offsets[i], descriptor_offsets[i]);
        snprintf(name, sizeof(name), "%s span offset[%zu]", label, i);
        check_size(name, signal->post_boundary_span_offsets[i], span_offsets[i]);
        snprintf(name, sizeof(name), "%s audio-bank recognized[%zu]", label, i);
        check_int(name, signal->audio_bank_id_recognized[i], 1);
        snprintf(name, sizeof(name), "%s audio-bank id[%zu]", label, i);
        check_u32(name, signal->audio_bank_id[i], audio_bank_ids[i]);
        snprintf(name, sizeof(name), "%s audio-bank id offset[%zu]", label, i);
        check_size(name, signal->audio_bank_id_offsets[i], span_offsets[i] - 4u);
        snprintf(name, sizeof(name), "%s audio-bank prefix offset[%zu]", label, i);
        check_size(name,
                   signal->audio_bank_prefix_offsets[i],
                   span_offsets[i] - 4u - sizeof(g_audio_bank_prefix));
    }

    check_size("raw descriptor sector[0]",
               signal->descriptor_raw_sector_numbers[0],
               first_descriptor_sector);
    check_size("raw descriptor sector[1]",
               signal->descriptor_raw_sector_numbers[1],
               first_descriptor_sector + 4u);
    check_size("raw descriptor sector[2]",
               signal->descriptor_raw_sector_numbers[2],
               first_descriptor_sector + 8u);
    check_size("raw descriptor user offset[0]",
               signal->descriptor_raw_sector_user_offsets[0],
               0x406u);
    check_size("raw descriptor user offset[1]",
               signal->descriptor_raw_sector_user_offsets[1],
               0x406u);
    check_size("raw descriptor user offset[2]",
               signal->descriptor_raw_sector_user_offsets[2],
               0x584u);
    check_size("raw span sector[0]",
               signal->post_boundary_span_raw_sector_numbers[0],
               first_span_sector);
    check_size("raw span sector[1]",
               signal->post_boundary_span_raw_sector_numbers[1],
               first_span_sector + 738u);
    check_size("raw span sector[2]",
               signal->post_boundary_span_raw_sector_numbers[2],
               first_descriptor_sector + 12u);
    check_size("raw span user offset[0]",
               signal->post_boundary_span_raw_sector_user_offsets[0],
               0u);
    check_size("raw span user offset[1]",
               signal->post_boundary_span_raw_sector_user_offsets[1],
               0u);
    check_size("raw span user offset[2]",
               signal->post_boundary_span_raw_sector_user_offsets[2],
               0u);
}

static void check_raw_user_data_contract(
    const char *label,
    const uint8_t *data,
    size_t size,
    const char *md5_hex,
    const size_t descriptor_offsets[THERON_TRACK02_MAX_BANK_ANCHORS],
    const size_t span_offsets[THERON_TRACK02_MAX_BANK_ANCHORS],
    const uint32_t audio_bank_ids[THERON_TRACK02_MAX_BANK_ANCHORS]) {

    size_t sector_count = 0u;
    size_t user_data_size = 0u;
    size_t copied_size = 0u;
    uint8_t *user_data = NULL;
    Theron_Track02SignalStatus status;
    size_t i;

    status = theron_v1_track02_raw_user_data_size(size,
                                                  md5_hex,
                                                  &sector_count,
                                                  &user_data_size);
    check_int("raw user-data size status",
              status,
              THERON_TRACK02_SIGNAL_OK);
    check_size("raw user-data sector count",
               sector_count,
               size / THERON_TRACK02_RAW_SECTOR_BYTES);
    check_size("raw user-data byte count",
               user_data_size,
               sector_count * THERON_TRACK02_RAW_USER_DATA_BYTES);

    user_data = (uint8_t *)malloc(user_data_size);
    if (!user_data) {
        printf("FAIL %s raw user-data allocation\n", label);
        ++g_fail;
        return;
    }
    status = theron_v1_track02_copy_raw_user_data(data,
                                                  size,
                                                  md5_hex,
                                                  user_data,
                                                  user_data_size,
                                                  &copied_size);
    check_int("raw user-data copy status",
              status,
              THERON_TRACK02_SIGNAL_OK);
    check_size("raw user-data copied byte count",
               copied_size,
               user_data_size);

    for (i = 0; i < THERON_TRACK02_MAX_BANK_ANCHORS; ++i) {
        char name[128];
        size_t user_offset = 0u;
        const size_t id_offset = span_offsets[i] - 4u;

        status = theron_v1_track02_raw_offset_to_user_offset(
            descriptor_offsets[i],
            size,
            md5_hex,
            &user_offset);
        snprintf(name, sizeof(name), "%s descriptor raw->user status[%zu]",
                 label, i);
        check_int(name, status, THERON_TRACK02_SIGNAL_OK);
        snprintf(name, sizeof(name), "%s descriptor raw->user bytes[%zu]",
                 label, i);
        check_bytes(name,
                    user_data + user_offset,
                    data + descriptor_offsets[i],
                    sizeof(g_descriptor));

        status = theron_v1_track02_raw_offset_to_user_offset(
            span_offsets[i],
            size,
            md5_hex,
            &user_offset);
        snprintf(name, sizeof(name), "%s span raw->user status[%zu]",
                 label, i);
        check_int(name, status, THERON_TRACK02_SIGNAL_OK);
        snprintf(name, sizeof(name), "%s span raw->user bytes[%zu]",
                 label, i);
        check_bytes(name,
                    user_data + user_offset,
                    data + span_offsets[i],
                    sizeof(g_post_boundary_span));

        status = theron_v1_track02_raw_offset_to_user_offset(
            id_offset,
            size,
            md5_hex,
            &user_offset);
        snprintf(name, sizeof(name), "%s audio id outside user-data[%zu]",
                 label, i);
        check_int(name, status, THERON_TRACK02_SIGNAL_NOT_FOUND);
        (void)audio_bank_ids;
    }

    {
        Theron_Track02UserDataWindowCatalog catalog;
        size_t descriptor_roles = 0u;
        size_t span_roles = 0u;
        size_t initial_roles = 0u;

        status = theron_v1_track02_catalog_user_data_windows(data,
                                                             size,
                                                             md5_hex,
                                                             &catalog);
        check_int("raw user-data window catalog status",
                  status,
                  THERON_TRACK02_SIGNAL_OK);
        check_size("raw user-data window catalog entries",
                   catalog.entry_count,
                   7u);
        check_size("raw user-data window catalog overflow",
                   catalog.overflow_count,
                   0u);
        for (i = 0; i < catalog.entry_count; ++i) {
            const Theron_Track02UserDataWindow *entry = &catalog.entries[i];
            char name[128];
            size_t expected_user_offset = 0u;

            status = theron_v1_track02_raw_offset_to_user_offset(
                entry->raw_offset,
                size,
                md5_hex,
                &expected_user_offset);
            snprintf(name, sizeof(name), "%s catalog entry user offset[%zu]",
                     label, i);
            check_int(name, status, THERON_TRACK02_SIGNAL_OK);
            snprintf(name, sizeof(name), "%s catalog entry offset match[%zu]",
                     label, i);
            check_size(name, entry->user_data_offset, expected_user_offset);

            if (entry->role ==
                THERON_TRACK02_USER_DATA_WINDOW_BANK_DESCRIPTOR_TABLE) {
                ++descriptor_roles;
                check_size("raw user-data catalog descriptor bytes",
                           entry->byte_count,
                           sizeof(g_descriptor));
            } else if (entry->role ==
                       THERON_TRACK02_USER_DATA_WINDOW_POST_BOUNDARY_SPAN) {
                ++span_roles;
                check_size("raw user-data catalog span bytes",
                           entry->byte_count,
                           sizeof(g_post_boundary_span));
            } else if (entry->role ==
                       THERON_TRACK02_USER_DATA_WINDOW_INITIAL_LEVEL_CANDIDATE) {
                ++initial_roles;
                check_size("raw user-data catalog initial candidate bytes",
                           entry->byte_count,
                           12u + 32u * 27u);
            } else {
                printf("FAIL %s catalog unexpected role[%zu]=%s\n",
                       label,
                       i,
                       theron_v1_track02_user_data_window_role_name(entry->role));
                ++g_fail;
            }
        }
        check_size("raw user-data catalog descriptor role count",
                   descriptor_roles,
                   THERON_TRACK02_MAX_BANK_ANCHORS);
        check_size("raw user-data catalog span role count",
                   span_roles,
                   THERON_TRACK02_MAX_BANK_ANCHORS);
        check_size("raw user-data catalog initial role count",
                   initial_roles,
                   1u);

        {
            uint8_t role_copy[12u + 32u * 27u];
            Theron_Track02UserDataWindow window;
            size_t role_copied = 0u;

            status = theron_v1_track02_copy_user_data_window_by_role(
                data,
                size,
                md5_hex,
                THERON_TRACK02_USER_DATA_WINDOW_BANK_DESCRIPTOR_TABLE,
                0u,
                role_copy,
                sizeof(role_copy),
                &role_copied,
                &window);
            check_int("raw user-data role descriptor copy status",
                      status,
                      THERON_TRACK02_SIGNAL_OK);
            check_size("raw user-data role descriptor copy bytes",
                       role_copied,
                       sizeof(g_descriptor));
            check_size("raw user-data role descriptor raw offset",
                       window.raw_offset,
                       descriptor_offsets[0]);
            check_bytes("raw user-data role descriptor copy payload",
                        role_copy,
                        g_descriptor,
                        sizeof(g_descriptor));

            status = theron_v1_track02_copy_user_data_window_by_role(
                data,
                size,
                md5_hex,
                THERON_TRACK02_USER_DATA_WINDOW_POST_BOUNDARY_SPAN,
                0u,
                role_copy,
                sizeof(role_copy),
                &role_copied,
                &window);
            check_int("raw user-data role span copy status",
                      status,
                      THERON_TRACK02_SIGNAL_OK);
            check_size("raw user-data role span copy bytes",
                       role_copied,
                       sizeof(g_post_boundary_span));
            check_size("raw user-data role span raw offset",
                       window.raw_offset,
                       span_offsets[0]);
            check_bytes("raw user-data role span copy payload",
                        role_copy,
                        g_post_boundary_span,
                        sizeof(g_post_boundary_span));

            status = theron_v1_track02_copy_user_data_window_by_role(
                data,
                size,
                md5_hex,
                THERON_TRACK02_USER_DATA_WINDOW_INITIAL_LEVEL_CANDIDATE,
                0u,
                role_copy,
                sizeof(role_copy),
                &role_copied,
                &window);
            check_int("raw user-data role initial copy status",
                      status,
                      THERON_TRACK02_SIGNAL_OK);
            check_size("raw user-data role initial copy bytes",
                       role_copied,
                       12u + 32u * 27u);
            check_int("raw user-data role initial width hi", role_copy[0], 0);
            check_int("raw user-data role initial width lo", role_copy[1], 32);
            check_int("raw user-data role initial height hi", role_copy[2], 0);
            check_int("raw user-data role initial height lo", role_copy[3], 27);
            check_int("raw user-data role initial seed marker",
                      role_copy[4],
                      1);
            check_int("raw user-data role initial level marker",
                      role_copy[8],
                      0);
            check_int("raw user-data role initial level value",
                      role_copy[9],
                      38);

            status = theron_v1_track02_copy_user_data_window_by_role(
                data,
                size,
                md5_hex,
                THERON_TRACK02_USER_DATA_WINDOW_BANK_DESCRIPTOR_TABLE,
                THERON_TRACK02_MAX_BANK_ANCHORS,
                role_copy,
                sizeof(role_copy),
                &role_copied,
                &window);
            check_int("raw user-data role missing occurrence",
                      status,
                      THERON_TRACK02_SIGNAL_NOT_FOUND);

            status = theron_v1_track02_copy_user_data_window_by_role(
                data,
                size,
                md5_hex,
                THERON_TRACK02_USER_DATA_WINDOW_INITIAL_LEVEL_CANDIDATE,
                0u,
                role_copy,
                16u,
                &role_copied,
                &window);
            check_int("raw user-data role capacity guard",
                      status,
                      THERON_TRACK02_SIGNAL_BAD_INPUT);
        }
    }

    {
        Theron_Track02StartupTextMarkerCatalog text_catalog;
        Theron_Track02StartupTextMarkerKind expected_kind;
        const char *expected_text;
        size_t expected_text_size;
        size_t expected_first_raw_offset;

        status = theron_v1_track02_catalog_startup_text_markers(
            data,
            size,
            md5_hex,
            &text_catalog);
        check_int("raw startup text marker catalog status",
                  status,
                  THERON_TRACK02_SIGNAL_OK);
        check_size("raw startup text marker count",
                   text_catalog.marker_count,
                   7u);
        check_size("raw startup text marker overflow",
                   text_catalog.overflow_count,
                   0u);

        if (strcmp(md5_hex, THERON_TRACK02_MD5_US_BIN) == 0) {
            expected_kind =
                THERON_TRACK02_STARTUP_TEXT_US_RESURRECT_THERON_PROMPT;
            expected_text = "GO AWAY AND RESURRECT THERON";
            expected_text_size = strlen(expected_text);
            expected_first_raw_offset = 0xa0722u;
        } else {
            expected_kind =
                THERON_TRACK02_STARTUP_TEXT_JP_CHAMPION_ROSTER_CLUSTER;
            expected_text = "THERON";
            expected_text_size = strlen(expected_text);
            expected_first_raw_offset = 0xb3d98u;
        }

        for (i = 0; i < text_catalog.marker_count; ++i) {
            const Theron_Track02StartupTextMarker *marker =
                &text_catalog.markers[i];
            char name[128];
            size_t expected_user_offset = 0u;

            snprintf(name, sizeof(name), "%s text marker kind[%zu]",
                     label, i);
            check_int(name, marker->kind, expected_kind);
            snprintf(name, sizeof(name), "%s text marker occurrence[%zu]",
                     label, i);
            check_size(name, marker->occurrence_index, i);
            snprintf(name, sizeof(name), "%s text marker raw->user[%zu]",
                     label, i);
            status = theron_v1_track02_raw_offset_to_user_offset(
                marker->raw_offset,
                size,
                md5_hex,
                &expected_user_offset);
            check_int(name, status, THERON_TRACK02_SIGNAL_OK);
            snprintf(name, sizeof(name), "%s text marker user offset[%zu]",
                     label, i);
            check_size(name, marker->user_data_offset, expected_user_offset);
            snprintf(name, sizeof(name), "%s text marker payload[%zu]",
                     label, i);
            check_bytes(name,
                        data + marker->raw_offset,
                        (const uint8_t *)expected_text,
                        expected_text_size);
        }
        if (text_catalog.marker_count > 0u) {
            check_size("raw startup text marker first raw offset",
                       text_catalog.markers[0].raw_offset,
                       expected_first_raw_offset);
        }
    }

    {
        Theron_Track02StartupRosterNameCatalog roster_catalog;
        status = theron_v1_track02_catalog_startup_roster_names(
            data,
            size,
            md5_hex,
            &roster_catalog);
        if (strcmp(md5_hex, THERON_TRACK02_MD5_JP_BIN) == 0) {
            static const char *expected_names[] = {
                "THERON", "MARA", "LINOS", "HEXA", "HAKAR", "TIRAN", "DOTAN"
            };
            static const size_t expected_raw_offsets[] = {
                0xb3d98u, 0xb3dd1u, 0xb3e1au, 0xb3e5eu,
                0xb3ea3u, 0xb3ee4u, 0xb3f2eu
            };
            check_int("JP startup roster name catalog status",
                      status,
                      THERON_TRACK02_SIGNAL_OK);
            check_size("JP startup roster name count",
                       roster_catalog.name_count,
                       sizeof(expected_names) / sizeof(expected_names[0]));
            check_size("JP startup roster name overflow",
                       roster_catalog.overflow_count,
                       0u);
            for (i = 0u;
                 i < sizeof(expected_names) / sizeof(expected_names[0]);
                 ++i) {
                char name[128];
                size_t expected_user_offset = 0u;

                snprintf(name, sizeof(name), "JP roster name[%zu]", i);
                check_str_eq(roster_catalog.names[i].name,
                             expected_names[i],
                             name);
                snprintf(name, sizeof(name), "JP roster raw offset[%zu]", i);
                check_size(name,
                           roster_catalog.names[i].raw_offset,
                           expected_raw_offsets[i]);
                status = theron_v1_track02_raw_offset_to_user_offset(
                    expected_raw_offsets[i],
                    size,
                    md5_hex,
                    &expected_user_offset);
                snprintf(name, sizeof(name), "JP roster raw->user[%zu]", i);
                check_int(name, status, THERON_TRACK02_SIGNAL_OK);
                snprintf(name, sizeof(name), "JP roster user offset[%zu]", i);
                check_size(name,
                           roster_catalog.names[i].user_data_offset,
                           expected_user_offset);
            }
        } else if (strcmp(md5_hex, THERON_TRACK02_MD5_US_BIN) == 0) {
            check_int("US startup roster name catalog unsupported",
                      status,
                      THERON_TRACK02_SIGNAL_UNSUPPORTED_VARIANT);
        }
    }

    free(user_data);
}

static void probe_track(const char *label,
                        const char *env_name,
                        const char *default_file,
                        const char *expected_md5,
                        Theron_Track02SignalStatus expected_status) {
    char path[512];
    const char *env_path = getenv(env_name);
    const char *path_to_read;
    uint8_t *data = NULL;
    size_t size = 0;
    char md5_hex[33] = {0};
    Theron_Track02BankSignal signal;
    Theron_Track02SignalStatus status;

    if (env_path && env_path[0]) {
        path_to_read = env_path;
    } else {
        default_data_path(default_file, path);
        path_to_read = path;
    }

    if (!file_exists(path_to_read)) {
        printf("SKIP %s: no Track 02 image at %s\n", label, path_to_read);
        ++g_skip;
        return;
    }
    if (!m12_file_md5_hex(path_to_read, md5_hex)) {
        printf("FAIL %s: could not compute MD5 for %s\n", label, path_to_read);
        ++g_fail;
        return;
    }
    if (strcmp(md5_hex, expected_md5) != 0) {
        printf("FAIL %s: MD5 %s does not match expected %s\n",
               label, md5_hex, expected_md5);
        ++g_fail;
        return;
    }
    if (!read_file(path_to_read, &data, &size)) {
        printf("FAIL %s: could not read %s\n", label, path_to_read);
        ++g_fail;
        return;
    }

    status = theron_v1_track02_find_bank_signal(data, size, md5_hex, &signal);
    printf("%s: md5=%s size=%zu status=%s variant=%s\n",
           label, md5_hex, size,
           theron_v1_track02_signal_status_name(status),
           theron_v1_track02_variant_name(signal.variant));

    check_int("track status", status, expected_status);
    if (expected_status == THERON_TRACK02_SIGNAL_OK) {
        printf("%s: anchors=%zu descriptor=0x%zx bytes=%zu boundary=0x%zx zero_bytes=%zu span_bytes=%zu span_occurrences=%zu\n",
               label,
               signal.anchor_count,
               signal.descriptor_offset,
               signal.descriptor_size,
               signal.next_nonzero_offset,
               signal.post_descriptor_zero_bytes,
               signal.post_boundary_span_size,
               signal.post_boundary_span_occurrence_count);
        if (signal.variant == THERON_TRACK02_VARIANT_US_ISO) {
            check_iso_signal(&signal);
        } else if (signal.variant == THERON_TRACK02_VARIANT_US_BIN) {
            check_raw_signal("US raw",
                             &signal,
                             THERON_TRACK02_VARIANT_US_BIN,
                             g_us_bin_descriptor_offsets,
                             g_us_bin_span_offsets,
                             g_us_audio_bank_ids,
                             3141u,
                             1263u);
            check_raw_user_data_contract("US raw",
                                         data,
                                         size,
                                         md5_hex,
                                         g_us_bin_descriptor_offsets,
                                         g_us_bin_span_offsets,
                                         g_us_audio_bank_ids);
        } else if (signal.variant == THERON_TRACK02_VARIANT_JP_BIN) {
            check_raw_signal("JP raw",
                             &signal,
                             THERON_TRACK02_VARIANT_JP_BIN,
                             g_jp_bin_descriptor_offsets,
                             g_jp_bin_span_offsets,
                             g_jp_audio_bank_ids,
                             3140u,
                             1262u);
            check_raw_user_data_contract("JP raw",
                                         data,
                                         size,
                                         md5_hex,
                                         g_jp_bin_descriptor_offsets,
                                         g_jp_bin_span_offsets,
                                         g_jp_audio_bank_ids);
        } else {
            printf("FAIL %s: unexpected OK variant %s\n",
                   label,
                   theron_v1_track02_variant_name(signal.variant));
            ++g_fail;
        }
    } else if (expected_status == THERON_TRACK02_SIGNAL_INSUFFICIENT_ZERO_IMAGE) {
        check_int("zero-image variant", signal.variant, THERON_TRACK02_VARIANT_JP_REV1_ISO);
        check_no_reported_offset("zero-image leaves no offset", &signal);
    }

    free(data);
}

static void probe_negative_fixture(void) {
    uint8_t zeros[64] = {0};
    Theron_Track02BankSignal signal;
    Theron_Track02SignalStatus status =
        theron_v1_track02_find_bank_signal(zeros,
                                           sizeof(zeros),
                                           "00000000000000000000000000000000",
                                           &signal);
    check_int("unsupported fixture stays unsupported",
              status,
              THERON_TRACK02_SIGNAL_UNSUPPORTED_VARIANT);
}

static void probe_jp_zero_image_fixture(void) {
    uint8_t zeros[4096] = {0};
    Theron_Track02BankSignal signal;
    Theron_Track02SignalStatus status =
        theron_v1_track02_find_bank_signal(zeros,
                                           sizeof(zeros),
                                           THERON_TRACK02_MD5_JP_REV1_ISO,
                                           &signal);
    check_int("JP Rev 1 zero fixture is insufficient evidence",
              status,
              THERON_TRACK02_SIGNAL_INSUFFICIENT_ZERO_IMAGE);
    check_int("JP Rev 1 zero fixture is hash-gated",
              signal.variant,
              THERON_TRACK02_VARIANT_JP_REV1_ISO);
    check_no_reported_offset("JP Rev 1 zero fixture leaves no offset", &signal);
}

static void probe_descriptor_only_negative_fixture(void) {
    uint8_t *fixture = (uint8_t *)calloc(1u, 0x3010u);
    Theron_Track02BankSignal signal;
    Theron_Track02SignalStatus status;

    if (!fixture) {
        printf("FAIL descriptor-only fixture: allocation failed\n");
        ++g_fail;
        return;
    }

    memcpy(fixture + 0x1584u, g_descriptor, sizeof(g_descriptor));
    status = theron_v1_track02_find_bank_signal(fixture,
                                                0x3010u,
                                                THERON_TRACK02_MD5_US_ISO,
                                                &signal);
    check_int("descriptor-only fixture needs boundary prefix",
              status,
              THERON_TRACK02_SIGNAL_NOT_FOUND);
    free(fixture);
}

static void probe_boundary_prefix_only_negative_fixture(void) {
    static const uint8_t prefix[16] = {
        0xbe, 0x80, 0xfe, 0x80, 0x34, 0x81, 0x76, 0x81,
        0xd0, 0x81, 0x2a, 0x80, 0x2b, 0x80, 0x38, 0x80
    };
    uint8_t *fixture = (uint8_t *)calloc(1u, 0x302cu);
    Theron_Track02BankSignal signal;
    Theron_Track02SignalStatus status;

    if (!fixture) {
        printf("FAIL boundary-prefix-only fixture: allocation failed\n");
        ++g_fail;
        return;
    }

    memcpy(fixture + 0x1584u, g_descriptor, sizeof(g_descriptor));
    memcpy(fixture + 0x3000u, prefix, sizeof(prefix));
    status = theron_v1_track02_find_bank_signal(fixture,
                                                0x302cu,
                                                THERON_TRACK02_MD5_US_ISO,
                                                &signal);
    check_int("boundary prefix alone is not a post-boundary span",
              status,
              THERON_TRACK02_SIGNAL_NOT_FOUND);
    free(fixture);
}

static void probe_raw_user_data_synthetic_fixture(void) {
    enum {
        sector_count = 2,
        raw_size = sector_count * THERON_TRACK02_RAW_SECTOR_BYTES,
        user_size = sector_count * THERON_TRACK02_RAW_USER_DATA_BYTES
    };
    uint8_t raw[raw_size];
    uint8_t user[user_size];
    uint8_t range[80];
    size_t sectors = 0u;
    size_t bytes = 0u;
    size_t copied = 0u;
    size_t user_offset = 0u;
    Theron_Track02SignalStatus status;
    size_t i;

    memset(raw, 0xee, sizeof(raw));
    memset(user, 0, sizeof(user));
    for (i = 0u; i < THERON_TRACK02_RAW_USER_DATA_BYTES; ++i) {
        raw[THERON_TRACK02_RAW_USER_DATA_OFFSET + i] =
            (uint8_t)(i & 0xffu);
        raw[THERON_TRACK02_RAW_SECTOR_BYTES +
            THERON_TRACK02_RAW_USER_DATA_OFFSET + i] =
            (uint8_t)((i + 17u) & 0xffu);
    }

    status = theron_v1_track02_raw_user_data_size(sizeof(raw),
                                                  THERON_TRACK02_MD5_US_BIN,
                                                  &sectors,
                                                  &bytes);
    check_int("synthetic raw user-data size status",
              status,
              THERON_TRACK02_SIGNAL_OK);
    check_size("synthetic raw sector count", sectors, sector_count);
    check_size("synthetic raw user bytes", bytes, user_size);

    status = theron_v1_track02_copy_raw_user_data(raw,
                                                  sizeof(raw),
                                                  THERON_TRACK02_MD5_US_BIN,
                                                  user,
                                                  sizeof(user),
                                                  &copied);
    check_int("synthetic raw user-data copy status",
              status,
              THERON_TRACK02_SIGNAL_OK);
    check_size("synthetic raw copied bytes", copied, user_size);
    check_int("synthetic sector 0 first user byte", user[0], 0);
    check_int("synthetic sector 0 last user byte",
              user[THERON_TRACK02_RAW_USER_DATA_BYTES - 1u],
              0xff);
    check_int("synthetic sector 1 first user byte",
              user[THERON_TRACK02_RAW_USER_DATA_BYTES],
              17);

    status = theron_v1_track02_copy_raw_user_data_range(
        raw,
        sizeof(raw),
        THERON_TRACK02_MD5_US_BIN,
        THERON_TRACK02_RAW_USER_DATA_OFFSET + 33u,
        16u,
        range,
        sizeof(range),
        &user_offset);
    check_int("synthetic raw user-data range copy status",
              status,
              THERON_TRACK02_SIGNAL_OK);
    check_size("synthetic raw user-data range copy offset",
               user_offset,
               33u);
    check_int("synthetic raw user-data range first byte", range[0], 33);
    check_int("synthetic raw user-data range last byte", range[15], 48);

    status = theron_v1_track02_copy_raw_user_data_range(
        raw,
        sizeof(raw),
        THERON_TRACK02_MD5_US_BIN,
        THERON_TRACK02_RAW_USER_DATA_OFFSET +
            THERON_TRACK02_RAW_USER_DATA_BYTES - 8u,
        24u,
        range,
        sizeof(range),
        &user_offset);
    check_int("synthetic raw user-data cross-sector copy status",
              status,
              THERON_TRACK02_SIGNAL_OK);
    check_size("synthetic raw user-data cross-sector offset",
               user_offset,
               THERON_TRACK02_RAW_USER_DATA_BYTES - 8u);
    check_int("synthetic raw user-data cross-sector first byte",
              range[0],
              0xf8);
    check_int("synthetic raw user-data cross-sector sector1 first byte",
              range[8],
              17);

    status = theron_v1_track02_copy_raw_user_data_range(
        raw,
        sizeof(raw),
        THERON_TRACK02_MD5_US_BIN,
        THERON_TRACK02_RAW_USER_DATA_OFFSET + 33u,
        sizeof(range) + 1u,
        range,
        sizeof(range),
        &user_offset);
    check_int("synthetic raw user-data range capacity guard",
              status,
              THERON_TRACK02_SIGNAL_BAD_INPUT);

    status = theron_v1_track02_copy_raw_user_data_range(
        raw,
        sizeof(raw),
        THERON_TRACK02_MD5_US_BIN,
        0u,
        4u,
        range,
        sizeof(range),
        &user_offset);
    check_int("synthetic raw user-data range header rejected",
              status,
              THERON_TRACK02_SIGNAL_NOT_FOUND);

    status = theron_v1_track02_raw_offset_to_user_offset(
        THERON_TRACK02_RAW_USER_DATA_OFFSET + 33u,
        sizeof(raw),
        THERON_TRACK02_MD5_US_BIN,
        &user_offset);
    check_int("synthetic raw offset maps to user-data",
              status,
              THERON_TRACK02_SIGNAL_OK);
    check_size("synthetic raw offset user index", user_offset, 33u);

    status = theron_v1_track02_raw_offset_to_user_offset(
        THERON_TRACK02_RAW_SECTOR_BYTES +
            THERON_TRACK02_RAW_USER_DATA_OFFSET + 5u,
        sizeof(raw),
        THERON_TRACK02_MD5_US_BIN,
        &user_offset);
    check_int("synthetic second-sector raw offset maps",
              status,
              THERON_TRACK02_SIGNAL_OK);
    check_size("synthetic second-sector user index",
               user_offset,
               THERON_TRACK02_RAW_USER_DATA_BYTES + 5u);

    status = theron_v1_track02_raw_offset_to_user_offset(
        0u,
        sizeof(raw),
        THERON_TRACK02_MD5_US_BIN,
        &user_offset);
    check_int("synthetic raw sector header offset rejected",
              status,
              THERON_TRACK02_SIGNAL_NOT_FOUND);

    status = theron_v1_track02_raw_offset_to_user_offset(
        THERON_TRACK02_RAW_USER_DATA_OFFSET +
            THERON_TRACK02_RAW_USER_DATA_BYTES,
        sizeof(raw),
        THERON_TRACK02_MD5_US_BIN,
        &user_offset);
    check_int("synthetic raw EDC/ECC offset rejected",
              status,
              THERON_TRACK02_SIGNAL_NOT_FOUND);

    status = theron_v1_track02_raw_user_data_size(sizeof(raw),
                                                  THERON_TRACK02_MD5_US_ISO,
                                                  &sectors,
                                                  &bytes);
    check_int("synthetic ISO user-data strip unsupported",
              status,
              THERON_TRACK02_SIGNAL_UNSUPPORTED_VARIANT);

    status = theron_v1_track02_raw_user_data_size(sizeof(raw) - 1u,
                                                  THERON_TRACK02_MD5_US_BIN,
                                                  &sectors,
                                                  &bytes);
    check_int("synthetic partial raw sector rejected",
              status,
              THERON_TRACK02_SIGNAL_NOT_FOUND);
}

static void fill_raw_anchor_fixture(uint8_t *fixture,
                                    const size_t descriptor_offsets[THERON_TRACK02_MAX_BANK_ANCHORS],
                                    const size_t span_offsets[THERON_TRACK02_MAX_BANK_ANCHORS],
                                    const uint32_t audio_bank_ids[THERON_TRACK02_MAX_BANK_ANCHORS]) {
    size_t i;
    for (i = 0; i < THERON_TRACK02_MAX_BANK_ANCHORS; ++i) {
        const size_t span_offset = span_offsets[i];
        const size_t id_offset = span_offset - 4u;
        const size_t prefix_offset = id_offset - sizeof(g_audio_bank_prefix);

        memcpy(fixture + descriptor_offsets[i], g_descriptor, sizeof(g_descriptor));
        memcpy(fixture + prefix_offset,
               g_audio_bank_prefix,
               sizeof(g_audio_bank_prefix));
        fixture[id_offset + 0] = (uint8_t)(audio_bank_ids[i] & 0xFFu);
        fixture[id_offset + 1] = (uint8_t)((audio_bank_ids[i] >> 8) & 0xFFu);
        fixture[id_offset + 2] = (uint8_t)((audio_bank_ids[i] >> 16) & 0xFFu);
        fixture[id_offset + 3] = (uint8_t)((audio_bank_ids[i] >> 24) & 0xFFu);
        memcpy(fixture + span_offset,
               g_post_boundary_span,
               sizeof(g_post_boundary_span));
    }
}

/* Backwards-compatible overload: zero audio-bank ids. */
static void fill_raw_anchor_fixture_no_audio_bank(uint8_t *fixture,
                                                  const size_t descriptor_offsets[THERON_TRACK02_MAX_BANK_ANCHORS],
                                                  const size_t span_offsets[THERON_TRACK02_MAX_BANK_ANCHORS]) {
    uint32_t zeros[THERON_TRACK02_MAX_BANK_ANCHORS] = {0u, 0u, 0u};
    fill_raw_anchor_fixture(fixture, descriptor_offsets, span_offsets, zeros);
}

static void probe_raw_bin_positive_fixture(const char *label,
                                           const char *md5_hex,
                                           Theron_Track02Variant variant,
                                           const size_t descriptor_offsets[THERON_TRACK02_MAX_BANK_ANCHORS],
                                           const size_t span_offsets[THERON_TRACK02_MAX_BANK_ANCHORS],
                                           const uint32_t audio_bank_ids[THERON_TRACK02_MAX_BANK_ANCHORS],
                                           size_t first_descriptor_sector,
                                           size_t first_span_sector) {
    const size_t fixture_size = span_offsets[2] + sizeof(g_post_boundary_span) + 1u;
    uint8_t *fixture = (uint8_t *)calloc(1u, fixture_size);
    Theron_Track02BankSignal signal;
    Theron_Track02SignalStatus status;

    if (!fixture) {
        printf("FAIL %s: allocation failed\n", label);
        ++g_fail;
        return;
    }

    fill_raw_anchor_fixture(fixture,
                            descriptor_offsets,
                            span_offsets,
                            audio_bank_ids);
    status = theron_v1_track02_find_bank_signal(fixture,
                                                fixture_size,
                                                md5_hex,
                                                &signal);
    check_int(label, status, THERON_TRACK02_SIGNAL_OK);
    check_raw_signal(label,
                     &signal,
                     variant,
                     descriptor_offsets,
                     span_offsets,
                     audio_bank_ids,
                     first_descriptor_sector,
                     first_span_sector);
    free(fixture);
}

static void probe_raw_bin_missing_anchor_fixture(void) {
    const size_t fixture_size = g_us_bin_span_offsets[2] + sizeof(g_post_boundary_span) + 1u;
    uint8_t *fixture = (uint8_t *)calloc(1u, fixture_size);
    Theron_Track02BankSignal signal;
    Theron_Track02SignalStatus status;

    if (!fixture) {
        printf("FAIL raw missing-anchor fixture: allocation failed\n");
        ++g_fail;
        return;
    }

    fill_raw_anchor_fixture_no_audio_bank(fixture,
                                          g_us_bin_descriptor_offsets,
                                          g_us_bin_span_offsets);
    fixture[g_us_bin_descriptor_offsets[1]] ^= 0x01u;
    status = theron_v1_track02_find_bank_signal(fixture,
                                                fixture_size,
                                                THERON_TRACK02_MD5_US_BIN,
                                                &signal);
    check_int("raw BIN requires all three descriptor anchors",
              status,
              THERON_TRACK02_SIGNAL_NOT_FOUND);
    free(fixture);
}

/* Probe the one-shot audio-bank marker helper on a synthetic fixture.
 *
 * The helper reads offsets from the variant-fixed table (g_us_bin_post_*
 * or g_jp_bin_post_*), so the synthetic fixture must place data at those
 * same offsets.  To keep the fixture bounded we only place data at the
 * lowest anchor (anchor 0); the rest of the buffer stays zero (which
 * will fail the helper's prefix check). */
static void probe_audio_bank_marker_synthetic_fixture(void) {
    static const size_t us_span_offsets[THERON_TRACK02_MAX_BANK_ANCHORS] = {
        0x2d53e0u, 0x47d040u, 0x712840u
    };
    static const size_t jp_span_offsets[THERON_TRACK02_MAX_BANK_ANCHORS] = {
        0x2d4ab0u, 0x47c710u, 0x711f10u
    };
    static const uint32_t synth_id_jp_anchor0 = 0x12345678u;
    const size_t fixture_size = jp_span_offsets[0] + sizeof(g_post_boundary_span);
    uint8_t *fixture = (uint8_t *)calloc(1u, fixture_size);
    Theron_Track02BankSignal signal;
    Theron_Track02SignalStatus status;
    const size_t span_offset = jp_span_offsets[0];
    const size_t id_offset = span_offset - 4u;
    const size_t prefix_offset = id_offset - sizeof(g_audio_bank_prefix);

    if (!fixture) {
        printf("FAIL audio-bank marker fixture: allocation failed\n");
        ++g_fail;
        return;
    }

    /* Place the 12-byte sentinel + 4-byte id + 44-byte post-boundary span
     * at the JP anchor 0 offset.  No descriptors are placed because the
     * audio-bank helper does not consult them. */
    memcpy(fixture + prefix_offset,
           g_audio_bank_prefix,
           sizeof(g_audio_bank_prefix));
    fixture[id_offset + 0] = (uint8_t)(synth_id_jp_anchor0 & 0xFFu);
    fixture[id_offset + 1] = (uint8_t)((synth_id_jp_anchor0 >> 8) & 0xFFu);
    fixture[id_offset + 2] = (uint8_t)((synth_id_jp_anchor0 >> 16) & 0xFFu);
    fixture[id_offset + 3] = (uint8_t)((synth_id_jp_anchor0 >> 24) & 0xFFu);
    memcpy(fixture + span_offset,
           g_post_boundary_span,
           sizeof(g_post_boundary_span));

    status = theron_v1_track02_find_audio_bank_marker(fixture,
                                                      fixture_size,
                                                      THERON_TRACK02_MD5_JP_BIN,
                                                      0u,
                                                      &signal.audio_bank_id[0],
                                                      &signal.audio_bank_id_offsets[0],
                                                      &signal.audio_bank_prefix_offsets[0]);
    check_int("synthetic JP anchor 0 audio-bank marker",
              status,
              THERON_TRACK02_SIGNAL_OK);
    check_u32("synthetic JP anchor 0 audio-bank id",
              signal.audio_bank_id[0],
              synth_id_jp_anchor0);
    check_size("synthetic JP anchor 0 id offset",
               signal.audio_bank_id_offsets[0],
               jp_span_offsets[0] - 4u);
    check_size("synthetic JP anchor 0 prefix offset",
               signal.audio_bank_prefix_offsets[0],
               jp_span_offsets[0] - 4u - sizeof(g_audio_bank_prefix));

    /* US anchor 0 should fail because we only placed JP data. */
    status = theron_v1_track02_find_audio_bank_marker(fixture,
                                                      fixture_size,
                                                      THERON_TRACK02_MD5_US_BIN,
                                                      0u,
                                                      &signal.audio_bank_id[0],
                                                      &signal.audio_bank_id_offsets[0],
                                                      &signal.audio_bank_prefix_offsets[0]);
    check_int("US anchor 0 on JP-placed fixture is not-found",
              status,
              THERON_TRACK02_SIGNAL_NOT_FOUND);
    // US fixture_size is too small to contain 0x2d53e0; verify by checking
    // that even placing data at offset 0 makes no difference without
    // changing the input size.  We do not need a second fixture for this
    // since the size-vs-offset bounds check is exercised above.

    // Missing-prefix negative fixture: keep the 4-byte LE word but
    // overwrite the 12-byte sentinel with zeros.
    memset(fixture, 0, fixture_size);
    fixture[jp_span_offsets[0] - 4u + 0] = 0x11u;
    fixture[jp_span_offsets[0] - 4u + 1] = 0x22u;
    fixture[jp_span_offsets[0] - 4u + 2] = 0x33u;
    fixture[jp_span_offsets[0] - 4u + 3] = 0x44u;
    status = theron_v1_track02_find_audio_bank_marker(fixture,
                                                      fixture_size,
                                                      THERON_TRACK02_MD5_JP_BIN,
                                                      0u,
                                                      &signal.audio_bank_id[0],
                                                      &signal.audio_bank_id_offsets[0],
                                                      &signal.audio_bank_prefix_offsets[0]);
    check_int("missing-prefix anchor is rejected",
              status,
              THERON_TRACK02_SIGNAL_NOT_FOUND);
    check_u32("missing-prefix zeros out id",
              signal.audio_bank_id[0],
              0u);

    /* Out-of-range anchor index negative fixture. */
    status = theron_v1_track02_find_audio_bank_marker(fixture,
                                                      fixture_size,
                                                      THERON_TRACK02_MD5_US_BIN,
                                                      THERON_TRACK02_MAX_BANK_ANCHORS,
                                                      &signal.audio_bank_id[0],
                                                      &signal.audio_bank_id_offsets[0],
                                                      &signal.audio_bank_prefix_offsets[0]);
    check_int("out-of-range anchor index is bad-input",
              status,
              THERON_TRACK02_SIGNAL_BAD_INPUT);

    /* Silence unused-static-const warning for US anchor table (the
     * check above uses the JP fixture). */
    (void)us_span_offsets;

    free(fixture);
}

static void probe_audio_bank_marker_unsupported_variant_fixture(void) {
    /* US ISO and JP Rev 1 ISO are unsupported on the audio-bank marker:
     *   US ISO is a partial extract with no anchor;
     *   JP Rev 1 ISO is hash-verified but zero-filled in the available
     *   image so no offset is claimed. */
    static const uint8_t zeros[64] = {0};
    uint32_t got_id = 0xDEADBEEFu;
    size_t got_id_offset = 7u;
    size_t got_prefix_offset = 7u;
    Theron_Track02SignalStatus status_us_iso =
        theron_v1_track02_find_audio_bank_marker(zeros,
                                                 sizeof(zeros),
                                                 THERON_TRACK02_MD5_US_ISO,
                                                 0u,
                                                 &got_id,
                                                 &got_id_offset,
                                                 &got_prefix_offset);
    Theron_Track02SignalStatus status_jp_rev1 =
        theron_v1_track02_find_audio_bank_marker(zeros,
                                                 sizeof(zeros),
                                                 THERON_TRACK02_MD5_JP_REV1_ISO,
                                                 0u,
                                                 &got_id,
                                                 &got_id_offset,
                                                 &got_prefix_offset);
    Theron_Track02SignalStatus status_unknown =
        theron_v1_track02_find_audio_bank_marker(zeros,
                                                 sizeof(zeros),
                                                 "00000000000000000000000000000000",
                                                 0u,
                                                 &got_id,
                                                 &got_id_offset,
                                                 &got_prefix_offset);

    check_int("US ISO audio-bank marker is unsupported",
              status_us_iso,
              THERON_TRACK02_SIGNAL_UNSUPPORTED_VARIANT);
    check_int("JP Rev 1 ISO audio-bank marker is unsupported",
              status_jp_rev1,
              THERON_TRACK02_SIGNAL_UNSUPPORTED_VARIANT);
    check_int("unknown MD5 audio-bank marker is unsupported",
              status_unknown,
              THERON_TRACK02_SIGNAL_UNSUPPORTED_VARIANT);
    check_u32("US ISO zeros out-id on unsupported",
              got_id,
              0u);
    check_size("US ISO zeros out-id-offset on unsupported",
               got_id_offset,
               0u);

    /* NULL inputs are bad-input. */
    {
        Theron_Track02SignalStatus bad =
            theron_v1_track02_find_audio_bank_marker(NULL,
                                                     0u,
                                                     THERON_TRACK02_MD5_US_BIN,
                                                     0u,
                                                     &got_id,
                                                     &got_id_offset,
                                                     &got_prefix_offset);
        check_int("NULL data is bad-input",
                  bad,
                  THERON_TRACK02_SIGNAL_BAD_INPUT);
    }
}

static void probe_audio_bank_marker_real_data(const char *md5_hex,
                                              const char *env_name,
                                              const char *default_file,
                                              const uint32_t expected_ids[THERON_TRACK02_MAX_BANK_ANCHORS],
                                              const size_t expected_span_offsets[THERON_TRACK02_MAX_BANK_ANCHORS]) {
    char path[512];
    const char *env_path = getenv(env_name);
    const char *path_to_read;
    uint8_t *data = NULL;
    size_t size = 0;
    char md5_hex_local[33] = {0};
    size_t i;

    if (env_path && env_path[0]) {
        path_to_read = env_path;
    } else {
        default_data_path(default_file, path);
        path_to_read = path;
    }

    if (!file_exists(path_to_read)) {
        printf("SKIP audio-bank marker real-data: no Track 02 image at %s\n",
               path_to_read);
        ++g_skip;
        return;
    }
    if (!m12_file_md5_hex(path_to_read, md5_hex_local)) {
        printf("FAIL audio-bank marker real-data: could not compute MD5 for %s\n",
               path_to_read);
        ++g_fail;
        return;
    }
    if (strcmp(md5_hex_local, md5_hex) != 0) {
        printf("FAIL audio-bank marker real-data: MD5 %s does not match expected %s\n",
               md5_hex_local, md5_hex);
        ++g_fail;
        return;
    }
    if (!read_file(path_to_read, &data, &size)) {
        printf("FAIL audio-bank marker real-data: could not read %s\n",
               path_to_read);
        ++g_fail;
        return;
    }

    /* Narrow receipt: lock all three audio-bank marker tuples in the
     * (US|JP) raw Track 02 BIN.  See theron_v1_track02_source_evidence().
     * This remains byte evidence only; no playback or ADPCM decode claim. */
    for (i = 0; i < THERON_TRACK02_MAX_BANK_ANCHORS; ++i) {
        uint32_t got_id = 0u;
        size_t got_id_offset = 0u;
        size_t got_prefix_offset = 0u;
        char label[96];
        const size_t expected_id_offset = expected_span_offsets[i] - 4u;
        Theron_Track02SignalStatus status =
            theron_v1_track02_find_audio_bank_marker(data,
                                                     size,
                                                     md5_hex_local,
                                                     i,
                                                     &got_id,
                                                     &got_id_offset,
                                                     &got_prefix_offset);

        printf("audio-bank marker real-data: md5=%s anchor=%zu status=%s id=0x%08x id_offset=0x%zx\n",
               md5_hex_local,
               i,
               theron_v1_track02_signal_status_name(status),
               (unsigned)got_id,
               got_id_offset);
        snprintf(label, sizeof(label), "real-data audio-bank marker status[%zu]", i);
        check_int(label, status, THERON_TRACK02_SIGNAL_OK);
        snprintf(label, sizeof(label), "real-data audio-bank id word[%zu]", i);
        check_u32(label, got_id, expected_ids[i]);
        snprintf(label, sizeof(label), "real-data audio-bank id offset[%zu]", i);
        check_size(label, got_id_offset, expected_id_offset);
        snprintf(label, sizeof(label), "real-data audio-bank prefix offset[%zu]", i);
        check_size(label,
                   got_prefix_offset,
                   expected_id_offset - sizeof(g_audio_bank_prefix));
    }

    free(data);
}

int main(void) {
    printf("=== Theron V1 Track 02 Bank Evidence Probe ===\n");
    printf("%s\n", theron_v1_track02_source_evidence());

    probe_negative_fixture();
    probe_jp_zero_image_fixture();
    probe_descriptor_only_negative_fixture();
    probe_boundary_prefix_only_negative_fixture();
    probe_raw_user_data_synthetic_fixture();
    probe_audio_bank_marker_synthetic_fixture();
    probe_audio_bank_marker_unsupported_variant_fixture();
    probe_audio_bank_marker_real_data(THERON_TRACK02_MD5_US_BIN,
                                      "FIRESTAFF_THERON_TRACK02_US_BIN",
                                      "theron-extras/usa/Dungeon Master - Theron's Quest (USA) (Track 02).bin",
                                      g_us_audio_bank_ids,
                                      g_us_bin_span_offsets);
    probe_audio_bank_marker_real_data(THERON_TRACK02_MD5_JP_BIN,
                                      "FIRESTAFF_THERON_TRACK02_JP_BIN",
                                      "theron-extras/japan/Dungeon Master - Theron's Quest (Japan) (Track 02).bin",
                                      g_jp_audio_bank_ids,
                                      g_jp_bin_span_offsets);
    probe_raw_bin_positive_fixture("US raw BIN synthetic anchors",
                                   THERON_TRACK02_MD5_US_BIN,
                                   THERON_TRACK02_VARIANT_US_BIN,
                                   g_us_bin_descriptor_offsets,
                                   g_us_bin_span_offsets,
                                   g_us_audio_bank_ids,
                                   3141u,
                                   1263u);
    probe_raw_bin_positive_fixture("JP raw BIN synthetic anchors",
                                   THERON_TRACK02_MD5_JP_BIN,
                                   THERON_TRACK02_VARIANT_JP_BIN,
                                   g_jp_bin_descriptor_offsets,
                                   g_jp_bin_span_offsets,
                                   g_jp_audio_bank_ids,
                                   3140u,
                                   1262u);
    probe_raw_bin_missing_anchor_fixture();
    probe_track("US ISO bank descriptor",
                "FIRESTAFF_THERON_TRACK02_US",
                "theron/TQUS02End.iso",
                THERON_TRACK02_MD5_US_ISO,
                THERON_TRACK02_SIGNAL_OK);
    probe_track("US raw Track 02 bank anchors",
                "FIRESTAFF_THERON_TRACK02_US_BIN",
                "theron-extras/usa/Dungeon Master - Theron's Quest (USA) (Track 02).bin",
                THERON_TRACK02_MD5_US_BIN,
                THERON_TRACK02_SIGNAL_OK);
    probe_track("JP raw Track 02 bank anchors",
                "FIRESTAFF_THERON_TRACK02_JP_BIN",
                "theron-extras/japan/Dungeon Master - Theron's Quest (Japan) (Track 02).bin",
                THERON_TRACK02_MD5_JP_BIN,
                THERON_TRACK02_SIGNAL_OK);
    probe_track("JP Rev 1 ISO zero-image guard",
                "FIRESTAFF_THERON_TRACK02_JP_REV1",
                "theron/TQJP02End.iso",
                THERON_TRACK02_MD5_JP_REV1_ISO,
                THERON_TRACK02_SIGNAL_INSUFFICIENT_ZERO_IMAGE);

    printf("summary: fail=%d skip=%d\n", g_fail, g_skip);
    return g_fail ? 1 : 0;
}
