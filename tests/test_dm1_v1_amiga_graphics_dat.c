#include "dm1_v1_amiga_graphics_dat.h"
#include "dm1_v1_original_save_amiga_handoff.h"
#include "dm1_v1_original_save_classifier.h"
#include "firestaff_amiga_adf.h"
#include "firestaff_zip_extract.h"
#include "memory_dungeon_dat_pc34_compat.h"
#include "memory_tick_orchestrator_pc34_compat.h"
#include "dm1_v1_event_timer_pc34_compat.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_pass = 0, g_fail = 0;
#define CHECK(cond, msg) do { \
    if (cond) { g_pass++; } \
    else { g_fail++; printf("FAIL %s\n", msg); } \
} while (0)

static uint16_t test_read_be16(const uint8_t *bytes) {
    return (uint16_t)(((uint16_t)bytes[0] << 8) | bytes[1]);
}

/* F0434 stores map descriptors, the compact per-column table, SFT and text
 * ahead of DB0..DB15.  This test-only cursor intentionally follows the A20
 * big-endian header itself, so a host-endian loader cannot accidentally make
 * the Group/Projectile byte-field assertion pass. */
static size_t test_amiga_tail_thing_offset(const uint8_t *tail,
                                           size_t tail_size,
                                           unsigned int wanted_type) {
    static const uint8_t record_bytes[16] = {
        4u, 6u, 4u, 8u, 16u, 4u, 4u, 4u,
        4u, 8u, 4u, 0u, 0u, 0u, 8u, 4u
    };
    size_t cursor;
    unsigned int columns = 0u;
    unsigned int map;
    unsigned int type;
    if (!tail || wanted_type >= 16u || tail_size < 44u) return 0u;
    for (map = 0u; map < tail[4u]; ++map) {
        size_t map_offset = 44u + (size_t)map * 16u;
        if (map_offset + 10u > tail_size) return 0u;
        columns += ((unsigned int)(test_read_be16(tail + map_offset + 8u) >> 6u) & 0x1fu) + 1u;
    }
    cursor = 44u + (size_t)tail[4u] * 16u + (size_t)columns * 2u +
             (size_t)test_read_be16(tail + 10u) * 2u +
             (size_t)test_read_be16(tail + 6u) * 2u;
    for (type = 0u; type < wanted_type; ++type) {
        cursor += (size_t)test_read_be16(tail + 12u + type * 2u) *
                  record_bytes[type];
    }
    return cursor <= tail_size ? cursor : 0u;
}

static void test_null_rejection(void) {
    CHECK(dm1_v1_amiga_graphics_probe(NULL, 0) == 0, "null_data");
    CHECK(dm1_v1_amiga_graphics_probe(NULL, 1000) == 0, "null_data_nonzero_size");
}

static void test_small_rejection(void) {
    uint8_t buf[4] = {0x02, 0x3f, 0x00, 0x00};
    CHECK(dm1_v1_amiga_graphics_probe(buf, 4) == 0, "too_small");
}

static void test_wrong_count(void) {
    uint8_t buf[400000];
    memset(buf, 0, sizeof(buf));
    buf[0] = 0x00; buf[1] = 0x01; /* count=1 in BE */
    CHECK(dm1_v1_amiga_graphics_probe(buf, 400000) == 0, "wrong_count");
}

static void test_pc34_format_rejection(void) {
    uint8_t buf[400000];
    memset(buf, 0, sizeof(buf));
    buf[0] = 0x01; buf[1] = 0x80; /* 0x8001 LE marker */
    CHECK(dm1_v1_amiga_graphics_probe(buf, 400000) == 0, "pc34_marker");
}

static void test_synthetic_probe(void) {
    /* Build a synthetic Amiga GRAPHICS.DAT with 575 graphics, all 0 bytes */
    uint16_t count = 575;
    size_t header = 2 + (size_t)count * 4;
    uint8_t *buf = calloc(header, 1);
    buf[0] = (uint8_t)(count >> 8);
    buf[1] = (uint8_t)(count & 0xff);
    /* All comp/decomp are 0, data area = 0, size = header */
    CHECK(dm1_v1_amiga_graphics_probe(buf, header) == 0, "synthetic_too_small");
    free(buf);
}

static void test_synthetic_valid(void) {
    uint16_t count = 575;
    size_t header = 2 + (size_t)count * 4;
    size_t data_per_item = 700;
    size_t total = header + (size_t)count * data_per_item;
    if (total < 350000 || total > 420000) {
        printf("SKIP synthetic_valid: size %zu out of range\n", total);
        return;
    }
    uint8_t *buf = calloc(total, 1);
    buf[0] = (uint8_t)(count >> 8);
    buf[1] = (uint8_t)(count & 0xff);
    for (uint16_t i = 0; i < count; i++) {
        uint16_t sz = (uint16_t)data_per_item;
        /* comp sizes (BE) */
        buf[2 + i * 2 + 0] = (uint8_t)(sz >> 8);
        buf[2 + i * 2 + 1] = (uint8_t)(sz & 0xff);
        /* decomp sizes (BE) */
        buf[2 + count * 2 + i * 2 + 0] = (uint8_t)(sz >> 8);
        buf[2 + count * 2 + i * 2 + 1] = (uint8_t)(sz & 0xff);
    }
    CHECK(dm1_v1_amiga_graphics_probe(buf, total) == 1, "synthetic_valid_probe");

    DM1_V1_AmigaGraphicsReceipt r;
    CHECK(dm1_v1_amiga_graphics_receipt(buf, total, &r) == 0, "synthetic_valid_receipt");
    CHECK(r.is_amiga == 1, "synthetic_is_amiga");
    CHECK(r.graphic_count == 575, "synthetic_count");
    CHECK(r.lang == DM1_AMIGA_LANG_UNKNOWN, "synthetic_lang_unknown");
    free(buf);
}

