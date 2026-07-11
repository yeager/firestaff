/*
 * firestaff_dm1_pc34_greatstone_items_visual_probe.c
 *
 * Real-data verifier for Greatstone/SCK's DM PC 3.4 graphics.dat mapfile.
 *
 * Scope:
 *   - Parse the local sck.jar@1.5.1 `dm_pc43_en_gd.map` descriptor.
 *   - Open a local DM1 PC 3.4 GRAPHICS.DAT through the same M11 asset
 *     loader used by runtime drawing.
 *   - For every visual IMG3 row, prove that the item number resolves to
 *     the same GRAPHICS.DAT index, expands to a non-empty 4-bit indexed
 *     bitmap, and only uses palette indices 0..15.
 *   - Verify that the mapfile palette declarations are the known PC 3.4
 *     visual palette tags and that key runtime anchor graphics line up
 *     with Firestaff's public viewport/UI placement seams.
 *
 * Non-scope:
 *   - SND3/TXT1/FNT1/RAW1 rows are counted but not rendered.
 *   - This is Firestaff runtime/data evidence, not a paired DOS screenshot
 *     pixel-parity claim.
 */

#include "asset_loader_m11.h"
#include "firestaff_sck_mapfile.h"
#include "m11_game_view.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define PROBE_NAME "firestaff_dm1_pc34_greatstone_items_visual_probe"
#define TARGET_MAPFILE "dm_pc43_en_gd.map"
#define MAX_PATH_BYTES 1024

static int g_failures = 0;

static void fail_check(const char* label) {
    fprintf(stderr, "FAIL %s\n", label);
    ++g_failures;
}

static int is_regular_file(const char* path) {
    struct stat st;
    if (!path || stat(path, &st) != 0) {
        return 0;
    }
    return S_ISREG(st.st_mode) ? 1 : 0;
}

static int read_text_file(const char* path, char** outText) {
    FILE* f;
    long len;
    char* buf;
    size_t got;

    if (!path || !outText) {
        return 0;
    }
    f = fopen(path, "rb");
    if (!f) {
        return 0;
    }
    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        return 0;
    }
    len = ftell(f);
    if (len < 0) {
        fclose(f);
        return 0;
    }
    rewind(f);
    buf = (char*)malloc((size_t)len + 1u);
    if (!buf) {
        fclose(f);
        return 0;
    }
    got = fread(buf, 1, (size_t)len, f);
    fclose(f);
    if (got != (size_t)len) {
        free(buf);
        return 0;
    }
    buf[len] = '\0';
    *outText = buf;
    return 1;
}

static int path_join(char* out,
                     size_t outBytes,
                     const char* left,
                     const char* right) {
    int written;
    if (!out || outBytes == 0u || !left || !right) {
        return 0;
    }
    written = snprintf(out, outBytes, "%s/%s", left, right);
    return written > 0 && (size_t)written < outBytes;
}

static int resolve_corpus_mapfile(char* out, size_t outBytes) {
    const char* env = getenv("FIRESTAFF_GREATSTONE_SCK_DIR");
    const char* home;
    char candidate[MAX_PATH_BYTES];

    if (env && env[0] != '\0') {
        if (path_join(candidate, sizeof(candidate), env, TARGET_MAPFILE) &&
            is_regular_file(candidate)) {
            snprintf(out, outBytes, "%s", candidate);
            return 1;
        }
        snprintf(candidate, sizeof(candidate), "%s/db/map/%s", env, TARGET_MAPFILE);
        if (is_regular_file(candidate)) {
            snprintf(out, outBytes, "%s", candidate);
            return 1;
        }
    }

    home = getenv("HOME");
    if (home && home[0] != '\0') {
        snprintf(candidate, sizeof(candidate),
                 "%s/.cache/firestaff/greatstone-sck-mapfiles/db/map/%s",
                 home,
                 TARGET_MAPFILE);
        if (is_regular_file(candidate)) {
            snprintf(out, outBytes, "%s", candidate);
            return 1;
        }
    }
    return 0;
}

