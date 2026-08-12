#include "m11_game_view.h"
#include "dm2_v1_boot.h"
#include "dm2_v1_dungeon_loader.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    unsigned counts[0x80];
    unsigned first_map[0x80];
    unsigned first_x[0x80];
    unsigned first_y[0x80];
    unsigned first_w2[0x80];
    unsigned first_w4[0x80];
    unsigned first_w6[0x80];
} Census;

static void print_chain(const DM2_V1_DungeonData *dungeon, int level,
                        int x, int y)
{
    int thing;
    unsigned steps = 0;
    thing = dm2_v1_dungeon_get_first_thing(dungeon, level, x, y);
    printf("  chain map=%d x=%d y=%d:", level, x, y);
    while (thing >= 0 && steps++ < 32u) {
        int type = -1;
        int size = 0;
        const uint8_t *record = dm2_v1_dungeon_get_thing_record(
            dungeon, (uint16_t)thing, &type, NULL, &size);
        if (!record || size < 2) break;
        if (type == 3 && size >= 8)
            printf(" %04x/%02x", thing,
                   (unsigned)((record[2] | ((unsigned)record[3] << 8)) & 0x7fu));
        else
            printf(" %04x/db%d", thing, type);
        thing = dm2_v1_dungeon_get_next_thing(dungeon, (uint16_t)thing);
    }
    putchar('\n');
}

static int census_thing(void *user, uint16_t thing, int type, int index,
                        const uint8_t *record, int record_size,
                        int level, int x, int y)
{
    Census *c = (Census *)user;
    unsigned cls;
    (void)thing;
    (void)index;
    if (!c || type != 3 || !record || record_size < 8)
        return 0;
    cls = (unsigned)((record[2] | ((unsigned)record[3] << 8)) & 0x7fu);
    if (cls >= 0x80u)
        return 0;
    ++c->counts[cls];
    if (cls == 0x17u || cls == 0x18u || cls == 0x1au || cls == 0x46u)
        printf("  actuator type=%02x object=%04x map=%d x=%d y=%d w2=%04x w4=%04x w6=%04x\n",
               cls, thing, level, x, y,
               record[2] | ((unsigned)record[3] << 8),
               record[4] | ((unsigned)record[5] << 8),
               record[6] | ((unsigned)record[7] << 8));
    if (c->counts[cls] == 1u) {
        c->first_map[cls] = (unsigned)level;
        c->first_x[cls] = (unsigned)x;
        c->first_y[cls] = (unsigned)y;
        c->first_w2[cls] = record[2] | ((unsigned)record[3] << 8);
        c->first_w4[cls] = record[4] | ((unsigned)record[5] << 8);
        c->first_w6[cls] = record[6] | ((unsigned)record[7] << 8);
    }
    return 0;
}