static void test_receipt_null(void) {
    CHECK(dm1_v1_amiga_graphics_receipt(NULL, 0, NULL) == -1, "receipt_null");
}

static void test_compressed_rejection(void) {
    uint16_t count = 575;
    size_t header = 2 + (size_t)count * 4;
    size_t total = header + 400000;
    uint8_t *buf = calloc(total, 1);
    buf[0] = (uint8_t)(count >> 8);
    buf[1] = (uint8_t)(count & 0xff);
    /* comp[0] != decomp[0] */
    buf[2] = 0x00; buf[3] = 0x10;
    buf[2 + count * 2] = 0x00; buf[2 + count * 2 + 1] = 0x20;
    CHECK(dm1_v1_amiga_graphics_probe(buf, total) == 0, "compressed_rejected");
    free(buf);
}

typedef struct {
    int found;
    int valid;
    int executable_found;
    unsigned int immediate_color_writes;
    unsigned int copper_color_base_writes;
    unsigned int copper_caller_palette_handoffs;
    unsigned int copper_builder_calls;
    unsigned int copper_fade_producer_signatures;
    unsigned int copper_fade_builder_calls;
    int copper_builder_found;
    size_t copper_builder_offset;
    DM1_V1_AmigaGraphicsReceipt receipt;
    uint8_t *bytes;
    size_t size;
} RealGraphicsReceipt;

typedef struct {
    unsigned int file_count;
    size_t byte_count;
    int primary_classified;
    DM1OriginalSaveClassifyResult primary;
    int primary_f0435_result;
    Dm1V1AmigaSaveF0435Receipt primary_f0435;
    int primary_global_result;
    Dm1V1AmigaSaveF0435GlobalData primary_global;
    int primary_party_result;
    uint8_t primary_party[DM1_V1_AMIGA_SAVE_F0435_PARTY_BYTES];
    int primary_party_receipt_result;
    Dm1V1AmigaSavePartyReceipt primary_party_receipt;
    int primary_queue_receipt_result;
    Dm1V1AmigaSaveRuntimeQueueReceipt primary_queue_receipt;
    int primary_event_queue_result;
    struct DM1_EventQueue_V1 primary_event_queue;
    int primary_runtime_party_result;
    struct PartyState_Compat primary_runtime_party;
    uint8_t *primary_bytes;
    size_t primary_size;
    int backup_f0435_result;
    Dm1V1AmigaSaveF0435Receipt backup_f0435;
} RealSaveDiskReceipt;

