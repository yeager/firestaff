#include "m11_game_view.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

static int regular_file_has_bytes(const char* path)
{
    struct stat st;
    return path && path[0] && stat(path, &st) == 0 && st.st_size > 0;
}

static int data_dir_has_pc34(const char* dir)
{
    char dungeon[1024];
    char graphics[1024];
    if (!dir || !dir[0]) return 0;
    snprintf(dungeon, sizeof(dungeon), "%s/DUNGEON.DAT", dir);
    snprintf(graphics, sizeof(graphics), "%s/GRAPHICS.DAT", dir);
    return regular_file_has_bytes(dungeon) && regular_file_has_bytes(graphics);
}

static const char* resolve_data_dir(void)
{
    static char path[2048];
    const char* env = getenv("FIRESTAFF_DM1_DATA_DIR");
    const char* archive = getenv("FIRESTAFF_DM1_PC34_ARCHIVE");

    if (data_dir_has_pc34(env)) return env;
    if (env && env[0]) {
        snprintf(path, sizeof(path), "%s/DATA", env);
        if (data_dir_has_pc34(path)) return path;
    }
    return regular_file_has_bytes(archive) ? archive : NULL;
}

static int element_at(const struct DungeonDatState_Compat* dungeon,
                       int map, int x, int y)
{
    if (!dungeon || !dungeon->tilesLoaded || !dungeon->tiles || !dungeon->maps ||
        map < 0 || map >= (int)dungeon->header.mapCount || x < 0 || y < 0 ||
        x >= (int)dungeon->maps[map].width || y >= (int)dungeon->maps[map].height)
        return -1;
    return dungeon->tiles[map].squareData[x * (int)dungeon->maps[map].height + y] >> 5;
}

static int find_real_c15_capture(M11_GameViewState* state,
                                 unsigned char* framebuffer,
                                 M11_Dm1F0115C15RuntimeCaptureReceipt* receipt)
{
    const struct DungeonDatState_Compat* dungeon = state->world.dungeon;
    int mapIndex;

    static const int dx[4] = {0,1,0,-1}, dy[4] = {-1,0,1,0};
    if (!dungeon || !dungeon->maps) return 0;
    for (mapIndex = 1; mapIndex < (int)dungeon->header.mapCount; ++mapIndex) {
        const struct DungeonMapDesc_Compat* map = &dungeon->maps[mapIndex];
        int x;
        for (x = 0; x < (int)map->width; ++x) {
            int y;
            for (y = 0; y < (int)map->height; ++y) {
                int direction;
                if (element_at(dungeon, mapIndex, x, y) != 1) continue;
                for (direction = 0; direction < 4; ++direction) {
                    int i, tick;
                    struct ChampionState_Compat* champion;
                    struct ChampionLifecycleState_Compat* life;
                    if (element_at(dungeon, mapIndex, x + dx[direction], y + dy[direction]) != 1 ||
                        element_at(dungeon, mapIndex, x + 2*dx[direction], y + 2*dy[direction]) != 0)
                        continue;
                    state->world.partyMapIndex = mapIndex;
                    state->world.newPartyMapIndex = mapIndex;
                    state->world.party.mapIndex = mapIndex;
                    state->world.party.mapX = x;
                    state->world.party.mapY = y;
                    state->world.party.direction = direction;
                    /* RAM-only party setup; original map/graphics unchanged.
                     * MENU.C F0412 / original G0487: LO FUL IR launches a
                     * fireball. Its real wall impact must allocate a C15. */
                    state->world.party.championCount = 1;
                    state->world.party.activeChampionIndex = 0;
                    champion = &state->world.party.champions[0];
                    life = &state->world.lifecycle.champions[0];
                    F0600_CHAMPION_InitEmpty_Compat(champion);
                    memset(life, 0, sizeof(*life));
                    champion->present = 1;
                    champion->hp.current = champion->hp.maximum = 1000;
                    champion->stamina.current = champion->stamina.maximum = 1000;
                    champion->mana.current = champion->mana.maximum = 900;
                    champion->attributes[CHAMPION_ATTR_WISDOM] = 100;
                    champion->direction = direction;
                    for (i = 0; i < 20; ++i) life->skills20[i].experience = 1000000;
                    state->inventoryPanelActive = 0;
                    state->presentationMode = M12_PRESENTATION_V1_ORIGINAL;
                    state->world.masterRng.seed = 1;
                    if (!M11_GameView_OpenSpellPanel(state) ||
                        !M11_GameView_EnterRune(state, 0) ||
                        !M11_GameView_EnterRune(state, 3) ||
                        !M11_GameView_EnterRune(state, 3) ||
                        !M11_GameView_CastSpell(state)) return 0;
                    for (tick = 0; tick < 128; ++tick) {
                        memset(framebuffer, 0, 320 * 200);
                        M11_GameView_Draw(state, framebuffer, 320, 200);
                        memset(receipt, 0, sizeof(*receipt));
                        M11_GameView_GetDm1F0115C15RuntimeCaptureReceipt(receipt);
                        if (receipt->valid) {
                            for (i = 0; i < state->world.explosions.count; ++i) {
                                const struct ExplosionInstance_Compat* e = &state->world.explosions.entries[i];
                                if (e->slotIndex >= 0 && e->reserved0 &&
                                    e->sourceC15Fingerprint && e->explosionType == C000_EXPLOSION_FIREBALL &&
                                    e->mapIndex == mapIndex && e->mapX == x + dx[direction] &&
                                    e->mapY == y + dy[direction]) return 1;
                            }
                            fputs("capture lacks original C15 on preceding open square\n", stderr);
                            return 0;
                        }
                        (void)M11_GameView_AdvanceIdleTick(state);
                    }
                    fprintf(stderr, "public fireball wall impact produced no C15 capture (source C14=%d C15=%d)\n",
                        state->world.things->projectileCount, state->world.things->explosionCount);
                    return 0;
                }
            }
        }
    }
    return 0;
}

