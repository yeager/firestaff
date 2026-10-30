#include "dm1_snes_h03.h"
#include "dm1_snes_romgh.h"
#include "firestaff_7z_extract.h"
#include "firestaff_dungeon_query.h"
#include "firestaff_snes_romimg3.h"
#include "firestaff_x68k_media_receipt.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    unsigned int seen_mask;
    unsigned int passed_mask;
    unsigned int failure;
} SnesRomAudit;

typedef struct {
    const char *name;
    const char *sha256;
    unsigned int bit;
} SnesRomIdentity;

static const char g_sck_decoded_dungeon_sha256[] =
    "910bdb71e3d1d2dfe02b6e7d7b80c453b2fdda7d51168df9ea9d9a89368882d2";
static const char g_sck_us10_interface_romimg3_sha256[] =
    "c8322c353f73c233f8ef4050145eabae8dfd3f4b31c341ea2c02edeedd29356c";
static const char g_sck_us10_interface_pixels_sha256[] =
    "583cdd5d1e35098057018140cdda25dbceb41febe004671dbccbe8e6d9dfd1e9";
static const char g_jp10_interface_romimg3_sha256[] =
    "b51c0f15ca691e5d3380876e193b9b5c039090b5c93c568b0f37401ae744ea16";
static const char g_jp10_interface_pixels_sha256[] =
    "843236d3e640d414b2a8f8299842eff7c9d45457d67408f30e9b576e567ef96a";

static const SnesRomIdentity g_snes_roms[] = {
    {"DungeonMasterSNESEU10.smc",
     "7810392d21ceb27bd4366426b7708da64ce97a9985fb6cdf92f3e6954ca777dc", 1u},
    {"DungeonMasterSNESJP10.smc",
     "215fdc22b0df03dc4014eb1a15e8b6cf08b4919dd64a5e587739078bb8a00d9c", 2u},
    {"DungeonMasterSNESJP11.smc",
     "09820f8668069afed09f64877213279770dc8dea085543463af23936e5710b42", 4u},
    {"DungeonMasterSNESUS10.smc",
     "c061f977085444c5e955d627b7528cabd54bef900dd0d565ff281f5cf2e798b8", 8u}
};

static const SnesRomIdentity *identity_for(const char *name) {
    size_t i;
    for (i = 0; i < sizeof(g_snes_roms) / sizeof(g_snes_roms[0]); ++i) {
        if (strcmp(name, g_snes_roms[i].name) == 0) return &g_snes_roms[i];
    }
    return NULL;
}