static int real_save_disk_visitor(const char *name, const uint8_t *bytes,
                                  size_t size, void *user_data) {
    RealSaveDiskReceipt *receipt = (RealSaveDiskReceipt *)user_data;
    if (!name || !bytes || !receipt) return -1;
    ++receipt->file_count;
    receipt->byte_count += size;
    printf("AMIGA-SAVE-DISK file=%s bytes=%zu\n", name, size);
    if (strcmp(name, "DMGAMEG.DAT") == 0) {
        receipt->primary_bytes = malloc(size);
        if (!receipt->primary_bytes) return -1;
        memcpy(receipt->primary_bytes, bytes, size);
        receipt->primary_size = size;
        receipt->primary_classified =
            dm1_v1_original_save_classify_bytes(bytes, size,
                                                &receipt->primary);
        printf("AMIGA-SAVE-DISK primary shape=%s readiness=%s format=%u "
               "platform=%u dungeon=%u checksum=%d reason=%s\n",
               dm1_v1_original_save_shape_name(receipt->primary.shape),
               dm1_v1_original_save_readiness_name(receipt->primary.readiness),
               (unsigned)receipt->primary.format_id,
               (unsigned)receipt->primary.platform,
               (unsigned)receipt->primary.dungeon_id,
               receipt->primary.header_checksum_ok,
               receipt->primary.reason);
        receipt->primary_f0435_result =
            dm1_v1_original_save_amiga_f0435_receipt_bytes(
                bytes, size, &receipt->primary_f0435);
        receipt->primary_global_result =
            dm1_v1_original_save_amiga_f0435_global_data_bytes(
                bytes, size, &receipt->primary_global, NULL);
        receipt->primary_party_result =
            dm1_v1_original_save_amiga_f0435_party_part_bytes(
                bytes, size, receipt->primary_party, NULL);
        receipt->primary_party_receipt_result =
            dm1_v1_original_save_amiga_f0435_party_receipt_bytes(
                bytes, size, &receipt->primary_party_receipt, NULL);
        receipt->primary_queue_receipt_result =
            dm1_v1_original_save_amiga_f0435_runtime_queue_receipt_bytes(
                bytes, size, &receipt->primary_queue_receipt, NULL);
        receipt->primary_event_queue_result =
            dm1_v1_original_save_amiga_f0435_materialize_event_queue_bytes(
                bytes, size, &receipt->primary_event_queue, NULL);
        receipt->primary_runtime_party_result =
            dm1_v1_original_save_amiga_f0435_materialize_party_bytes(
                bytes, size, &receipt->primary_runtime_party, NULL);
        printf("AMIGA-SAVE-DISK primary F0435=%s parts=%u body_end=%u "
               "trailing=%u time=%u party=%u pose=%d,%d,%d map=%d events=%u/%u groups=%u/%u "
               "tail=%d dungeon=%u+%u maps=%u columns=%u raw=%u checksum=%04x/%04x\n",
               dm1_v1_original_save_amiga_f0435_result_name(
                   receipt->primary_f0435_result),
               (unsigned)receipt->primary_f0435.parts_authenticated,
               (unsigned)receipt->primary_f0435.authenticated_body_end_offset,
               (unsigned)receipt->primary_f0435.trailing_source_byte_count,
               (unsigned)receipt->primary_f0435.game_time,
               (unsigned)receipt->primary_f0435.party_champion_count,
               (int)receipt->primary_f0435.party_map_x,
               (int)receipt->primary_f0435.party_map_y,
               (int)receipt->primary_f0435.party_direction,
               (int)receipt->primary_f0435.party_map_index,
               (unsigned)receipt->primary_f0435.event_count,
               (unsigned)receipt->primary_f0435.event_maximum_count,
               (unsigned)receipt->primary_f0435.current_active_group_count,
               (unsigned)receipt->primary_f0435.maximum_active_group_count,
               receipt->primary_f0435.tail_authenticated,
               (unsigned)receipt->primary_f0435.dungeon_offset,
               (unsigned)receipt->primary_f0435.dungeon_byte_count,
               (unsigned)receipt->primary_f0435.dungeon_map_count,
               (unsigned)receipt->primary_f0435.dungeon_column_count,
               (unsigned)receipt->primary_f0435.dungeon_raw_map_byte_count,
               (unsigned)receipt->primary_f0435.dungeon_expected_checksum,
               (unsigned)receipt->primary_f0435.dungeon_actual_checksum);
        if (receipt->primary_queue_receipt_result ==
            DM1_V1_AMIGA_SAVE_F0435_OK) {
            printf("AMIGA-SAVE-DISK queue first_event time=%u type=%u priority=%u pos=%u,%u cell=%u effect=%u\n",
                   (unsigned)receipt->primary_queue_receipt.first_scheduled_map_time,
                   (unsigned)receipt->primary_queue_receipt.first_scheduled_type,
                   (unsigned)receipt->primary_queue_receipt.first_scheduled_priority,
                   (unsigned)receipt->primary_queue_receipt.first_scheduled_map_x,
                   (unsigned)receipt->primary_queue_receipt.first_scheduled_map_y,
                   (unsigned)receipt->primary_queue_receipt.first_scheduled_cell,
                   (unsigned)receipt->primary_queue_receipt.first_scheduled_effect);
        }
    } else if (strcmp(name, "DMGAMEG.BAK") == 0) {
        receipt->backup_f0435_result =
            dm1_v1_original_save_amiga_f0435_receipt_bytes(
                bytes, size, &receipt->backup_f0435);
        printf("AMIGA-SAVE-DISK backup F0435=%s parts=%u body_end=%u trailing=%u\n",
               dm1_v1_original_save_amiga_f0435_result_name(
                   receipt->backup_f0435_result),
               (unsigned)receipt->backup_f0435.parts_authenticated,
               (unsigned)receipt->backup_f0435.authenticated_body_end_offset,
               (unsigned)receipt->backup_f0435.trailing_source_byte_count);
    }
    return 0;
}

/* 68000 JSR d16(PC) is relative to the extension word (two bytes after the
 * opcode), not the end of its four-byte encoding. Keep this tiny decoder
 * local to the in-memory audit: it is a
 * structural receipt for the supplied executable, not a host emulator or a
 * substitute palette producer. */
static int m68k_jsr_pc_relative_target(const uint8_t *bytes, size_t size,
                                       size_t offset, size_t *out_target) {
    int displacement;
    long target;
    if (!bytes || !out_target || offset + 4u > size ||
        bytes[offset] != 0x4eu || bytes[offset + 1u] != 0xbau) {
        return 0;
    }
    displacement = ((int)bytes[offset + 2u] << 8) | bytes[offset + 3u];
    if ((displacement & 0x8000) != 0) displacement -= 0x10000;
    target = (long)offset + 2L + (long)displacement;
    if (target < 0L || (size_t)target >= size) return 0;
    *out_target = (size_t)target;
    return 1;
}

