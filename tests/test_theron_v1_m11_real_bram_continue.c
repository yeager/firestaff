#include "m11_game_view.h"
#include "theron_v1_mechanics.h"
#include "theron_v1_srm_classifier.h"
#include "theron_v1_startup_save_resume.h"
#include "theron_v1_track02_thing_data.h"
#include "theron_v1_world.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

static int fail(const char* message) {
    fprintf(stderr, "FAIL: %s\n", message);
    return 1;
}

static int set_test_environment(const char *name, const char *value) {
#if defined(_WIN32) || defined(_WIN64)
    return _putenv_s(name, value ? value : "") == 0;
#else
    return value ? setenv(name, value, 1) == 0 : unsetenv(name) == 0;
#endif
}

static int exercise_nearest_authentic_item_cycle(M11_GameViewState *view,
                                                 Theron_V1_World *world) {
    static const int movement_inputs[4] = {
        M12_MENU_INPUT_UP, M12_MENU_INPUT_STRAFE_RIGHT,
        M12_MENU_INPUT_DOWN, M12_MENU_INPUT_STRAFE_LEFT
    };
    int16_t parent_x[THERON_MAX_MAP_SIZE][THERON_MAX_MAP_SIZE];
    int16_t parent_y[THERON_MAX_MAP_SIZE][THERON_MAX_MAP_SIZE];
    int8_t parent_direction[THERON_MAX_MAP_SIZE][THERON_MAX_MAP_SIZE];
    int16_t queue_x[THERON_MAX_MAP_SIZE * THERON_MAX_MAP_SIZE];
    int16_t queue_y[THERON_MAX_MAP_SIZE * THERON_MAX_MAP_SIZE];
    uint8_t visited[THERON_MAX_MAP_SIZE][THERON_MAX_MAP_SIZE] = {{0}};
    int path_directions[THERON_MAX_MAP_SIZE * THERON_MAX_MAP_SIZE];
    int path_length = 0, queue_head = 0, queue_tail = 0;
    int target_x = -1, target_y = -1, target_direction = -1;
    int target_object_index = -1;
    const int dungeon = world->current_dungeon;
    const int level = world->current_level;
    int width, height;
    const int start_x = world->party.leader_x;
    const int start_y = world->party.leader_y;
    int facing = world->party.leader_dir & 3;

    if (dungeon < 1 || dungeon > THERON_DUNGEON_COUNT || level < 0 ||
        level >= THERON_MAX_LEVELS_PER_DUNGEON) return 0;
    width = world->levels[dungeon - 1][level].width;
    height = world->levels[dungeon - 1][level].height;
    if (width <= 0 || height <= 0 ||
        width > THERON_MAX_MAP_SIZE || height > THERON_MAX_MAP_SIZE ||
        start_x < 0 || start_y < 0 || start_x >= width || start_y >= height ||
        theron_v1_world_get_square(world, start_x, start_y) !=
            THERON_SQUARE_FLOOR) {
        return 0;
    }

    memset(parent_x, 0xff, sizeof(parent_x));
    memset(parent_y, 0xff, sizeof(parent_y));
    memset(parent_direction, 0xff, sizeof(parent_direction));
    queue_x[queue_tail] = (int16_t)start_x;
    queue_y[queue_tail++] = (int16_t)start_y;
    visited[start_y][start_x] = 1u;
    while (queue_head < queue_tail && target_object_index < 0) {
        const int x = queue_x[queue_head];
        const int y = queue_y[queue_head++];
        for (int direction = 0; direction < THERON_DIR_COUNT; ++direction) {
            const int item_x = x + g_theron_dir_dx[direction];
            const int item_y = y + g_theron_dir_dy[direction];
            if (item_x < 0 || item_y < 0 || item_x >= width ||
                item_y >= height) continue;
            for (int object_index = 0; object_index < world->object_count;
                 ++object_index) {
                const Theron_V1_Object *object =
                    &world->objects[object_index];
                const int carryable =
                    object->source_category == THERON_CAT_WEAPON ||
                    object->source_category == THERON_CAT_CLOTHING ||
                    object->source_category == THERON_CAT_SCROLL ||
                    object->source_category == THERON_CAT_POTION ||
                    object->source_category == THERON_CAT_MISC;
                if (object->dungeon_id == dungeon && object->level == level &&
                    object->x == item_x && object->y == item_y &&
                    object->source_origin_valid &&
                    object->source_property_valid && carryable &&
                    !(object->flags & THERON_OBJ_F_PICKED_UP)) {
                    target_x = x;
                    target_y = y;
                    target_direction = direction;
                    target_object_index = object_index;
                    break;
                }
            }
            if (target_object_index >= 0) break;
        }
        if (target_object_index >= 0) break;
        for (int direction = 0; direction < THERON_DIR_COUNT; ++direction) {
            const int nx = x + g_theron_dir_dx[direction];
            const int ny = y + g_theron_dir_dy[direction];
            if (nx < 0 || ny < 0 || nx >= width || ny >= height ||
                visited[ny][nx] ||
                theron_v1_world_get_square(world, nx, ny) !=
                    THERON_SQUARE_FLOOR) continue;
            visited[ny][nx] = 1u;
            parent_x[ny][nx] = (int16_t)x;
            parent_y[ny][nx] = (int16_t)y;
            parent_direction[ny][nx] = (int8_t)direction;
            queue_x[queue_tail] = (int16_t)nx;
            queue_y[queue_tail++] = (int16_t)ny;
        }
    }
    if (target_object_index < 0) return 0;

    for (int x = target_x, y = target_y; x != start_x || y != start_y;) {
        const int px = parent_x[y][x];
        const int py = parent_y[y][x];
        if (px < 0 || py < 0 ||
            path_length >= THERON_MAX_MAP_SIZE * THERON_MAX_MAP_SIZE)
            return 0;
        path_directions[path_length++] = parent_direction[y][x];
        x = px;
        y = py;
    }
    while (path_length > 0) {
        const int direction = path_directions[--path_length];
        const int relative = (direction - facing + THERON_DIR_COUNT) & 3;
        const int next_x = world->party.leader_x + g_theron_dir_dx[direction];
        const int next_y = world->party.leader_y + g_theron_dir_dy[direction];
        if (M11_GameView_HandleInput(view, movement_inputs[relative]) !=
                M11_GAME_INPUT_REDRAW || world->party.leader_x != next_x ||
            world->party.leader_y != next_y ||
            world->current_dungeon != dungeon || world->current_level != level)
            return 0;
        facing = direction;
    }
    while (facing != target_direction) {
        if (M11_GameView_HandleInput(view, M12_MENU_INPUT_TURN_RIGHT) !=
            M11_GAME_INPUT_REDRAW) return 0;
        facing = (facing + 1) & 3;
    }
    if (M11_GameView_HandleInput(view, M12_MENU_INPUT_PICKUP_ITEM) !=
            M11_GAME_INPUT_REDRAW ||
        !(world->objects[target_object_index].flags &
          THERON_OBJ_F_PICKED_UP)) return 0;
    if (world->party.active_slot < 0 ||
        world->party.active_slot >= THERON_MAX_CHAMPIONS) return 0;
    {
        int inventory_slot = -1;
        const int champion_slot = world->party.active_slot;
        for (int slot = 0; slot < THERON_INVENTORY_SLOTS; ++slot) {
            const Theron_V1_InventorySourceRecord *carried =
                &world->inventory_source[champion_slot][slot];
            if (carried->valid && carried->source_origin_valid &&
                carried->source_ref ==
                    world->objects[target_object_index].source_ref &&
                carried->source_index ==
                    world->objects[target_object_index].source_index &&
                carried->category ==
                    world->objects[target_object_index].source_category &&
                carried->property_valid &&
                memcmp(carried->property,
                       world->objects[target_object_index].source_property,
                       sizeof(carried->property)) == 0 &&
                carried->source_raw_size ==
                    world->objects[target_object_index].source_raw_size &&
                memcmp(carried->source_raw,
                       world->objects[target_object_index].source_raw,
                       carried->source_raw_size) == 0) {
                inventory_slot = slot;
                break;
            }
        }
        if (inventory_slot < 0) return 0;
        {
            const Theron_V1_InventorySourceRecord source =
                world->inventory_source[champion_slot][inventory_slot];
            int dropped_object_found = 0;
            if (M11_GameView_HandleInput(view, M12_MENU_INPUT_DROP_ITEM) !=
                M11_GAME_INPUT_REDRAW ||
                world->inventory_source[champion_slot][inventory_slot].valid ||
                world->party.champions[champion_slot].inventory[inventory_slot] !=
                    THERON_ITEM_NONE) return 0;
            for (int object_index = 0; object_index < world->object_count;
                 ++object_index) {
                const Theron_V1_Object *object = &world->objects[object_index];
                if (object->dungeon_id == dungeon && object->level == level &&
                    object->x == world->party.leader_x &&
                    object->y == world->party.leader_y &&
                    object->source_origin_valid &&
                    !(object->flags & THERON_OBJ_F_PICKED_UP) &&
                    object->source_ref == source.source_ref &&
                    object->source_index == source.source_index &&
                    object->source_category == source.category &&
                    object->source_property_valid &&
                    memcmp(object->source_property, source.property,
                           sizeof(source.property)) == 0 &&
                    object->source_raw_size == source.source_raw_size &&
                    memcmp(object->source_raw, source.source_raw,
                           source.source_raw_size) == 0) {
                    dropped_object_found = 1;
                    break;
                }
            }
            return dropped_object_found;
        }
    }
}