static int resolve_graphics_dat(int argc,
                                char** argv,
                                char* out,
                                size_t outBytes) {
    const char* env = getenv("FIRESTAFF_DM1_GRAPHICS_DAT");
    const char* home;
    char candidate[MAX_PATH_BYTES];

    if (argc > 1 && argv[1] && argv[1][0] != '\0') {
        if (is_regular_file(argv[1])) {
            snprintf(out, outBytes, "%s", argv[1]);
            return 1;
        }
        if (path_join(candidate, sizeof(candidate), argv[1], "dm1/GRAPHICS.DAT") &&
            is_regular_file(candidate)) {
            snprintf(out, outBytes, "%s", candidate);
            return 1;
        }
        if (path_join(candidate, sizeof(candidate), argv[1], "GRAPHICS.DAT") &&
            is_regular_file(candidate)) {
            snprintf(out, outBytes, "%s", candidate);
            return 1;
        }
    }

    if (env && env[0] != '\0' && is_regular_file(env)) {
        snprintf(out, outBytes, "%s", env);
        return 1;
    }

    home = getenv("HOME");
    if (home && home[0] != '\0') {
        snprintf(candidate, sizeof(candidate), "%s/.firestaff/data/dm1/GRAPHICS.DAT", home);
        if (is_regular_file(candidate)) {
            snprintf(out, outBytes, "%s", candidate);
            return 1;
        }
        snprintf(candidate, sizeof(candidate), "%s/.firestaff/data/GRAPHICS.DAT", home);
        if (is_regular_file(candidate)) {
            snprintf(out, outBytes, "%s", candidate);
            return 1;
        }
    }
    return 0;
}

static int streq(const char* a, const char* b) {
    return a && b && strcmp(a, b) == 0;
}

static int starts_with(const char* s, const char* prefix) {
    size_t n;
    if (!s || !prefix) {
        return 0;
    }
    n = strlen(prefix);
    return strncmp(s, prefix, n) == 0;
}

static int known_visual_palette(const char* attrs) {
    return streq(attrs, "PAL1") ||
           streq(attrs, "PAL=DM_PAL_TITLE") ||
           streq(attrs, "PAL=DM_PAL_ENTRANCE") ||
           streq(attrs, "PAL=DM_PAL_CREDITS");
}

static const FirestaffSckMapfileV2Item* find_item(
    const FirestaffSckMapfileV2* map,
    const char* number) {
    return FirestaffSckMapfileV2_FindByNumber(map, number);
}

static int verify_desc_contains(const FirestaffSckMapfileV2* map,
                                const char* number,
                                const char* text) {
    const FirestaffSckMapfileV2Item* item = find_item(map, number);
    if (!item ||
        (!strstr(item->description, text) &&
         !strstr(item->longDescription, text))) {
        fprintf(stderr, "FAIL map item %s missing desc fragment \"%s\"\n",
                number ? number : "(null)",
                text ? text : "(null)");
        ++g_failures;
        return 0;
    }
    return 1;
}

static int verify_loaded_size(M11_AssetLoader* loader,
                              unsigned int graphic,
                              unsigned short wantW,
                              unsigned short wantH,
                              const char* label) {
    const M11_AssetSlot* slot = M11_AssetLoader_Load(loader, graphic);
    if (!slot || !slot->loaded || slot->width != wantW || slot->height != wantH) {
        fprintf(stderr, "FAIL anchor %s graphic=%u got=%ux%u want=%ux%u\n",
                label,
                graphic,
                slot ? (unsigned int)slot->width : 0u,
                slot ? (unsigned int)slot->height : 0u,
                (unsigned int)wantW,
                (unsigned int)wantH);
        ++g_failures;
        return 0;
    }
    return 1;
}

