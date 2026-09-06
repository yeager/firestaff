/* Canonical I34E original-media entry point for the shared pixel oracle. */
#include "dm1_late_spell_panel_real_check.h"

int main(void) {
    const char *archive = getenv("FIRESTAFF_DM1_PC34_ARCHIVE");
    FILE *media;
    M11_GameViewState *state;
    int ok;
    if (!archive || !(media = fopen(archive, "rb"))) return 77;
    fclose(media);
    if (!asset_file_matches_md5(archive, "ee7b83cdb88c39c441a319f9610e97d6")) return 1;
    state = calloc(1, sizeof(*state));
    if (!state) return 1;
    M11_GameView_Init(state);
    ok = M11_GameView_StartDm1(state, archive) && check_late_spell_panel_real(state);
    M11_GameView_Shutdown(state);
    free(state);
    return ok ? 0 : 1;
}
