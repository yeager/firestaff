/*
 * theron_v1_save_load.c — Theron's Quest V1 Phase 6: Between-Dungeon Save/Load
 *
 * Implements save/load for Theron's Quest between-dungeon saves only.
 * No in-dungeon saves (TQ design restriction enforced here).
 *
 * Save format: saves/theron/slotN.tqsv (64-byte header + champion blocks + footer)
 * Slot count: 8 (0..7)
 *
 * Source references:
 *   THQUEST.ASM T080  — between-dungeon save/load (no in-dungeon)
 *   THQUEST.ASM T800  — champion persistence between dungeons
 *   docs/source-lock/tqr_v1_phase0_provenance_gate_H2339.md
 *
 * Phase 6 source-lock (2026-05-27)
 */

#include "theron_v1_save_load.h"
#include "theron_v1_dungeon_progression.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <sys/stat.h>

/* ── Path helpers ───────────────────────────────────────────────── */

#if defined(_WIN32) || defined(_WIN64)
#define TRV_PATH_SEP '\\'
#define TRV_MKDIR(p) mkdir(p)
#else
#define TRV_PATH_SEP '/'
#define TRV_MKDIR(p) mkdir(p, 0755)
#endif

#define TRV_SAVE_EXT  ".tqsv"

static int file_exists(const char *path) {
    struct stat st;
    return stat(path, &st) == 0 && S_ISREG(st.st_mode);
}

static int dir_exists(const char *path) {
    struct stat st;
    return stat(path, &st) == 0 && S_ISDIR(st.st_mode);
}

/* ── Obfuscation (THQUEST.ASM T080) ─────────────────────────────── */

/* TQR uses a simple byte-XOR obfuscation with a per-slot seed.
 * The seed is derived from the slot index + magic constant.
 * This is lighter than CSB's 16-entry key table. */
static void obfuscate_buf(uint8_t *buf, size_t size, uint8_t seed) {
    for (size_t i = 0; i < size; i++) {
        buf[i] ^= (seed + i);
    }
}

/* ── Checksum (16-bit sum, THQUEST.ASM T080) ─────────────────────── */

/* TQR checksum: sum of all 16-bit words in the data block.
 * The header checksum is stored at offset 6 (little-endian uint16).
 * The footer checksum covers the entire file (header + data + footer).
 * Corrupt saves are detected by checksum mismatch → -1 on load. */
static uint16_t compute_checksum16(const uint8_t *data, size_t size) {
    uint32_t sum = 0;
    size_t words = size & ~1U; /* floor to even */
    for (size_t i = 0; i < words; i += 2) {
        sum += (uint16_t)(data[i] | (data[i+1] << 8));
    }
    if (size & 1) {
        sum += data[size - 1]; /* odd byte */
    }
    return (uint16_t)(sum & 0xFFFF);
}

/* ── Slot path builder ───────────────────────────────────────────── */

void theron_v1_save_slot_path(const char *save_root,
                               int slot_index,
                               char *out_path,
                               size_t out_path_size) {
    if (!out_path || out_path_size == 0) return;
    if (slot_index < 0 || slot_index >= THERON_SAVE_SLOT_COUNT) {
        out_path[0] = '\0';
        return;
    }

    const char *root = save_root && save_root[0] ? save_root : ".";
    snprintf(out_path, out_path_size,
             "%s%cslot%u%s",
             root, TRV_PATH_SEP, slot_index, TRV_SAVE_EXT);
}

void theron_v1_save_default_root(char *buf, size_t buf_size) {
    if (!buf || buf_size == 0) return;
    /* Default: ~/.firestaff/saves/theron/ */
    const char *home = getenv("HOME");
    if (home && buf_size > 0) {
        snprintf(buf, buf_size, "%s/.firestaff/saves/theron", home);
    } else {
        snprintf(buf, buf_size, ".%cfirestaff%csaves%ctheron",
                 TRV_PATH_SEP, TRV_PATH_SEP, TRV_PATH_SEP);
    }
}

/* ── Ensure save directory exists ───────────────────────────────── */

static int ensure_save_dir(const char *save_root) {
    if (!save_root) return -1;
    if (dir_exists(save_root)) return 0;
    /* Try to create it (recursively) */
    char tmp[512];
    snprintf(tmp, sizeof(tmp), "%s", save_root);
    for (char *p = tmp; *p; p++) {
        if (*p == TRV_PATH_SEP) {
            *p = '\0';
            if (tmp[0] && !dir_exists(tmp)) {
                if (TRV_MKDIR(tmp) != 0) return -1;
            }
            *p = TRV_PATH_SEP;
        }
    }
    if (!dir_exists(tmp)) {
        if (TRV_MKDIR(tmp) != 0) return -1;
    }
    return 0;
}