static void verify_runtime_anchor_contracts(const FirestaffSckMapfileV2* map,
                                            M11_AssetLoader* loader) {
    int graphic = -1;
    int x = -1;
    int y = -1;
    int w = -1;
    int h = -1;

    if (!M11_GameView_GetV1ViewportBaseGraphic(0, &graphic, &x, &y, &w, &h) ||
        graphic != 79 || x != 0 || y != 0 || w != 224 || h != 39) {
        fail_check("viewport ceiling anchor graphic 79 at 0,0 224x39");
    }
    verify_desc_contains(map, "0079", "Ceiling");
    verify_loaded_size(loader, 79u, 224u, 39u, "ceiling");

    if (!M11_GameView_GetV1ViewportBaseGraphic(1, &graphic, &x, &y, &w, &h) ||
        graphic != 78 || x != 0 || y != 39 || w != 224 || h != 97) {
        fail_check("viewport floor anchor graphic 78 at 0,39 224x97");
    }
    verify_desc_contains(map, "0078", "Floor");
    verify_loaded_size(loader, 78u, 224u, 97u, "floor");

    if (M11_GameView_GetV1DialogBackdropGraphicId() != 17) {
        fail_check("dialog/inventory backdrop graphic id 17");
    }
    verify_desc_contains(map, "0017", "Character Sheet");
    verify_loaded_size(loader, 17u, 224u, 136u, "character sheet/backdrop");

    if (M11_GameView_GetV1ActionAreaGraphicId() != 10) {
        fail_check("action area graphic id 10");
    }
    verify_desc_contains(map, "0010", "Item Actions Area");
    verify_loaded_size(loader, 10u, 87u, 45u, "action area");

    if (M11_GameView_GetV1SpellAreaBackgroundGraphicId() != 9 ||
        M11_GameView_GetV1SpellAreaLinesGraphicId() != 11) {
        fail_check("spell area graphic ids 9/11");
    }
    verify_desc_contains(map, "0009", "Spell Casting Area");
    verify_desc_contains(map, "0011", "Switches");
    verify_loaded_size(loader, 9u, 87u, 25u, "spell area");

    if (M11_GameView_GetV1MovementArrowsGraphicId() != 13) {
        fail_check("movement arrows graphic id 13");
    }
    verify_desc_contains(map, "0013", "Movement Arrows");

    if (M11_GameView_GetV1ChampionPortraitGraphicId() != 26 ||
        M11_GameView_GetV1ChampionIconGraphicId() != 28) {
        fail_check("champion portrait/icon graphic ids 26/28");
    }
    verify_desc_contains(map, "0026", "Champions' Portraits");
    verify_desc_contains(map, "0028", "Champion Positions");
    verify_loaded_size(loader, 26u, 256u, 87u, "champion portraits");

    if (M11_GameView_GetV1SlotBoxNormalGraphicId() != 33 ||
        M11_GameView_GetV1SlotBoxWoundedGraphicId() != 34 ||
        M11_GameView_GetV1SlotBoxActingHandGraphicId() != 35) {
        fail_check("slot box graphic ids 33/34/35");
    }
    verify_desc_contains(map, "0033", "Gray Border Item Slot");
    verify_desc_contains(map, "0034", "Red Border Item Slot");
    verify_desc_contains(map, "0035", "Cyan Border Item Slot");

    if (M11_GameView_GetV1OpenScrollPanelGraphicId() != 23 ||
        M11_GameView_GetV1InventoryPanelGraphicId() != 20 ||
        M11_GameView_GetV1ObjectDescriptionPanelGraphicId() != 20 ||
        M11_GameView_GetV1ObjectDescriptionCircleGraphicId() != 29) {
        fail_check("inventory panel graphic ids 20/23/29");
    }
    verify_desc_contains(map, "0020", "Empty Information Area");
    verify_desc_contains(map, "0023", "Open Scroll");
    verify_desc_contains(map, "0029", "Circle Displayed");

    if (M11_GameView_GetV1ArrowOrEyeGraphicId(0) != 18 ||
        M11_GameView_GetV1ArrowOrEyeGraphicId(1) != 19) {
        fail_check("arrow/eye graphic ids 18/19");
    }
    verify_desc_contains(map, "0018", "Arrow Showing");
    verify_desc_contains(map, "0019", "Eye Showing");

    if (M11_GameView_GetV1FoodLabelGraphicId() != 30 ||
        M11_GameView_GetV1WaterLabelGraphicId() != 31 ||
        M11_GameView_GetV1PoisonLabelGraphicId() != 32) {
        fail_check("food/water/poison labels 30/31/32");
    }
    verify_desc_contains(map, "0030", "'Food' Label");
    verify_desc_contains(map, "0031", "'Water' Label");
    verify_desc_contains(map, "0032", "'Poisoned' Label");

    if (M11_GameView_GetV1StatusBoxGraphicId() != 7 ||
        M11_GameView_GetV1DeadStatusBoxGraphicId() != 8 ||
        M11_GameView_GetV1PartyShieldBorderGraphicId() != 37 ||
        M11_GameView_GetV1FireShieldBorderGraphicId() != 38 ||
        M11_GameView_GetV1SpellShieldBorderGraphicId() != 39) {
        fail_check("status box/border graphic ids 7/8/37/38/39");
    }

    if (M11_GameView_GetV1ChampionSmallDamageGraphicId() != 15 ||
        M11_GameView_GetV1ChampionBigDamageGraphicId() != 16 ||
        M11_GameView_GetV1CreatureDamageGraphicId() != 14) {
        fail_check("damage graphic ids 14/15/16");
    }

    if (M11_GameView_GetV1EndgameTheEndGraphicId() != 6 ||
        M11_GameView_GetV1EndgameChampionMirrorGraphicId() != 346) {
        fail_check("endgame graphic ids 6/346");
    }
    verify_desc_contains(map, "0006", "'The End' Label");
    verify_desc_contains(map, "0346", "Champion Mirror");

    if (M11_GameView_GetWallSetGraphicIndex(0, 86) != 86 ||
        M11_GameView_GetWallSetGraphicIndex(0, 97) != 97 ||
        M11_GameView_GetWallSetGraphicIndex(0, 107) != 107 ||
        M11_GameView_GetWallSetGraphicIndex(1, 86) != 126 ||
        M11_GameView_GetWallSetGraphicIndex(1, 107) != 147) {
        fail_check("wall-set graphic placement formula");
    }
    verify_desc_contains(map, "0086", "Door Left or Right Frame");
    verify_desc_contains(map, "0097", "Wall (Front 1)");
    verify_desc_contains(map, "0107", "Wall (Front 3)");

    verify_desc_contains(map, "0042", "Items Graphics 0");
    verify_desc_contains(map, "0048", "Items And Body Parts Graphics 6");
    verify_desc_contains(map, "0454", "Missile");
    verify_desc_contains(map, "0486", "Explosion");
    verify_desc_contains(map, "0498", "Item on floor");
    verify_desc_contains(map, "0584", "Creature 00");
}

