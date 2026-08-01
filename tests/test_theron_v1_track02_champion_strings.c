#include "theron_v1_track02_champion_strings.h"
#include <stdio.h>
#include <string.h>

#define ASSERT(cond, msg) do { if (!(cond)) { printf("FAIL: %s\n", msg); return 0; } } while (0)
#define TEST(name) printf("  %-60s", name)
#define PASS() do { printf("PASS\n"); return 1; } while (0)

static int test_classes(void) {
    TEST("4 champion classes (FIGHTER/NINJA/PRIEST/WIZARD)");
    ASSERT(strcmp(theron_v1_track02_us_class_name(0), "FIGHTER") == 0, "0");
    ASSERT(strcmp(theron_v1_track02_us_class_name(1), "NINJA") == 0, "1");
    ASSERT(strcmp(theron_v1_track02_us_class_name(2), "PRIEST") == 0, "2");
    ASSERT(strcmp(theron_v1_track02_us_class_name(3), "WIZARD") == 0, "3");
    ASSERT(theron_v1_track02_us_class_name(4) == NULL, "out of range");
    PASS();
}

static int test_stats(void) {
    TEST("6 stat names");
    ASSERT(strcmp(theron_v1_track02_us_stat_name(0), "STRENGTH") == 0, "0");
    ASSERT(strcmp(theron_v1_track02_us_stat_name(3), "VITALITY") == 0, "3");
    ASSERT(strcmp(theron_v1_track02_us_stat_name(4), "ANTI-MAGIC") == 0, "4");
    ASSERT(strcmp(theron_v1_track02_us_stat_name(5), "ANTI-FIRE") == 0, "5");
    ASSERT(theron_v1_track02_us_stat_name(6) == NULL, "out of range");
    PASS();
}

static int test_resources(void) {
    TEST("3 resource names (HEALTH/STAMINA/MANA)");
    ASSERT(strcmp(theron_v1_track02_us_resource_name(0), "HEALTH") == 0, "0");
    ASSERT(strcmp(theron_v1_track02_us_resource_name(1), "STAMINA") == 0, "1");
    ASSERT(strcmp(theron_v1_track02_us_resource_name(2), "MANA") == 0, "2");
    ASSERT(theron_v1_track02_us_resource_name(3) == NULL, "out of range");
    PASS();
}

static int test_skill_levels(void) {
    TEST("16 skill levels (NEOPHYTE through ARCHMASTER)");
    ASSERT(strcmp(theron_v1_track02_us_skill_level_name(0), "NEOPHYTE") == 0, "0");
    ASSERT(strcmp(theron_v1_track02_us_skill_level_name(7), "EXPERT") == 0, "7");
    ASSERT(strcmp(theron_v1_track02_us_skill_level_name(8), "MASTER") == 0, "8");
    ASSERT(strcmp(theron_v1_track02_us_skill_level_name(13), "MASTER") == 0, "13");
    ASSERT(strcmp(theron_v1_track02_us_skill_level_name(14), "ARCHMASTER") == 0, "14");
    ASSERT(theron_v1_track02_us_skill_level_name(16) == NULL, "out of range");
    PASS();
}

static int test_hand_actions(void) {
    TEST("5 hand-to-hand/movement actions");
    ASSERT(strcmp(theron_v1_track02_us_hand_action_name(0), "PUNCH") == 0, "PUNCH");
    ASSERT(strcmp(theron_v1_track02_us_hand_action_name(1), "KICK") == 0, "KICK");
    ASSERT(strcmp(theron_v1_track02_us_hand_action_name(2), "WAR CRY") == 0, "WAR CRY");
    ASSERT(strcmp(theron_v1_track02_us_hand_action_name(3), "STAB") == 0, "STAB");
    ASSERT(strcmp(theron_v1_track02_us_hand_action_name(4), "CLIMB DOWN") == 0, "CLIMB DOWN");
    ASSERT(theron_v1_track02_us_hand_action_name(5) == NULL, "out of range");
    PASS();
}

static int test_actions(void) {
    TEST("30 combat/spell action names (FREEZE LIFE through THROW)");
    ASSERT(strcmp(theron_v1_track02_us_action_name(0), "FREEZE LIFE") == 0, "FREEZE LIFE");
    ASSERT(strcmp(theron_v1_track02_us_action_name(1), "HIT") == 0, "HIT");
    ASSERT(strcmp(theron_v1_track02_us_action_name(9), "FIREBALL") == 0, "FIREBALL");
    ASSERT(strcmp(theron_v1_track02_us_action_name(12), "LIGHTNING") == 0, "LIGHTNING");
    ASSERT(strcmp(theron_v1_track02_us_action_name(22), "SPELLSHIELD") == 0, "SPELLSHIELD");
    ASSERT(strcmp(theron_v1_track02_us_action_name(29), "THROW") == 0, "THROW");
    ASSERT(theron_v1_track02_us_action_name(30) == NULL, "out of range");
    PASS();
}

static int test_combat_strings(void) {
    TEST("Combat strings (coin flip, reach, ammo)");
    ASSERT(strcmp(theron_v1_track02_us_heads(), "HEADS.") == 0, "heads");
    ASSERT(strcmp(theron_v1_track02_us_tails(), "TAILS.") == 0, "tails");
    ASSERT(strcmp(theron_v1_track02_us_cant_reach(), "CAN'T REACH") == 0, "reach");
    ASSERT(strcmp(theron_v1_track02_us_need_ammo(), "NEED AMMO") == 0, "ammo");
    PASS();
}

static int test_ui_messages(void) {
    TEST("7 UI interaction messages");
    ASSERT(theron_v1_track02_us_ui_message(0) != NULL, "practice");
    ASSERT(strstr(theron_v1_track02_us_ui_message(0), "PRACTICE") != NULL, "practice text");
    ASSERT(strstr(theron_v1_track02_us_ui_message(1), "MEANINGLESS") != NULL, "meaningless");
    ASSERT(strstr(theron_v1_track02_us_ui_message(2), "FLASK") != NULL, "flask");
    ASSERT(strstr(theron_v1_track02_us_ui_message(4), "GAINED") != NULL, "gained");
    ASSERT(strcmp(theron_v1_track02_us_ui_message(6), "IT COMES UP ") == 0, "comes up");
    ASSERT(theron_v1_track02_us_ui_message(7) == NULL, "out of range");
    PASS();
}

static int test_resurrect(void) {
    TEST("GO AWAY AND RESURRECT THERON message");
    ASSERT(strcmp(theron_v1_track02_us_go_away_resurrect(),
                  "GO AWAY AND RESURRECT THERON") == 0, "resurrect");
    PASS();
}

int main(void) {
    printf("Theron V1 Track 02 US Champion Strings\n");
    int pass = 0, total = 0;
    total++; pass += test_classes();
    total++; pass += test_stats();
    total++; pass += test_resources();
    total++; pass += test_skill_levels();
    total++; pass += test_hand_actions();
    total++; pass += test_actions();
    total++; pass += test_combat_strings();
    total++; pass += test_ui_messages();
    total++; pass += test_resurrect();
    printf("\n%d/%d passed\n", pass, total);
    return pass == total ? 0 : 1;
}