static int real_graphics_visitor(const char *name, const uint8_t *bytes,
                                 size_t size, void *user_data) {
    RealGraphicsReceipt *result = (RealGraphicsReceipt *)user_data;
    const char *disassembly = getenv("FIRESTAFF_DM1_AMIGA_DISASSEMBLY");
    if (!name || !bytes || !result) {
        return 1;
    }
    /* Amiga OCS COLOR00..COLOR31 live at 0xdff180..0xdff1be.  This optional
     * in-memory probe records the original executable's palette route; it
     * never materializes an ADF member.  DM 2.0 constructs Copper entries
     * with ADD.L #$00dff180,D0 rather than embedding MOVE.W #rgb,COLORxx. */
    if (disassembly && disassembly[0] && strcmp(name, "dm") == 0) {
        result->executable_found = 1;
        printf("AMIGA-DISASM dm bytes=%zu\n", size);
        for (size_t i = 0u; i + 10u <= size; ++i) {
            unsigned int register_offset;
            unsigned int rgb4;
            if (bytes[i] != 0x33u || bytes[i + 1u] != 0xfcu ||
                bytes[i + 4u] != 0x00u || bytes[i + 5u] != 0xdfu ||
                bytes[i + 6u] != 0xf1u || bytes[i + 7u] < 0x80u ||
                bytes[i + 7u] > 0xbeu || (bytes[i + 7u] & 1u) != 0u) {
                continue;
            }
            rgb4 = ((unsigned int)bytes[i + 2u] << 8) | bytes[i + 3u];
            register_offset = (unsigned int)bytes[i + 7u] - 0x80u;
            printf("AMIGA-DISASM move.w #$%03x,COLOR%u @0x%zx\n",
                   rgb4 & 0xfffu, register_offset / 2u, i);
            ++result->immediate_color_writes;
        }
        /* The dynamic producer begins by copying the saved source table
         * (-0x204c(A4)) to the 16-word working table (-0x2048(A4)).  Its
         * three RGB4 component loops then update that working table before
         * the original code calls the Copper builder eight times. */
        for (size_t i = 0u; i + 30u <= size; ++i) {
            static const uint8_t fade_producer_prefix[] = {
                0x4e, 0x55, 0x00, 0x00, 0x48, 0xe7, 0x0f, 0x20,
                0x41, 0xec, 0xdf, 0xb8, 0x24, 0x48, 0x7c, 0x00,
                0x30, 0x06, 0x48, 0xc0, 0xe3, 0x80, 0x20, 0x6c,
                0xdf, 0xb4, 0x32, 0x06, 0x48, 0xc1
            };
            if (memcmp(bytes + i, fade_producer_prefix,
                       sizeof(fade_producer_prefix)) == 0) {
                ++result->copper_fade_producer_signatures;
                printf("AMIGA-DISASM RGB4 fade producer @0x%zx\n", i);
            }
        }
        for (size_t i = 0u; i + 6u <= size; ++i) {
            if (bytes[i + 0u] == 0xd0u && bytes[i + 1u] == 0xbcu &&
                bytes[i + 2u] == 0x00u && bytes[i + 3u] == 0xdfu &&
                bytes[i + 4u] == 0xf1u && bytes[i + 5u] == 0x80u) {
                printf("AMIGA-DISASM add.l #$00dff180,D0 @0x%zx\n", i);
                ++result->copper_color_base_writes;
            }
        }
        /* The builder's prologue transfers its caller's word-table pointer
         * from 12(A5), then its first loop reads 16 words from that table
         * before forming COLOR00..COLOR15 Copper addresses.  This is an
         * original dynamic gameplay-palette handoff, not a static palette
         * embedded in GRAPHICS.DAT or a legitimate PC-VGA fallback. */
        for (size_t i = 0u; i + 10u <= size; ++i) {
            if (bytes[i + 0u] == 0x4eu && bytes[i + 1u] == 0x55u &&
                bytes[i + 2u] == 0x00u && bytes[i + 3u] == 0x00u &&
                bytes[i + 4u] == 0x2fu && bytes[i + 5u] == 0x04u &&
                bytes[i + 6u] == 0x29u && bytes[i + 7u] == 0x6du &&
                bytes[i + 8u] == 0x00u && bytes[i + 9u] == 0x0cu) {
                printf("AMIGA-DISASM Copper caller palette handoff @0x%zx\n",
                       i);
                ++result->copper_caller_palette_handoffs;
                result->copper_builder_found = 1;
                result->copper_builder_offset = i;
            }
        }
        /* Resolve every PC-relative JSR that reaches the identified builder.
         * Matching a target, rather than an arbitrary four-byte pattern,
         * makes this an executable control-flow receipt and avoids treating
         * coincidental bytes in data as palette calls. */
        for (size_t i = 0u; i + 4u <= size; ++i) {
            size_t target = 0u;
            if (result->copper_builder_found &&
                m68k_jsr_pc_relative_target(bytes, size, i, &target) &&
                target == result->copper_builder_offset) {
                ++result->copper_builder_calls;
                printf("AMIGA-DISASM palette JSR @0x%zx -> @0x%zx\n",
                       i, target);
                if (i >= 8u && bytes[i - 8u] == 0x48u &&
                    bytes[i - 7u] == 0x6cu && bytes[i - 6u] == 0xdfu &&
                    bytes[i - 5u] == 0xb8u && bytes[i - 4u] == 0x48u &&
                    bytes[i - 3u] == 0x6cu && bytes[i - 2u] == 0xdfu &&
                    bytes[i - 1u] == 0xb8u) {
                    ++result->copper_fade_builder_calls;
                }
            }
        }
        printf("AMIGA-DISASM COLOR immediate writes=%u, Copper COLOR base writes=%u, caller palette handoffs=%u, builder calls=%u, fade producer signatures=%u, fade builder calls=%u\n",
               result->immediate_color_writes,
               result->copper_color_base_writes,
               result->copper_caller_palette_handoffs,
               result->copper_builder_calls,
               result->copper_fade_producer_signatures,
               result->copper_fade_builder_calls);
    }
    if (strcmp(name, "graphics.dat") != 0) return 1;
    result->found = 1;
    result->valid = dm1_v1_amiga_graphics_receipt(bytes, size,
                                                   &result->receipt) == 0;
    if (result->valid) {
        result->bytes = malloc(size);
        if (!result->bytes) return -1;
        memcpy(result->bytes, bytes, size);
        result->size = size;
    }
    return 0;
}

