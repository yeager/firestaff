#include "m11_game_view.h"
#include "menu_startup_m12.h"

#include <stdio.h>
#include <string.h>
#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#endif

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

typedef struct RouteStepProbe {
    const char* captureName;
    const char* action;
    M12_MenuInput input;
    int expectResult;
} RouteStepProbe;

typedef struct RouteSnapshotProbe {
    const char* captureName;
    const char* action;
    int result;
    int mapIndex;
    int mapX;
    int mapY;
    int direction;
    unsigned int gameTick;
    int spellPanelOpen;
    int spellRuneCount;
    int inventoryPanelActive;
    int championCount;
    int activeChampionIndex;
    int rightHandThing;
} RouteSnapshotProbe;

static void ensure_deterministic_capture_champion(M11_GameViewState* game) {
    struct ChampionState_Compat* c;
    unsigned short weaponThing;
    int invSlot;
    if (!game) return;
    c = &game->world.party.champions[0];
    memset(c, 0, sizeof(*c));
    for (invSlot = 0; invSlot < CHAMPION_SLOT_COUNT; ++invSlot) {
        c->inventory[invSlot] = THING_NONE;
    }
    c->present = 1;
    memcpy(c->name, "HALK\0\0\0\0", 8);
    c->hp.current = 42; c->hp.maximum = 60;
    c->stamina.current = 35; c->stamina.maximum = 50;
    c->mana.current = 20; c->mana.maximum = 40;
    c->food = 80; c->water = 70;
    c->portraitIndex = 0;
    game->world.party.championCount = 1;
    game->world.party.activeChampionIndex = 0;
    if (game->world.things && game->world.things->weapons &&
        game->world.things->weaponCount > 0) {
        weaponThing = (unsigned short)((THING_TYPE_WEAPON << 10) | 0);
        game->world.things->weapons[0].type = 8;
        game->world.things->weapons[0].chargeCount = 0;
        game->world.things->weapons[0].lit = 0;
        c->inventory[CHAMPION_SLOT_HAND_RIGHT] = weaponThing;
    }
}

static void snapshot(const M11_GameViewState* game,
                     const RouteStepProbe* step,
                     int result,
                     RouteSnapshotProbe* out) {
    memset(out, 0, sizeof(*out));
    out->captureName = step->captureName;
    out->action = step->action;
    out->result = result;
    out->mapIndex = game->world.party.mapIndex;
    out->mapX = game->world.party.mapX;
    out->mapY = game->world.party.mapY;
    out->direction = game->world.party.direction;
    out->gameTick = game->world.gameTick;
    out->spellPanelOpen = game->spellPanelOpen;
    out->spellRuneCount = game->spellBuffer.runeCount;
    out->inventoryPanelActive = game->inventoryPanelActive;
    out->championCount = game->world.party.championCount;
    out->activeChampionIndex = game->world.party.activeChampionIndex;
    out->rightHandThing = (game->world.party.championCount > 0)
                              ? (int)game->world.party.champions[0].inventory[CHAMPION_SLOT_HAND_RIGHT]
                              : -1;
}

static void ensure_output_dir(const char* outDir) {
    if (!outDir || outDir[0] == '\0') return;
#ifdef _WIN32
    (void)_mkdir(outDir);
#else
    (void)mkdir(outDir, 0777);
#endif
}