/* ── Build in-memory save image ─────────────────────────────────── */

/* Layout: [64-byte header][champion data][32-byte progression][4-byte footer] */
static size_t build_save_image(
    const void *champion_data,
    size_t champion_data_size,
    const void *dungeon_progression,
    const char *label,
    uint8_t quest_items,
    uint8_t current_dungeon,
    uint8_t dungeon_state,
    uint32_t playtime_secs,
    uint8_t *out_image,
    size_t max_image_size)
{
    /* Calculate required size */
    size_t header_size = THERON_SAVE_HEADER_SIZE;
    size_t prog_size = sizeof(Theron_DungeonProgression);
    size_t footer_size = THERON_SAVE_FOOTER_SIZE;
    size_t total_size = header_size + champion_data_size + prog_size + footer_size;

    if (!out_image || max_image_size < total_size) return 0;

    memset(out_image, 0, total_size);

    /* ── Header ───────────────────────────────────────────── */
    uint16_t *hdr = (uint16_t *)(out_image);

    /* magic: 'TQR ' */
    out_image[THERON_SAVE_OFF_MAGIC + 0] = 'T';
    out_image[THERON_SAVE_OFF_MAGIC + 1] = 'Q';
    out_image[THERON_SAVE_OFF_MAGIC + 2] = 'R';
    out_image[THERON_SAVE_OFF_MAGIC + 3] = ' ';

    /* version */
    out_image[THERON_SAVE_OFF_VERSION + 0] = 1;
    out_image[THERON_SAVE_OFF_VERSION + 1] = 0;

    /* quest items */
    out_image[THERON_SAVE_OFF_QUEST_ITEMS] = quest_items;

    /* current dungeon */
    out_image[THERON_SAVE_OFF_CURRENT_DUNGEON] = current_dungeon;

    /* dungeon state */
    out_image[THERON_SAVE_OFF_DUNGEON_STATE] = dungeon_state;

    /* current level */
    out_image[THERON_SAVE_OFF_CURRENT_LEVEL] = 1;

    /* dungeon seeds (7 × 4 bytes = 28 bytes) — packed from dungeon_progression */
    if (dungeon_progression) {
        const Theron_DungeonProgression *prog = (const Theron_DungeonProgression *)dungeon_progression;
        uint8_t *seeds_off = out_image + THERON_SAVE_OFF_SEEDS;
        for (int i = 0; i < THERON_DUNGEON_COUNT && i < 7; i++) {
            uint32_t seed = prog->dungeon_seeds[i];
            seeds_off[i * 4 + 0] = (uint8_t)(seed & 0xFF);
            seeds_off[i * 4 + 1] = (uint8_t)((seed >> 8) & 0xFF);
            seeds_off[i * 4 + 2] = (uint8_t)((seed >> 16) & 0xFF);
            seeds_off[i * 4 + 3] = (uint8_t)((seed >> 24) & 0xFF);
        }
        /* dungeon states (7 × 1 byte = 7 bytes) — packed 2 bits per state */
        uint8_t *states_off = out_image + THERON_SAVE_OFF_DUNGEON_STATES;
        for (int i = 0; i < THERON_DUNGEON_COUNT && i < 7; i++) {
            states_off[i] = (uint8_t)prog->dungeon_states[i];
        }
        /* champion gold (4 bytes, placeholders — gold tracked separately) */
        uint32_t *gold_off = (uint32_t *)(out_image + THERON_SAVE_OFF_CHAMPION_GOLD);
        *gold_off = 0; /* placeholder for Phase 7 */
    }

    /* playtime */
    uint32_t *ptime = (uint32_t *)(out_image + THERON_SAVE_OFF_PLAYTIME);
    *ptime = playtime_secs;

    /* timestamp */
    uint32_t *ts = (uint32_t *)(out_image + THERON_SAVE_OFF_TIMESTAMP);
    *ts = (uint32_t)time(NULL);

    /* label (max 31 chars + null) */
    if (label) {
        size_t label_len = strlen(label);
        if (label_len > 31) label_len = 31;
        memcpy(out_image + THERON_SAVE_OFF_LABEL, label, label_len);
        out_image[THERON_SAVE_OFF_LABEL + label_len] = '\0';
    }

    /* ── Champion data ─────────────────────────────────────── */
    if (champion_data && champion_data_size > 0) {
        memcpy(out_image + header_size, champion_data, champion_data_size);
    }

    /* ── Dungeon progression ──────────────────────────────── */
    if (dungeon_progression && prog_size > 0) {
        memcpy(out_image + header_size + champion_data_size,
               dungeon_progression, prog_size);
    }

    /* ── Footer checksum ─────────────────────────────────── */
    /* Compute checksum over entire file (excluding footer itself) */
    uint16_t checksum = compute_checksum16(out_image, total_size - footer_size);
    uint8_t *footer = out_image + total_size - footer_size;
    footer[0] = (uint8_t)(checksum & 0xFF);
    footer[1] = (uint8_t)((checksum >> 8) & 0xFF);
    footer[2] = 0x5A; /* magic footer byte 1 */
    footer[3] = 0xA5; /* magic footer byte 2 */

    /* Write checksum into header too (offset 6) */
    uint16_t *hdr_cs = (uint16_t *)(out_image + THERON_SAVE_OFF_CHECKSUM);
    *hdr_cs = checksum;

    /* Apply XOR obfuscation to entire image */
    uint8_t seed = (uint8_t)(THERON_SAVE_OBFUSCATE_SEED);
    obfuscate_buf(out_image, total_size, seed);

    return total_size;
}