/* The supplied Amiga 2.0 preservation package is ZIP -> ZIP -> ADF. Read
 * its selected disk and GRAPHICS.DAT entirely in memory; no archive member
 * is materialized to the filesystem. */
static void test_real_amiga_v20_graphics_receipt(void) {
    const char *archive = getenv("FIRESTAFF_DM1_AMIGA_V20_ARCHIVE");
    uint8_t *inner = NULL;
    uint8_t *adf = NULL;
    size_t inner_size = 0U;
    size_t adf_size = 0U;
    FILE *stream;
    RealGraphicsReceipt result;

    if (!archive || !archive[0]) {
        printf("SKIP real_amiga_v20_graphics: archive not configured\n");
        return;
    }
    stream = fopen(archive, "rb");
    if (!stream) {
        printf("SKIP real_amiga_v20_graphics: archive unavailable\n");
        return;
    }
    fclose(stream);

    memset(&result, 0, sizeof(result));
    CHECK(firestaff_zip_extract_by_suffix(
              archive, "Dungeon Master v2.0 (1988)(FTL).zip", &inner,
              &inner_size) == 0,
          "real_outer_zip_member");
    if (!inner) return;
    CHECK(firestaff_zip_extract_memory_by_suffix(
              inner, inner_size, "Dungeon Master v2.0 (1988)(FTL).adf",
              &adf, &adf_size) == 0,
          "real_inner_adf_member");
    free(inner);
    if (!adf) return;
    CHECK(firestaff_amiga_adf_visit_ofs_files(adf, adf_size,
                                               real_graphics_visitor,
                                               &result) >= 0,
          "real_adf_visit");
    free(adf);
    CHECK(result.found == 1, "real_graphics_found");
    if (getenv("FIRESTAFF_DM1_AMIGA_DISASSEMBLY")) {
        CHECK(result.executable_found == 1, "real_executable_found");
        CHECK(result.immediate_color_writes == 0u,
              "real_no_immediate_color_writes");
        CHECK(result.copper_color_base_writes == 4u,
              "real_copper_color_base_writes");
        CHECK(result.copper_caller_palette_handoffs == 1u,
              "real_copper_caller_palette_handoff");
        CHECK(result.copper_builder_found == 1,
              "real_copper_builder_found");
        CHECK(result.copper_builder_calls == 2u,
              "real_palette_jsrs_target_copper_builder");
        CHECK(result.copper_fade_producer_signatures == 1u,
              "real_rgb4_fade_producer_signature");
        CHECK(result.copper_fade_builder_calls == 1u,
              "real_rgb4_fade_producer_calls_builder");
    }
    CHECK(result.valid == 1, "real_graphics_receipt");
    if (!result.valid) return;
    CHECK(result.receipt.is_amiga == 1, "real_graphics_is_amiga");
    CHECK(result.receipt.graphic_count == DM1_AMIGA_GRAPHICS_EXPECTED_COUNT,
          "real_graphics_count");
    CHECK(result.receipt.lang == DM1_AMIGA_LANG_EN, "real_graphics_lang_en");
    CHECK(result.receipt.version == DM1_AMIGA_VER_2_0,
          "real_graphics_version_v20");
    {
        uint8_t pixels[640u * 400u];
        uint16_t width = 0u, height = 0u;
        size_t index;
        unsigned int nonzero = 0u;
        CHECK(dm1_v1_amiga_graphics_decode(result.bytes, result.size, 0u,
                                            pixels, sizeof(pixels),
                                            &width, &height) == 1,
              "real_item_000_decode");
        CHECK(width > 0u && height > 0u && width <= 640u && height <= 400u,
              "real_item_000_dimensions");
        for (index = 0u; index < (size_t)width * height; ++index)
            nonzero += pixels[index] != 0u;
        CHECK(nonzero > 0u, "real_item_000_source_pixels");
    }
    free(result.bytes);
}

/* The supplied preservation archive also retains original save-disk ADFs.
 * Inspect the unmodified ordinary save disk in RAM before treating a saved
 * game as a gameplay capture.  A populated file system is useful provenance;
 * it is not, by itself, proof of the Copper palette active on a particular
 * frame. */
