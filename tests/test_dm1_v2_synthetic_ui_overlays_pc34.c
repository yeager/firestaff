#include "dm1_v2_journal.h"
#include "dm1_v2_journal_pc34.h"
#include "dm1_v2_minimap.h"
#include "dm1_v2_minimap_pc34.h"
#include "dm1_v2_tooltip_pc34.h"

#include <stdio.h>
#include <string.h>

static int failures;

#define CHECK(expr) do { \
    if (!(expr)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr); \
        ++failures; \
    } \
} while (0)

static void test_pc34_journal_retains_no_text_or_file(void) {
    M11_V2_JournalEntry entry;

    memset(&entry, 0xA5, sizeof(entry));
    v2_journal_init();
    v2_journal_add(M11_V2_JC_COMBAT, "invented combat history", 3, 900U);
    CHECK(v2_journal_get_page(0) == NULL);
    CHECK(v2_journal_get_page_count(0) == 0);
    v2_journal_next_page();
    v2_journal_prev_page();
    v2_journal_clear();
    CHECK(v2_journal_save("v2-journal.bin") == false);
    CHECK(v2_journal_load("v2-journal.bin") == false);
    CHECK(entry.text[0] == (char)0xA5);
}

static void test_legacy_journal_does_not_modify_or_paint(void) {
    DM1_V2_Journal journal;
    unsigned int framebuffer[32];
    unsigned int before[32];

    memset(&journal, 0xA5, sizeof(journal));
    memset(framebuffer, 0x3C, sizeof(framebuffer));
    memcpy(before, framebuffer, sizeof(framebuffer));
    dm1_v2_journal_init(&journal);
    dm1_v2_journal_add(&journal, 42, 1, 2, "host journal entry");
    dm1_v2_journal_render(&journal, framebuffer, 8, 4);
    CHECK(((const unsigned char *)&journal)[0] == 0xA5);
    CHECK(memcmp(framebuffer, before, sizeof(framebuffer)) == 0);
}

static void test_minimap_has_no_cache_or_framebuffer_overlay(void) {
    M11_V2_Minimap pc34_map;
    DM1_V2_Minimap legacy_map;
    unsigned char indexed[64];
    unsigned char indexed_before[64];
    unsigned int rgba[64];
    unsigned int rgba_before[64];

    memset(&pc34_map, 0xA5, sizeof(pc34_map));
    memset(&legacy_map, 0x5A, sizeof(legacy_map));
    memset(indexed, 0x12, sizeof(indexed));
    memset(rgba, 0x34, sizeof(rgba));
    memcpy(indexed_before, indexed, sizeof(indexed));
    memcpy(rgba_before, rgba, sizeof(rgba));

    v2_minimap_init(&pc34_map);
    v2_minimap_reveal_tile(&pc34_map, 1, 2, M11_V2_TILE_DOOR);
    v2_minimap_set_party(&pc34_map, 1, 2, 3);
    v2_minimap_toggle_visible(&pc34_map);
    v2_minimap_zoom(&pc34_map, true);
    v2_minimap_render(&pc34_map, indexed, 8, 8);
    CHECK(v2_minimap_is_explored(&pc34_map, 1, 2) == false);
    CHECK(((const unsigned char *)&pc34_map)[0] == 0xA5);
    CHECK(memcmp(indexed, indexed_before, sizeof(indexed)) == 0);

    dm1_v2_minimap_init(&legacy_map, 32, 32);
    dm1_v2_minimap_update(&legacy_map, 1, 2, 3);
    dm1_v2_minimap_reveal(&legacy_map, 3, 4);
    dm1_v2_minimap_render(&legacy_map, rgba, 8, 8);
    CHECK(((const unsigned char *)&legacy_map)[0] == 0x5A);
    CHECK(memcmp(rgba, rgba_before, sizeof(rgba)) == 0);
}

static void test_tooltip_has_no_text_timer_or_pixels(void) {
    unsigned char framebuffer[64];
    unsigned char before[64];
    const char *evidence;

    memset(framebuffer, 0xCC, sizeof(framebuffer));
    memcpy(before, framebuffer, sizeof(framebuffer));
    v2_tooltip_init();
    v2_tooltip_show("synthetic tooltip", 2, 3);
    v2_tooltip_update(1.0f);
    v2_tooltip_render(framebuffer, 8, 8);
    v2_tooltip_hide();
    CHECK(v2_tooltip_is_visible() == false);
    CHECK(memcmp(framebuffer, before, sizeof(framebuffer)) == 0);
    CHECK(v2_tooltip_source_lock_ok() == 1U);
    evidence = v2_tooltip_get_source_evidence();
    CHECK(strstr(evidence, "PC34 owns") != NULL);
    CHECK(strstr(evidence, "no host glyph") != NULL);
}

int main(void) {
    test_pc34_journal_retains_no_text_or_file();
    test_legacy_journal_does_not_modify_or_paint();
    test_minimap_has_no_cache_or_framebuffer_overlay();
    test_tooltip_has_no_text_timer_or_pixels();

    if (failures != 0) {
        fprintf(stderr, "dm1_v2_synthetic_ui_overlays_pc34: %d failure(s)\n", failures);
        return 1;
    }
    puts("dm1_v2_synthetic_ui_overlays_pc34: ok");
    return 0;
}