static int audit_member(const char *name, const uint8_t *bytes,
                        size_t byte_count, void *user_data) {
    SnesRomAudit *audit = (SnesRomAudit *)user_data;
    const SnesRomIdentity *identity = identity_for(name);
    char digest[65];
    uint8_t *dungeon;
    size_t part_a = 0u, part_b = 0u, consumed_a = 0u, consumed_b = 0u;
    int maps;

    if (!identity) return 0;
    if (!audit || !bytes || (audit->seen_mask & identity->bit) != 0u) {
        if (audit) audit->failure = 1u;
        return -1;
    }
    audit->seen_mask |= identity->bit;

    if (byte_count != 1049088u ||
        firestaff_x68k_media_receipt_sha256_hex(bytes, byte_count,
                                                 digest, sizeof(digest)) != 0 ||
        strcmp(digest, identity->sha256) != 0) {
        fprintf(stderr, "FAIL: DM1 SNES source identity changed: %s\n", name);
        audit->failure = 1u;
        return -1;
    }

    /* The archival ROMs contain a 512-byte copier header. ROMDUNGEON begins
     * at 0x40000 in the SNES address space, hence file offset 0x40200. */
    dungeon = (uint8_t *)malloc(40000u);
    if (!dungeon) {
        audit->failure = 1u;
        return -1;
    }
    if (!dm1_snes_h03_decode_slot(bytes + 0x40200u,
                                 DM1_SNES_H03_SLOT_SIZE,
                                 dungeon, 40000u, &part_a, &consumed_a) ||
        !dm1_snes_h03_decode_slot(bytes + 0x44200u,
                                 DM1_SNES_H03_SLOT_SIZE,
                                 dungeon + part_a, 40000u - part_a,
                                 &part_b, &consumed_b) ||
        part_a != 16384u || part_b != 16996u ||
        consumed_a != 13332u || consumed_b != 12034u) {
        fprintf(stderr, "FAIL: DM1 SNES H03 slots rejected: %s\n", name);
        free(dungeon);
        audit->failure = 1u;
        return -1;
    }

    if (firestaff_x68k_media_receipt_sha256_hex(
            dungeon, part_a + part_b, digest, sizeof(digest)) != 0 ||
        strcmp(digest, g_sck_decoded_dungeon_sha256) != 0) {
        fprintf(stderr, "FAIL: DM1 SNES DUNGEON differs from SCK output: %s\n",
                name);
        free(dungeon);
        audit->failure = 1u;
        return -1;
    }
    maps = fs_dungeon_load_dat(dungeon, (int)(part_a + part_b));
    free(dungeon);
    if (maps != 14 || fs_dungeon_get_start_x() != 1 ||
        fs_dungeon_get_start_y() != 3 || fs_dungeon_get_start_dir() != 2) {
        fprintf(stderr, "FAIL: DM1 SNES DUNGEON structure mismatch: %s\n", name);
        audit->failure = 1u;
        return -1;
    }

    {
        Dm1SnesRomghEntry floor_asset;
        Dm1SnesRomghEntry item_asset;
        Dm1SnesRomghEntry interface_asset;
        const size_t romgh_offset = 34256u; /* map offset 33744 + copier header */
        const size_t romgh_header_size = 2087u;

        if (!dm1_snes_romgh_resolve(bytes, byte_count, romgh_offset,
                                    romgh_header_size, 460u, &floor_asset) ||
            !dm1_snes_romgh_resolve(bytes, byte_count, romgh_offset,
                                    romgh_header_size, 470u, &item_asset) ||
            !dm1_snes_romgh_resolve(bytes, byte_count, romgh_offset,
                                    romgh_header_size, 477u, &interface_asset) ||
            floor_asset.source_offset != 559126u ||
            floor_asset.source_size != 5880u || floor_asset.compression != 0u ||
            item_asset.source_offset != 590336u ||
            item_asset.source_size != 4096u || item_asset.table_length != 4096u ||
            item_asset.compression != 0u ||
            interface_asset.source_offset != 623104u ||
            interface_asset.compression != 3u ||
            (strcmp(name, "DungeonMasterSNESJP10.smc") == 0 &&
             interface_asset.source_size != 4640u) ||
            (strcmp(name, "DungeonMasterSNESJP11.smc") == 0 &&
             interface_asset.source_size != 4640u) ||
            ((strcmp(name, "DungeonMasterSNESUS10.smc") == 0 ||
              strcmp(name, "DungeonMasterSNESEU10.smc") == 0) &&
             interface_asset.source_size != 4409u)) {
            fprintf(stderr, "FAIL: DM1 SNES ROMGH source spans differ: %s\n", name);
            audit->failure = 1u;
            return -1;
        }

        {
            uint8_t decoded_graphics[8192];
            uint8_t indexed_pixels[128u * 128u];
            const int is_japanese =
                strcmp(name, "DungeonMasterSNESJP10.smc") == 0 ||
                strcmp(name, "DungeonMasterSNESJP11.smc") == 0;
            size_t graphics_size = 0u, graphics_consumed = 0u;
            if (!dm1_snes_h03_decode_slot(bytes + interface_asset.source_offset,
                                          interface_asset.source_size,
                                          decoded_graphics, sizeof(decoded_graphics),
                                          &graphics_size, &graphics_consumed) ||
                graphics_size != sizeof(decoded_graphics) ||
                graphics_consumed != interface_asset.source_size) {
                fprintf(stderr, "FAIL: DM1 SNES ROMIMG3 H03 asset 999477: %s\n", name);
                audit->failure = 1u;
                return -1;
            }

            if (firestaff_x68k_media_receipt_sha256_hex(
                    decoded_graphics, graphics_size, digest, sizeof(digest)) != 0 ||
                strcmp(digest, is_japanese ? g_jp10_interface_romimg3_sha256 :
                                              g_sck_us10_interface_romimg3_sha256) != 0 ||
                FirestaffSnesRomimg3_DecodeTiles(
                    decoded_graphics, sizeof(decoded_graphics), 16u, 16u,
                    indexed_pixels, sizeof(indexed_pixels)) != 0 ||
                firestaff_x68k_media_receipt_sha256_hex(
                    indexed_pixels, sizeof(indexed_pixels), digest,
                    sizeof(digest)) != 0 ||
                strcmp(digest, is_japanese ? g_jp10_interface_pixels_sha256 :
                                              g_sck_us10_interface_pixels_sha256) != 0) {
                fprintf(stderr, "FAIL: DM1 SNES ROMIMG3 999477 decoded hash differs: %s\n",
                        name);
                audit->failure = 1u;
                return -1;
            }

            if (!is_japanese) {
                puts("PASS: authenticated DM1 SNES US10 ROMIMG3 999477 matches SCK");
            } else {
                puts("PASS: authenticated DM1 SNES JP ROMIMG3 999477 matches pinned decode");
            }
        }
    }
    audit->passed_mask |= identity->bit;
    printf("PASS: authenticated DM1 SNES ROMDUNGEON %s maps=%d bytes=%zu\n",
           name, maps, part_a + part_b);
    return 0;
}

int main(void) {
    const char *archive = getenv("FIRESTAFF_DM1_SNES_7Z");
    FILE *probe;
    SnesRomAudit audit;
    const unsigned int expected = 15u;

    if (!archive || !archive[0]) {
        puts("SKIP: authentic DM1 SNES archive path is not configured");
        return 77;
    }
    probe = fopen(archive, "rb");
    if (!probe) {
        puts("SKIP: authentic DM1 SNES archive is unavailable");
        return 77;
    }
    fclose(probe);

    memset(&audit, 0, sizeof(audit));
    if (!firestaff_7z_visit_files(archive, audit_member, &audit) ||
        audit.failure || audit.seen_mask != expected ||
        audit.passed_mask != expected) {
        fprintf(stderr, "FAIL: DM1 SNES ROM audit incomplete (seen=%u passed=%u)\n",
                audit.seen_mask, audit.passed_mask);
        return 1;
    }
    puts("PASS: all four authenticated DM1 SNES ROMDUNGEON revisions");
    return 0;
}
