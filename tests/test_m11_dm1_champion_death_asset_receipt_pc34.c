/* Real-PC34 F0319 lifecycle receipt.  The mirror catalogue and portrait are
 * decoded from user-supplied original data; the forced zero-health transition
 * then proves that M11 preserves source portrait bytes while owning one death
 * record, poison cleanup, and a single bones materialization. */
#include "m11_game_view.h"
#include "dm1_v1_resurrection_pc34_compat.h"
#include "memory_dungeon_dat_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int count_bones_for_champion(const struct DungeonThings_Compat* things,
                                    int championIndex) {
    int i;
    int count = 0;
    if (!things || !things->junks) return 0;
    for (i = 0; i < things->junkCount; ++i) {
        const struct DungeonJunk_Compat* junk = &things->junks[i];
        if (junk->type == DM1_JUNK_TYPE_BONES && junk->doNotDiscard &&
            junk->chargeCount == (unsigned char)championIndex) {
            ++count;
        }
    }
    return count;
}

int main(void) {
    const char* dataDir = getenv("FIRESTAFF_DM1_DATA_DIR");
    M11_GameViewState state;
    struct ChampionState_Compat* champion;
    unsigned char portraitBefore[CHAMPION_PORTRAIT_BITMAP_BYTE_COUNT];
    int bonesBefore;
    int bonesAfter;

    if (!dataDir || !dataDir[0]) {
        puts("skip: FIRESTAFF_DM1_DATA_DIR is not selected");
        return 0;
    }
    M11_GameView_Init(&state);
    if (!M11_GameView_StartDm1(&state, dataDir)) {
        M11_GameView_Shutdown(&state);
        return 1;
    }
    if (M11_GameView_GetMirrorCatalogCount(&state) <= 0 ||
        !M11_GameView_RecruitChampionByMirrorOrdinal(&state, 0)) {
        fprintf(stderr, "real PC34 mirror record unavailable\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }

    champion = &state.world.party.champions[0];
    if (!champion->portraitBitmapValid || champion->hp.maximum == 0) {
        fprintf(stderr, "real PC34 champion portrait/vitals unavailable\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }
    memcpy(portraitBefore, champion->portraitBitmap, sizeof(portraitBefore));
    state.world.party.direction = 2;
    state.world.party.activeChampionIndex = 0;
    champion->hp.current = 0;
    champion->poisonDose = 9;
    champion->direction = 0;
    state.world.lifecycle.champions[0].poisonEventCount = 1;
    bonesBefore = count_bones_for_champion(state.world.things, 0);

    M11_GameView_ProbeCheckPartyDeath(&state);
    bonesAfter = count_bones_for_champion(state.world.things, 0);
    if (bonesAfter != bonesBefore + 1 || champion->poisonDose != 0 ||
        state.world.lifecycle.champions[0].poisonEventCount != 0 ||
        champion->direction != 2 || !champion->portraitBitmapValid ||
        memcmp(portraitBefore, champion->portraitBitmap, sizeof(portraitBefore)) != 0) {
        fprintf(stderr, "real PC34 F0319 lifecycle receipt failed\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }
    M11_GameView_ProbeCheckPartyDeath(&state);
    if (count_bones_for_champion(state.world.things, 0) != bonesAfter) {
        fprintf(stderr, "real PC34 F0319 emitted duplicate bones\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }
    printf("ok: real PC34 F0319 preserves C026 portrait and emits one bones record\n");
    M11_GameView_Shutdown(&state);
    return 0;
}
