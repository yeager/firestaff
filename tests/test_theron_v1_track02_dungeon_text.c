#include "theron_v1_track02_dungeon_text.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    assert(strcmp(theron_v1_track02_us_dungeon_name(0), "AKUTUBA") == 0);
    assert(strcmp(theron_v1_track02_us_dungeon_name(1), "DRATOR") == 0);
    assert(strcmp(theron_v1_track02_us_dungeon_name(2), "FORMIC") == 0);
    assert(strcmp(theron_v1_track02_us_dungeon_name(3), "SARMON") == 0);
    assert(strcmp(theron_v1_track02_us_dungeon_name(4), "SHADO") == 0);
    assert(strcmp(theron_v1_track02_us_dungeon_name(5), "THIEF") == 0);
    assert(strcmp(theron_v1_track02_us_dungeon_name(6), "DEMON") == 0);
    assert(theron_v1_track02_us_dungeon_name(7) == NULL);

    assert(strcmp(theron_v1_track02_us_treasure_name(0), "Shield Defiant") == 0);
    assert(strcmp(theron_v1_track02_us_treasure_name(1), "Taza Boots") == 0);
    assert(strcmp(theron_v1_track02_us_treasure_name(2), "Taza Poleyn") == 0);
    assert(strcmp(theron_v1_track02_us_treasure_name(3), "Soulcage") == 0);
    assert(strcmp(theron_v1_track02_us_treasure_name(4), "Taza Armour") == 0);
    assert(strcmp(theron_v1_track02_us_treasure_name(5), "Tazahelm") == 0);
    assert(strcmp(theron_v1_track02_us_treasure_name(6), "Retaliator") == 0);
    assert(theron_v1_track02_us_treasure_name(7) == NULL);

    for (unsigned i = 0; i < 7; i++) {
        const char *story = theron_v1_track02_us_dungeon_story(i);
        assert(story != NULL);
        assert(strlen(story) > 100);
    }
    assert(theron_v1_track02_us_dungeon_story(7) == NULL);

    assert(strstr(theron_v1_track02_us_dungeon_story(0), "Ak-Tu-Ba") != NULL);
    assert(strstr(theron_v1_track02_us_dungeon_story(1), "Drator") != NULL);
    assert(strstr(theron_v1_track02_us_dungeon_story(2), "Formicia") != NULL);
    assert(strstr(theron_v1_track02_us_dungeon_story(3), "Sarmon") != NULL);
    assert(strstr(theron_v1_track02_us_dungeon_story(4), "Shadodan") != NULL);
    assert(strstr(theron_v1_track02_us_dungeon_story(5), "Gigglers") != NULL);
    assert(strstr(theron_v1_track02_us_dungeon_story(6), "Demon's Gate") != NULL);

    for (unsigned i = 0; i < 7; i++) {
        const char *msg = theron_v1_track02_us_retrieval_message(i);
        assert(msg != NULL);
        assert(strstr(msg, "THERON has retrieved") != NULL);
    }
    assert(theron_v1_track02_us_retrieval_message(7) == NULL);

    assert(strstr(theron_v1_track02_us_retrieval_message(0), "Shield Defiant") != NULL);
    assert(strstr(theron_v1_track02_us_retrieval_message(6), "Retaliator") != NULL);

    assert(strcmp(theron_v1_track02_us_game_speed_label(), "GAME SPEED") == 0);

    printf("PASS: theron_v1_track02_dungeon_text\n");
    return 0;
}
