/*
 * test_csb_v1_amiga_asset_probe.c — classify CSB Amiga game data files
 * through the fingerprint table using known MD5 hex strings.
 *
 * Requires: ~/.firestaff/data/csb-amiga/ with extracted game files.
 */

#include "firestaff_game_data_fingerprint.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_classify_hex(const char *label, const char *md5hex,
                              FirestaffGame expected_game,
                              FirestaffPlatform expected_platform,
                              FirestaffFileType expected_type)
{
    FirestaffGameDataClassifyResult res =
        firestaff_game_data_classify_hex(md5hex);

    if (!res.valid) {
        printf("  FAIL: %s (%s) not in fingerprint table\n", label, md5hex);
        assert(0 && "fingerprint not found");
        return;
    }

    assert(res.entry->game == expected_game);
    assert(res.entry->platform == expected_platform);
    assert(res.entry->file_type == expected_type);
    printf("  PASS: %-20s => %s\n", label, res.entry->description);
}

int main(void)
{
    puts("test_csb_v1_amiga_asset_probe:");

    /* CSB Amiga v3.1/3.3 Multilanguage GRAPHICS.DAT */
    test_classify_hex("Graphics.DAT",
                      "61fbfd56887c94adc26888a9491c6611",
                      FIRESTAFF_GAME_CSB,
                      FIRESTAFF_PLATFORM_AMIGA,
                      FIRESTAFF_FILE_GRAPHICS_DAT);

    /* CSB Amiga v3.3 Dungeon.DAT (English default) */
    test_classify_hex("Dungeon.DAT",
                      "6695d2acebce49f95db1d8f3a5c733de",
                      FIRESTAFF_GAME_CSB,
                      FIRESTAFF_PLATFORM_AMIGA,
                      FIRESTAFF_FILE_DUNGEON_DAT);

    /* CSB Amiga v3.3 French DungeonF.DAT */
    test_classify_hex("DungeonF.DAT",
                      "5ece6270669693f7f48bd2e1e350cdb6",
                      FIRESTAFF_GAME_CSB,
                      FIRESTAFF_PLATFORM_AMIGA,
                      FIRESTAFF_FILE_DUNGEON_DAT);

    /* CSB Amiga v3.3 German DungeonG.DAT */
    test_classify_hex("DungeonG.DAT",
                      "7926a41466c7113c082a8d766b2e5a16",
                      FIRESTAFF_GAME_CSB,
                      FIRESTAFF_PLATFORM_AMIGA,
                      FIRESTAFF_FILE_DUNGEON_DAT);

    /* CSB Amiga v3.3 TITL.DAT */
    test_classify_hex("TITL.DAT",
                      "5b590ea3a6f5eed513b5678b01468ee4",
                      FIRESTAFF_GAME_CSB,
                      FIRESTAFF_PLATFORM_AMIGA,
                      FIRESTAFF_FILE_TITL_DAT);

    /* CSB Amiga v3.3 ENDA.DAT */
    test_classify_hex("ENDA.DAT",
                      "9f2b73ff73ad0032810d79021c900ca9",
                      FIRESTAFF_GAME_CSB,
                      FIRESTAFF_PLATFORM_AMIGA,
                      FIRESTAFF_FILE_ENDA_DAT);

    /* CSB Amiga 3.1 original KAOS.FTL, extracted from the original ADF. */
    test_classify_hex("KAOS.FTL",
                      "dbb79832c9cc3db82886ba8d3f72748a",
                      FIRESTAFF_GAME_CSB,
                      FIRESTAFF_PLATFORM_AMIGA,
                      FIRESTAFF_FILE_KAOS_FTL);

    /* CSB Amiga v3.3 SWSH.FTL */
    test_classify_hex("SWSH.FTL",
                      "ff3872baaed8ee4e83ee3c0684b2eeec",
                      FIRESTAFF_GAME_CSB,
                      FIRESTAFF_PLATFORM_AMIGA,
                      FIRESTAFF_FILE_SWSH_FTL);

    /* Pre-existing entries */
    test_classify_hex("HCSB.DAT",
                      "bbf3ada2da9722577feea4fa213b32f1",
                      FIRESTAFF_GAME_CSB,
                      FIRESTAFF_PLATFORM_AMIGA,
                      FIRESTAFF_FILE_HCSB_DAT);

    test_classify_hex("CEDTLS.DAT",
                      "b367d58374a799de88bc1a24c6320771",
                      FIRESTAFF_GAME_CSB,
                      FIRESTAFF_PLATFORM_AMIGA,
                      FIRESTAFF_FILE_CEDTLS_DAT);

    test_classify_hex("dragon.amg",
                      "bd85386535697df62bdbae73740fe435",
                      FIRESTAFF_GAME_CSB,
                      FIRESTAFF_PLATFORM_AMIGA,
                      FIRESTAFF_FILE_UTILITY_AMG);

    /* New file type enum values */
    assert(FIRESTAFF_FILE_DUNGEON_DAT == 11);
    assert(FIRESTAFF_FILE_TITL_DAT == 12);
    assert(FIRESTAFF_FILE_ENDA_DAT == 13);
    assert(FIRESTAFF_FILE_SWSH_FTL == 14);

    puts("ok: CSB Amiga asset probe (11 fingerprints verified)");
    return 0;
}
