/*
 * theron_v1_startup_receipt.c -- Theron's Quest V1 startup real-asset
 * receipt implementation.
 *
 * The receipt is a bounded, hashes-only summary that proves a Theron
 * Track 02 launch reached the existing boot/M11 handoff without claiming
 * playability.  See include/theron_v1_startup_receipt.h for the field
 * semantics and the source/lock list.
 *
 * Design constraints honored here:
 *   1. Determinism.  No time-of-day, no random state, no thread-local
 *      counters leak into the receipt.
 *   2. No game data in the repository.  The receipt only stores MD5 hex
 *      strings and path strings; bytes are read on demand and discarded.
 *   3. No-data hosts stay green.  When the caller does not supply a
 *      Track 02 path/MD5, the placeholder path emits a fully populated
 *      receipt (NO_DATA_PLACEHOLDER) so CI does not need to gate on
 *      local Track 02 staging.
 *   4. Rejection is auditable.  Every non-success path still writes a
 *      receipt so the log line explains why (skip reason / rejection
 *      reason).
 *   5. Source/lock citation is always populated.  Even the no-data
 *      placeholder includes the module name + MD5 list so the receipt
 *      is self-describing.
 *
 * Source/evidence:
 *   - src/theron/theron_v1_track02.c (Track 02 bank-signal decoder)
 *   - src/theron/theron_v1_boot.c    (boot profile + direct launch)
 *   - include/asset_status_m12.h m12_file_md5_hex (file-MD5 helper)
 *   - src/shared/asset_status_m12.c g_theronVersions (known Track 02 MD5s)
 *   - docs/source-lock/tqr_v1_phase0_provenance_gate_H2339.md
 *   - docs/source-lock/tqr_v1_phase1_boot_H2338.md
 *   - docs/source-lock/tqr_v1_track02_bank_signal_2026-06-03.md
 *
 * ReDMCSB has no Theron code; the Theron-side evidence is local byte
 * inspection of the four cataloged Track 02 MD5s.
 */

#include "theron_v1_startup_receipt.h"
#include "asset_status_m12.h"
#include "theron_v1_chapter_marker.h"
#include "theron_v1_startup_flow.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

/* ── Known Track 02 MD5 set (mirrors g_theronVersions in ─────────────
 * src/shared/asset_status_m12.c, kept inline so the receipt module is
 * self-contained and does not pull the full M12 asset catalog just to
 * recognise four hash strings. */
static const char *const g_known_track02_md5s[] = {
    "b7afb338ad31be1025b53f9aff12d73a", /* JP Track 02 BIN              */
    "f23601102138f87c33025877767ebf76", /* US Track 02 BIN              */
    "397039af02d50d15c70b74088eb8a1cb", /* JP Rev 1 Track 02 ISO        */
    "3d8b78571dcd0e6eb8eb4b01eeb7fbba", /* US Track 02 ISO              */
    NULL
};

/* ── Tiny helpers ─────────────────────────────────────────────────── */

static size_t safe_str_copy(char *dst, size_t dst_size, const char *src) {
    size_t i;
    if (!dst || dst_size == 0) return 0;
    if (!src) {
        dst[0] = '\0';
        return 0;
    }
    for (i = 0; i + 1u < dst_size && src[i] != '\0'; ++i) {
        dst[i] = src[i];
    }
    dst[i] = '\0';
    return i;
}

/* FNV-1a 32-bit hash over a byte buffer.
 *
 * Deterministic, allocation-free, no time-of-day, no global state.
 * The 32-bit width is enough to spot duplicates in CI logs without
 * being a cryptographic identifier. */
static uint32_t fnv1a_32(const void *data, size_t size, uint32_t seed) {
    const uint8_t *p = (const uint8_t *)data;
    uint32_t h = seed;
    size_t i;
    if (!p) return h;
    /* FNV-1a constants per the published algorithm. */
    for (i = 0; i < size; ++i) {
        h ^= (uint32_t)p[i];
        h *= 0x01000193u;
    }
    return h;
}

/* Hash a NUL-terminated string into the running FNV-1a state. */
static uint32_t fnv1a_str(uint32_t h, const char *s) {
    if (!s) return h;
    return fnv1a_32(s, strlen(s), h);
}

/* ── Reset / placeholder ─────────────────────────────────────────── */

void theron_v1_startup_receipt_reset(Theron_V1_StartupReceipt *receipt) {
    if (!receipt) return;
    memset(receipt, 0, sizeof(*receipt));
    receipt->descriptor_window_entry_index = -1;
    receipt->m11_dispatch_source_kind = -1;
}

static void set_verdict(Theron_V1_StartupReceipt *receipt,
                         Theron_V1_StartupReceiptVerdict verdict,
                         const char *verdict_name) {
    receipt->verdict = verdict;
    safe_str_copy(receipt->verdict_name,
                  sizeof(receipt->verdict_name),
                  verdict_name);
}

static void set_variant(Theron_V1_StartupReceipt *receipt,
                         Theron_Track02Variant variant) {
    receipt->variant = variant;
    safe_str_copy(receipt->variant_name,
                  sizeof(receipt->variant_name),
                  theron_v1_track02_variant_name(variant));
}

static void populate_startup_mirror_summary(Theron_V1_StartupReceipt *receipt) {
    uint32_t portrait_min = 0u;
    uint32_t portrait_max = 0u;
    uint32_t class_mask = 0u;
    int i;

    if (!receipt) return;
    receipt->startup_mirror_count = THERON_STARTUP_HERO_MIRROR_COUNT;
    receipt->startup_companion_limit = THERON_STARTUP_MAX_COMPANIONS;

    for (i = 0; i < THERON_STARTUP_HERO_MIRROR_COUNT; ++i) {
        const Theron_StartupMirrorMeta *meta =
            theron_v1_startup_mirror_meta(i);
        uint32_t portrait;
        if (!meta) continue;
        portrait = (uint32_t)meta->portrait_index;
        if (portrait_min == 0u || portrait < portrait_min) {
            portrait_min = portrait;
        }
        if (portrait > portrait_max) {
            portrait_max = portrait;
        }
        if (meta->primary_class >= 0 &&
            meta->primary_class < THERON_CLASS_COUNT) {
            class_mask |= (uint32_t)(1u << (uint32_t)meta->primary_class);
        }
    }

    receipt->startup_portrait_min = portrait_min;
    receipt->startup_portrait_max = portrait_max;
    receipt->startup_class_mask = class_mask;
    receipt->startup_fallback_label_count = THERON_STARTUP_HERO_MIRROR_COUNT;
    receipt->startup_decoded_art_count = 0u;
}

