#include "m11_game_view.h"

#include "dm1_v1_combat_pc34_compat.h"
#include "dm1_v1_dungeon_thing_data_pc34_compat.h"
#include "dm1_v1_projectile_explosion_render_pc34_compat.h"
#include "dm1_v1_throw_shoot_pc34_compat.h"
#include "memory_dungeon_dat_pc34_compat.h"
#include "memory_projectile_pc34_compat.h"

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
    const char* root;
    const char* home;

    if (data_dir_has_pc34(env)) return env;
    root = getenv("FIRESTAFF_DATA");
    if (data_dir_has_pc34(root)) return root;
    if (root && root[0]) {
        snprintf(path, sizeof(path), "%s/dm1", root);
        if (data_dir_has_pc34(path)) return path;
    }
    home = getenv("HOME");
    if (home && home[0]) {
        snprintf(path, sizeof(path), "%s/.firestaff/data/dm1", home);
        if (data_dir_has_pc34(path)) return path;
        snprintf(path, sizeof(path), "%s/.firestaff/data", home);
        if (data_dir_has_pc34(path)) return path;
    }
    return NULL;
}

static unsigned short make_thing(int type, int index)
{
    return (unsigned short)(((type & 0x0f) << 10) | (index & 0x03ff));
}

static int square_is_open(const struct DungeonDatState_Compat* dungeon,
                          int mapIndex,
                          int x,
                          int y)
{
    const struct DungeonMapDesc_Compat* map;
    const struct DungeonMapTiles_Compat* tiles;
    int square;
    if (!dungeon || !dungeon->tilesLoaded || !dungeon->maps ||
        !dungeon->tiles || mapIndex < 0 ||
        mapIndex >= (int)dungeon->header.mapCount) {
        return 0;
    }
    map = &dungeon->maps[mapIndex];
    tiles = &dungeon->tiles[mapIndex];
    if (x < 0 || y < 0 || x >= (int)map->width || y >= (int)map->height) {
        return 0;
    }
    square = tiles->squareData[x * (int)map->height + y];
    square = (square & DUNGEON_SQUARE_MASK_TYPE) >> 5;
    return square == DUNGEON_ELEMENT_CORRIDOR ||
           square == DUNGEON_ELEMENT_PIT ||
           square == DUNGEON_ELEMENT_TELEPORTER;
}

static void direction_step(int direction, int* dx, int* dy)
{
    static const int kDx[4] = {0, 1, 0, -1};
    static const int kDy[4] = {-1, 0, 1, 0};
    *dx = kDx[direction & 3];
    *dy = kDy[direction & 3];
}

static int find_real_open_launch_pose(const struct DungeonDatState_Compat* dungeon,
                                      int* outMap,
                                      int* outX,
                                      int* outY,
                                      int* outDirection)
{
    int mapIndex;
    if (!dungeon || !outMap || !outX || !outY || !outDirection) return 0;
    for (mapIndex = 0; mapIndex < (int)dungeon->header.mapCount; ++mapIndex) {
        const struct DungeonMapDesc_Compat* map = &dungeon->maps[mapIndex];
        int x;
        int y;
        for (x = 0; x < (int)map->width; ++x) {
            for (y = 0; y < (int)map->height; ++y) {
                int direction;
                if (!square_is_open(dungeon, mapIndex, x, y)) continue;
                for (direction = 0; direction < 4; ++direction) {
                    int dx;
                    int dy;
                    direction_step(direction, &dx, &dy);
                    if (square_is_open(dungeon, mapIndex, x + dx, y + dy)) {
                        *outMap = mapIndex;
                        *outX = x;
                        *outY = y;
                        *outDirection = direction;
                        return 1;
                    }
                }
            }
        }
    }
    return 0;
}