static int run_one(const char *zip, const char *source_id)
{
    M11_GameViewState state;
    M11_GameLaunchSpec spec;
    DM2_V1_BootProfile *profile;
    DM2_V1_DungeonData *dungeon;
    Census census;
    unsigned level;

    memset(&state, 0, sizeof(state));
    memset(&spec, 0, sizeof(spec));
    memset(&census, 0, sizeof(census));
    spec.title = "Dungeon Master II Macintosh";
    spec.gameId = "dm2";
    spec.dataDir = zip;
    spec.sourceId = source_id;
    spec.presentationWidth = 320;
    spec.presentationHeight = 200;
    spec.launcherOptionsBound = 1;
    M11_GameView_Init(&state);
    if (!M11_GameView_Start(&state, &spec)) {
        fprintf(stderr, "Mac census launch failed: %s\n", source_id);
        M11_GameView_Shutdown(&state);
        return 1;
    }
    {
        unsigned char framebuffer[320u * 200u];
        memset(framebuffer, 0, sizeof(framebuffer));
        while (state.dm2MacMovieActive)
            M11_GameView_Draw(&state, framebuffer, 320, 200);
    }
    if (M11_GameView_HandleInput(&state, M12_MENU_INPUT_ACCEPT) ==
            M11_GAME_INPUT_IGNORED ||
        state.dm2State.startup_menu_active ||
        !state.dm2State.level_loaded) {
        fprintf(stderr, "Mac census New Game failed: %s\n", source_id);
        M11_GameView_Shutdown(&state);
        return 1;
    }
    {
        unsigned char inventory_frame[320u * 200u];
        if (M11_GameView_HandleInput(
                &state, M12_MENU_INPUT_INVENTORY_TOGGLE) !=
                M11_GAME_INPUT_REDRAW || !state.inventoryPanelActive) {
            fprintf(stderr, "Mac authenticated CHARSHEET inventory did not open: %s\n",
                    source_id);
            M11_GameView_Shutdown(&state);
            return 1;
        }
        memset(inventory_frame, 0, sizeof(inventory_frame));
        M11_GameView_Draw(&state, inventory_frame, 320, 200);
        if (M11_GameView_HandleInput(&state, M12_MENU_INPUT_BACK) !=
                M11_GAME_INPUT_REDRAW || state.inventoryPanelActive) {
            fprintf(stderr, "Mac CHARSHEET inventory did not close: %s\n",
                    source_id);
            M11_GameView_Shutdown(&state);
            return 1;
        }
        puts("  authenticated Mac CHARSHEET inventory frame accepted");
    }
    if (M11_GameView_HandleInput(
            &state, M12_MENU_INPUT_CHAMPION_1_INVENTORY) !=
            M11_GAME_INPUT_REDRAW || !state.inventoryPanelActive ||
        state.world.party.activeChampionIndex != 0) {
        fprintf(stderr, "Mac F1 champion inventory owner did not open: %s\n",
                source_id);
        M11_GameView_Shutdown(&state);
        return 1;
    }
    if (M11_GameView_HandleInput(&state, M12_MENU_INPUT_BACK) !=
            M11_GAME_INPUT_REDRAW || state.inventoryPanelActive) {
        fprintf(stderr, "Mac F1 champion inventory owner did not close: %s\n",
                source_id);
        M11_GameView_Shutdown(&state);
        return 1;
    }
    puts("  authenticated Mac F1 champion inventory command accepted");
    profile = (DM2_V1_BootProfile *)state.dm2BootProfile;
    dungeon = profile ? (DM2_V1_DungeonData *)profile->dungeon_data : NULL;
    if (!dungeon || !dungeon->record_graph_complete) {
        fprintf(stderr, "Mac census has no complete dungeon graph: %s\n",
                source_id);
        M11_GameView_Shutdown(&state);
        return 1;
    }
    for (level = 0; level < (unsigned)dungeon->level_count; ++level) {
        int y;
        for (y = 0; y < dungeon->level_heights[level]; ++y) {
            int x;
            for (x = 0; x < dungeon->level_widths[level]; ++x) {
                (void)dm2_v1_dungeon_walk_square_things(
                    dungeon, (int)level, x, y, 256, census_thing, &census);
            }
        }
    }
    {
        const unsigned classes[] = { 0x17u, 0x18u, 0x1au };
        size_t i;
        for (i = 0; i < sizeof(classes) / sizeof(classes[0]); ++i) {
            unsigned cls = classes[i];
            if (census.counts[cls]) {
                int tx = (int)((census.first_w6[cls] >> 6) & 0x1fu);
                int ty = (int)((census.first_w6[cls] >> 11) & 0x1fu);
                int first = dm2_v1_dungeon_get_first_thing(
                    dungeon, (int)census.first_map[cls], tx, ty);
                int type = -1;
                (void)dm2_v1_dungeon_get_thing_record(
                    dungeon, (uint16_t)first, &type, NULL, NULL);
                printf("  type=%02x target=%u,%d,%d tile=%d first_type=%d\n",
                       cls, census.first_map[cls], tx, ty,
                       dm2_v1_dungeon_get_square_type(
                           dungeon, (int)census.first_map[cls], tx, ty), type);
                print_chain(dungeon, (int)census.first_map[cls],
                            (int)census.first_x[cls],
                            (int)census.first_y[cls]);
            }
        }
    }
    printf("%s: 0x17=%u", source_id, census.counts[0x17]);
    if (census.counts[0x17])
        printf(" @%u,%u,%u w2=%04x w4=%04x w6=%04x", census.first_map[0x17],
               census.first_x[0x17], census.first_y[0x17], census.first_w2[0x17],
               census.first_w4[0x17], census.first_w6[0x17]);
    printf("; 0x18=%u", census.counts[0x18]);
    if (census.counts[0x18])
        printf(" @%u,%u,%u w2=%04x w4=%04x w6=%04x", census.first_map[0x18],
               census.first_x[0x18], census.first_y[0x18], census.first_w2[0x18],
               census.first_w4[0x18], census.first_w6[0x18]);
    printf("; 0x1a=%u", census.counts[0x1a]);
    if (census.counts[0x1a])
        printf(" @%u,%u,%u w2=%04x w4=%04x w6=%04x", census.first_map[0x1a],
               census.first_x[0x1a], census.first_y[0x1a], census.first_w2[0x1a],
               census.first_w4[0x1a], census.first_w6[0x1a]);
    printf("; 0x46=%u", census.counts[0x46]);
    if (census.counts[0x46])
        printf(" @%u,%u,%u w2=%04x w4=%04x w6=%04x", census.first_map[0x46],
               census.first_x[0x46], census.first_y[0x46], census.first_w2[0x46],
               census.first_w4[0x46], census.first_w6[0x46]);
    putchar('\n');
    M11_GameView_Shutdown(&state);
    return 0;
}

int main(void)
{
    const char *retail = getenv("FIRESTAFF_DM2_MAC_EN_ZIP");
    const char *demo = getenv("FIRESTAFF_DM2_MAC_EN_DEMO_ZIP");
    if (!retail && !demo) {
        puts("SKIP: authentic DM2 Mac ZIP environment is not set");
        return 0;
    }
    if (demo && run_one(demo, "mac-en-demo") != 0)
        return 1;
    if (retail && run_one(retail, "mac-en-retail") != 0)
        return 1;
    return 0;
}