static void verify_visual_rows(const FirestaffSckMapfileV2* map,
                               M11_AssetLoader* loader,
                               unsigned int* outImg3,
                               unsigned int* outNull,
                               unsigned int* outSkipped,
                               unsigned int* outPal1,
                               unsigned int* outSpecialPal) {
    unsigned int i;

    for (i = 0; i < map->itemCount; ++i) {
        const FirestaffSckMapfileV2Item* item = &map->items[i];

        if (streq(item->type, "NULL")) {
            ++*outNull;
            if (!streq(item->attributes, "NULL")) {
                fprintf(stderr, "FAIL NULL row %s attr=%s\n",
                        item->number, item->attributes);
                ++g_failures;
            }
            continue;
        }
        if (!streq(item->type, "IMG3")) {
            ++*outSkipped;
            continue;
        }

        ++*outImg3;
        if (!known_visual_palette(item->attributes)) {
            fprintf(stderr, "FAIL IMG3 row %s unknown palette attr=%s\n",
                    item->number, item->attributes);
            ++g_failures;
        } else if (streq(item->attributes, "PAL1")) {
            ++*outPal1;
        } else {
            ++*outSpecialPal;
        }

        if (!item->hasNumericNumber || item->numericNumber >= loader->graphicCount) {
            fprintf(stderr, "FAIL IMG3 row %s invalid index count=%u\n",
                    item->number, (unsigned int)loader->graphicCount);
            ++g_failures;
            continue;
        }

        {
            const M11_AssetSlot* slot = M11_AssetLoader_Load(loader, item->numericNumber);
            unsigned long pixelCount;
            unsigned long p;
            unsigned int nonzero = 0u;

            if (!slot || !slot->loaded || !slot->pixels ||
                slot->graphicIndex != item->numericNumber ||
                slot->width == 0u || slot->height == 0u) {
                fprintf(stderr, "FAIL IMG3 row %s did not load desc=\"%s\"\n",
                        item->number, item->description);
                ++g_failures;
                continue;
            }

            pixelCount = (unsigned long)slot->width * (unsigned long)slot->height;
            for (p = 0; p < pixelCount; ++p) {
                if (slot->pixels[p] > 15u) {
                    fprintf(stderr, "FAIL IMG3 row %s pixel[%lu]=%u outside palette\n",
                            item->number, p, (unsigned int)slot->pixels[p]);
                    ++g_failures;
                    break;
                }
                if (slot->pixels[p] != 0u) {
                    ++nonzero;
                }
            }
            if (nonzero == 0u && !starts_with(item->description, "Empty image")) {
                fprintf(stderr, "FAIL IMG3 row %s all-zero visual desc=\"%s\"\n",
                        item->number, item->description);
                ++g_failures;
            }
        }
    }
}