int main(void)
{
    const char* dataDir = resolve_data_dir();
    const char* selectedDataDir = getenv("FIRESTAFF_DM1_DATA_DIR");
    M11_GameViewState state;
    M11_Dm1F0115C15RuntimeCaptureReceipt receipt;
    unsigned char framebuffer[320 * 200];
    int mode;

    if (!dataDir) {
        if (selectedDataDir && selectedDataDir[0]) {
            fputs("configured PC34 DUNGEON.DAT/GRAPHICS.DAT is unavailable\n",
                  stderr);
            return 1;
        }
        puts("skip: local DM1 PC34 DUNGEON.DAT/GRAPHICS.DAT not available");
        return 77;
    }
    M11_GameView_Init(&state);
    if (!M11_GameView_StartDm1(&state, dataDir) || !state.assetsAvailable ||
        !find_real_c15_capture(&state, framebuffer, &receipt)) {
        M11_GameView_Shutdown(&state);
        fputs("FAIL: original PC34 startup/fireball C15 capture failed\n", stderr);
        return 1;
    }
    for (mode = M12_PRESENTATION_V1_ORIGINAL; mode <= M12_PRESENTATION_V21_UPSCALED; ++mode) {
        state.presentationMode = mode;
        memset(framebuffer, 0, sizeof(framebuffer));
        M11_GameView_Draw(&state, framebuffer, 320, 200);
        M11_GameView_GetDm1F0115C15RuntimeCaptureReceipt(&receipt);
        if (!receipt.valid || receipt.runtimeTick != state.world.gameTick ||
            receipt.sourceTick != state.world.gameTick ||
            receipt.materialFNV1a == 0u || receipt.requestedMaterialCount <= 0 ||
            receipt.completedMaterialCount != receipt.requestedMaterialCount) {
            fprintf(stderr, "real C15 material did not complete final M11 capture\n");
            M11_GameView_Shutdown(&state);
            return 1;
        }
    }
    state.world.gameTick++;
    state.assetsAvailable = 0;
    memset(framebuffer, 0, sizeof(framebuffer));
    M11_GameView_Draw(&state, framebuffer, 320, 200);
    memset(&receipt, 0xff, sizeof(receipt));
    M11_GameView_GetDm1F0115C15RuntimeCaptureReceipt(&receipt);
    if (receipt.valid || receipt.runtimeTick != 0u ||
        receipt.materialFNV1a != 0u || receipt.requestedMaterialCount != 0 ||
        receipt.completedMaterialCount != 0) {
        fprintf(stderr, "missing PC34 explosion material retained stale C15 capture\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }
    M11_GameView_Shutdown(&state);
    puts("ok: real PC34 C15 deferred explosion material reaches final M11 capture");
    return 0;
}
