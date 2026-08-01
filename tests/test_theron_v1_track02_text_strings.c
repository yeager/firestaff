#include "theron_v1_track02_text_strings.h"
#include <stdio.h>
#include <string.h>

#define ASSERT(cond, msg) do { if (!(cond)) { printf("FAIL: %s\n", msg); return 0; } } while (0)
#define TEST(name) printf("  %-60s", name)
#define PASS() do { printf("PASS\n"); return 1; } while (0)

static int test_level_names(void) {
    TEST("15 level names present and correct format");
    for (unsigned i = 0; i < 15; i++) {
        const char *name = theron_v1_track02_us_level_name(i);
        ASSERT(name != NULL, "NULL level name");
        ASSERT(strlen(name) == 8, "wrong length");
        ASSERT(strncmp(name, "LEVEL ", 6) == 0, "wrong prefix");
    }
    ASSERT(strcmp(theron_v1_track02_us_level_name(0), "LEVEL  1") == 0, "L1");
    ASSERT(strcmp(theron_v1_track02_us_level_name(9), "LEVEL 10") == 0, "L10");
    ASSERT(strcmp(theron_v1_track02_us_level_name(14), "LEVEL 15") == 0, "L15");
    ASSERT(theron_v1_track02_us_level_name(15) == NULL, "out of range");
    PASS();
}

static int test_quest_messages(void) {
    TEST("7 quest retrieval messages");
    ASSERT(theron_v1_track02_us_quest_message(0) != NULL, "NULL msg 0");
    ASSERT(strstr(theron_v1_track02_us_quest_message(0), "Shield Defiant") != NULL, "msg 0");
    ASSERT(strstr(theron_v1_track02_us_quest_message(1), "Taza Boots") != NULL, "msg 1");
    ASSERT(strstr(theron_v1_track02_us_quest_message(4), "Taza Armour") != NULL, "msg 4");
    ASSERT(strstr(theron_v1_track02_us_quest_message(6), "Retaliator") != NULL, "msg 6");
    ASSERT(theron_v1_track02_us_quest_message(7) == NULL, "out of range");
    PASS();
}

static int test_save_slot_labels(void) {
    TEST("3 save slot labels");
    ASSERT(strcmp(theron_v1_track02_us_save_slot_label(0), "FILE_1") == 0, "slot 0");
    ASSERT(strcmp(theron_v1_track02_us_save_slot_label(1), "FILE_2") == 0, "slot 1");
    ASSERT(strcmp(theron_v1_track02_us_save_slot_label(2), "FILE_3") == 0, "slot 2");
    ASSERT(theron_v1_track02_us_save_slot_label(3) == NULL, "out of range");
    PASS();
}

static int test_ui_prompts(void) {
    TEST("Save/load UI prompts");
    ASSERT(strcmp(theron_v1_track02_us_play_prompt(), "WHICH FILE DO YOU PLAY?") == 0, "play");
    ASSERT(strcmp(theron_v1_track02_us_load_prompt(), "WHICH FILE DO YOU LOAD?") == 0, "load");
    ASSERT(strcmp(theron_v1_track02_us_yes_label(), "YES") == 0, "yes");
    ASSERT(strcmp(theron_v1_track02_us_no_label(), "NO") == 0, "no");
    PASS();
}

static int test_status_strings(void) {
    TEST("Status condition strings");
    ASSERT(strcmp(theron_v1_track02_us_status_string(0), "POISONED") == 0, "poisoned");
    ASSERT(strcmp(theron_v1_track02_us_status_string(1), "BROKEN") == 0, "broken");
    ASSERT(strcmp(theron_v1_track02_us_status_string(2), "CURSED") == 0, "cursed");
    ASSERT(strcmp(theron_v1_track02_us_status_string(3), ", ") == 0, "comma");
    ASSERT(strcmp(theron_v1_track02_us_status_string(4), " AND ") == 0, "and");
    ASSERT(theron_v1_track02_us_status_string(5) == NULL, "out of range");
    PASS();
}

int main(void) {
    printf("Theron V1 Track 02 US Text Strings\n");
    int pass = 0, total = 0;
    total++; pass += test_level_names();
    total++; pass += test_quest_messages();
    total++; pass += test_save_slot_labels();
    total++; pass += test_ui_prompts();
    total++; pass += test_status_strings();
    printf("\n%d/%d passed\n", pass, total);
    return pass == total ? 0 : 1;
}