static void populate_startup_chapter_summary(
    Theron_V1_StartupReceipt *receipt,
    const Theron_V1_BootProfile *profile) {
    Theron_ChapterMarker marker;

    if (!receipt) return;
    theron_v1_chapter_marker_compute(profile, NULL, NULL, &marker);
    safe_str_copy(receipt->startup_chapter_label,
                  sizeof(receipt->startup_chapter_label),
                  marker.chapter_label);
    safe_str_copy(receipt->startup_quest_summary,
                  sizeof(receipt->startup_quest_summary),
                  marker.quest_summary);
    safe_str_copy(receipt->startup_next_dungeon_hint,
                  sizeof(receipt->startup_next_dungeon_hint),
                  marker.next_dungeon_hint);
    receipt->startup_quest_item_total =
        (uint32_t)marker.quest_item_total;
    receipt->startup_quest_items_collected =
        (uint32_t)marker.quest_items_collected;
}

void theron_v1_startup_receipt_set_placeholder(Theron_V1_StartupReceipt *receipt) {
    if (!receipt) return;
    theron_v1_startup_receipt_reset(receipt);
    populate_startup_mirror_summary(receipt);
    populate_startup_chapter_summary(receipt, NULL);

    set_verdict(receipt,
                THERON_V1_STARTUP_RECEIPT_NO_DATA_PLACEHOLDER,
                "no-data-placeholder");
    set_variant(receipt, THERON_TRACK02_VARIANT_UNKNOWN);

    safe_str_copy(receipt->skip_reason_note,
                  sizeof(receipt->skip_reason_note),
                  "no Track 02 file supplied (CI / no-data host)");
    safe_str_copy(receipt->source_evidence,
                  sizeof(receipt->source_evidence),
                  "theron_v1_startup_receipt.c placeholder; "
                  "Track 02 MD5s: "
                  THERON_TRACK02_MD5_JP_BIN ", "
                  THERON_TRACK02_MD5_US_BIN ", "
                  THERON_TRACK02_MD5_JP_REV1_ISO ", "
                  THERON_TRACK02_MD5_US_ISO);

    /* M11 dispatch is intentionally not populated for the placeholder;
     * the receipt fields stay at 0 so the consumer can detect the
     * no-data case without a separate flag. */
    receipt->m11_dispatch_source_kind = -1;

    /* Session-tick token: still computed so placeholder receipts are
     * sortable / de-dupable in CI logs. */
    receipt->session_tick_token = theron_v1_startup_receipt_session_tick(receipt);
}

/* ── MD5 recognition ──────────────────────────────────────────────── */

