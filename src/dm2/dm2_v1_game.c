#include "dm2_v1_game.h"
#include "asset_find_by_hash.h"
#include <string.h>
#include <stdio.h>

/* Known DM2 DUNGEON.DAT MD5 hashes.
 * DM2 dungeon files are inside zip archives on disk — until extraction
 * is implemented, these hashes cover extracted variants. Add hashes as
 * verified versions are confirmed. */
static const char *const dm2_dungeon_hashes[] = {
    "6caccd7875009e82fe2e28e7f6d6adc0",  /* DM2 PC English DUNGEON.DAT */
    NULL
};

/* Known DM2 GRAPHICS.DAT MD5 hashes (for fallback graphics search) */
static const char *const dm2_graphics_hashes[] = {
    "25247ede4dabb6a71e5dabdfbcd5907d",  /* PC English */
    "b4d733576ea60c41737f79f212faf528",  /* PC French */
    "e52ab5e01715042b16a4dcff02052e5d",  /* PC German/English JewelCase */
    NULL
};

void dm2_v1_init(DM2_V1_GameState *state, const char *data_dir) {
    if (!state) return;
    memset(state, 0, sizeof(*state));
    state->data_dir = data_dir;
    state->party_x = 15;
    state->party_y = 15;
    state->party_dir = 0;
    state->gold = 100;
    state->time_of_day = 720;
}

int dm2_v1_load_dungeon(DM2_V1_GameState *state) {
    char path[ASSET_PATH_MAX];
    if (!state || !state->data_dir) return -1;

    /* Hash-based search for dungeon data */
    if (dm2_dungeon_hashes[0] != NULL &&
        asset_find_by_md5_list(state->data_dir, dm2_dungeon_hashes,
                               path, sizeof(path), NULL, 4)) {
        printf("DM2: found dungeon at %s (hash-verified)\n", path);
        /* TODO: parse DM2 DUNGEON.DAT */
        return 0;
    }

    printf("DM2: no verified dungeon hash available yet — "
           "dungeon files need to be extracted from zip archives first\n");
    return -1;
}

int dm2_v1_enter_shop(DM2_V1_GameState *state) {
    if (!state || !state->outdoor) return -1;
    printf("DM2: entering shop (gold: %d)\n", state->gold);
    return 0;
}

int dm2_v1_is_outdoor(const DM2_V1_GameState *state) {
    return state ? state->outdoor : 0;
}