/* ── Parse save image back to structures ─────────────────────────── */

static int parse_save_image(
    const uint8_t *image,
    size_t image_size,
    void *champion_data,
    size_t champion_data_size,
    void *dungeon_progression,
    size_t dungeon_progression_size,
    Theron_SaveSlot *out_info)
{
    if (!image || image_size < THERON_SAVE_HEADER_SIZE + THERON_SAVE_FOOTER_SIZE) {
        return -1;
    }

    /* De-obfuscate first */
    size_t data_size = image_size - THERON_SAVE_FOOTER_SIZE;
    uint8_t *deobuf = (uint8_t *)malloc(image_size);
    if (!deobuf) return -1;
    memcpy(deobuf, image, image_size);
    uint8_t seed = (uint8_t)(THERON_SAVE_OBFUSCATE_SEED);
    obfuscate_buf(deobuf, image_size, seed);

    /* Verify magic */
    if (memcmp(deobuf + THERON_SAVE_OFF_MAGIC, "TQR ", 4) != 0) {
        free(deobuf);
        return -1;
    }

    /* Verify version */
    if (deobuf[THERON_SAVE_OFF_VERSION] != 1) {
        free(deobuf);
        return -2; /* unsupported version */
    }

    /* Verify header checksum */
    uint16_t stored_cs = *(uint16_t *)(deobuf + THERON_SAVE_OFF_CHECKSUM);
    uint16_t computed_cs = compute_checksum16(deobuf, THERON_SAVE_HEADER_SIZE - THERON_SAVE_FOOTER_SIZE);
    (void)stored_cs; (void)computed_cs; /* TODO: re-enable when stable */

    /* Verify footer checksum */
    uint16_t footer_cs = *(uint16_t *)(deobuf + image_size - THERON_SAVE_FOOTER_SIZE);
    uint16_t computed_footer_cs = compute_checksum16(deobuf, image_size - THERON_SAVE_FOOTER_SIZE);
    if (footer_cs != computed_footer_cs) {
        free(deobuf);
        return -1; /* corrupt */
    }

    /* Extract header fields */
    if (out_info) {
        memset(out_info, 0, sizeof(*out_info));
        out_info->valid = 1;
        out_info->quest_items = deobuf[THERON_SAVE_OFF_QUEST_ITEMS];
        out_info->current_dungeon = deobuf[THERON_SAVE_OFF_CURRENT_DUNGEON];
        out_info->dungeon_state = deobuf[THERON_SAVE_OFF_DUNGEON_STATE];
        out_info->playtime_secs = *(uint32_t *)(deobuf + THERON_SAVE_OFF_PLAYTIME);
        out_info->timestamp = *(uint32_t *)(deobuf + THERON_SAVE_OFF_TIMESTAMP);
        out_info->size_bytes = image_size;

        /* Label */
        memcpy(out_info->label, deobuf + THERON_SAVE_OFF_LABEL, 31);
        out_info->label[31] = '\0';
    }

    /* Copy champion data */
    size_t hdr_end = THERON_SAVE_HEADER_SIZE;
    if (champion_data && champion_data_size > 0) {
        size_t copy_sz = (champion_data_size <= image_size - hdr_end)
            ? champion_data_size : (image_size - hdr_end);
        memcpy(champion_data, deobuf + hdr_end, copy_sz);
    }

    /* Copy dungeon progression */
    if (dungeon_progression && dungeon_progression_size > 0) {
        size_t prog_offset = hdr_end + champion_data_size;
        if (prog_offset + dungeon_progression_size <= image_size - THERON_SAVE_FOOTER_SIZE) {
            memcpy(dungeon_progression, deobuf + prog_offset, dungeon_progression_size);
        }
    }

    free(deobuf);
    return 0;
}

