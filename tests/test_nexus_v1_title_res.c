#include "nexus_v1_title_cg.h"
#include "nexus_v1_res.h"
#include "nexus_v1_font012.h"
#include "nexus_v1_iso_reader.h"
#include "nexus_v1_engine.h"
#include "nexus_v1_ui_surfaces.h"
#include "nexus_v1_champions.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned int g_japan_real_passed;
static unsigned int g_japan_real_skipped;

static uint8_t *load_file(const char *path, int *out_size) {
    FILE *f = fopen(path, "rb");
    uint8_t *buf;
    long sz;
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    buf = (uint8_t *)malloc((size_t)sz);
    if (!buf) { fclose(f); return NULL; }
    if ((long)fread(buf, 1, (size_t)sz, f) != sz) {
        free(buf); fclose(f); return NULL;
    }
    fclose(f);
    *out_size = (int)sz;
    return buf;
}

static uint8_t *load_retail_file(const char *root, const char *name,
                                 int *out_size) {
    static const char *const cue_names[] = {
        "Dungeon Master Nexus (Japan).cue",
        "Dungeon Master Nexus (English).cue",
        "Dungeon Master Nexus (French).cue",
        NULL
    };
    Nexus_ISOReader iso;
    const Nexus_ISOFile *member;
    char path[1024];
    int index;
    uint8_t *data;

    if (!root || !name || !out_size) return NULL;
    snprintf(path, sizeof(path), "%s/%s", root, name);
    data = load_file(path, out_size);
    if (data) return data;
    memset(&iso, 0, sizeof(iso));
    if (strlen(root) >= 4U && strcmp(root + strlen(root) - 4U, ".cue") == 0) {
        if (nexus_iso_open_cue(&iso, root) <= 0) return NULL;
    } else {
        for (index = 0; cue_names[index]; ++index) {
            snprintf(path, sizeof(path), "%s/%s", root, cue_names[index]);
            if (nexus_iso_open_cue(&iso, path) > 0) break;
        }
        if (!iso.valid) return NULL;
    }
    member = nexus_iso_find(&iso, name);
    if (!member || member->size == 0U || member->size > (uint32_t)INT_MAX) {
        nexus_iso_close(&iso);
        return NULL;
    }
    data = (uint8_t *)malloc(member->size);
    if (!data || nexus_iso_read_file(&iso, member, data, (int)member->size) !=
                     (int)member->size) {
        free(data);
        data = NULL;
    } else {
        *out_size = (int)member->size;
    }
    nexus_iso_close(&iso);
    return data;
}

static const char *retail_root(char *out, size_t out_size) {
    const char *data_dir = getenv("FIRESTAFF_NEXUS_DATA_DIR");
    const char *home = getenv("HOME");

    if (data_dir && data_dir[0]) {
        snprintf(out, out_size, "%s", data_dir);
        return out;
    }
    if (home && home[0]) {
        snprintf(out, out_size, "%s/.firestaff/data/nexus", home);
        return out;
    }
    return NULL;
}

static uint16_t read_be16(const uint8_t *p) {
    return (uint16_t)(((uint16_t)p[0] << 8) | p[1]);
}

/* Compare the production PLRD projection against the exact bytes read from
 * each hash-authenticated regional retail member.  This checks format
 * preservation only; it does not assign gameplay meaning to equipment words
 * or PLRD's unlabelled fields. */
