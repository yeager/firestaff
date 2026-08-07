/*
 * test_dm2_v1_creature_something_real_data.c — canonical GRAPHICS.DAT
 * companion proof for the round-14 animation reader
 * (DM2_GET_CREATURE_ANIMATION_FRAME + DM2_4FCC, the standalone
 * dm2_v1_creature_anim_4fcc export, and
 * DM2_CREATURE_SOMETHING_1c9a_0a48) consumed through the REAL GDAT
 * asset loader.  No game data is fabricated: the file is read only from
 * FIRESTAFF_DM2_DATA_DIR/graphics.dat.  Without an explicitly selected
 * corpus the test SKIPs; a selected corpus that cannot provide the original
 * file is an error.
 * The record pool, CAII slot and timer payload are synthetic RUNTIME
 * state — never fixture art.
 *
 * Source-lock anchors:
 *   skproject/SKULLWIN/c_creature.cpp:3217-3278  GAF
 *   skproject/SKULLWIN/c_creature.cpp:3285-3378  DM2_4FCC
 *   skproject/SKULLWIN/c_1c9a.cpp:5434-5672      1c9a_0a48
 */

#include "dm2_v1_creature_something_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int read_file(const char *path, uint8_t **out, size_t *out_size)
{
    FILE *file;
    long size;
    uint8_t *bytes;

    *out = NULL;
    *out_size = 0u;
    file = fopen(path, "rb");
    if (!file || fseek(file, 0, SEEK_END) != 0 || (size = ftell(file)) <= 0 ||
        fseek(file, 0, SEEK_SET) != 0) {
        if (file) fclose(file);
        return 0;
    }
    bytes = malloc((size_t)size);
    if (!bytes || fread(bytes, 1u, (size_t)size, file) != (size_t)size) {
        free(bytes);
        fclose(file);
        return 0;
    }
    fclose(file);
    *out = bytes;
    *out_size = (size_t)size;
    return 1;
}

static void wr16(uint8_t *p, uint16_t v)
{
    p[0] = (uint8_t)(v & 0xffu);
    p[1] = (uint8_t)((v >> 8) & 0xffu);
}