int theron_v1_startup_receipt_md5_is_known(const char *expected_md5) {
    size_t i;
    if (!expected_md5 || !expected_md5[0]) return 0;
    for (i = 0; g_known_track02_md5s[i] != NULL; ++i) {
        if (strcmp(expected_md5, g_known_track02_md5s[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

/* ── File helpers (stat-only, never bytes) ────────────────────────── */

static int stat_size_or_zero(const char *path) {
    struct stat st;
    if (!path || !path[0]) return -1;
    if (stat(path, &st) != 0) return -1;
    if (st.st_size <= 0) return 0;
    return (int)st.st_size;
}

/* Read a file into a heap buffer.  Caller frees.  Returns 1 on success. */
static int read_file_bytes(const char *path, uint8_t **out, size_t *out_size) {
    FILE *fp;
    long sz;
    uint8_t *buf;

    if (!path || !out || !out_size) return 0;
    fp = fopen(path, "rb");
    if (!fp) return 0;
    if (fseek(fp, 0, SEEK_END) != 0) { fclose(fp); return 0; }
    sz = ftell(fp);
    if (sz <= 0) { fclose(fp); return 0; }
    if (fseek(fp, 0, SEEK_SET) != 0) { fclose(fp); return 0; }
    buf = (uint8_t *)malloc((size_t)sz);
    if (!buf) { fclose(fp); return 0; }
    if (fread(buf, 1, (size_t)sz, fp) != (size_t)sz) {
        free(buf); fclose(fp); return 0;
    }
    fclose(fp);
    *out = buf;
    *out_size = (size_t)sz;
    return 1;
}

/* ── Real-asset receipt ───────────────────────────────────────────── */

int theron_v1_startup_receipt_from_file(const char *track02_path,
                                         const char *expected_md5,
                                         Theron_V1_StartupReceipt *receipt) {
    Theron_V1_BootProfile profile;
    Theron_Track02BankSignal signal;
    Theron_Track02SignalStatus signal_status;
    uint8_t *data = NULL;
    size_t size = 0;
    char md5_hex[33];
    int file_size;
    int m12_hash_ok = 0;
    int bank_signal_ok = 0;

    if (!receipt) return 0;
    theron_v1_startup_receipt_reset(receipt);
    populate_startup_mirror_summary(receipt);
    populate_startup_chapter_summary(receipt, NULL);

    /* Empty / NULL inputs downgrade to a placeholder with a clear note.
     * We intentionally still emit a placeholder receipt rather than
     * rejecting silently so the audit trail stays complete. */
    if (!track02_path || !track02_path[0]) {
        theron_v1_startup_receipt_set_placeholder(receipt);
        safe_str_copy(receipt->skip_reason_note,
                      sizeof(receipt->skip_reason_note),
                      "empty Track 02 path; placeholder emitted");
        receipt->session_tick_token =
            theron_v1_startup_receipt_session_tick(receipt);
        return 0;
    }
    if (!expected_md5 || !expected_md5[0] ||
        !theron_v1_startup_receipt_md5_is_known(expected_md5)) {
        set_verdict(receipt,
                    THERON_V1_STARTUP_RECEIPT_REJECTED,
                    "rejected");
        set_variant(receipt, THERON_TRACK02_VARIANT_UNKNOWN);
        safe_str_copy(receipt->track02_path,
                      sizeof(receipt->track02_path),
                      track02_path);
        safe_str_copy(receipt->skip_reason_note,
                      sizeof(receipt->skip_reason_note),
                      expected_md5 && expected_md5[0]
                        ? "MD5 not in known TQ Track 02 set"
                        : "empty MD5");
        receipt->m11_dispatch_source_kind = -1;
        receipt->session_tick_token =
            theron_v1_startup_receipt_session_tick(receipt);
        return 0;
    }

    /* File presence + size (no bytes read yet). */
    file_size = stat_size_or_zero(track02_path);
    if (file_size <= 0) {
        set_verdict(receipt,
                    THERON_V1_STARTUP_RECEIPT_SKIPPED,
                    "skipped");
        set_variant(receipt,
                    theron_v1_track02_variant_for_md5(expected_md5));
        safe_str_copy(receipt->track02_path,
                      sizeof(receipt->track02_path),
                      track02_path);
        safe_str_copy(receipt->track02_md5_hex,
                      sizeof(receipt->track02_md5_hex),
                      expected_md5);
        safe_str_copy(receipt->skip_reason_note,
                      sizeof(receipt->skip_reason_note),
                      "Track 02 file not present on host");
        receipt->m11_dispatch_source_kind = -1;
        receipt->session_tick_token =
            theron_v1_startup_receipt_session_tick(receipt);
        return 0;
    }

    /* Round-trip the file's MD5 against the catalog-supplied hash.  This
     * is the boot side of the contract; if the file drifted on disk the
     * caller-supplied MD5 is no longer trustworthy. */
    md5_hex[0] = '\0';
    m12_hash_ok = m12_file_md5_hex(track02_path, md5_hex);
    if (!m12_hash_ok || strcmp(md5_hex, expected_md5) != 0) {
        set_verdict(receipt,
                    THERON_V1_STARTUP_RECEIPT_REJECTED,
                    "rejected");
        set_variant(receipt,
                    theron_v1_track02_variant_for_md5(expected_md5));
        safe_str_copy(receipt->track02_path,
                      sizeof(receipt->track02_path),
                      track02_path);
        safe_str_copy(receipt->track02_md5_hex,
                      sizeof(receipt->track02_md5_hex),
                      expected_md5);
        safe_str_copy(receipt->skip_reason_note,
                      sizeof(receipt->skip_reason_note),
                      m12_hash_ok
                        ? "file MD5 does not match expected MD5"
                        : "could not compute file MD5");
        receipt->m11_dispatch_source_kind = -1;
        receipt->session_tick_token =
            theron_v1_startup_receipt_session_tick(receipt);
        return 0;
    }

    /* All gates passed so far — run the boot + bank-signal pipeline. */
    safe_str_copy(receipt->track02_path,
                  sizeof(receipt->track02_path),
                  track02_path);
    safe_str_copy(receipt->track02_md5_hex,
                  sizeof(receipt->track02_md5_hex),
                  md5_hex);
    receipt->track02_byte_count = (uint64_t)file_size;
    set_variant(receipt,
                theron_v1_track02_variant_for_md5(expected_md5));

    /* Direct launch path.  Uses theron_v1_boot_load_verified_path() so
     * no stat/fallback walk happens in this receipt path.  We do NOT
     * call theron_v1_boot_enter_game() (that allocates Theron state and
     * would require SDL); the receipt is purely a startup summary. */
    theron_v1_boot_profile_init(&profile);
    if (theron_v1_boot_load_verified_path(&profile, track02_path, expected_md5) != 0) {
        set_verdict(receipt,
                    THERON_V1_STARTUP_RECEIPT_REJECTED,
                    "rejected");
        safe_str_copy(receipt->skip_reason_note,
                      sizeof(receipt->skip_reason_note),
                      "theron_v1_boot_load_verified_path rejected "
                      "the verified Track 02 path");
        receipt->m11_dispatch_source_kind = -1;
        receipt->session_tick_token =
            theron_v1_startup_receipt_session_tick(receipt);
        return 0;
    }

    receipt->boot_profile_platform        = profile.platform;
    safe_str_copy(receipt->boot_profile_version_id,
                  sizeof(receipt->boot_profile_version_id),
                  profile.version_id);
    receipt->boot_profile_assets_verified  = profile.assets_verified;
    receipt->boot_profile_tick_rate_hz     = profile.deterministic.tick_rate_hz;
    receipt->boot_profile_max_champions    = profile.deterministic.max_champions;
    receipt->boot_profile_dungeon_count    = profile.deterministic.dungeon_count;
    receipt->boot_profile_dungeon_seed     = profile.deterministic.dungeon_seed;
    populate_startup_chapter_summary(receipt, &profile);

    /* M11 dispatch is recorded as a *kind marker* only.  The full M11
     * start would require SDL + Theron world allocation which is out of
     * scope for the receipt surface (the existing M11 direct-launch test
     * covers that path under --enable-sdl).  We populate the enum value
     * a real asset-catalog launch would select so consumers can verify
     * the right dispatch kind was identified without doing the SDL work. */
    receipt->m11_dispatch_source_kind = 1 /* M11_GAME_SOURCE_THERON_TRACK02 */;
    receipt->m11_view_active = 0;       /* would flip to 1 inside M11_GameView_Start */
    receipt->m11_world_present = 0;     /* would flip to 1 alongside world init */

    /* Bank-signal decoder — only for raw BIN / ISO variants that actually
     * carry the known anchors.  JP Rev 1 ISO is allowed to be a zero-fill
     * (the receipt still surfaces the deterministic skip contract). */
    if (receipt->variant == THERON_TRACK02_VARIANT_UNKNOWN) {
        /* Variant should already be set; defensive guard. */
        receipt->variant = theron_v1_track02_variant_for_md5(expected_md5);
    }

    /* Read bytes for the bank-signal decoder.  Bytes are heap-local and
     * freed before return so the repository never sees them. */
    if (!read_file_bytes(track02_path, &data, &size)) {
        set_verdict(receipt,
                    THERON_V1_STARTUP_RECEIPT_SKIPPED,
                    "skipped");
        safe_str_copy(receipt->skip_reason_note,
                      sizeof(receipt->skip_reason_note),
                      "could not read Track 02 file for bank-signal decoder");
        receipt->m11_dispatch_source_kind = -1;
        receipt->session_tick_token =
            theron_v1_startup_receipt_session_tick(receipt);
        return 0;
    }

    memset(&signal, 0, sizeof(signal));
    signal_status = theron_v1_track02_find_bank_signal(
        data, size, expected_md5, &signal);
    bank_signal_ok = (signal_status == THERON_TRACK02_SIGNAL_OK) ||
                     (receipt->variant == THERON_TRACK02_VARIANT_JP_REV1_ISO &&
                      (signal_status == THERON_TRACK02_SIGNAL_INSUFFICIENT_ZERO_IMAGE ||
                       signal_status == THERON_TRACK02_SIGNAL_OK));

    if (!bank_signal_ok) {
        free(data);
        set_verdict(receipt,
                    THERON_V1_STARTUP_RECEIPT_SKIPPED,
                    "skipped");
        safe_str_copy(receipt->skip_reason_note,
                      sizeof(receipt->skip_reason_note),
                      "bank-signal decoder did not confirm anchors for this MD5");
        receipt->session_tick_token =
            theron_v1_startup_receipt_session_tick(receipt);
        return 0;
    }

    /* Populate the bank-signal summary fields.  For JP Rev 1 ISO the
     * offsets remain zero (the decoder intentionally reports
     * INSUFFICIENT_ZERO_IMAGE without claiming an offset). */
    receipt->descriptor_offset         = (uint64_t)signal.descriptor_offset;
    receipt->descriptor_size           = (uint64_t)signal.descriptor_size;
    receipt->descriptor_value_count    = (uint64_t)signal.value_count;
    receipt->descriptor_stride         = (uint32_t)signal.stride;
    receipt->anchor_count              = (uint64_t)signal.anchor_count;
    if (signal.anchor_count > 0u) {
        receipt->post_boundary_span_offset =
            (uint64_t)signal.post_boundary_span_offsets[0];
    } else {
        receipt->post_boundary_span_offset = 0u;
    }
    receipt->post_boundary_span_size   = (uint64_t)signal.post_boundary_span_size;
    receipt->next_nonzero_offset       = (uint64_t)signal.next_nonzero_offset;

    if (signal.descriptor_offset != 0u &&
        signal.descriptor_size >=
            THERON_TRACK02_MAX_DESCRIPTOR_TABLE_ENTRIES * 2u &&
        signal.descriptor_offset < size) {
        Theron_Track02DescriptorTable table;
        Theron_Track02DescriptorEntrySemanticBinding entries[
            THERON_TRACK02_MAX_DESCRIPTOR_TABLE_ENTRIES];
        Theron_Track02TableDecodeStatus table_status;
        size_t i;

        table_status = theron_v1_track02_decode_descriptor_table(
            data + signal.descriptor_offset,
            size - signal.descriptor_offset,
            (uint16_t)signal.stride,
            &table);
        if (table_status == THERON_TRACK02_TABLE_DECODE_OK &&
            theron_v1_track02_bind_descriptor_entry_roles(
                data,
                size,
                signal.descriptor_offset,
                &table,
                entries) == THERON_TRACK02_TABLE_DECODE_OK) {
            receipt->descriptor_window_entry_index =
                (int32_t)theron_v1_track02_find_descriptor_window_entry_index(
                    entries,
                    table.entry_count);
            for (i = 0; i < table.entry_count; ++i) {
                switch (entries[i].role) {
                case THERON_TRACK02_DESCRIPTOR_ENTRY_ROLE_RESERVED_ZERO_FILL:
                    ++receipt->descriptor_role_zero_fill_count;
                    break;
                case THERON_TRACK02_DESCRIPTOR_ENTRY_ROLE_PRE_DESCRIPTOR_DATA:
                    ++receipt->descriptor_role_pre_data_count;
                    break;
                case THERON_TRACK02_DESCRIPTOR_ENTRY_ROLE_POST_DESCRIPTOR_DATA:
                    ++receipt->descriptor_role_post_data_count;
                    break;
                case THERON_TRACK02_DESCRIPTOR_ENTRY_ROLE_CONTAINS_DESCRIPTOR_TABLE:
                    ++receipt->descriptor_role_descriptor_table_count;
                    receipt->descriptor_byte_before =
                        (uint32_t)entries[i].byte_before_descriptor;
                    receipt->descriptor_byte_before_is_rts =
                        entries[i].byte_before_descriptor_is_rts;
                    receipt->descriptor_first_nonzero_after =
                        (uint64_t)entries[i].first_nonzero_after_descriptor;
                    receipt->descriptor_all_zero_after =
                        entries[i].all_zero_after_descriptor;
                    break;
                default:
                    break;
                }
            }
        }
    }

    {
        Theron_Track02UserDataWindowCatalog catalog;
        if (theron_v1_track02_catalog_user_data_windows(
                data,
                size,
                expected_md5,
                &catalog) == THERON_TRACK02_SIGNAL_OK) {
            size_t i;
            receipt->user_data_window_count = (uint32_t)catalog.entry_count;
            receipt->user_data_window_overflow_count =
                (uint32_t)catalog.overflow_count;
            for (i = 0u; i < catalog.entry_count; ++i) {
                switch (catalog.entries[i].role) {
                case THERON_TRACK02_USER_DATA_WINDOW_BANK_DESCRIPTOR_TABLE:
                    ++receipt->user_data_window_descriptor_count;
                    break;
                case THERON_TRACK02_USER_DATA_WINDOW_POST_BOUNDARY_SPAN:
                    ++receipt->user_data_window_span_count;
                    break;
                case THERON_TRACK02_USER_DATA_WINDOW_INITIAL_LEVEL_CANDIDATE:
                    ++receipt->user_data_window_initial_count;
                    break;
                case THERON_TRACK02_USER_DATA_WINDOW_UNKNOWN:
                default:
                    break;
                }
            }
        }
    }

    {
        Theron_Track02StartupTextMarkerCatalog text_catalog;
        if (theron_v1_track02_catalog_startup_text_markers(
                data,
                size,
                expected_md5,
                &text_catalog) == THERON_TRACK02_SIGNAL_OK) {
            size_t i;
            receipt->startup_text_marker_count =
                (uint32_t)text_catalog.marker_count;
            receipt->startup_text_marker_overflow_count =
                (uint32_t)text_catalog.overflow_count;
            for (i = 0u; i < text_catalog.marker_count; ++i) {
                switch (text_catalog.markers[i].kind) {
                case THERON_TRACK02_STARTUP_TEXT_US_RESURRECT_THERON_PROMPT:
                    ++receipt->startup_text_us_prompt_count;
                    break;
                case THERON_TRACK02_STARTUP_TEXT_JP_CHAMPION_ROSTER_CLUSTER:
                    ++receipt->startup_text_jp_roster_count;
                    break;
                case THERON_TRACK02_STARTUP_TEXT_UNKNOWN:
                default:
                    break;
                }
            }
        }
    }
    {
        Theron_Track02StartupRosterNameCatalog roster_catalog;
        if (theron_v1_track02_catalog_startup_roster_names(
                data,
                size,
                expected_md5,
                &roster_catalog) == THERON_TRACK02_SIGNAL_OK) {
            size_t i;
            receipt->startup_roster_name_count =
                (uint32_t)roster_catalog.name_count;
            receipt->startup_roster_overflow_count =
                (uint32_t)roster_catalog.overflow_count;
            for (i = 0u; i < roster_catalog.name_count; ++i) {
                if (roster_catalog.names[i].title_offset_valid &&
                    roster_catalog.names[i].title[0] != '\0') {
                    ++receipt->startup_roster_title_count;
                }
            }
        }
    }

    if (signal.anchor_count > 0u) {
        Theron_V1_Level initial_level;
        Theron_Track02LevelHandoff initial_handoff;
        Theron_Track02LevelHandoffStatus initial_status;

        initial_status = theron_v1_track02_load_initial_level_candidate(
            data,
            size,
            expected_md5,
            signal.descriptor_offsets[0],
            THERON_DUNGEON_1_HALL_OF_RECORDS,
            0,
            &initial_level,
            &initial_handoff);
        receipt->initial_candidate_binding_status =
            initial_handoff.binding_status;
        receipt->initial_candidate_count =
            (uint64_t)initial_handoff.candidate_count;
        receipt->initial_candidate_expected_offset =
            (uint64_t)initial_handoff.expected_offset;
        receipt->initial_candidate_user_data_offset_valid =
            initial_handoff.user_data_offset_valid;
        receipt->initial_candidate_user_data_offset =
            (uint64_t)initial_handoff.user_data_offset;
        if (initial_status == THERON_TRACK02_LEVEL_HANDOFF_OK &&
            initial_handoff.binding_status ==
                THERON_TRACK02_LEVEL_HANDOFF_OK &&
            initial_handoff.absolute_offset ==
                initial_handoff.expected_offset) {
            receipt->initial_candidate_found = 1;
            receipt->initial_candidate_offset =
                (uint64_t)initial_handoff.absolute_offset;
            receipt->initial_candidate_size =
                (uint64_t)initial_handoff.byte_count;
            receipt->initial_candidate_width =
                (uint32_t)initial_handoff.header_width;
            receipt->initial_candidate_height =
                (uint32_t)initial_handoff.header_height;
            receipt->initial_candidate_seed = initial_handoff.header_seed;
            receipt->initial_candidate_level_index =
                (uint32_t)initial_handoff.header_level_index;
            receipt->initial_candidate_start_x = initial_level.start_x;
            receipt->initial_candidate_start_y = initial_level.start_y;
            receipt->initial_candidate_start_dir = initial_level.start_dir;
            receipt->initial_candidate_descriptor_delta =
                (uint64_t)initial_handoff.descriptor_delta;
            receipt->initial_candidate_anchor_match =
                initial_handoff.matches_initial_anchor;
        }
    }

    free(data);
    data = NULL;

    set_verdict(receipt,
                THERON_V1_STARTUP_RECEIPT_REAL_ASSET_RECEIPT,
                "real-asset-receipt");
    safe_str_copy(receipt->source_evidence,
                  sizeof(receipt->source_evidence),
                  "theron_v1_startup_receipt.c real-asset receipt; "
                  "boot direct-launch + Track 02 bank-signal decoder; "
                  "MD5: ");
    {
        size_t evid_len = strlen(receipt->source_evidence);
        size_t md5_len = strlen(expected_md5);
        if (evid_len + md5_len < sizeof(receipt->source_evidence)) {
            memcpy(receipt->source_evidence + evid_len,
                   expected_md5,
                   md5_len + 1u);
        }
    }

    receipt->session_tick_token =
        theron_v1_startup_receipt_session_tick(receipt);
    return 1;
}

/* ── Session tick token ──────────────────────────────────────────── */

uint32_t theron_v1_startup_receipt_session_tick(const Theron_V1_StartupReceipt *receipt) {
    /* FNV-1a seed chosen so the placeholder token is non-zero; this is
     * not a security property — only a CI log de-dup helper. */
    uint32_t h = 0x811c9dc5u ^ 0xA110CAFEu;
    if (!receipt) return h;
    /* Walk every primitive + string field deterministically.  The order
     * is fixed; do not reorder without bumping the seed. */
    h = fnv1a_32(&receipt->verdict, sizeof(receipt->verdict), h);
    h = fnv1a_str(h, receipt->verdict_name);
    h = fnv1a_32(&receipt->variant, sizeof(receipt->variant), h);
    h = fnv1a_str(h, receipt->variant_name);
    h = fnv1a_str(h, receipt->track02_path);
    h = fnv1a_str(h, receipt->track02_md5_hex);
    h = fnv1a_32(&receipt->track02_byte_count,
                 sizeof(receipt->track02_byte_count), h);
    h = fnv1a_32(&receipt->descriptor_offset,
                 sizeof(receipt->descriptor_offset), h);
    h = fnv1a_32(&receipt->descriptor_size,
                 sizeof(receipt->descriptor_size), h);
    h = fnv1a_32(&receipt->descriptor_value_count,
                 sizeof(receipt->descriptor_value_count), h);
    h = fnv1a_32(&receipt->descriptor_stride,
                 sizeof(receipt->descriptor_stride), h);
    h = fnv1a_32(&receipt->anchor_count,
                 sizeof(receipt->anchor_count), h);
    h = fnv1a_32(&receipt->post_boundary_span_offset,
                 sizeof(receipt->post_boundary_span_offset), h);
    h = fnv1a_32(&receipt->post_boundary_span_size,
                 sizeof(receipt->post_boundary_span_size), h);
    h = fnv1a_32(&receipt->next_nonzero_offset,
                 sizeof(receipt->next_nonzero_offset), h);
    h = fnv1a_32(&receipt->descriptor_role_zero_fill_count,
                 sizeof(receipt->descriptor_role_zero_fill_count), h);
    h = fnv1a_32(&receipt->descriptor_role_pre_data_count,
                 sizeof(receipt->descriptor_role_pre_data_count), h);
    h = fnv1a_32(&receipt->descriptor_role_post_data_count,
                 sizeof(receipt->descriptor_role_post_data_count), h);
    h = fnv1a_32(&receipt->descriptor_role_descriptor_table_count,
                 sizeof(receipt->descriptor_role_descriptor_table_count), h);
    h = fnv1a_32(&receipt->descriptor_window_entry_index,
                 sizeof(receipt->descriptor_window_entry_index), h);
    h = fnv1a_32(&receipt->descriptor_byte_before,
                 sizeof(receipt->descriptor_byte_before), h);
    h = fnv1a_32(&receipt->descriptor_byte_before_is_rts,
                 sizeof(receipt->descriptor_byte_before_is_rts), h);
    h = fnv1a_32(&receipt->descriptor_first_nonzero_after,
                 sizeof(receipt->descriptor_first_nonzero_after), h);
    h = fnv1a_32(&receipt->descriptor_all_zero_after,
                 sizeof(receipt->descriptor_all_zero_after), h);
    h = fnv1a_32(&receipt->user_data_window_count,
                 sizeof(receipt->user_data_window_count), h);
    h = fnv1a_32(&receipt->user_data_window_descriptor_count,
                 sizeof(receipt->user_data_window_descriptor_count), h);
    h = fnv1a_32(&receipt->user_data_window_span_count,
                 sizeof(receipt->user_data_window_span_count), h);
    h = fnv1a_32(&receipt->user_data_window_initial_count,
                 sizeof(receipt->user_data_window_initial_count), h);
    h = fnv1a_32(&receipt->user_data_window_overflow_count,
                 sizeof(receipt->user_data_window_overflow_count), h);
    h = fnv1a_32(&receipt->startup_text_marker_count,
                 sizeof(receipt->startup_text_marker_count), h);
    h = fnv1a_32(&receipt->startup_text_us_prompt_count,
                 sizeof(receipt->startup_text_us_prompt_count), h);
    h = fnv1a_32(&receipt->startup_text_jp_roster_count,
                 sizeof(receipt->startup_text_jp_roster_count), h);
    h = fnv1a_32(&receipt->startup_text_marker_overflow_count,
                 sizeof(receipt->startup_text_marker_overflow_count), h);
    h = fnv1a_32(&receipt->startup_roster_name_count,
                 sizeof(receipt->startup_roster_name_count), h);
    h = fnv1a_32(&receipt->startup_roster_title_count,
                 sizeof(receipt->startup_roster_title_count), h);
    h = fnv1a_32(&receipt->startup_roster_overflow_count,
                 sizeof(receipt->startup_roster_overflow_count), h);
    h = fnv1a_32(&receipt->initial_candidate_found,
                 sizeof(receipt->initial_candidate_found), h);
    h = fnv1a_32(&receipt->initial_candidate_offset,
                 sizeof(receipt->initial_candidate_offset), h);
    h = fnv1a_32(&receipt->initial_candidate_size,
                 sizeof(receipt->initial_candidate_size), h);
    h = fnv1a_32(&receipt->initial_candidate_width,
                 sizeof(receipt->initial_candidate_width), h);
    h = fnv1a_32(&receipt->initial_candidate_height,
                 sizeof(receipt->initial_candidate_height), h);
    h = fnv1a_32(&receipt->initial_candidate_seed,
                 sizeof(receipt->initial_candidate_seed), h);
    h = fnv1a_32(&receipt->initial_candidate_level_index,
                 sizeof(receipt->initial_candidate_level_index), h);
    h = fnv1a_32(&receipt->initial_candidate_start_x,
                 sizeof(receipt->initial_candidate_start_x), h);
    h = fnv1a_32(&receipt->initial_candidate_start_y,
                 sizeof(receipt->initial_candidate_start_y), h);
    h = fnv1a_32(&receipt->initial_candidate_start_dir,
                 sizeof(receipt->initial_candidate_start_dir), h);
    h = fnv1a_32(&receipt->initial_candidate_descriptor_delta,
                 sizeof(receipt->initial_candidate_descriptor_delta), h);
    h = fnv1a_32(&receipt->initial_candidate_anchor_match,
                 sizeof(receipt->initial_candidate_anchor_match), h);
    h = fnv1a_32(&receipt->initial_candidate_binding_status,
                 sizeof(receipt->initial_candidate_binding_status), h);
    h = fnv1a_32(&receipt->initial_candidate_count,
                 sizeof(receipt->initial_candidate_count), h);
    h = fnv1a_32(&receipt->initial_candidate_expected_offset,
                 sizeof(receipt->initial_candidate_expected_offset), h);
    h = fnv1a_32(&receipt->initial_candidate_user_data_offset_valid,
                 sizeof(receipt->initial_candidate_user_data_offset_valid), h);
    h = fnv1a_32(&receipt->initial_candidate_user_data_offset,
                 sizeof(receipt->initial_candidate_user_data_offset), h);
    h = fnv1a_32(&receipt->startup_mirror_count,
                 sizeof(receipt->startup_mirror_count), h);
    h = fnv1a_32(&receipt->startup_companion_limit,
                 sizeof(receipt->startup_companion_limit), h);
    h = fnv1a_32(&receipt->startup_portrait_min,
                 sizeof(receipt->startup_portrait_min), h);
    h = fnv1a_32(&receipt->startup_portrait_max,
                 sizeof(receipt->startup_portrait_max), h);
    h = fnv1a_32(&receipt->startup_class_mask,
                 sizeof(receipt->startup_class_mask), h);
    h = fnv1a_32(&receipt->startup_fallback_label_count,
                 sizeof(receipt->startup_fallback_label_count), h);
    h = fnv1a_32(&receipt->startup_decoded_art_count,
                 sizeof(receipt->startup_decoded_art_count), h);
    h = fnv1a_str(h, receipt->startup_chapter_label);
    h = fnv1a_str(h, receipt->startup_quest_summary);
    h = fnv1a_str(h, receipt->startup_next_dungeon_hint);
    h = fnv1a_32(&receipt->startup_quest_item_total,
                 sizeof(receipt->startup_quest_item_total), h);
    h = fnv1a_32(&receipt->startup_quest_items_collected,
                 sizeof(receipt->startup_quest_items_collected), h);
    h = fnv1a_32(&receipt->boot_profile_platform,
                 sizeof(receipt->boot_profile_platform), h);
    h = fnv1a_str(h, receipt->boot_profile_version_id);
    h = fnv1a_32(&receipt->boot_profile_assets_verified,
                 sizeof(receipt->boot_profile_assets_verified), h);
    h = fnv1a_32(&receipt->boot_profile_tick_rate_hz,
                 sizeof(receipt->boot_profile_tick_rate_hz), h);
    h = fnv1a_32(&receipt->boot_profile_max_champions,
                 sizeof(receipt->boot_profile_max_champions), h);
    h = fnv1a_32(&receipt->boot_profile_dungeon_count,
                 sizeof(receipt->boot_profile_dungeon_count), h);
    h = fnv1a_32(&receipt->boot_profile_dungeon_seed,
                 sizeof(receipt->boot_profile_dungeon_seed), h);
    h = fnv1a_32(&receipt->m11_dispatch_source_kind,
                 sizeof(receipt->m11_dispatch_source_kind), h);
    h = fnv1a_32(&receipt->m11_view_active,
                 sizeof(receipt->m11_view_active), h);
    h = fnv1a_32(&receipt->m11_world_present,
                 sizeof(receipt->m11_world_present), h);
    h = fnv1a_str(h, receipt->skip_reason_note);
    h = fnv1a_str(h, receipt->source_evidence);
    return h;
}

/* ── One-line ASCII renderer ─────────────────────────────────────── */

size_t theron_v1_startup_receipt_to_line(const Theron_V1_StartupReceipt *receipt,
                                          char *buf, size_t buf_size) {
    int n;
    if (!receipt || !buf || buf_size == 0) return 0;
    n = snprintf(buf, buf_size,
                 "verdict=%s variant=%s path=%s md5=%s bytes=%llu "
                 "descriptor_off=0x%llx descriptor_size=%llu "
                 "value_count=%llu stride=0x%x anchors=%llu "
                 "span_off=0x%llx span_size=%llu next_nonzero=0x%llx "
                 "roles=z%u/pre%u/post%u/desc%u desc_entry=%d "
                 "desc_prev=0x%x desc_prev_rts=%d "
                 "desc_first_after=0x%llx desc_zero_after=%d "
                 "user_windows=%u user_desc=%u user_span=%u "
                 "user_initial=%u user_overflow=%u "
                 "startup_text_markers=%u startup_text_us=%u "
                 "startup_text_jp=%u startup_text_overflow=%u "
                 "startup_roster_names=%u startup_roster_titles=%u "
                 "startup_roster_overflow=%u "
                 "initial_candidate=%d initial_off=0x%llx "
                 "initial_size=%llu initial_header=%ux%u "
                 "initial_seed=0x%x initial_level=0x%x "
                 "initial_start=(%d,%d,%d) initial_delta=0x%llx "
                 "initial_anchor=%d initial_bind=%d initial_bind_name=%s "
                 "initial_count=%llu initial_expected=0x%llx "
                 "initial_user_valid=%d initial_user_off=0x%llx "
                 "mirrors=%u companions=%u portrait_range=%u..%u "
                 "class_mask=0x%x mirror_fallback_labels=%u "
                 "mirror_decoded_art=%u chapter=\"%s\" "
                 "quest=\"%s\" next=\"%s\" quest_total=%u "
                 "quest_items=0x%x "
                 "boot_platform=%d boot_version=%s boot_verified=%d "
                 "tick_hz=%u champions=%u dungeons=%u seed=%u "
                 "m11_kind=%d m11_active=%d m11_world=%d "
                 "note=\"%s\" session_tick=0x%08x",
                 receipt->verdict_name,
                 receipt->variant_name,
                 receipt->track02_path[0] ? receipt->track02_path : "(none)",
                 receipt->track02_md5_hex[0] ? receipt->track02_md5_hex
                                              : "(none)",
                 (unsigned long long)receipt->track02_byte_count,
                 (unsigned long long)receipt->descriptor_offset,
                 (unsigned long long)receipt->descriptor_size,
                 (unsigned long long)receipt->descriptor_value_count,
                 (unsigned)receipt->descriptor_stride,
                 (unsigned long long)receipt->anchor_count,
                 (unsigned long long)receipt->post_boundary_span_offset,
                 (unsigned long long)receipt->post_boundary_span_size,
                 (unsigned long long)receipt->next_nonzero_offset,
                 (unsigned)receipt->descriptor_role_zero_fill_count,
                 (unsigned)receipt->descriptor_role_pre_data_count,
                 (unsigned)receipt->descriptor_role_post_data_count,
                 (unsigned)receipt->descriptor_role_descriptor_table_count,
                 (int)receipt->descriptor_window_entry_index,
                 (unsigned)receipt->descriptor_byte_before,
                 receipt->descriptor_byte_before_is_rts,
                 (unsigned long long)receipt->descriptor_first_nonzero_after,
                 receipt->descriptor_all_zero_after,
                 (unsigned)receipt->user_data_window_count,
                 (unsigned)receipt->user_data_window_descriptor_count,
                 (unsigned)receipt->user_data_window_span_count,
                 (unsigned)receipt->user_data_window_initial_count,
                 (unsigned)receipt->user_data_window_overflow_count,
                 (unsigned)receipt->startup_text_marker_count,
                 (unsigned)receipt->startup_text_us_prompt_count,
                 (unsigned)receipt->startup_text_jp_roster_count,
                 (unsigned)receipt->startup_text_marker_overflow_count,
                 (unsigned)receipt->startup_roster_name_count,
                 (unsigned)receipt->startup_roster_title_count,
                 (unsigned)receipt->startup_roster_overflow_count,
                 receipt->initial_candidate_found,
                 (unsigned long long)receipt->initial_candidate_offset,
                 (unsigned long long)receipt->initial_candidate_size,
                 (unsigned)receipt->initial_candidate_width,
                 (unsigned)receipt->initial_candidate_height,
                 (unsigned)receipt->initial_candidate_seed,
                 (unsigned)receipt->initial_candidate_level_index,
                 receipt->initial_candidate_start_x,
                 receipt->initial_candidate_start_y,
                 receipt->initial_candidate_start_dir,
                 (unsigned long long)receipt->initial_candidate_descriptor_delta,
                 receipt->initial_candidate_anchor_match,
                 (int)receipt->initial_candidate_binding_status,
                 theron_v1_track02_level_handoff_status_name(
                     (Theron_Track02LevelHandoffStatus)
                         receipt->initial_candidate_binding_status),
                 (unsigned long long)receipt->initial_candidate_count,
                 (unsigned long long)receipt->initial_candidate_expected_offset,
                 receipt->initial_candidate_user_data_offset_valid,
                 (unsigned long long)receipt->initial_candidate_user_data_offset,
                 (unsigned)receipt->startup_mirror_count,
                 (unsigned)receipt->startup_companion_limit,
                 (unsigned)receipt->startup_portrait_min,
                 (unsigned)receipt->startup_portrait_max,
                 (unsigned)receipt->startup_class_mask,
                 (unsigned)receipt->startup_fallback_label_count,
                 (unsigned)receipt->startup_decoded_art_count,
                 receipt->startup_chapter_label[0]
                    ? receipt->startup_chapter_label : "(none)",
                 receipt->startup_quest_summary[0]
                    ? receipt->startup_quest_summary : "(none)",
                 receipt->startup_next_dungeon_hint[0]
                    ? receipt->startup_next_dungeon_hint : "(none)",
                 (unsigned)receipt->startup_quest_item_total,
                 (unsigned)receipt->startup_quest_items_collected,
                 (int)receipt->boot_profile_platform,
                 receipt->boot_profile_version_id[0]
                    ? receipt->boot_profile_version_id : "(none)",
                 receipt->boot_profile_assets_verified,
                 (unsigned)receipt->boot_profile_tick_rate_hz,
                 (unsigned)receipt->boot_profile_max_champions,
                 (unsigned)receipt->boot_profile_dungeon_count,
                 (unsigned)receipt->boot_profile_dungeon_seed,
                 (int)receipt->m11_dispatch_source_kind,
                 receipt->m11_view_active,
                 receipt->m11_world_present,
                 receipt->skip_reason_note[0]
                    ? receipt->skip_reason_note : "(none)",
                 (unsigned)receipt->session_tick_token);
    if (n < 0) return 0;
    if ((size_t)n >= buf_size) return buf_size > 0 ? buf_size - 1 : 0;
    return (size_t)n;
}

const char *theron_v1_startup_receipt_source_evidence(void) {
    return "theron_v1_startup_receipt.c: hashes-only receipt surface over "
           "src/theron/theron_v1_track02.c (Track 02 bank-signal decoder), "
           "src/theron/theron_v1_boot.c (boot profile + direct launch), "
           "include/asset_status_m12.h m12_file_md5_hex (file MD5 helper), "
           "and the four known TQ Track 02 MD5s "
           THERON_TRACK02_MD5_JP_BIN " / "
           THERON_TRACK02_MD5_US_BIN " / "
           THERON_TRACK02_MD5_JP_REV1_ISO " / "
           THERON_TRACK02_MD5_US_ISO
           " (mirrored from src/shared/asset_status_m12.c g_theronVersions). "
           "ReDMCSB has no Theron code; the Theron-side evidence is local "
           "byte inspection of those four Track 02 binaries/ISOs and the "
           "docs/source-lock/tqr_v1_phase{0,1,2}_*.md provenance notes. "
           "Honest scope: receipt + boot + bank-signal summary only; no "
           "M11 dispatch tick, no Track 02 dungeon-record decode, no "
           "playability claim, no README screenshot promotion.";
}