static void test_real_amiga_v20_save_disk_receipt(void) {
    const char *archive = getenv("FIRESTAFF_DM1_AMIGA_V20_ARCHIVE");
    static const char save_zip[] =
        "Dungeon Master v2.0 (1988)(FTL)[save disk].zip";
    static const char save_adf[] =
        "Dungeon Master v2.0 (1988)(FTL)[save disk].adf";
    uint8_t *inner = NULL;
    uint8_t *adf = NULL;
    size_t inner_size = 0u;
    size_t adf_size = 0u;
    RealSaveDiskReceipt receipt;
    FILE *stream;

    if (!archive || !archive[0]) {
        printf("SKIP real_amiga_v20_save_disk: archive not configured\n");
        return;
    }
    stream = fopen(archive, "rb");
    if (!stream) {
        printf("SKIP real_amiga_v20_save_disk: archive unavailable\n");
        return;
    }
    fclose(stream);
    memset(&receipt, 0, sizeof(receipt));
    CHECK(firestaff_zip_extract_by_suffix(archive, save_zip, &inner,
                                          &inner_size) == 0,
          "real_save_outer_zip_member");
    if (!inner) return;
    CHECK(firestaff_zip_extract_memory_by_suffix(inner, inner_size, save_adf,
                                                 &adf, &adf_size) == 0,
          "real_save_inner_adf_member");
    free(inner);
    if (!adf) return;
    CHECK(adf_size == 901120u, "real_save_adf_size");
    CHECK(firestaff_amiga_adf_visit_ofs_files(adf, adf_size,
                                              real_save_disk_visitor,
                                              &receipt) >= 0,
          "real_save_adf_visit");
    free(adf);
    printf("AMIGA-SAVE-DISK files=%u payload_bytes=%zu\n",
           receipt.file_count, receipt.byte_count);
    CHECK(receipt.file_count == 2u && receipt.byte_count == 98004u,
          "real_save_disk_expected_file_set");
    CHECK(receipt.primary_classified == 1,
          "real_save_primary_classified");
    CHECK(receipt.primary.header_checksum_ok == 1,
          "real_save_primary_header_checksum");
    CHECK(receipt.primary.shape == DM1_ORIGINAL_SAVE_SHAPE_ORIGINAL_COMPAT_FAMILY,
          "real_save_primary_compat_family");
    CHECK(receipt.primary_f0435_result == DM1_V1_AMIGA_SAVE_F0435_OK,
          "real_save_primary_f0435_body");
    CHECK(receipt.primary_global_result == DM1_V1_AMIGA_SAVE_F0435_OK &&
          receipt.primary_global.game_time == receipt.primary_f0435.game_time &&
          receipt.primary_global.party_champion_count ==
              receipt.primary_f0435.party_champion_count &&
          receipt.primary_global.party_map_x == receipt.primary_f0435.party_map_x &&
          receipt.primary_global.party_map_y == receipt.primary_f0435.party_map_y &&
          receipt.primary_global.party_direction ==
              receipt.primary_f0435.party_direction &&
          receipt.primary_global.party_map_index ==
              receipt.primary_f0435.party_map_index &&
          receipt.primary_global.event_count == receipt.primary_f0435.event_count &&
          receipt.primary_global.event_maximum_count ==
              receipt.primary_f0435.event_maximum_count,
          "real_save_primary_plaintext_global_matches_authenticated_receipt");
    CHECK(receipt.primary_party_result == DM1_V1_AMIGA_SAVE_F0435_OK &&
          receipt.primary_party[0] != 0u,
          "real_save_primary_plaintext_amiga_c2_party_part");
    CHECK(receipt.primary_party_receipt_result == DM1_V1_AMIGA_SAVE_F0435_OK &&
          receipt.primary_party_receipt.champions[0].direction <= 3u &&
          receipt.primary_party_receipt.champions[0].health_current <=
              receipt.primary_party_receipt.champions[0].health_maximum &&
          receipt.primary_party_receipt.champions[0].stamina_current <=
              receipt.primary_party_receipt.champions[0].stamina_maximum &&
          receipt.primary_party_receipt.champions[0].mana_current <=
              receipt.primary_party_receipt.champions[0].mana_maximum,
          "real_save_primary_a20_c2_champion_receipt");
    CHECK(receipt.primary_queue_receipt_result == DM1_V1_AMIGA_SAVE_F0435_OK &&
          receipt.primary_queue_receipt.timeline_membership_valid == 1 &&
          receipt.primary_queue_receipt.active_group_capacity ==
              receipt.primary_f0435.maximum_active_group_count &&
          receipt.primary_queue_receipt.event_capacity ==
              receipt.primary_f0435.event_maximum_count &&
          receipt.primary_queue_receipt.scheduled_event_count ==
              receipt.primary_f0435.event_count,
          "real_save_primary_a20_c1_c3_c4_queue_receipt");
    CHECK(receipt.primary_event_queue_result == DM1_V1_AMIGA_SAVE_F0435_OK &&
          receipt.primary_event_queue.gameTick == receipt.primary_f0435.game_time &&
          receipt.primary_event_queue.eventCount == receipt.primary_f0435.event_count &&
          receipt.primary_event_queue.timeline[0] < receipt.primary_f0435.event_maximum_count &&
          receipt.primary_event_queue.events[receipt.primary_event_queue.timeline[0]].type ==
              DM1_EVENT_WATCHDOG,
          "real_save_primary_a20_c3_c4_materializes_native_watchdog_queue");
    CHECK(receipt.primary_runtime_party_result == DM1_V1_AMIGA_SAVE_F0435_OK &&
          receipt.primary_runtime_party.championCount ==
              receipt.primary_f0435.party_champion_count &&
          receipt.primary_runtime_party.mapX == receipt.primary_f0435.party_map_x &&
          receipt.primary_runtime_party.mapY == receipt.primary_f0435.party_map_y &&
          receipt.primary_runtime_party.direction == receipt.primary_f0435.party_direction &&
          receipt.primary_runtime_party.champions[0].present == 1 &&
          receipt.primary_runtime_party.champions[0].hp.current ==
              receipt.primary_party_receipt.champions[0].health_current,
          "real_save_primary_a20_c0_c2_materializes_native_party");
    CHECK(receipt.primary_f0435.header_authenticated == 1 &&
          receipt.primary_f0435.body_authenticated == 1 &&
          receipt.primary_f0435.tail_authenticated == 1 &&
          receipt.primary_f0435.parts_authenticated == 5u &&
          receipt.primary_f0435.portrait_byte_count == 1856u &&
          receipt.primary_f0435.dungeon_map_count == 14u &&
          receipt.primary_f0435.dungeon_column_count == 412u &&
          receipt.primary_f0435.dungeon_expected_checksum ==
              receipt.primary_f0435.dungeon_actual_checksum,
          "real_save_primary_f0435_and_f0434");
    {
        uint8_t *active_groups = NULL;
        uint8_t *events = NULL;
        size_t active_group_size = 0u;
        size_t event_size = 0u;
        active_groups = malloc(receipt.primary_f0435.part_byte_counts[1]);
        events = malloc(receipt.primary_f0435.part_byte_counts[3]);
        CHECK(dm1_v1_original_save_amiga_f0435_part_bytes(
                  receipt.primary_bytes, receipt.primary_size, 1u,
                  active_groups, receipt.primary_f0435.part_byte_counts[1],
                  &active_group_size, NULL) == DM1_V1_AMIGA_SAVE_F0435_OK &&
              active_group_size == receipt.primary_f0435.part_byte_counts[1] &&
              active_group_size == (size_t)receipt.primary_f0435.maximum_active_group_count * 16u,
              "real_save_primary_a20_c1_active_group_part");
        CHECK(dm1_v1_original_save_amiga_f0435_part_bytes(
                  receipt.primary_bytes, receipt.primary_size, 3u,
                  events, receipt.primary_f0435.part_byte_counts[3],
                  &event_size, NULL) == DM1_V1_AMIGA_SAVE_F0435_OK &&
              event_size == receipt.primary_f0435.part_byte_counts[3] &&
              event_size == (size_t)receipt.primary_f0435.event_maximum_count * 10u,
              "real_save_primary_a20_c3_event_part");
        free(active_groups);
        free(events);
    }
    {
        uint8_t *tail = NULL;
        size_t tail_size = 0u;
        struct DungeonDatState_Compat dungeon;
        struct DungeonThings_Compat things;
        struct GameWorld_Compat materialized_world;
        struct GameWorld_Compat session_world;
        struct DM1_EventQueue_V1 session_queue;
        size_t group_offset;
        size_t projectile_offset;
        memset(&dungeon, 0, sizeof(dungeon));
        memset(&things, 0, sizeof(things));
        memset(&materialized_world, 0, sizeof(materialized_world));
        memset(&session_world, 0, sizeof(session_world));
        memset(&session_queue, 0, sizeof(session_queue));
        if (receipt.primary_f0435.dungeon_byte_count != 0u)
            tail = malloc(receipt.primary_f0435.dungeon_byte_count);
        CHECK(dm1_v1_original_save_amiga_f0435_dungeon_tail_bytes(
                  receipt.primary_bytes, receipt.primary_size, tail,
                  receipt.primary_f0435.dungeon_byte_count, &tail_size, NULL) ==
                  DM1_V1_AMIGA_SAVE_F0435_OK &&
              tail_size == receipt.primary_f0435.dungeon_byte_count &&
              memcmp(tail, receipt.primary_bytes + receipt.primary_f0435.dungeon_offset,
                     tail_size) == 0,
              "real_save_primary_big_endian_dungeon_tail_copy");
        CHECK(F0505_DUNGEON_LoadTailBufferAmigaBE_Compat(
                  tail, (int)tail_size, &dungeon, &things) == 1 &&
              dungeon.header.mapCount == receipt.primary_f0435.dungeon_map_count &&
              dungeon.dungeonColumnCount == receipt.primary_f0435.dungeon_column_count &&
              things.squareFirstThingCount ==
                  receipt.primary_f0435.dungeon_square_first_thing_count &&
              dungeon.tilesLoaded == 1 && things.loaded == 1,
              "real_save_primary_f0434_amiga_be_runtime_reader");
        CHECK(dm1_v1_original_save_amiga_f0435_materialize_dungeon_world_bytes(
                  receipt.primary_bytes, receipt.primary_size,
                  &materialized_world, NULL) == DM1_V1_AMIGA_SAVE_F0435_OK &&
              materialized_world.ownsDungeon == 1 &&
              materialized_world.dungeon && materialized_world.things &&
              materialized_world.dungeon->originalSaveTailPristine == 1 &&
              materialized_world.dungeon->originalSaveTailByteCount ==
                  (int)tail_size &&
              memcmp(materialized_world.dungeon->originalSaveTailBytes, tail,
                     tail_size) == 0,
              "real_save_primary_amiga_handoff_materializes_pristine_world");
        CHECK(dm1_v1_original_save_amiga_f0435_materialize_session_bytes(
                  receipt.primary_bytes, receipt.primary_size, &session_world,
                  &session_queue, NULL) == DM1_V1_AMIGA_SAVE_F0435_OK &&
              session_world.ownsDungeon == 1 && session_world.dungeon &&
              session_world.things &&
              session_world.gameTick == receipt.primary_f0435.game_time &&
              session_world.party.championCount ==
                  receipt.primary_f0435.party_champion_count &&
              session_world.party.mapIndex == receipt.primary_f0435.party_map_index &&
              session_world.party.mapX == receipt.primary_f0435.party_map_x &&
              session_world.party.mapY == receipt.primary_f0435.party_map_y &&
              session_world.magic.magicalLightAmount ==
                  receipt.primary_party_receipt.magical_light_amount &&
              session_world.magic.event73CountThievesEye ==
                  receipt.primary_party_receipt.thieves_eye_count &&
              session_world.magic.event79CountFootprints ==
                  receipt.primary_party_receipt.footprints_count &&
              session_world.magic.event71CountInvisibility ==
                  receipt.primary_party_receipt.invisibility_count &&
              session_world.magic.partyShieldDefense ==
                  receipt.primary_party_receipt.shield_defense &&
              session_world.magic.fireShieldDefense ==
                  receipt.primary_party_receipt.fire_shield_defense &&
              session_world.magic.spellShieldDefense ==
                  receipt.primary_party_receipt.spell_shield_defense &&
              session_world.freezeLifeTicks ==
                  receipt.primary_party_receipt.freeze_life_ticks &&
              session_world.disabledMovementTicks ==
                  receipt.primary_global.disabled_movement_ticks &&
              session_world.projectileDisabledMovementTicks ==
                  receipt.primary_global.projectile_disabled_movement_ticks &&
              session_world.lastProjectileDisabledMovementDirection ==
                  receipt.primary_global.last_projectile_disabled_movement_direction &&
              session_world.lifecycle.gameTime == receipt.primary_f0435.game_time &&
              session_world.lifecycle.lastCreatureAttackTime ==
                  (uint32_t)receipt.primary_global.last_creature_attack_time &&
              session_world.lifecycle.status.partyShieldDefense ==
                  receipt.primary_party_receipt.shield_defense &&
              session_world.lifecycle.status.invisibilityCount ==
                  receipt.primary_party_receipt.invisibility_count &&
              session_world.timeline.count == receipt.primary_f0435.event_count &&
              session_world.timeline.events[0].kind == TIMELINE_EVENT_WATCHDOG &&
              session_world.timeline.events[0].fireAtTick == 300u &&
              session_queue.eventCount == receipt.primary_f0435.event_count &&
              session_queue.events[session_queue.timeline[0]].type ==
                  DM1_EVENT_WATCHDOG,
              "real_save_primary_amiga_handoff_materializes_atomic_session_candidate");
        group_offset = test_amiga_tail_thing_offset(tail, tail_size,
                                                     THING_TYPE_GROUP);
        projectile_offset = test_amiga_tail_thing_offset(
            tail, tail_size, THING_TYPE_PROJECTILE);
        CHECK(things.groupCount > 0 && group_offset != 0u &&
              group_offset + 6u <= tail_size && things.groups &&
              things.groups[0].creatureType == tail[group_offset + 4u] &&
              things.groups[0].cells == tail[group_offset + 5u],
              "real_save_primary_amiga_group_byte_fields_not_swapped");
        CHECK(things.projectileCount > 0 && projectile_offset != 0u &&
              projectile_offset + 6u <= tail_size && things.projectiles &&
              things.projectiles[0].kineticEnergy == tail[projectile_offset + 4u] &&
              things.projectiles[0].attack == tail[projectile_offset + 5u],
              "real_save_primary_amiga_projectile_byte_fields_not_swapped");
        F0504_DUNGEON_FreeThingData_Compat(&things);
        F0500_DUNGEON_FreeDatHeader_Compat(&dungeon);
        F0504_DUNGEON_FreeThingData_Compat(materialized_world.things);
        F0500_DUNGEON_FreeDatHeader_Compat(materialized_world.dungeon);
        free(materialized_world.things);
        free(materialized_world.dungeon);
        F0504_DUNGEON_FreeThingData_Compat(session_world.things);
        F0500_DUNGEON_FreeDatHeader_Compat(session_world.dungeon);
        free(session_world.things);
        free(session_world.dungeon);
        free(tail);
    }
    CHECK(receipt.backup_f0435_result == DM1_V1_AMIGA_SAVE_F0435_OK &&
          receipt.backup_f0435.parts_authenticated == 5u &&
          receipt.backup_f0435.tail_authenticated == 1 &&
          receipt.backup_f0435.dungeon_expected_checksum ==
              receipt.backup_f0435.dungeon_actual_checksum,
          "real_save_backup_f0435_and_f0434");
    free(receipt.primary_bytes);
}

int main(void) {
    test_null_rejection();
    test_small_rejection();
    test_wrong_count();
    test_pc34_format_rejection();
    test_synthetic_probe();
    test_synthetic_valid();
    test_receipt_null();
    test_compressed_rejection();
    test_real_amiga_v20_graphics_receipt();
    test_real_amiga_v20_save_disk_receipt();
    printf("dm1_v1_amiga_graphics_dat: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
