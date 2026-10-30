#include "dm1_snes_h03.h"
#include "firestaff_7z_extract.h"
#include "firestaff_dungeon_query.h"
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