static int verify_retail_plrd_rows(const char *cue_name,
                                   const uint8_t *member_data,
                                   int member_size,
                                   const Nexus_V1_Engine *engine) {
    size_t plrd = 0U;
    Nexus_V1_ResDecodeResult resources;
    const Nexus_V1_ResEntry *tabl;
    int row;
    size_t offset;

    if (!cue_name || !member_data || member_size <= 0 || !engine ||
        engine->champions.champion_count != NEXUS_NEXUS_PLRD_CHAMPION_COUNT ||
        !nexus_v1_res_decode(member_data, member_size, &resources) ||
        !resources.valid || !(tabl = nexus_v1_res_find(&resources, "TABL", 0)) ||
        tabl->size < 8U + 216U * 2U)
        return 0;
    for (offset = 0; offset + 8U +
             NEXUS_NEXUS_PLRD_CHAMPION_COUNT * 64U + 4U <=
             (size_t)member_size; ++offset) {
        if (memcmp(member_data + offset, "PLRD", 4U) == 0 &&
            memcmp(member_data + offset + 8U +
                   NEXUS_NEXUS_PLRD_CHAMPION_COUNT * 64U,
                   "CRET", 4U) == 0) {
            plrd = offset + 8U;
            break;
        }
    }
    if (!plrd) {
        fprintf(stderr, "FAIL: %s::RLOWFIX.BIN PLRD/CRET row bounds\n",
                cue_name);
        return 0;
    }
    for (row = 0; row < NEXUS_NEXUS_PLRD_CHAMPION_COUNT; ++row) {
        const uint8_t *source = member_data + plrd + (size_t)row * 64U;
        const Nexus_V1_Champion *champion = &engine->champions.champions[row];
        int slot;
        int inventory_slot;
        if (!champion->roster_row_available || champion->health !=
                read_be16(source + 6U) || champion->stamina !=
                read_be16(source + 8U) || champion->mana !=
                read_be16(source + 10U) || champion->max_health !=
                read_be16(source + 6U) || champion->max_stamina !=
                read_be16(source + 8U) || champion->max_mana !=
                read_be16(source + 10U) || champion->luck != source[12] ||
            champion->strength != source[13] ||
            champion->dexterity != source[14] ||
            champion->wisdom != source[15] || champion->vitality != source[16] ||
            champion->anti_magic != source[17] ||
            champion->anti_fire != source[18] ||
            champion->fighter_level != source[19] ||
            champion->ninja_level != source[20] ||
            champion->priest_level != source[21] ||
            champion->wizard_level != source[22] ||
            champion->portrait_type != source[23] || champion->food != 0 ||
            champion->water != 0 || champion->gold != 0 || champion->alive != 0 ||
            champion->portrait_index != -1 || champion->name_ascii[0] != '\0' ||
            champion->name_jp[0] != '\0') {
            fprintf(stderr,
                    "FAIL: %s::RLOWFIX.BIN PLRD row %d field/source mismatch\n",
                    cue_name, row);
            return 0;
        }
        for (slot = 0; slot < 6; ++slot) {
            uint32_t code_offset = tabl->offset + 8U +
                (uint32_t)source[slot] * 2U;
            if (champion->name_tabl_index[slot] != source[slot] ||
                code_offset + 2U > tabl->offset + tabl->size ||
                champion->name_tabl_code[slot] !=
                    read_be16(member_data + code_offset)) {
                fprintf(stderr,
                        "FAIL: %s::RLOWFIX.BIN PLRD row %d TABL reference %d\n",
                        cue_name, row, slot);
                return 0;
            }
        }
        for (slot = 0; slot < NEXUS_SLOT_COUNT; ++slot) {
            int expected = -1;
            if (slot < 10) {
                uint16_t item = read_be16(source + 24U + 4U * (unsigned)slot);
                expected = item == 0xffffU ? -1 : (int)item;
            }
            if (champion->slots[slot] != expected) {
                fprintf(stderr,
                        "FAIL: %s::RLOWFIX.BIN PLRD row %d equipment word %d\n",
                        cue_name, row, slot);
                return 0;
            }
        }
        for (inventory_slot = 0; inventory_slot < 30; ++inventory_slot) {
            if (champion->inventory[inventory_slot] != 0xffU) {
                fprintf(stderr,
                        "FAIL: %s::RLOWFIX.BIN PLRD row %d unbound inventory\n",
                        cue_name, row);
                return 0;
            }
        }
    }
    printf("  PASS %s::RLOWFIX.BIN all 20 PLRD source rows\n", cue_name);
    return 1;
}

