#include "theron_v1_track02_dungeon_lore.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>

static void test_dungeon_count(void)
{
    assert(THERON_TRACK02_DUNGEON_COUNT == 7);
    printf("  PASS: dungeon_count\n");
}

static void test_lore_akutuba(void)
{
    const char *lore = theron_v1_track02_us_dungeon_lore(0);
    (void)lore;
    assert(lore != NULL);
    assert(strstr(lore, "Ak-Tu-Ba") != NULL);
    assert(strstr(lore, "Shield Defiant") != NULL);
    assert(strstr(lore, "Mummies") != NULL);
    printf("  PASS: lore_akutuba\n");
}

static void test_lore_drator(void)
{
    const char *lore = theron_v1_track02_us_dungeon_lore(1);
    (void)lore;
    assert(lore != NULL);
    assert(strstr(lore, "Drator") != NULL);
    assert(strstr(lore, "Taza Boots") != NULL);
    assert(strstr(lore, "Cult of Deaths") != NULL);
    printf("  PASS: lore_drator\n");
}

static void test_lore_formic(void)
{
    const char *lore = theron_v1_track02_us_dungeon_lore(2);
    (void)lore;
    assert(lore != NULL);
    assert(strstr(lore, "Formicia") != NULL);
    assert(strstr(lore, "Taza Poleyn") != NULL);
    assert(strstr(lore, "Trolins") != NULL);
    printf("  PASS: lore_formic\n");
}

static void test_lore_sarmon(void)
{
    const char *lore = theron_v1_track02_us_dungeon_lore(3);
    (void)lore;
    assert(lore != NULL);
    assert(strstr(lore, "Sarmon") != NULL);
    assert(strstr(lore, "Soulcage") != NULL);
    printf("  PASS: lore_sarmon\n");
}

static void test_lore_shado(void)
{
    const char *lore = theron_v1_track02_us_dungeon_lore(4);
    (void)lore;
    assert(lore != NULL);
    assert(strstr(lore, "Shadodan") != NULL);
    assert(strstr(lore, "Taza Armour") != NULL);
    printf("  PASS: lore_shado\n");
}

static void test_lore_thief(void)
{
    const char *lore = theron_v1_track02_us_dungeon_lore(5);
    (void)lore;
    assert(lore != NULL);
    assert(strstr(lore, "Gigglers") != NULL);
    assert(strstr(lore, "Tazahelm") != NULL);
    assert(strstr(lore, "Nordoor") != NULL);
    printf("  PASS: lore_thief\n");
}

static void test_lore_demon(void)
{
    const char *lore = theron_v1_track02_us_dungeon_lore(6);
    (void)lore;
    assert(lore != NULL);
    assert(strstr(lore, "Sargoth") != NULL);
    assert(strstr(lore, "Retaliator") != NULL);
    assert(strstr(lore, "Demon's Gate") != NULL);
    printf("  PASS: lore_demon\n");
}

static void test_bounds(void)
{
    assert(theron_v1_track02_us_dungeon_lore(7) == NULL);
    assert(theron_v1_track02_us_dungeon_lore(255) == NULL);
    printf("  PASS: bounds\n");
}

static void test_save_strings(void)
{
    assert(strcmp(theron_v1_track02_us_file_exists_warning(),
                 "THAT FILE ALREADY EXISTS!") == 0);
    assert(strcmp(theron_v1_track02_us_replace_label(), "REPLACE") == 0);
    assert(strcmp(theron_v1_track02_us_no_label(), "NO") == 0);
    printf("  PASS: save_strings\n");
}

int main(void)
{
    printf("test_theron_v1_track02_dungeon_lore:\n");
    test_dungeon_count();
    test_lore_akutuba();
    test_lore_drator();
    test_lore_formic();
    test_lore_sarmon();
    test_lore_shado();
    test_lore_thief();
    test_lore_demon();
    test_bounds();
    test_save_strings();
    printf("All tests passed.\n");
    return 0;
}
