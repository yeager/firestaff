#include "theron_v1_track02_combat_messages.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    /* Item conditions */
    assert(strcmp(theron_v1_track02_item_condition(0), "CONSUMABLE") == 0);
    assert(strcmp(theron_v1_track02_item_condition(1), "POISONED") == 0);
    assert(strcmp(theron_v1_track02_item_condition(3), "CURSED") == 0);
    assert(strcmp(theron_v1_track02_item_condition(5), " AND ") == 0);
    assert(theron_v1_track02_item_condition(7) == NULL);

    /* Combat feedback */
    assert(strcmp(theron_v1_track02_combat_feedback(0), "IT COMES UP ") == 0);
    assert(strcmp(theron_v1_track02_combat_feedback(1), "HEADS.") == 0);
    assert(strcmp(theron_v1_track02_combat_feedback(2), "TAILS.") == 0);
    assert(strcmp(theron_v1_track02_combat_feedback(3), "CAN'T REACH") == 0);
    assert(strcmp(theron_v1_track02_combat_feedback(4), "NEED AMMO") == 0);
    assert(theron_v1_track02_combat_feedback(5) == NULL);

    /* System messages */
    assert(strcmp(theron_v1_track02_system_wake_up(), "WAKE UP") == 0);
    assert(strcmp(theron_v1_track02_system_game_frozen(), "GAME FROZEN") == 0);
    assert(strcmp(theron_v1_track02_system_resurrected(), "RESURRECTED.") == 0);
    assert(strcmp(theron_v1_track02_system_pass(), "PASS") == 0);
    assert(strcmp(theron_v1_track02_resurrect_theron(), "GO AWAY AND RESURRECT THERON") == 0);

    /* File select */
    assert(strcmp(theron_v1_track02_file_select_play(), "WHICH FILE DO YOU PLAY?") == 0);

    /* CD-ROM2 requirement */
    assert(strstr(theron_v1_track02_cdrom2_requirement(), "SUPER CD-ROM2") != NULL);

    printf("PASS: theron_v1_track02_combat_messages\n");
    return 0;
}