static int write_outputs(const char* outDir, const RouteSnapshotProbe* rows, int count) {
    char mdPath[1024];
    char jsonPath[1024];
    FILE* md;
    FILE* js;
    int i;
    ensure_output_dir(outDir);
    snprintf(mdPath, sizeof(mdPath), "%s/pass76_capture_route_state_probe.md", outDir);
    snprintf(jsonPath, sizeof(jsonPath), "%s/pass76_capture_route_state_probe.json", outDir);
    md = fopen(mdPath, "w");
    js = fopen(jsonPath, "w");
    if (!md || !js) {
        if (md) fclose(md);
        if (js) fclose(js);
        return 0;
    }
    fprintf(md, "# Pass 76 capture-route state probe\n\n");
    fprintf(md, "This probe mirrors `verification-screens/capture_firestaff_ingame_series.c` as state, not pixels. It is a Firestaff fixture-route contract only; it does not claim the original DOS route is identical.\n\n");
    fprintf(md, "| capture | action | result | tick | map | x | y | dir | spellOpen | runes | inventoryOpen | champions | activeChampion | rightHandThing |\n");
    fprintf(md, "| --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | --- |\n");
    fprintf(js, "{\n  \"schema\": \"pass76_capture_route_state_probe.v1\",\n  \"honesty\": \"Firestaff deterministic fixture route only; original DOS semantic route remains separate.\",\n  \"snapshots\": [\n");
    for (i = 0; i < count; ++i) {
        const RouteSnapshotProbe* r = &rows[i];
        fprintf(md, "| %s | %s | %d | %u | %d | %d | %d | %d | %d | %d | %d | %d | %d | 0x%04X |\n",
                r->captureName, r->action, r->result, r->gameTick, r->mapIndex, r->mapX,
                r->mapY, r->direction, r->spellPanelOpen, r->spellRuneCount,
                r->inventoryPanelActive, r->championCount, r->activeChampionIndex,
                (unsigned int)(r->rightHandThing & 0xFFFF));
        fprintf(js,
                "    {\"capture\":\"%s\",\"action\":\"%s\",\"result\":%d,\"tick\":%u,\"mapIndex\":%d,\"mapX\":%d,\"mapY\":%d,\"direction\":%d,\"spellPanelOpen\":%d,\"spellRuneCount\":%d,\"inventoryPanelActive\":%d,\"championCount\":%d,\"activeChampionIndex\":%d,\"rightHandThing\":%d}%s\n",
                r->captureName, r->action, r->result, r->gameTick, r->mapIndex, r->mapX,
                r->mapY, r->direction, r->spellPanelOpen, r->spellRuneCount,
                r->inventoryPanelActive, r->championCount, r->activeChampionIndex,
                r->rightHandThing, i == count - 1 ? "" : ",");
    }
    fprintf(js, "  ]\n}\n");
    fclose(md);
    fclose(js);
    printf("wrote %s and %s\n", mdPath, jsonPath);
    return 1;
}

int main(int argc, char** argv) {
    const char* dataDir;
    const char* outDir;
    M12_StartupMenuState menu;
    M11_GameViewState game;
    RouteSnapshotProbe rows[6];
    int rowCount = 0;
    int ok = 1;
    const RouteStepProbe steps[] = {
        {"01_ingame_start_latest", "start", M12_MENU_INPUT_NONE, M11_GAME_INPUT_REDRAW},
        {"02_ingame_turn_right_latest", "right", M12_MENU_INPUT_RIGHT, M11_GAME_INPUT_REDRAW},
        {"03_ingame_move_forward_latest", "up", M12_MENU_INPUT_UP, M11_GAME_INPUT_REDRAW},
        {"04_ingame_spell_panel_latest", "spell_rune_1", M12_MENU_INPUT_SPELL_RUNE_1, M11_GAME_INPUT_REDRAW},
        {"05_ingame_after_cast_latest", "spell_cast", M12_MENU_INPUT_SPELL_CAST, M11_GAME_INPUT_IGNORED},
        {"06_ingame_inventory_panel_latest", "spell_clear+inventory", M12_MENU_INPUT_INVENTORY_TOGGLE, M11_GAME_INPUT_REDRAW},
    };
    if (argc < 3) {
        fprintf(stderr, "usage: %s DATA_DIR OUT_DIR\n", argv[0]);
        return 2;
    }
    dataDir = argv[1];
    outDir = argv[2];
    M12_StartupMenu_InitWithDataDir(&menu, dataDir);
    M11_GameView_Init(&game);
    if (!M11_GameView_OpenSelectedMenuEntry(&game, &menu)) {
        fprintf(stderr, "failed to open DM1 game view\n");
        return 1;
    }
    ensure_deterministic_capture_champion(&game);

    snapshot(&game, &steps[0], M11_GAME_INPUT_REDRAW, &rows[rowCount++]);
    {
        int i;
        for (i = 1; i < 6; ++i) {
            int result;
            if (i == 5) {
                (void)M11_GameView_HandleInput(&game, M12_MENU_INPUT_SPELL_CLEAR);
                ensure_deterministic_capture_champion(&game);
            }
            result = M11_GameView_HandleInput(&game, steps[i].input);
            snapshot(&game, &steps[i], result, &rows[rowCount++]);
            if (result != steps[i].expectResult) ok = 0;
        }
    }
    if (rows[0].mapIndex != 0 || rows[0].mapX != 1 || rows[0].mapY != 3 || rows[0].direction != 2) ok = 0;
    if (rows[1].direction != 3) ok = 0;
    if (rows[2].mapX != 0 || rows[2].mapY != 3 || rows[2].direction != 3) ok = 0;
    if (!rows[3].spellPanelOpen || rows[3].spellRuneCount != 1) ok = 0;
    if (rows[4].spellPanelOpen == 0) ok = 0;
    if (!rows[5].inventoryPanelActive) ok = 0;

    if (!write_outputs(outDir, rows, rowCount)) ok = 0;
    M11_GameView_Shutdown(&game);
    printf("%s capture route state probe\n", ok ? "PASS" : "FAIL");
    return ok ? 0 : 1;
}