static int test_regional_member_identity(const char *cue_name,
                                         const char *member_name,
                                         const char *expected_md5) {
    char data_dir[1024];
    const char *root = retail_root(data_dir, sizeof(data_dir));
    char cue_path[1024];
    Nexus_V1_Engine engine;
    Nexus_V1_LevelAuxSourceReceipt receipt;
    Nexus_V1_ResDecodeResult decoded;
    uint8_t *member_data;
    int member_size = 0;
    int written;
    int result;
    FILE *cue_file;

    if (!root) {
        printf("  SKIP regional %s::%s (data directory unavailable)\n",
               cue_name, member_name);
        if (strcmp(cue_name, "Dungeon Master Nexus (Japan).cue") == 0 &&
            strcmp(member_name, "RLOWFIX.BIN") == 0)
            ++g_japan_real_skipped;
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    written = snprintf(cue_path, sizeof(cue_path), "%s/%s", root,
                       cue_name);
    if (written < 0 || (size_t)written >= sizeof(cue_path)) return 1;
    cue_file = fopen(cue_path, "rb");
    if (!cue_file) {
        printf("  SKIP regional %s::%s (CUE not staged)\n", cue_name,
               member_name);
        if (strcmp(cue_name, "Dungeon Master Nexus (Japan).cue") == 0 &&
            strcmp(member_name, "RLOWFIX.BIN") == 0)
            ++g_japan_real_skipped;
        return 0;
    }
    fclose(cue_file);
    memset(&engine, 0, sizeof(engine));
    result = nexus_v1_init(&engine, cue_path);
    if (result != 0 || engine.source != NEXUS_SRC_ISO ||
        nexus_v1_named_asset_source_receipt(&engine, member_name, &receipt) != 0 ||
        !receipt.exact_source_entry_observed ||
        !receipt.canonical_hash_verified ||
        strcmp(receipt.canonical_md5, expected_md5) != 0) {
        fprintf(stderr,
                "FAIL: %s::%s canonical identity init=%d source=%d "
                "exact=%d verified=%d md5=%s expected=%s\n",
                cue_name, member_name, result, (int)engine.source,
                receipt.exact_source_entry_observed,
                receipt.canonical_hash_verified, receipt.canonical_md5,
                expected_md5);
        if (engine.initialized) nexus_v1_shutdown(&engine);
        return 1;
    }
    member_data = nexus_v1_read_file(&engine, member_name, &member_size);
    if (!member_data || member_size <= 0 ||
        !nexus_v1_res_decode(member_data, member_size, &decoded) ||
        !decoded.valid) {
        fprintf(stderr, "FAIL: %s::%s authenticated RES* decode\n", cue_name,
                member_name);
        free(member_data);
        nexus_v1_shutdown(&engine);
        return 1;
    }
    if (strcmp(member_name, "RLOWFIX.BIN") == 0) {
        if (!verify_retail_plrd_rows(cue_name, member_data, member_size,
                                     &engine)) {
            free(member_data);
            nexus_v1_shutdown(&engine);
            return 1;
        }
    }
    free(member_data);
    printf("  PASS regional %s::%s md5=%s RES* entries=%u\n", cue_name,
           member_name, receipt.canonical_md5, decoded.entry_count);
    if (strcmp(cue_name, "Dungeon Master Nexus (Japan).cue") == 0 &&
        strcmp(member_name, "RLOWFIX.BIN") == 0)
        ++g_japan_real_passed;
    nexus_v1_shutdown(&engine);
    return 0;
}

static int test_french_logobg_identity(void) {
    char data_dir[1024];
    const char *root = retail_root(data_dir, sizeof(data_dir));
    const char *expected_md5 = "c594ac2c06e07a9e26a9945668a7b08a";
    char cue_path[1024];
    Nexus_V1_Engine engine;
    Nexus_V1_LevelAuxSourceReceipt receipt;
    Nexus_UI_Manager ui;
    uint8_t *member_data;
    int member_size = 0;
    int written;
    int result;
    FILE *cue_file;

    if (!root) {
        puts("  SKIP regional French::LOGOBG.DG2 (data directory unavailable)");
        return 0;
    }
    written = snprintf(cue_path, sizeof(cue_path),
                       "%s/Dungeon Master Nexus (French).cue", root);
    if (written < 0 || (size_t)written >= sizeof(cue_path)) return 1;
    cue_file = fopen(cue_path, "rb");
    if (!cue_file) {
        puts("  SKIP regional French::LOGOBG.DG2 (CUE not staged)");
        return 0;
    }
    fclose(cue_file);
    memset(&engine, 0, sizeof(engine));
    result = nexus_v1_init(&engine, cue_path);
    if (result != 0 || engine.source != NEXUS_SRC_ISO ||
        nexus_v1_named_asset_source_receipt(&engine, "LOGOBG.DG2",
                                             &receipt) != 0 ||
        !receipt.exact_source_entry_observed ||
        !receipt.canonical_hash_verified ||
        strcmp(receipt.canonical_md5, expected_md5) != 0) {
        fprintf(stderr, "FAIL: French LOGOBG.DG2 authentic identity\n");
        if (engine.initialized) nexus_v1_shutdown(&engine);
        return 1;
    }
    member_data = nexus_v1_read_file(&engine, "LOGOBG.DG2", &member_size);
    nexus_ui_manager_init(&ui);
    if (!member_data || member_size != 72198 ||
        nexus_ui_load_logobg(&ui, member_data, member_size, NULL) <= 0 ||
        ui.surfaces[NEXUS_SURFACE_LOGOBG].w != 320 ||
        ui.surfaces[NEXUS_SURFACE_LOGOBG].h != 224) {
        fprintf(stderr, "FAIL: French LOGOBG.DG2 bounded PP decode\n");
        free(member_data);
        nexus_ui_manager_free(&ui);
        nexus_v1_shutdown(&engine);
        return 1;
    }
    printf("  PASS regional French::LOGOBG.DG2 md5=%s PP=320x224\n",
           receipt.canonical_md5);
    free(member_data);
    nexus_ui_manager_free(&ui);
    nexus_v1_shutdown(&engine);
    return 0;
}

static int test_title_cg(void) {
    char root[512];
    uint8_t *data;
    int size = 0;
    Nexus_V1_TitleCgDecodeResult r;

    if (!retail_root(root, sizeof(root))) {
        printf("  SKIP title_cg (Nexus data root is unset)\n");
        return 0;
    }
    data = load_retail_file(root, "TITLE.CG", &size);
    if (!data) { printf("  SKIP title_cg (no file)\n"); return 0; }

    if (!nexus_v1_title_cg_decode(data, size, &r)) {
        printf("  FAIL title_cg decode\n");
        free(data);
        return 1;
    }

    if (r.tile_count != NEXUS_TITLE_CG_TILE_COUNT) {
        printf("  FAIL tile_count=%d expected=%d\n",
               r.tile_count, NEXUS_TITLE_CG_TILE_COUNT);
        free(data);
        return 1;
    }

    printf("  PASS title_cg: tiles=%d hash=0x%08X\n",
           r.tile_count, r.tile_hash);
    free(data);
    return 0;
}

static int test_res_file(const char *name) {
    char root[512];
    uint8_t *data;
    int size = 0, i;
    Nexus_V1_ResDecodeResult r;

    if (!retail_root(root, sizeof(root))) {
        return 0;
    }
    data = load_retail_file(root, name, &size);
    if (!data) { printf("  SKIP %s (not found)\n", name); return 0; }

    if (!nexus_v1_res_decode(data, size, &r)) {
        printf("  FAIL %s: RES* decode failed\n", name);
        free(data);
        return 1;
    }

    printf("  PASS %-14s entries=%d size=%u\n", name, r.entry_count, r.file_size);
    for (i = 0; i < r.entry_count && i < 8; ++i) {
        printf("    [%d] %s#%u off=0x%X size=%u\n",
               i, r.entries[i].tag, r.entries[i].index,
               r.entries[i].offset, r.entries[i].size);
    }
    if (r.entry_count > 8) printf("    ... +%d more\n", r.entry_count - 8);

    {
        uint8_t *tampered = (uint8_t *)malloc((size_t)size);
        if (!tampered) {
            free(data);
            return 1;
        }
        memcpy(tampered, data, (size_t)size);
        tampered[7] ^= 1U; /* RES* declared size must equal the source. */
        if (nexus_v1_res_decode(tampered, size, &r)) {
            free(tampered);
            free(data);
            return 1;
        }
        free(tampered);
    }

    free(data);
    return 0;
}

static int test_font012_headers(void) {
    char root[512];
    uint8_t *data;
    int size = 0;
    Nexus_V1_ResDecodeResult res;
    Nexus_V1_Font012Receipt receipt;
    uint8_t glyph[12 * 12];
    const uint32_t indices[] = {0U, 1U, 2U};
    const uint32_t counts[] = {291U, 250U, 710U};
    const uint32_t widths[] = {6U, 12U, 12U};
    const uint32_t offsets[] = {0xC0U, 0x1C2CU, 0x3F78U};
    int i;

    if (!retail_root(root, sizeof(root))) {
        return 0;
    }
    data = load_retail_file(root, "RLOWFIX.BIN", &size);
    if (!data) { printf("  SKIP FONT012 (no file)\n"); return 0; }
    if (!nexus_v1_res_decode(data, size, &res)) { free(data); return 1; }
    for (i = 0; i < 3; ++i) {
        const Nexus_V1_ResEntry *entry =
            nexus_v1_res_find(&res, "FONT", (int)indices[i]);
        if (!entry || entry->offset != offsets[i] ||
            !nexus_v1_font012_parse(data + entry->offset, entry->size,
                                     indices[i], &receipt) || !receipt.valid ||
            receipt.character_count != counts[i] ||
            receipt.character_width != widths[i] ||
            receipt.character_height != 12U) {
            free(data);
            return 1;
        }
        if (!nexus_v1_font012_decode_glyph(data + entry->offset, entry->size,
                                           indices[i], 0U, glyph,
                                           sizeof(glyph))) {
            free(data);
            return 1;
        }
    }
    free(data);
    puts("  PASS FONT012 headers: FONT#0/#1/#2 retail geometry admitted");
    return 0;
}

int main(int argc, char **argv) {
    int fail = 0;
    int real_japan_only = argc == 2 &&
        strcmp(argv[1], "--real-japan-only") == 0;
    if (argc > 1 && !real_japan_only) {
        fprintf(stderr, "usage: %s [--real-japan-only]\n", argv[0]);
        return 2;
    }
    printf("=== Nexus V1 TITLE.CG & RES* Decoder ===\n");
    if (!real_japan_only) {
        fail += test_title_cg();
        fail += test_res_file("TITLE.BIN");
        fail += test_res_file("RLOWFIX.BIN");
    }
    fail += test_regional_member_identity(
        "Dungeon Master Nexus (Japan).cue", "RLOWFIX.BIN",
        "bb650a4e6f7b6374ba8aa86a61f8f523");
    if (!real_japan_only) {
        fail += test_regional_member_identity(
            "Dungeon Master Nexus (English).cue", "RLOWFIX.BIN",
            "14c3a7e6fed2dc9e53a727640d4c9348");
        fail += test_regional_member_identity(
            "Dungeon Master Nexus (English).cue", "TITLE.BIN",
            "0b293be24d06eb550b27442ac9e8924c");
        fail += test_regional_member_identity(
            "Dungeon Master Nexus (French).cue", "RLOWFIX.BIN",
            "ecbecff383d6ee8330e68e38417be9c8");
        fail += test_regional_member_identity(
            "Dungeon Master Nexus (French).cue", "TITLE.BIN",
            "5c917a7db5bb0409d5d84086886c9aa6");
        fail += test_regional_member_identity(
            "Dungeon Master Nexus (French).cue", "GAMEOVER.BIN",
            "d692c8f25400cdcd44559194873c1e12");
        fail += test_french_logobg_identity();
        fail += test_res_file("RHIFIX.BIN");
        fail += test_res_file("POTEFT.BIN");
        fail += test_font012_headers();
    }
    printf("summary: fail=%d\n", fail);
    if (real_japan_only && fail == 0 && g_japan_real_passed == 0U &&
        g_japan_real_skipped != 0U) {
        puts("SKIP: authentic Japanese Nexus CUE/RLOWFIX.BIN was unavailable");
        return 77;
    }
    return fail ? 1 : 0;
}