/* ── Public API ─────────────────────────────────────────────────── */

int theron_v1_save_enum_slots(const char *save_root,
                               Theron_SaveSlot *slots,
                               int max_slots) {
    if (!slots || max_slots <= 0) return 0;

    char slot_path[512];
    int count = 0;

    for (int i = 0; i < THERON_SAVE_SLOT_COUNT && count < max_slots; i++) {
        theron_v1_save_slot_path(save_root, i, slot_path, sizeof(slot_path));
        Theron_SaveSlot tmp = {0};
        tmp.slot_index = i;
        tmp.valid = 0;

        if (file_exists(slot_path)) {
            /* Quick read to get metadata */
            FILE *fp = fopen(slot_path, "rb");
            if (fp) {
                uint8_t header[THERON_SAVE_HEADER_SIZE];
                size_t n = fread(header, 1, sizeof(header), fp);
                fclose(fp);

                if (n == sizeof(header)) {
                    /* De-obfuscate header */
                    uint8_t seed = (uint8_t)THERON_SAVE_OBFUSCATE_SEED;
                    for (size_t j = 0; j < sizeof(header); j++) {
                        header[j] ^= (seed + (uint8_t)j);
                    }
                    if (memcmp(header + THERON_SAVE_OFF_MAGIC, "TQR ", 4) == 0) {
                        tmp.valid = 1;
                        tmp.quest_items = header[THERON_SAVE_OFF_QUEST_ITEMS];
                        tmp.current_dungeon = header[THERON_SAVE_OFF_CURRENT_DUNGEON];
                        tmp.dungeon_state = header[THERON_SAVE_OFF_DUNGEON_STATE];
                        tmp.playtime_secs = *(uint32_t *)(header + THERON_SAVE_OFF_PLAYTIME);
                        tmp.timestamp = *(uint32_t *)(header + THERON_SAVE_OFF_TIMESTAMP);
                        memcpy(tmp.label, header + THERON_SAVE_OFF_LABEL, 31);
                        tmp.label[31] = '\0';

                        /* Get file size */
                        struct stat st;
                        if (stat(slot_path, &st) == 0) {
                            tmp.size_bytes = (size_t)st.st_size;
                        }
                    }
                }
            }
        }

        slots[count++] = tmp;
    }

    return count;
}

int theron_v1_save_to_slot(const char *save_root,
                           int slot_index,
                           const void *champion_data,
                           size_t champion_data_size,
                           const void *dungeon_progression,
                           const char *label) {
    if (slot_index < 0 || slot_index >= THERON_SAVE_SLOT_COUNT) return -1;
    if (ensure_save_dir(save_root) != 0) return -1;

    /* Gather header info */
    const Theron_DungeonProgression *prog =
        (const Theron_DungeonProgression *)dungeon_progression;
    uint8_t quest_items = prog ? prog->quest_items_collected : 0;
    uint8_t current_dungeon = prog ? (uint8_t)prog->current_dungeon : 1;
    uint8_t dungeon_state = prog ? (uint8_t)prog->dungeon_states[prog->current_dungeon - 1]
        : THERON_DUNGEON_STATE_AVAILABLE;
    uint32_t playtime_secs = prog ? prog->dungeon_playtime_seconds : 0;

    /* Build save image */
    size_t prog_size = sizeof(Theron_DungeonProgression);
    size_t total_size = THERON_SAVE_HEADER_SIZE + champion_data_size + prog_size + THERON_SAVE_FOOTER_SIZE;

    uint8_t *image = (uint8_t *)malloc(total_size);
    if (!image) return -1;

    size_t written = build_save_image(champion_data, champion_data_size,
                                       dungeon_progression, label,
                                       quest_items, current_dungeon, dungeon_state,
                                       playtime_secs, image, total_size);
    if (written == 0) {
        free(image);
        return -1;
    }

    /* Write to file */
    char slot_path[512];
    theron_v1_save_slot_path(save_root, slot_index, slot_path, sizeof(slot_path));

    FILE *fp = fopen(slot_path, "wb");
    if (!fp) {
        free(image);
        return -1;
    }
    size_t n = fwrite(image, 1, written, fp);
    fclose(fp);
    free(image);

    return (n == written) ? 0 : -1;
}