int main(int argc, char** argv) {
    char mapPath[MAX_PATH_BYTES];
    char graphicsPath[MAX_PATH_BYTES];
    char* mapText = NULL;
    char err[256];
    FirestaffSckMapfileV2 map;
    M11_AssetLoader loader;
    unsigned int img3 = 0u;
    unsigned int nullRows = 0u;
    unsigned int skippedNonVisual = 0u;
    unsigned int pal1 = 0u;
    unsigned int specialPal = 0u;

    if (!resolve_corpus_mapfile(mapPath, sizeof(mapPath))) {
        printf("%s SKIPPED no Greatstone SCK mapfile corpus "
               "(run tools/fetch_greatstone_sck_mapfiles.sh)\n",
               PROBE_NAME);
        return 0;
    }
    if (!resolve_graphics_dat(argc, argv, graphicsPath, sizeof(graphicsPath))) {
        printf("%s SKIPPED no DM1 PC 3.4 GRAPHICS.DAT\n", PROBE_NAME);
        return 0;
    }
    if (!read_text_file(mapPath, &mapText)) {
        fprintf(stderr, "%s FAIL could not read %s\n", PROBE_NAME, mapPath);
        return 1;
    }

    memset(&map, 0, sizeof(map));
    memset(err, 0, sizeof(err));
    if (!FirestaffSckMapfile_ParseSck2Text(mapText, &map, err, sizeof(err))) {
        fprintf(stderr, "%s FAIL parse %s err=%s\n", PROBE_NAME, mapPath, err);
        free(mapText);
        return 1;
    }
    if (!streq(map.format, "DMCSB2") || !streq(map.endian, "LITTLE")) {
        fprintf(stderr, "%s FAIL map header format=%s endian=%s\n",
                PROBE_NAME, map.format, map.endian);
        free(mapText);
        return 1;
    }

    memset(&loader, 0, sizeof(loader));
    if (!M11_AssetLoader_Init(&loader, graphicsPath)) {
        fprintf(stderr, "%s FAIL could not init M11 asset loader for %s\n",
                PROBE_NAME, graphicsPath);
        free(mapText);
        return 1;
    }

    verify_visual_rows(&map,
                       &loader,
                       &img3,
                       &nullRows,
                       &skippedNonVisual,
                       &pal1,
                       &specialPal);
    verify_runtime_anchor_contracts(&map, &loader);

    printf("%s %s map=%s graphics=%s rows=%u img3=%u pal1=%u "
           "specialPal=%u null=%u nonVisualSkipped=%u graphicCount=%u\n",
           PROBE_NAME,
           g_failures == 0 ? "PASS" : "FAIL",
           mapPath,
           graphicsPath,
           map.itemCount,
           img3,
           pal1,
           specialPal,
           nullRows,
           skippedNonVisual,
           (unsigned int)loader.graphicCount);

    M11_AssetLoader_Shutdown(&loader);
    free(mapText);
    return g_failures == 0 ? 0 : 1;
}