static int find_real_throwable_weapon(const struct DungeonThings_Compat* things,
                                      int* outIndex,
                                      int* outType,
                                      DM1_ProjectileMaterialResolutionPc34* outMaterial)
{
    int index;
    if (!things || !outIndex || !outType || !outMaterial) {
        return 0;
    }
    for (index = 0; index < things->weaponCount; ++index) {
        DM1_WeaponInfo info;
        DM1_ProjectileMaterialResolutionPc34 material;
        const unsigned char* raw = dm1_v1_dungeon_get_thing_data_pc34(
            things, make_thing(THING_TYPE_WEAPON, index));
        int objectWeight;
        int weaponType;
        int aspectOrdinal;
        if (!raw || !dm1_v1_dungeon_get_object_weight_f0140_pc34(
                        things, make_thing(THING_TYPE_WEAPON, index),
                        &objectWeight)) {
            continue;
        }
        weaponType = raw[2] & 0x7f;
        if (dm1_weapon_info_pc34(weaponType, &info) <= 0) continue;
        aspectOrdinal = (info.attributes >> 8) & 0x1f;
        if (aspectOrdinal <= 0) continue;
        if (!dm1_v1_projectile_material_resolve_pc34(
                PROJECTILE_SUBTYPE_KINETIC_ARROW,
                THING_TYPE_WEAPON,
                weaponType,
                aspectOrdinal,
                &material) ||
            !material.uses_object_aspect ||
            material.graphic_index < 0) {
            continue;
        }
        *outIndex = index;
        *outType = weaponType;
        *outMaterial = material;
        return 1;
    }
    return 0;
}

static const struct ProjectileInstance_Compat* first_live_projectile(
    const struct ProjectileList_Compat* projectiles)
{
    int i;
    if (!projectiles) return NULL;
    for (i = 0; i < PROJECTILE_LIST_CAPACITY; ++i) {
        const struct ProjectileInstance_Compat* projectile =
            &projectiles->entries[i];
        if (projectile->slotIndex >= 0 && projectile->reserved3 != 0) {
            return projectile;
        }
    }
    return NULL;
}