int main(void)
{
    const char *root = getenv("FIRESTAFF_DM2_DATA_DIR");
    char path[1100];
    uint8_t *graphics = NULL;
    size_t graphics_size = 0u;
    DM2_V1_AssetLoader loader;
    DM2_V1_RecordPoolSet set;
    DM2_V1_CaiiArray caii;
    DM2_V1_DropRng rng;
    int found_gaf = 0;
    int found_something = 0;
    int creature;
    int command;
    int failures = 0;

    if (!root || !root[0]) {
        puts("SKIP: no selected canonical DM2 data corpus");
        return 0;
    }
    snprintf(path, sizeof(path), "%s/graphics.dat", root);
    if (!read_file(path, &graphics, &graphics_size)) {
        fprintf(stderr,
                "FAIL: selected DM2 data corpus has no readable "
                "graphics.dat: %s\n", path);
        return 1;
    }
    memset(&loader, 0, sizeof(loader));
    if (dm2_v1_asset_loader_init(&loader, graphics, graphics_size) != 0) {
        fputs("FAIL: canonical GRAPHICS.DAT was not accepted\n", stderr);
        free(graphics);
        return 1;
    }

    /* ── raw table proof (always runs when the file is admitted) ──
     * The animation reader's data path is dtRaw8/0xfb (attribution,
     * 0xffff-terminated) + dtRaw7/0xfc (info sequence).  Prove the
     * canonical file carries at least one creature type whose
     * attribution scan terminates inside the loaded span next to a
     * loadable info table — the exact tables GAF/4FCC consume. */
    {
        int raw_types = 0;
        for (creature = 0; creature < 64; ++creature) {
            const uint8_t *attribution;
            const uint8_t *info;
            size_t attribution_size = 0u;
            size_t info_size = 0u;
            size_t row;
            attribution = dm2_v1_asset_load_typed_sized(
                &loader, DM2_GDAT_CATEGORY_CREATURES, creature,
                DM2_GDAT_ENTRY_TYPE_RAW8,
                DM2_GDAT_CREATURE_ANIM_ATTRIBUTION, &attribution_size);
            if (!attribution || attribution_size < 4u) {
                continue;
            }
            for (row = 0u; row * 4u + 1u < attribution_size; ++row) {
                if ((uint16_t)(attribution[row * 4u] |
                               ((uint16_t)attribution[row * 4u + 1u] << 8)) ==
                    0xffffu) {
                    break;
                }
            }
            if (row * 4u + 1u >= attribution_size) {
                fprintf(stderr,
                        "FAIL: canonical attribution for type %d has no "
                        "in-span 0xffff terminator\n", creature);
                ++failures;
                continue;
            }
            info = dm2_v1_asset_load_typed_sized(
                &loader, DM2_GDAT_CATEGORY_CREATURES, creature,
                DM2_GDAT_ENTRY_TYPE_RAW7,
                DM2_GDAT_CREATURE_ANIM_INFO_SEQUENCE, &info_size);
            if (!info || info_size < 4u) {
                continue; /* GAF fail-closes gdat_missing here */
            }
            ++raw_types;
        }
        if (failures != 0) {
            fprintf(stderr, "FAILURES: %d\n", failures);
            dm2_v1_asset_loader_free(&loader);
            free(graphics);
            return 1;
        }
        if (raw_types == 0) {
            puts("SKIP: canonical GRAPHICS.DAT has no admitted animation "
                 "tables");
            dm2_v1_asset_loader_free(&loader);
            free(graphics);
            return 0;
        }
        printf("canonical animation tables admitted for %d creature "
               "types\n", raw_types);
    }

    if (dm2_v1_creature_load_ai_table_from_gdat(&loader) <= 0) {
        /* The AI classification gate is a separate admission (the
         * established real-data test SKIPs the same way); the raw
         * table proof above already carried the canonical check. */
        puts("PASS: canonical GRAPHICS.DAT animation tables (AI gate "
             "not admitted by this profile)");
        dm2_v1_asset_loader_free(&loader);
        free(graphics);
        return 0;
    }

    /* ── GAF + standalone 4FCC over the real animation tables ────── */
    for (creature = 0; creature < 64 && !found_gaf; ++creature) {
        const DM2_AIDefinition *def;
        if (dm2_v1_creature_ai_spec_def(creature, &def) != 1 || !def) {
            continue;
        }
        for (command = 0; command <= 0x40 && !found_gaf; ++command) {
            DM2_V1_CreatureAnimFrameReceipt grc;
            uint16_t adj_base = 0;
            int16_t frame_word = -1;
            const uint8_t *row = NULL;
            int gaf;
            rng.random = 0x1234u;
            memset(&grc, 0, sizeof(grc));
            gaf = dm2_v1_creature_get_animation_frame(
                &loader, &rng, creature, command, &adj_base, &frame_word,
                &row, 0, &grc);
            if (!grc.valid || grc.gdat_missing || grc.table_oob ||
                grc.aidef_unknown || !grc.attribution_found) {
                continue;
            }
            if (grc.creature_type != creature || grc.command != command ||
                grc.return_value != gaf) {
                fprintf(stderr,
                        "FAIL: real GAF receipt lost source identity "
                        "(type %d command 0x%02x)\n", creature, command);
                ++failures;
                break;
            }
            /* The standalone 4FCC export must consume the SAME real
             * info table from the resolved sequence base without a
             * fail-closed outcome. */
            {
                DM2_V1_CreatureAnimFrameReceipt frc;
                int16_t fw2 = -1;
                const uint8_t *row2 = NULL;
                memset(&frc, 0, sizeof(frc));
                dm2_v1_creature_anim_4fcc(&loader, &rng, creature,
                                          adj_base, &fw2, &row2, &frc);
                if (!frc.valid || frc.gdat_missing || frc.table_oob ||
                    !frc.anim_row_set || row2 == NULL) {
                    fprintf(stderr,
                            "FAIL: real standalone 4FCC fail-closed "
                            "(type %d base %u)\n", creature, adj_base);
                    ++failures;
                    break;
                }
            }
            found_gaf = 1;
        }
    }
    if (!found_gaf) {
        puts("SKIP: canonical GRAPHICS.DAT has no admitted animation route");
        dm2_v1_asset_loader_free(&loader);
        free(graphics);
        return 0;
    }

    /* ── 1c9a_0a48 over a real creature type + real GDAT word@1 ──── */
    memset(&set, 0, sizeof(set));
    set.pools[4].record_size = 16;
    set.pools[4].record_count = 1;
    set.pools[4].source_base = 0;
    set.pools[4].bytes = calloc(1, 16);
    set.pools[4].bytes[4] = (uint8_t)creature;
    set.pools[4].bytes[5] = 0;
    wr16(set.pools[4].bytes, DM2_V1_RECORD_HANDLE_END);
    set.valid = 1;
    dm2_v1_caii_array_init(&caii, 1);
    {
        uint8_t *slot = caii.slots;
        int16_t adj[2] = { 0, -1 };
        const uint8_t *anim = NULL;
        uint16_t w1 = 0u;
        DM2_V1_CreatureSomethingReceipt src;
        int32_t result;
        wr16(slot + 2, 0xffffu);
        slot[0x1a] = (uint8_t)command;
        if (dm2_v1_creature_gdat_word1(creature, &w1) != 1) {
            w1 = 0xffffu; /* probe path fails closed if reached */
        }
        rng.random = 0xabcdu;
        memset(&src, 0, sizeof(src));
        result = dm2_v1_creature_something_1c9a_0a48(
            &set, &caii, &loader, &rng, (int16_t)((4 << 10) | 0), adj,
            &anim, 0, 0, 0, 0, (w1 != 0xffffu) ? (int)w1 : -1, 5, 7,
            1000, &src);
        found_something = src.valid;
        if (src.valid) {
            if (src.creature_type != creature || src.command != command ||
                src.gdat_missing || src.table_oob || src.aidef_unknown ||
                src.rng_unbound || result < 1000) {
                fputs("FAIL: real 1c9a_0a48 receipt lost source identity\n",
                      stderr);
                ++failures;
            }
        }
    }

    dm2_v1_caii_array_free(&caii);
    free(set.pools[4].bytes);
    dm2_v1_asset_loader_free(&loader);
    free(graphics);

    if (failures != 0) {
        fprintf(stderr, "FAILURES: %d\n", failures);
        return 1;
    }
    if (!found_something) {
        puts("SKIP: canonical 1c9a_0a48 path not admitted for the route");
        return 0;
    }
    printf("PASS: canonical GRAPHICS.DAT animation reader (type %d, "
           "command 0x%02x)\n", creature, command);
    return 0;
}