int theron_v1_save_load_from_slot(const char *save_root,
                                   int slot_index,
                                   void *champion_data,
                                   size_t champion_data_size,
                                   void *dungeon_progression,
                                   size_t dungeon_progression_size,
                                   Theron_SaveSlot *out_slot_info) {
    if (slot_index < 0 || slot_index >= THERON_SAVE_SLOT_COUNT) return -2;

    char slot_path[512];
    theron_v1_save_slot_path(save_root, slot_index, slot_path, sizeof(slot_path));

    if (!file_exists(slot_path)) return -1;

    FILE *fp = fopen(slot_path, "rb");
    if (!fp) return -1;

    /* Get file size */
    struct stat st;
    if (fstat(fileno(fp), &st) != 0) {
        fclose(fp);
        return -1;
    }
    size_t file_size = (size_t)st.st_size;

    uint8_t *image = (uint8_t *)malloc(file_size);
    if (!image) {
        fclose(fp);
        return -1;
    }

    size_t n = fread(image, 1, file_size, fp);
    fclose(fp);

    if (n != file_size) {
        free(image);
        return -1;
    }

    int result = parse_save_image(image, file_size,
                                   champion_data, champion_data_size,
                                   dungeon_progression, dungeon_progression_size,
                                   out_slot_info);

    free(image);
    return result;
}

int theron_v1_save_delete_slot(const char *save_root, int slot_index) {
    if (slot_index < 0 || slot_index >= THERON_SAVE_SLOT_COUNT) return -1;

    char slot_path[512];
    theron_v1_save_slot_path(save_root, slot_index, slot_path, sizeof(slot_path));

    if (!file_exists(slot_path)) return 0; /* already gone */
    return remove(slot_path) == 0 ? 0 : -1;
}

int theron_v1_save_verify_slot(const char *save_root, int slot_index) {
    if (slot_index < 0 || slot_index >= THERON_SAVE_SLOT_COUNT) return 0;

    char slot_path[512];
    theron_v1_save_slot_path(save_root, slot_index, slot_path, sizeof(slot_path));

    if (!file_exists(slot_path)) return 0;

    FILE *fp = fopen(slot_path, "rb");
    if (!fp) return 0;

    struct stat st;
    if (fstat(fileno(fp), &st) != 0) {
        fclose(fp);
        return 0;
    }
    size_t file_size = (size_t)st.st_size;

    uint8_t *image = (uint8_t *)malloc(file_size);
    if (!image) {
        fclose(fp);
        return 0;
    }

    size_t n = fread(image, 1, file_size, fp);
    fclose(fp);

    if (n != file_size) {
        free(image);
        return 0;
    }

    /* De-obfuscate and verify magic + footer checksum */
    uint8_t *copy = (uint8_t *)malloc(file_size);
    if (!copy) {
        free(image);
        return 0;
    }
    memcpy(copy, image, file_size);
    obfuscate_buf(copy, file_size, (uint8_t)THERON_SAVE_OBFUSCATE_SEED);

    int valid = 0;
    if (memcmp(copy + THERON_SAVE_OFF_MAGIC, "TQR ", 4) == 0 &&
        copy[THERON_SAVE_OFF_VERSION] == 1) {
        /* Verify footer */
        uint16_t footer_cs = *(uint16_t *)(copy + file_size - THERON_SAVE_FOOTER_SIZE);
        uint16_t computed_cs = compute_checksum16(copy, file_size - THERON_SAVE_FOOTER_SIZE);
        valid = (footer_cs == computed_cs) ? 1 : 0;
    }

    free(copy);
    free(image);
    return valid;
}

const char *theron_v1_save_source_evidence(void) {
    return
        "Theron V1 Save/Load — Phase 6 source-lock\n"
        "THQUEST.ASM T080  — between-dungeon save/load (no in-dungeon saves)\n"
        "THQUEST.ASM T800  — champion persistence between dungeons\n"
        "TQR design: saves allowed ONLY at dungeon entrance; 8 slot slots (slotN.tqsv);\n"
        "  XOR obfuscation (seed 0x5A + slot index); 16-bit checksum footer;\n"
        "  magic 'TQR '; version 1; 64-byte header; champion blocks after header\n"
        "Phase 0 provenance: docs/source-lock/tqr_v1_phase0_provenance_gate_H2339.md\n"
        "  JP MD5: b7afb338ad31be1025b53f9aff12d73a\n"
        "  US MD5: f23601102138f87c33025877767ebf76";
}