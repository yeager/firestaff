#include "m11_game_view.h"
#include "theron_v1_mechanics.h"
#include "theron_v1_startup_save_resume.h"
#include "theron_v1_world.h"

#include <stdio.h>
#include <string.h>

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

static int fail(const char* message) {
    fprintf(stderr, "FAIL: %s\n", message);
    return 1;
}

int main(int argc, char** argv) {
    M11_GameLaunchSpec spec;
    M11_GameViewState view;
    Theron_V1_World* world;
    Theron_V1PceBramBodyReceipt source_body;
    int source_body_ready;
    int i;
    int failures = 0;

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
            static const int relative_directions[4] = { 0, 1, 2, 3 };
            static const int movement_inputs[4] = {
                M12_MENU_INPUT_UP,
                M12_MENU_INPUT_STRAFE_RIGHT,
                M12_MENU_INPUT_DOWN,
                M12_MENU_INPUT_STRAFE_LEFT
            };
            int moved = 0;
            int start_x = world->party.leader_x;
            int start_y = world->party.leader_y;
            int facing = world->party.leader_dir & 3;
            for (i = 0; i < 4 && !moved; ++i) {
                int direction = (facing + relative_directions[i]) & 3;
                int target_x = start_x + g_theron_dir_dx[direction];
                int target_y = start_y + g_theron_dir_dy[direction];
                uint8_t target_square;
                if (target_x < 0 || target_y < 0 ||
                    target_x >= world->levels[1][0].width ||
                    target_y >= world->levels[1][0].height) {
                    continue;
                }
                target_square = theron_v1_world_get_square(
                    world, target_x, target_y);
                if (!THERON_SQUARE_IS_PASSABLE(target_square)) continue;
                (void)M11_GameView_HandleInput(&view, movement_inputs[i]);
                moved = world->party.leader_x != start_x ||
                        world->party.leader_y != start_y;
            }
            if (!moved) {
                ++failures;
                fprintf(stderr,
                        "FAIL: native movement reaches an adjacent passable tile in the authentic dungeon 2 map (start=%d,%d facing=%d)\n",
                        start_x, start_y, facing);
            }
        }
    }
    M11_GameView_Shutdown(&view);
    if (failures) {
        fprintf(stderr, "Theron authenticated M11 Continue FAILED (%d checks)\n",
                failures);
        return 1;
    }
    puts("Theron authenticated M11 Continue checks passed");
    return 0;
}