int main(void)
{
    const char* dataDir = resolve_data_dir();
    M11_GameViewState state;
    unsigned char framebuffer[320 * 200];
    M11_Dm1ProjectileHostPresentationReceipt receipt;
    M11_Dm1F0115C2900RuntimeCaptureReceipt capture;
    DM1_ProjectileMaterialResolutionPc34 material;
    const struct ProjectileInstance_Compat* projectile;
    int mapIndex = -1;
    int mapX = -1;
    int mapY = -1;
    int direction = -1;
    int weaponIndex = -1;
    int weaponType = -1;
    unsigned short weaponThing;
    unsigned char* rawWeapon;

    if (!dataDir) {
        puts("skip: local DM1 PC34 DUNGEON.DAT/GRAPHICS.DAT not available");
        return 0;
    }

    memset(&state, 0, sizeof(state));
    M11_GameView_Init(&state);
    if (!M11_GameView_StartDm1(&state, dataDir)) {
        fprintf(stderr, "configured DM1 PC34 runtime did not start\n");
        return 1;
    }
    if (!state.assetsAvailable) {
        fprintf(stderr, "DM1 PC34 GRAPHICS.DAT did not bind to M11\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }
    if (!find_real_open_launch_pose(state.world.dungeon, &mapIndex, &mapX,
                                    &mapY, &direction) ||
        !find_real_throwable_weapon(state.world.things, &weaponIndex,
                                    &weaponType, &material)) {
        puts("skip: real DM1 data lacks an open throw lane or projectile weapon");
        M11_GameView_Shutdown(&state);
        return 0;
    }

    weaponThing = make_thing(THING_TYPE_WEAPON, weaponIndex);
    state.world.partyMapIndex = mapIndex;
    state.world.newPartyMapIndex = mapIndex;
    state.world.party.mapIndex = mapIndex;
    state.world.party.mapX = mapX;
    state.world.party.mapY = mapY;
    state.world.party.direction = direction;
    state.world.party.championCount = 1;
    state.world.party.activeChampionIndex = 0;
    memset(&state.world.party.champions[0], 0,
           sizeof(state.world.party.champions[0]));
    memcpy(state.world.party.champions[0].name, "THROW", 5);
    state.world.party.champions[0].present = 1;
    state.world.party.champions[0].hp.current = 100;
    state.world.party.champions[0].hp.maximum = 100;
    state.world.party.champions[0].stamina.current = 200;
    state.world.party.champions[0].stamina.maximum = 200;
    state.world.party.champions[0].attributes[CHAMPION_ATTR_STRENGTH] = 70;
    state.world.party.champions[0].maxLoad = 700;
    state.world.party.champions[0].cell = (unsigned char)(direction & 3);
    state.world.party.champions[0].direction = (unsigned char)(direction & 3);
    state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] =
        weaponThing;

    if (!M11_GameView_TriggerNonMeleeActionByIndex(
            &state, 0, DM1_ACTION_THROW)) {
        fprintf(stderr, "F0328 THROW did not accept real weapon type %d\n",
                weaponType);
        M11_GameView_Shutdown(&state);
        return 1;
    }
    projectile = first_live_projectile(&state.world.projectiles);
    if (!projectile || (unsigned short)projectile->reserved1 != weaponThing) {
        fprintf(stderr, "F0328 did not leave a live projectile with associated weapon\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }

    state.world.partyMapIndex = projectile->mapIndex;
    state.world.newPartyMapIndex = projectile->mapIndex;
    state.world.party.mapIndex = projectile->mapIndex;
    state.world.party.mapX = projectile->mapX;
    state.world.party.mapY = projectile->mapY;
    state.world.party.direction = projectile->direction & 3;
    state.world.gameTick++;

    memset(framebuffer, 0, sizeof(framebuffer));
    M11_GameView_Draw(&state, framebuffer, 320, 200);
    memset(&receipt, 0, sizeof(receipt));
    M11_GameView_GetDm1ProjectileHostPresentationReceipt(&receipt);
    memset(&capture, 0, sizeof(capture));
    M11_GameView_GetDm1F0115C2900RuntimeCaptureReceipt(&capture);
    if (!receipt.valid || !receipt.projectileLane ||
        receipt.objectMaterial != 1 ||
        receipt.graphicsId != material.graphic_index ||
        receipt.objectAspectIndex != material.aspect_index ||
        receipt.destinationW <= 0 || receipt.destinationH <= 0 ||
        receipt.assetWidth <= 0 || receipt.assetHeight <= 0) {
        fprintf(stderr,
                "live thrown object did not reach object-material viewport receipt "
                "(valid=%d object=%d gfx=%d/%d aspect=%d/%d)\n",
                receipt.valid,
                receipt.objectMaterial,
                receipt.graphicsId,
                material.graphic_index,
                receipt.objectAspectIndex,
                material.aspect_index);
        M11_GameView_Shutdown(&state);
        return 1;
    }
    if (!capture.valid || capture.runtimeTick != state.world.gameTick ||
        capture.sourceTick != state.world.gameTick ||
        capture.materialFNV1a == 0u || capture.requestedMaterialCount <= 0 ||
        capture.completedMaterialCount != capture.requestedMaterialCount) {
        fprintf(stderr, "real C2900 material did not complete final M11 capture\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }

    /* F0142/F0115 may not revive the decoded WEAPON mirror after the raw
     * F0156 source record drifts.  The projectile remains live, but its
     * object material is fail-closed for this frame. */
    rawWeapon = state.world.things->rawThingData[THING_TYPE_WEAPON] +
        weaponIndex * 4;
    rawWeapon[2] = 0x7f;
    state.world.gameTick++;
    memset(framebuffer, 0, sizeof(framebuffer));
    M11_GameView_Draw(&state, framebuffer, 320, 200);
    memset(&receipt, 0, sizeof(receipt));
    M11_GameView_GetDm1ProjectileHostPresentationReceipt(&receipt);
    memset(&capture, 0xff, sizeof(capture));
    M11_GameView_GetDm1F0115C2900RuntimeCaptureReceipt(&capture);
    if (receipt.valid) {
        fprintf(stderr,
                "F0115 accepted a decoded thrown weapon after raw F0156 drift\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }
    if (capture.valid || capture.runtimeTick != 0u ||
        capture.materialFNV1a != 0u || capture.requestedMaterialCount != 0 ||
        capture.completedMaterialCount != 0) {
        fprintf(stderr, "stale C2900 material retained final M11 capture\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }

    M11_GameView_Shutdown(&state);
    puts("ok: real PC34 F0328 C2900 material reaches final M11 capture");
    return 0;
}
