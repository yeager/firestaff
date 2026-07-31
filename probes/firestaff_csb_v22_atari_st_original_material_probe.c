/*
 * Real-media regression for CSB-A02.
 *
 * The probe never creates bitmap data. When the locally staged Atari ST pair
 * is present, it re-hashes both files, verifies the normal CSB M11 entry
 * gate accepts them, then proves that V2.2 leaves ceiling, item, and field
 * shapes to the already-rendered V1 original-material pass. The original
 * bitmap identities remain intentionally unbound until the F0112/F0115
 * routes are decoded from GRAPHICS.DAT.
 *
 * ReDMCSB: DUNVIEW.C F0112 (ceiling/pit), F0115 (objects, projectiles,
 * explosions/fields), and F0128 (viewport order).
 * CSBWin: Viewport.cpp:7290 (viewport layout and composition boundary).
 */

#include "asset_status_m12.h"
#include "csb_v1_boot.h"
#include "csb_v22_inplace_route_pc34.h"
#include "csb_v22_shapes.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CSB_ATARI_ST_GRAPHICS_MD5 "e0ce7ac5160ca5540e90cf09ab9fad49"
#define CSB_ATARI_ST_DUNGEON_MD5  "6695d2acebce49f95db1d8f3a5c733de"

static int check_count = 0;
static int failure_count = 0;

static void check(const char *name, int ok)
{
    ++check_count;
    if (ok) {
        printf("PASS %s\n", name);
    } else {
        ++failure_count;
        printf("FAIL %s\n", name);
    }
}

static const char *probe_path(const char *env_name, const char *fallback)
{
    const char *value = getenv(env_name);
    return (value && value[0]) ? value : fallback;
}

static void check_unbound_route(int shape, const char *expected_reason,
                                const char *name)
{
    char asset_id[CSB_V22_ASSET_ID_MAX];
    char category[CSB_V22_CATEGORY_MAX];
    char reason[CSB_V22_REASON_MAX];
    int rc = csb_v22_inplace_route_for_shape(shape, 1,
                                              asset_id, sizeof(asset_id),
                                              category, sizeof(category),
                                              reason, sizeof(reason));
    check(name, rc == 0 && asset_id[0] == '\0' && category[0] == '\0' &&
          strcmp(reason, expected_reason) == 0);
}

int main(void)
{
    const char *graphics_path = probe_path(
        "FIRESTAFF_CSB_A02_GRAPHICS_DAT",
        "/Users/bosse/.firestaff/data/csb-atari-st-2x/GRAPHICS.DAT");
    const char *dungeon_path = probe_path(
        "FIRESTAFF_CSB_A02_DUNGEON_DAT",
        "/Users/bosse/.firestaff/data/csb-atari-st-2x/DUNGEON.DAT");
    char graphics_md5[33];
    char dungeon_md5[33];
    char reason[256];

    if (!m12_file_md5_hex(graphics_path, graphics_md5) ||
        !m12_file_md5_hex(dungeon_path, dungeon_md5)) {
        printf("SKIP CSB-A02 Atari ST pair not staged; set FIRESTAFF_CSB_A02_GRAPHICS_DAT and FIRESTAFF_CSB_A02_DUNGEON_DAT\n");
        return 0;
    }

    check("CSB_A02_ATARI_ST_GRAPHICS_HASH",
          strcmp(graphics_md5, CSB_ATARI_ST_GRAPHICS_MD5) == 0);
    check("CSB_A02_ATARI_ST_DUNGEON_HASH",
          strcmp(dungeon_md5, CSB_ATARI_ST_DUNGEON_MD5) == 0);
    check("CSB_A02_ATARI_ST_M11_GATE",
          csb_v1_boot_graphics_dungeon_m11_entry_gate(
              graphics_md5, dungeon_md5, reason, sizeof(reason)) == 1);

    check_unbound_route(CSB_V22_SHAPE_CEILING_PLAIN,
                        "v1_original_material_unbound_ceiling",
                        "CSB_A02_CEILING_RETAINS_V1");
    check_unbound_route(CSB_V22_SHAPE_ITEM,
                        "v1_original_material_unbound_item",
                        "CSB_A02_ITEM_RETAINS_V1");
    check_unbound_route(CSB_V22_SHAPE_FIELD_TELEPORTER,
                        "v1_original_material_unbound_field_teleporter",
                        "CSB_A02_FIELD_RETAINS_V1");
    printf("CSB-A02 Atari ST original-material probe: %d/%d passed\n",
           check_count - failure_count, check_count);
    return failure_count == 0 ? 0 : 1;
}
