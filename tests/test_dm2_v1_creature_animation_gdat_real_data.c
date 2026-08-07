/* Canonical GRAPHICS.DAT proof for the source-owned dynamic V5 creature
 * animation table route. No fixture, art, save, or runtime AI state is made. */
#include "dm2_v1_asset_loader.h"
#include "dm2_v1_creature.h"
#include "dm2_v1_creature_animation_gdat.h"
#include "dm2_v1_dungeon_loader.h"

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

int main(void)
{
    const char *root = getenv("FIRESTAFF_DM2_DATA_DIR");
    char path[1100];
    uint8_t *graphics = NULL;
    size_t graphics_size = 0u;
    uint8_t *dungeon_bytes = NULL;
    size_t dungeon_size = 0u;
    DM2_V1_AssetLoader loader;
    DM2_V1_DungeonData dungeon;
    int found = 0;
    int found_0958 = 0;
    int found_fd = 0;
    memset(&dungeon, 0, sizeof(dungeon));

    if (!root || !root[0]) {
        puts("SKIP: FIRESTAFF_DM2_DATA_DIR is not set");
        return 0;
    }
    /* Asset admission is hash based and case-preserving external media often
     * uses the DOS spelling. The real-data probe must not turn that into a
     * filename requirement. */
    snprintf(path, sizeof(path), "%s/graphics.dat", root);
    if (!read_file(path, &graphics, &graphics_size)) {
        snprintf(path, sizeof(path), "%s/GRAPHICS.DAT", root);
        if (!read_file(path, &graphics, &graphics_size)) {
            fputs("FAIL: selected canonical DM2 GRAPHICS.DAT is unreadable\n", stderr);
            return 1;
        }
    }
    memset(&loader, 0, sizeof(loader));
    if (dm2_v1_asset_loader_init(&loader, graphics, graphics_size) != 0) {
        fputs("FAIL: canonical GRAPHICS.DAT was not accepted\n", stderr);
        free(graphics);
        return 1;
    }
    if (dm2_v1_creature_load_ai_table_from_gdat(&loader) <= 0) {
        fputs("FAIL: selected GRAPHICS.DAT has no admitted source AI classification\n",
              stderr);
        dm2_v1_asset_loader_free(&loader);
        free(graphics);
        return 1;
    }

    /* SKProject's QUERY_CREATURE_AI_SPEC_FROM_TYPE is deliberately a
     * two-stage lookup: CREATURES[type].word(0x05) selects the AI row before
     * any AIDefinition fields are consumed.  The real PC-DOS corpus must
     * prove a non-identity mapping here; otherwise a direct type-as-row
     * fallback could masquerade as source parity. */
    {
        int mapped_types = 0;
        int remapped_types = 0;
        for (int creature = 0; creature < DM2_CREATURE_TYPE_COUNT; ++creature) {
            uint16_t row = 0u;
            const DM2_AIDefinition *def = NULL;
            int has_row = dm2_v1_creature_ai_row(creature, &row);
            int has_def = dm2_v1_creature_ai_spec_def(creature, &def);
            if (has_row != has_def || (has_row && (!def || row >= DM2_AI_TABLE_SIZE))) {
                fprintf(stderr,
                        "FAIL: real CREATURES/type AI-row receipt lost its two-stage owner "
                        "(type=%d row=%u has_row=%d has_def=%d def=%p)\n",
                        creature, row, has_row, has_def, (const void *)def);
                dm2_v1_asset_loader_free(&loader);
                free(graphics);
                return 1;
            }
            if (has_row) {
                ++mapped_types;
                if (row != (uint16_t)creature) ++remapped_types;
            }
        }
        /* The mounted PC-English profile is a fixed source corpus, not a
         * generic fixture: its 74 CREATURES word@0x05 owners contain 73
         * non-identity mappings. Types 54 and 127 have no source owner in
         * this profile and must remain unavailable rather than falling back
         * to type-as-row. */
        {
            uint16_t row = 0u;
            if (mapped_types != 74 || remapped_types != 73 ||
                dm2_v1_creature_ai_row(54, &row) != 0 ||
                dm2_v1_creature_ai_row(127, &row) != 0) {
                fprintf(stderr,
                        "FAIL: real PC-DOS AI owner census changed "
                        "(mapped=%d remapped=%d type54=%d type127=%d)\n",
                        mapped_types, remapped_types,
                        dm2_v1_creature_ai_row(54, &row),
                        dm2_v1_creature_ai_row(127, &row));
                dm2_v1_asset_loader_free(&loader);
                free(graphics);
                return 1;
            }
        }
        if (mapped_types == 0 || remapped_types == 0) {
            fputs("FAIL: real PC-DOS corpus did not prove a non-identity AI-row mapping\n",
                  stderr);
            dm2_v1_asset_loader_free(&loader);
            free(graphics);
            return 1;
        }
        printf("PASS: real CREATURES/type AI-row mapping (%d mapped, %d remapped)\n",
               mapped_types, remapped_types);
    }

    for (int creature = 0; creature < DM2_AI_TABLE_SIZE && !found; ++creature) {
        const DM2_AIDefinition *ai = dm2_v1_creature_ai_spec(creature);
        if (!ai || (ai->w0AIFlags & DM2_AIFLAG_STATIC) != 0u) continue;
        for (uint16_t command = 0u; command <= 0x40u; ++command) {
            DM2_V1_CreatureAnimationGdatReceipt receipt;
            if (dm2_v1_creature_animation_gdat_select_dynamic_v5(
                    &loader, creature, command, 0xffffu, ai->w0AIFlags, 2,
                    &receipt)) {
                if (!receipt.valid || !receipt.dynamic ||
                    receipt.creature_type != (uint8_t)creature ||
                    receipt.command != command ||
                    receipt.previous_frame != 0xffffu ||
                    receipt.direction != 2u || receipt.table_hash == 0u) {
                    fputs("FAIL: dynamic animation receipt lost source identity\n", stderr);
                    dm2_v1_asset_loader_free(&loader);
                    free(graphics);
                    return 1;
                }
                {
                    DM2_V1_CreatureAnimationGdatReceipt held;
                    if (!dm2_v1_creature_animation_gdat_select_dynamic_v5(
                                &loader, creature, command,
                                receipt.selected_frame, ai->w0AIFlags, 2,
                                &held) ||
                        held.selected_frame != receipt.selected_frame ||
                        held.previous_frame != receipt.selected_frame ||
                        held.image_id != receipt.image_id) {
                        fputs("FAIL: V5 animation step did not hold at the real FC terminator\n",
                              stderr);
                        dm2_v1_asset_loader_free(&loader);
                        free(graphics);
                        return 1;
                    }
                }
                {
                    DM2_V1_CreatureAnimation0958Receipt frame_receipt;
                    uint16_t timer_word = 0u;
                    if (dm2_v1_creature_animation_gdat_query_0958(
                            &loader, creature, receipt.sequence_offset,
                            &timer_word, 0u, &frame_receipt) != 1 ||
                        !frame_receipt.valid ||
                        frame_receipt.animation_base != receipt.sequence_offset ||
                        frame_receipt.query_index != 0u ||
                        frame_receipt.timer_word_before != 0u ||
                        frame_receipt.timer_word_after != 0u) {
                        fputs("FAIL: DM2_1c9a_0958 lost the real 0xfc row owner\n",
                              stderr);
                        dm2_v1_asset_loader_free(&loader);
                        free(graphics);
                        return 1;
                    }
                    {
                        uint8_t image_field = 0u;
                        if (!dm2_v1_creature_animation_gdat_image_field(
                                &loader, creature, frame_receipt.cursor_w2, 2u,
                                &image_field) || image_field == 0xffu) {
                            fputs("FAIL: real FD image selector was not source-bound\n",
                                  stderr);
                            dm2_v1_asset_loader_free(&loader);
                            free(graphics);
                            return 1;
                        }
                        found_fd = 1;
                    }
                    found_0958 = 1;
                }
                found = 1;
                break;
            }
        }
    }
    dm2_v1_asset_loader_free(&loader);
    free(graphics);
    if (!found || !found_0958 || !found_fd) {
        fputs("FAIL: selected GRAPHICS.DAT has no admitted dynamic V5 animation route\n",
              stderr);
        return 1;
    }

    /* Bind DM2_query_1c9a_02c3 to real G1 DB4 roots.  Dynamic roots must
     * stop at the absent CAII owner; they may not borrow the test's earlier
     * caller-supplied animation_base.  Static roots, when present, use the
     * DB4 +8 cursor directly. */
    {
        const char *root = getenv("FIRESTAFF_DM2_DATA_DIR");
        char dungeon_path[1100];
        int roots = 0;
        int dynamic_blocked = 0;
        int static_bound = 0;
        if (!root || !root[0]) {
            fputs("FAIL: real-data root disappeared before DB4 cursor audit\n",
                  stderr);
            dm2_v1_asset_loader_free(&loader);
            free(graphics);
            return 1;
        }
        snprintf(dungeon_path, sizeof(dungeon_path), "%s/dungeon.dat", root);
        if (!read_file(dungeon_path, &dungeon_bytes, &dungeon_size) ||
            dm2_v1_dungeon_load(&dungeon, dungeon_bytes,
                                (int)dungeon_size) != 0) {
            fputs("FAIL: real DUNGEON.DAT was not accepted for DB4 cursor audit\n",
                  stderr);
            dm2_v1_asset_loader_free(&loader);
            free(graphics);
            free(dungeon_bytes);
            return 1;
        }
        for (int map = 0; map < dungeon.level_count; ++map) {
            for (int x = 0; x < dungeon.level_widths[map]; ++x) {
                for (int y = 0; y < dungeon.level_heights[map]; ++y) {
                    DM2_V1_G1DungeonSceneClassificationReceipt scene;
                    const DM2_V1_G1DirectChainNode *node;
                    uint8_t *record;
                    const DM2_AIDefinition *ai;
                    DM2_V1_CreatureAnimation0958Receipt receipt;
                    int result;
                    memset(&scene, 0, sizeof(scene));
                    if (!dm2_v1_dungeon_classify_g1_direct_root_scene(
                            &dungeon, map, x, y, &scene) ||
                        scene.root_class != DM2_V1_G1_SCENE_ROOT_CREATURE ||
                        scene.chain.node_count < 1) continue;
                    node = &scene.chain.nodes[0];
                    if (node->type != 4 || node->record_offset < 0 ||
                        node->record_size < 12 ||
                        node->record_offset + node->record_size >
                            dungeon.raw_size) continue;
                    record = dungeon.raw_data + node->record_offset;
                    ai = dm2_v1_creature_ai_spec(record[4]);
                    if (!ai) continue;
                    ++roots;
                    result = dm2_v1_creature_animation_gdat_query_0958_record(
                        &loader, record, (size_t)node->record_size, ai,
                        NULL, 0u, 0u, &receipt);
                    if ((ai->w0AIFlags & DM2_AIFLAG_STATIC) != 0u) {
                        if (receipt.cursor_owner_bound &&
                            receipt.cursor_static_owner) ++static_bound;
                    } else if (!result && receipt.blocked_caii_owner) {
                        ++dynamic_blocked;
                    }
                }
            }
        }
        if (roots == 0 || (dynamic_blocked == 0 && static_bound == 0)) {
            fprintf(stderr,
                    "FAIL: real DB4 cursor owner audit had no source-bound outcome "
                    "(roots=%d dynamic_blocked=%d static_bound=%d)\n",
                    roots, dynamic_blocked, static_bound);
            dm2_v1_dungeon_free(&dungeon);
            dm2_v1_asset_loader_free(&loader);
            free(graphics);
            free(dungeon_bytes);
            return 1;
        }
        printf("PASS: real DB4 cursor owner audit (%d roots, %d CAII-gated, %d static)\n",
               roots, dynamic_blocked, static_bound);
        dm2_v1_dungeon_free(&dungeon);
    }
    free(dungeon_bytes);
    puts("PASS: canonical dynamic creature command resolves only through real FB/FC/FD GDAT tables");
    return 0;
}