int main(int argc, char** argv) {
    M11_GameLaunchSpec spec;
    M11_GameViewState view;
    Theron_V1_World* world;
    Theron_V1PceBramBodyReceipt source_body;
    int source_body_ready;
    int i;
    int failures = 0;
    char saved_bram_path[THERON_V1_SRM_PATH_MAX] = {0};
    uint8_t saved_bram_before_dungeon[THERON_V1_PCE_BRAM_BYTES];

    if (argc != 4) {
        fprintf(stderr, "usage: %s DATA_DIR TRACK02 BRAM\n", argv[0]);
        return 2;
    }

    memset(&spec, 0, sizeof(spec));
    spec.title = "THERON'S QUEST";
    spec.gameId = "theron";
    spec.sourceId = "theron";
    spec.dataDir = argv[1];
    spec.verifiedAssetPath = argv[2];
    spec.verifiedAssetMd5 = "f23601102138f87c33025877767ebf76";
    spec.savePath = argv[3];
    spec.rendererBackend = M12_RENDERER_BACKEND_SOFTWARE;
    spec.presentationMode = M12_PRESENTATION_V1_ORIGINAL;
    spec.sourceKind = M11_GAME_SOURCE_BUILTIN_CATALOG;

    M11_GameView_Init(&view);
    if (!M11_GameView_Start(&view, &spec)) {
        fprintf(stderr, "M11 startup detail: %s\n", view.inspectDetail);
        return fail("M11 starts with authenticated US Track 02 and Backup RAM");
    }
    world = (Theron_V1_World*)view.theronWorld;
    source_body_ready = theron_v1_pce_bram_decode_original_body_path(
        argv[3], &source_body) && source_body.layout_verified &&
        source_body.semantics_verified;
    if (!source_body_ready) {
        ++failures;
        fprintf(stderr, "FAIL: authentic Continue test decodes the source-verified Backup RAM body\n");
    }
    if (!world || view.theronState.save_resume_claim !=
                      THERON_V1_STARTUP_RESUME_SRM ||
        view.theronState.save_resume_srm_active_slot != 0 ||
        view.theronState.save_resume_srm_import_status !=
            THERON_V1_SRM_PROGRESS_IMPORT_OK) {
        ++failures;
        fprintf(stderr, "FAIL: M11 admits the authenticated original Backup RAM slot\n");
    }
    if (view.theronState.startup_phase != THERON_STARTUP_PHASE_TITLE ||
        M11_GameView_HandleInput(&view, M12_MENU_INPUT_ACCEPT) !=
            M11_GAME_INPUT_REDRAW ||
        view.theronState.startup_phase !=
            THERON_STARTUP_PHASE_STAGE_SELECT) {
        ++failures;
        fprintf(stderr, "FAIL: authenticated M11 title opens stage selection\n");
    }
    if (M11_GameView_HandleInput(&view, M12_MENU_INPUT_UP) !=
            M11_GAME_INPUT_REDRAW ||
        !view.theronState.save_resume_continue_focus) {
        ++failures;
        fprintf(stderr, "FAIL: Continue receives focus in the authentic startup menu\n");
    }
    if (M11_GameView_HandleInput(&view, M12_MENU_INPUT_ACCEPT) !=
        M11_GAME_INPUT_REDRAW) {
        ++failures;
        fprintf(stderr, "FAIL: explicit M11 Continue action is handled\n");
    }
    if (!strstr(view.inspectDetail, "continued original Backup RAM slot=0")) {
        ++failures;
        fprintf(stderr, "FAIL: M11 Continue receipt identifies the authentic slot (detail=%s)\n",
                view.inspectDetail);
    }
    if (view.theronState.startup_phase !=
            THERON_STARTUP_PHASE_STAGE_SELECT ||
        view.theronState.level_loaded) {
        ++failures;
        fprintf(stderr, "FAIL: Continue remains at the source-backed stage boundary before an explicit dungeon-entry action\n");
    }
    if (M11_GameView_HandleInput(&view, M12_MENU_INPUT_SAVE_GAME) !=
            M11_GAME_INPUT_REDRAW ||
        strcmp(view.lastOutcome, "GAME SAVED") != 0 ||
        strcmp(view.inspectTitle, "THERON SAVE") != 0 ||
        strncmp(view.inspectDetail,
                "ORIGINAL BACKUP RAM SAVED TO ", 29) != 0) {
        ++failures;
        fprintf(stderr,
                "FAIL: authentic Continue progress saves at the source-owned between-stage boundary (outcome=%s template=%s title=%s detail=%s)\n",
                view.lastOutcome, view.theronState.save_resume_srm_root,
                view.inspectTitle, view.inspectDetail);
    } else {
        const char *path = view.inspectDetail + 29;
        Theron_V1PceBramReceipt saved_receipt;
        FILE *saved_file;
        size_t saved_bytes;
        snprintf(saved_bram_path, sizeof(saved_bram_path), "%s", path);
        saved_file = fopen(saved_bram_path, "rb");
        saved_bytes = saved_file
            ? fread(saved_bram_before_dungeon, 1,
                    sizeof(saved_bram_before_dungeon), saved_file)
            : 0u;
        if (saved_file) fclose(saved_file);
        if (saved_bytes != sizeof(saved_bram_before_dungeon) ||
            !saved_bram_path[0] ||
            theron_v1_pce_bram_classify_path(
                saved_bram_path, &saved_receipt) !=
                THERON_V1_PCE_BRAM_READY ||
            !saved_receipt.save_body_layout_proven ||
            !saved_receipt.save_body_campaign_valid) {
            ++failures;
            fprintf(stderr,
                    "FAIL: stage-boundary input creates a complete original BRAM file at %s\n",
                    saved_bram_path);
        }
    }
    if (!world || !source_body_ready ||
        source_body.ram_267c_campaign_byte != 0x01u ||
        world->campaign_completion_mask != 0x01u ||
        world->progression.dungeon_states[0] !=
            THERON_DUNGEON_STATE_COMPLETE ||
        world->progression.quest_items_collected != 0u ||
        world->progression.quest_complete) {
        ++failures;
        fprintf(stderr,
                "FAIL: authentic Akutuba campaign bit restores chapter completion without inventing quest-item collection\n");
    }
    if (world->party.champions[0].max_health != 175 ||
        world->party.champions[0].max_stamina != 1500 ||
        world->party.champions[0].max_mana != 50 ||
        world->timer_count != 0) {
        ++failures;
        fprintf(stderr, "FAIL: M11 Continue applies only authenticated Theron save fields (max=%u/%u/%u objects=%u timers=%u)\n",
                world->party.champions[0].max_health,
                world->party.champions[0].max_stamina,
                world->party.champions[0].max_mana,
                world->object_count, world->timer_count);
    }
    if (source_body_ready) {
        const Theron_V1_Champion *theron = &world->party.champions[0];
        const int16_t attributes[7] = {
            theron->luck, theron->strength, theron->dexterity,
            theron->wisdom, theron->vitality, theron->anti_magic,
            theron->anti_fire
        };
        int body_mismatches = 0;
        for (i = 0; i < 7; ++i) {
            if (attributes[i] != source_body.theron_max_attributes[i]) {
                ++body_mismatches;
            }
        }
        for (i = 0; i < 20; ++i) {
            if (theron->skill_temporary_experience[i] !=
                    source_body.theron_skill_temporary_experience[i] ||
                theron->skill_experience[i] !=
                    source_body.theron_skill_experience[i]) {
                ++body_mismatches;
            }
        }
        if (body_mismatches != 0) {
            ++failures;
            fprintf(stderr,
                    "FAIL: authentic Continue restores all seven source attributes and 20 temporary/persistent skill-experience pairs (%d mismatches)\n",
                    body_mismatches);
        }
    }
    if (M11_GameView_HandleInput(&view, M12_MENU_INPUT_DOWN) !=
            M11_GAME_INPUT_REDRAW ||
        view.theronState.startup_phase !=
            THERON_STARTUP_PHASE_STAGE_SELECT ||
        view.theronState.selected_dungeon != 2) {
        ++failures;
        fprintf(stderr,
                "FAIL: authentic Continue skips the completed first chapter and focuses unlocked dungeon 2 (phase=%d dungeon=%d)\n",
                view.theronState.startup_phase,
                view.theronState.selected_dungeon);
    } else if (M11_GameView_HandleInput(&view, M12_MENU_INPUT_ACCEPT) !=
                   M11_GAME_INPUT_REDRAW ||
               view.theronState.startup_phase !=
                   THERON_STARTUP_PHASE_SOUL_ROOM ||
               view.theronState.selected_dungeon != 2 ||
               view.theronState.level_loaded) {
        ++failures;
        fprintf(stderr,
                "FAIL: authentic Continue enters the source-backed next chapter's Soul Room before forcefield admission (phase=%d dungeon=%d level_loaded=%d)\n",
                view.theronState.startup_phase,
                view.theronState.selected_dungeon,
                view.theronState.level_loaded);
    } else if (view.theronState.startup_roster_name_count <= 0 ||
               M11_GameView_HandleInput(&view, M12_MENU_INPUT_ACCEPT) !=
                   M11_GAME_INPUT_REDRAW ||
               view.theronState.startup_phase !=
                   THERON_STARTUP_PHASE_READY ||
               view.theronState.selected_mirrors_mask == 0) {
        ++failures;
        fprintf(stderr,
                "FAIL: authentic Track 02 roster admits a Soul Room champion selection without a generated name (phase=%d roster=%d mirrors=0x%x)\n",
                view.theronState.startup_phase,
                view.theronState.startup_roster_name_count,
                view.theronState.selected_mirrors_mask);
    }
    if (view.theronState.startup_phase == THERON_STARTUP_PHASE_READY &&
        view.theronState.selected_dungeon == 2 &&
        view.theronState.selected_mirrors_mask != 0) {
        for (i = 0;
             i <= THERON_STARTUP_HERO_MIRROR_COUNT &&
             view.theronState.startup_cursor !=
                 THERON_STARTUP_HERO_MIRROR_COUNT;
             ++i) {
            if (M11_GameView_HandleInput(&view, M12_MENU_INPUT_DOWN) !=
                M11_GAME_INPUT_REDRAW) {
                break;
            }
        }
        if (view.theronState.startup_cursor !=
            THERON_STARTUP_HERO_MIRROR_COUNT) {
            ++failures;
            fprintf(stderr,
                    "FAIL: authentic Soul Room navigation reaches the forcefield after source roster selection (cursor=%d)\n",
                    view.theronState.startup_cursor);
        } else if (M11_GameView_HandleInput(&view, M12_MENU_INPUT_ACCEPT) !=
                       M11_GAME_INPUT_REDRAW ||
                   view.theronState.startup_phase !=
                       THERON_STARTUP_PHASE_IN_DUNGEON ||
                   !view.theronState.level_loaded ||
                   world->current_dungeon != 2 ||
                   world->current_level != 0 ||
                   !world->levels[1][0].source_header_verified ||
                   world->object_count <= 0 ||
                   world->party.champion_count != 2) {
            ++failures;
            fprintf(stderr,
                    "FAIL: authenticated Continue -> dungeon 2 forcefield handoff loads original level and selected party (phase=%d dungeon=%d level_loaded=%d world=%d/%d objects=%d champions=%d)\n",
                    view.theronState.startup_phase,
                    view.theronState.selected_dungeon,
                    view.theronState.level_loaded,
                    world->current_dungeon,
                    world->current_level,
                    world->object_count,
                    world->party.champion_count);
        } else {
            M11_GameInputResult save_result;
            M11_GameView_DismissDialogOverlay(&view);
            save_result = M11_GameView_HandleInput(
                &view, M12_MENU_INPUT_SAVE_GAME);
            if (save_result != M11_GAME_INPUT_REDRAW ||
                view.dialogOverlayActive) {
                ++failures;
                fprintf(stderr,
                        "FAIL: in-dungeon Theron save input result=%d dialog=%d phase=%d level=%d\n",
                        save_result, view.dialogOverlayActive,
                        view.theronState.startup_phase,
                        view.theronState.level_loaded);
            }
            {
                uint8_t saved_bram_after_dungeon[
                    THERON_V1_PCE_BRAM_BYTES];
                FILE *saved_file = saved_bram_path[0]
                    ? fopen(saved_bram_path, "rb") : NULL;
                size_t saved_bytes = saved_file
                    ? fread(saved_bram_after_dungeon, 1,
                            sizeof(saved_bram_after_dungeon), saved_file)
                    : 0u;
                if (saved_file) fclose(saved_file);
                if (saved_bytes != sizeof(saved_bram_after_dungeon) ||
                    memcmp(saved_bram_before_dungeon,
                           saved_bram_after_dungeon,
                           sizeof(saved_bram_after_dungeon)) != 0) {
                    ++failures;
                    fprintf(stderr,
                            "FAIL: in-dungeon save input leaves the last original BRAM file unchanged\n");
                }
            }
            static const int movement_inputs[4] = {
                M12_MENU_INPUT_UP,
                M12_MENU_INPUT_STRAFE_RIGHT,
                M12_MENU_INPUT_DOWN,
                M12_MENU_INPUT_STRAFE_LEFT
            };
            int16_t parent_x[THERON_MAX_MAP_SIZE][THERON_MAX_MAP_SIZE];
            int16_t parent_y[THERON_MAX_MAP_SIZE][THERON_MAX_MAP_SIZE];
            int16_t queue_x[THERON_MAX_MAP_SIZE * THERON_MAX_MAP_SIZE];
            int16_t queue_y[THERON_MAX_MAP_SIZE * THERON_MAX_MAP_SIZE];
            int8_t parent_direction[THERON_MAX_MAP_SIZE]
                                   [THERON_MAX_MAP_SIZE];
            uint8_t visited[THERON_MAX_MAP_SIZE][THERON_MAX_MAP_SIZE] = {{0}};
            int path_directions[THERON_MAX_MAP_SIZE * THERON_MAX_MAP_SIZE];
            int path_length = 0;
            int queue_head = 0;
            int queue_tail = 0;
            int target_x = -1;
            int target_y = -1;
            int start_x = world->party.leader_x;
            int start_y = world->party.leader_y;
            int facing = world->party.leader_dir & 3;
            int moved_steps = 0;
            memset(parent_x, 0xff, sizeof(parent_x));
            memset(parent_y, 0xff, sizeof(parent_y));
            memset(parent_direction, 0xff, sizeof(parent_direction));
            if (start_x >= 0 && start_y >= 0 &&
                start_x < world->levels[1][0].width &&
                start_y < world->levels[1][0].height &&
                theron_v1_world_get_square(world, start_x, start_y) ==
                    THERON_SQUARE_FLOOR) {
                queue_x[queue_tail] = (int16_t)start_x;
                queue_y[queue_tail++] = (int16_t)start_y;
                visited[start_y][start_x] = 1u;
            }
            while (queue_head < queue_tail && target_x < 0) {
                int x = queue_x[queue_head];
                int y = queue_y[queue_head++];
                int depth = 0;
                int walk_x = x;
                int walk_y = y;
                while (walk_x != start_x || walk_y != start_y) {
                    int px = parent_x[walk_y][walk_x];
                    int py = parent_y[walk_y][walk_x];
                    if (px < 0 || py < 0 || ++depth >= 3) break;
                    walk_x = px;
                    walk_y = py;
                }
                if ((x != start_x || y != start_y) && depth >= 3) {
                    target_x = x;
                    target_y = y;
                    break;
                }
                for (int direction = 0; direction < THERON_DIR_COUNT;
                     ++direction) {
                    int nx = x + g_theron_dir_dx[direction];
                    int ny = y + g_theron_dir_dy[direction];
                    if (nx < 0 || ny < 0 ||
                        nx >= world->levels[1][0].width ||
                        ny >= world->levels[1][0].height ||
                        visited[ny][nx] ||
                        theron_v1_world_get_square(world, nx, ny) !=
                            THERON_SQUARE_FLOOR) {
                        continue;
                    }
                    visited[ny][nx] = 1u;
                    parent_x[ny][nx] = (int16_t)x;
                    parent_y[ny][nx] = (int16_t)y;
                    parent_direction[ny][nx] = (int8_t)direction;
                    queue_x[queue_tail] = (int16_t)nx;
                    queue_y[queue_tail++] = (int16_t)ny;
                }
            }
            if (target_x >= 0) {
                int x = target_x;
                int y = target_y;
                while (x != start_x || y != start_y) {
                    int previous_x = parent_x[y][x];
                    int previous_y = parent_y[y][x];
                    if (previous_x < 0 || previous_y < 0 ||
                        path_length >=
                            THERON_MAX_MAP_SIZE * THERON_MAX_MAP_SIZE) {
                        path_length = 0;
                        break;
                    }
                    path_directions[path_length++] = parent_direction[y][x];
                    x = previous_x;
                    y = previous_y;
                }
            }
            for (i = path_length - 1; i >= 0; --i) {
                int direction = path_directions[i];
                int relative = (direction - facing + THERON_DIR_COUNT) & 3;
                int next_x = world->party.leader_x +
                    g_theron_dir_dx[direction];
                int next_y = world->party.leader_y +
                    g_theron_dir_dy[direction];
                if (M11_GameView_HandleInput(
                        &view, movement_inputs[relative]) !=
                        M11_GAME_INPUT_REDRAW ||
                    world->current_dungeon != 2 ||
                    world->current_level != 0 ||
                    world->party.leader_x != next_x ||
                    world->party.leader_y != next_y) {
                    break;
                }
                ++moved_steps;
            }
            if (moved_steps < 3) {
                ++failures;
                fprintf(stderr,
                        "FAIL: native input traverses three source-map floor steps after authentic Continue (start=%d,%d facing=%d path=%d moved=%d)\n",
                        start_x, start_y, facing, path_length, moved_steps);
            } else {
                const uint8_t quest_items_before =
                    world->progression.quest_items_collected;
                if (!exercise_nearest_authentic_item_cycle(&view, world)) {
                    ++failures;
                    fprintf(stderr,
                            "FAIL: native M11 pickup/drop preserves an approachable authentic Track 02 object through inventory\n");
                }
                if (world->progression.quest_items_collected !=
                    quest_items_before) {
                    ++failures;
                    fprintf(stderr,
                            "FAIL: an authentic item pickup leaves unproven quest-item progress unchanged\n");
                }
            }
        }
    }
    M11_GameView_Shutdown(&view);
    if (saved_bram_path[0]) {
        Theron_V1PceBramBodyReceipt saved_body;
        if (!theron_v1_pce_bram_decode_original_body_path(
                saved_bram_path, &saved_body) ||
            !saved_body.layout_verified || !saved_body.semantics_verified ||
            !set_test_environment("FIRESTAFF_THERON_BRAM_PATH",
                                  saved_bram_path)) {
            ++failures;
            fprintf(stderr,
                    "FAIL: written BRAM decodes and selects as the next authentic M11 Continue source\n");
        } else {
            spec.savePath = saved_bram_path;
            M11_GameView_Init(&view);
            if (!M11_GameView_Start(&view, &spec) ||
                view.theronState.save_resume_claim !=
                    THERON_V1_STARTUP_RESUME_SRM ||
                M11_GameView_HandleInput(&view, M12_MENU_INPUT_ACCEPT) !=
                    M11_GAME_INPUT_REDRAW ||
                M11_GameView_HandleInput(&view, M12_MENU_INPUT_UP) !=
                    M11_GAME_INPUT_REDRAW ||
                !view.theronState.save_resume_continue_focus ||
                M11_GameView_HandleInput(&view, M12_MENU_INPUT_ACCEPT) !=
                    M11_GAME_INPUT_REDRAW ||
                view.theronState.startup_phase !=
                    THERON_STARTUP_PHASE_STAGE_SELECT ||
                view.theronState.level_loaded) {
                ++failures;
                fprintf(stderr,
                        "FAIL: fresh native M11 Continue accepts the BRAM emitted by Save Game\n");
            } else {
                Theron_V1_World *roundtrip_world =
                    (Theron_V1_World *)view.theronWorld;
                int attributes[7];
                int attributes_match = 0;
                const Theron_V1_Champion *roundtrip_theron =
                    roundtrip_world &&
                            roundtrip_world->party.champion_count > 0
                        ? &roundtrip_world->party.champions[0]
                        : NULL;
                if (roundtrip_theron) {
                    attributes[0] = roundtrip_theron->luck;
                    attributes[1] = roundtrip_theron->strength;
                    attributes[2] = roundtrip_theron->dexterity;
                    attributes[3] = roundtrip_theron->wisdom;
                    attributes[4] = roundtrip_theron->vitality;
                    attributes[5] = roundtrip_theron->anti_magic;
                    attributes[6] = roundtrip_theron->anti_fire;
                    attributes_match = 1;
                    for (i = 0; i < 7; ++i) {
                        if (attributes[i] !=
                            saved_body.theron_max_attributes[i]) {
                            attributes_match = 0;
                        }
                    }
                }
                if (!roundtrip_world || !roundtrip_theron ||
                    roundtrip_world->campaign_completion_mask !=
                        (saved_body.ram_267c_campaign_byte & 0x7fu) ||
                    roundtrip_theron->max_health !=
                        (int)saved_body.theron_max_health ||
                    roundtrip_theron->max_stamina !=
                        (int)saved_body.theron_max_stamina ||
                    roundtrip_theron->max_mana !=
                        (int)saved_body.theron_max_mana ||
                    !attributes_match ||
                    memcmp(roundtrip_theron->skill_temporary_experience,
                           saved_body.theron_skill_temporary_experience,
                           sizeof(saved_body.theron_skill_temporary_experience)) !=
                        0 ||
                    memcmp(roundtrip_theron->skill_experience,
                           saved_body.theron_skill_experience,
                           sizeof(saved_body.theron_skill_experience)) != 0) {
                    ++failures;
                    fprintf(stderr,
                            "FAIL: native Continue restores only campaign and Theron fields present in the written BRAM\n");
                }
            }
            M11_GameView_Shutdown(&view);
        }
    }
    if (!set_test_environment("FIRESTAFF_THERON_BRAM_PATH", NULL)) {
        ++failures;
        fprintf(stderr, "FAIL: isolate the fresh authentic campaign from saved Continue state\n");
    } else {
        int fresh_game_ready = 1;
        spec.savePath = NULL;
        M11_GameView_Init(&view);
        if (!M11_GameView_Start(&view, &spec)) {
            ++failures;
            fprintf(stderr, "FAIL: native M11 starts a fresh game from authentic US Track 02\n");
            fresh_game_ready = 0;
        }
        if (fresh_game_ready) {
            world = (Theron_V1_World *)view.theronWorld;
            if (!world ||
                view.theronState.startup_phase != THERON_STARTUP_PHASE_TITLE ||
                view.theronState.save_resume_claim != THERON_V1_STARTUP_RESUME_NONE ||
                M11_GameView_HandleInput(&view, M12_MENU_INPUT_ACCEPT) !=
                    M11_GAME_INPUT_REDRAW ||
                view.theronState.startup_phase !=
                    THERON_STARTUP_PHASE_STAGE_SELECT ||
                view.theronState.selected_dungeon != 1 ||
                M11_GameView_HandleInput(&view, M12_MENU_INPUT_ACCEPT) !=
                    M11_GAME_INPUT_REDRAW ||
                view.theronState.startup_phase !=
                    THERON_STARTUP_PHASE_SOUL_ROOM ||
                view.theronState.level_loaded ||
                view.theronState.startup_roster_name_count <= 0 ||
                M11_GameView_HandleInput(&view, M12_MENU_INPUT_ACCEPT) !=
                    M11_GAME_INPUT_REDRAW ||
                view.theronState.startup_phase != THERON_STARTUP_PHASE_READY ||
                view.theronState.selected_mirrors_mask == 0) {
                ++failures;
                fprintf(stderr,
                        "FAIL: authentic fresh campaign selects Ak-Tu-Ba and admits the source roster through the Soul Room\n");
                fresh_game_ready = 0;
            }
        }
        if (fresh_game_ready) {
            for (i = 0;
                 i <= THERON_STARTUP_HERO_MIRROR_COUNT &&
                 view.theronState.startup_cursor !=
                     THERON_STARTUP_HERO_MIRROR_COUNT;
                 ++i) {
                if (M11_GameView_HandleInput(&view, M12_MENU_INPUT_DOWN) !=
                    M11_GAME_INPUT_REDRAW) break;
            }
            if (view.theronState.startup_cursor !=
                    THERON_STARTUP_HERO_MIRROR_COUNT ||
                M11_GameView_HandleInput(&view, M12_MENU_INPUT_ACCEPT) !=
                    M11_GAME_INPUT_REDRAW ||
                view.theronState.startup_phase !=
                    THERON_STARTUP_PHASE_IN_DUNGEON ||
                !view.theronState.level_loaded || !world ||
                world->current_dungeon != 1 || world->current_level != 0 ||
                !world->levels[0][0].source_header_verified ||
                world->progression.quest_items_collected != 0u) {
                ++failures;
                fprintf(stderr,
                        "FAIL: authentic fresh Ak-Tu-Ba game admits the real level without inventing quest progress\n");
            }
        }
        M11_GameView_Shutdown(&view);
    }
    if (failures) {
        fprintf(stderr, "Theron authenticated M11 Continue FAILED (%d checks)\n",
                failures);
        return 1;
    }
    puts("Theron authenticated M11 Continue checks passed");
    return 0;
}
