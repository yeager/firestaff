#include "dm1_v1_fmtowns_music_tables.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void test_level_songs_byte_exact(void) {
    static const uint16_t expected[15] = {
        0x0000, 0x0006, 0x0007, 0x000c, 0x0009,
        0x0004, 0x0003, 0x000a, 0x000d, 0x000e,
        0x000f, 0x0008, 0x0011, 0x0001, 0x0000
    };
    for (int i = 0; i < 15; ++i) {
        assert(dm1_v1_fmtowns_level_songs[i] == expected[i]);
    }
}

static void test_title_asset_ids(void) {
    assert(DM1_V1_FMTOWNS_TITLE_PRESENTS_ASSET == 12);
    assert(DM1_V1_FMTOWNS_TITLE_DUNGEON_ASSET == 13);
    assert(DM1_V1_FMTOWNS_TITLE_MASTER_ASSET == 14);
}

static void test_icon_pal(void) {
    assert(dm1_v1_fmtowns_icon_pal[0] == 0x09);
    assert(dm1_v1_fmtowns_icon_pal[1] == 0x0a);
    assert(dm1_v1_fmtowns_icon_pal[2] == 0x0b);
}

static void test_dm_music(void) {
    assert(dm1_v1_fmtowns_dm_music_defaults[0] == 0x03);
    assert(dm1_v1_fmtowns_dm_music_defaults[1] == 0x04);
    assert(dm1_v1_fmtowns_dm_music_defaults[2] == 0x10);
    assert(dm1_v1_fmtowns_dm_music_defaults[3] == 0x0b);
}

static void test_spell_costs(void) {
    /* Byte-exact match of the 32-byte SPELL_COSTS table. */
    static const uint8_t expected[32] = {
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x02, 0x03,
        0x04, 0x05, 0x06, 0x07, 0x04, 0x05, 0x06, 0x07,
        0x07, 0x09, 0x02, 0x02, 0x03, 0x04, 0x06, 0x07,
        0x08, 0x0c, 0x10, 0x14, 0x18, 0x1c, 0x00, 0x00
    };
    for (int i = 0; i < 32; ++i) {
        assert(dm1_v1_fmtowns_spell_costs[i] == expected[i]);
    }
}

static void test_level_song_lookup(void) {
    assert(dm1_v1_fmtowns_level_song_for_level_pc34(0) == 0x0000);
    assert(dm1_v1_fmtowns_level_song_for_level_pc34(3) == 0x000c);
    assert(dm1_v1_fmtowns_level_song_for_level_pc34(12) == 0x0011);
    assert(dm1_v1_fmtowns_level_song_for_level_pc34(14) == 0x0000);
    /* Out of range returns END. */
    assert(dm1_v1_fmtowns_level_song_for_level_pc34(15) == 0xffff);
    assert(dm1_v1_fmtowns_level_song_for_level_pc34(9999) == 0xffff);
}

static void test_player_color_and_spell_mult(void) {
    static const uint8_t pc[8] = {0x07, 0x0b, 0x08, 0x0e, 0x05, 0x05, 0x04, 0x06};
    static const uint8_t sm[8] = {0x08, 0x0c, 0x10, 0x14, 0x18, 0x1c, 0x00, 0x00};
    for (int i = 0; i < 8; ++i) {
        assert(dm1_v1_fmtowns_player_color[i] == pc[i]);
        assert(dm1_v1_fmtowns_spell_mult[i] == sm[i]);
    }
}

static void test_real_data_round_trip(void) {
    const char *path = getenv("FIRESTAFF_DM1_FMTOWNS_EDM_EXP");
    FILE *fp;
    uint8_t buf[64];
    if (!path || !path[0]) { puts("SKIP: no EDM.EXP"); return; }
    fp = fopen(path, "rb");
    if (!fp) { puts("SKIP: cannot open"); return; }

    /* LEVEL_SONGS @ 0x3fbcc: 30 bytes (15 words). */
    if (fseek(fp, 0x200 + 0x3fbcc, SEEK_SET) != 0) { fclose(fp); puts("SKIP: seek"); return; }
    if (fread(buf, 1, 30, fp) != 30) { fclose(fp); puts("SKIP: read"); return; }
    for (int i = 0; i < 15; ++i) {
        uint16_t w = (uint16_t)(buf[i*2] | (buf[i*2+1] << 8));
        assert(w == dm1_v1_fmtowns_level_songs[i]);
    }

    /* SPELL_COSTS @ 0x24388: 32 bytes. */
    if (fseek(fp, 0x200 + 0x24388, SEEK_SET) != 0) { fclose(fp); puts("SKIP: seek"); return; }
    if (fread(buf, 1, 32, fp) != 32) { fclose(fp); puts("SKIP: read"); return; }
    for (int i = 0; i < 32; ++i) {
        assert(buf[i] == dm1_v1_fmtowns_spell_costs[i]);
    }

    /* ICON_PAL @ 0x28f44: 6 bytes (3 words). */
    if (fseek(fp, 0x200 + 0x28f44, SEEK_SET) != 0) { fclose(fp); puts("SKIP: seek"); return; }
    if (fread(buf, 1, 6, fp) != 6) { fclose(fp); puts("SKIP: read"); return; }
    for (int i = 0; i < 3; ++i) {
        uint16_t w = (uint16_t)(buf[i*2] | (buf[i*2+1] << 8));
        assert(w == dm1_v1_fmtowns_icon_pal[i]);
    }

    /* DM_MUSIC @ 0x3fa80: 4 bytes. */
    if (fseek(fp, 0x200 + 0x3fa80, SEEK_SET) != 0) { fclose(fp); puts("SKIP: seek"); return; }
    if (fread(buf, 1, 4, fp) != 4) { fclose(fp); puts("SKIP: read"); return; }
    for (int i = 0; i < 4; ++i) {
        assert(buf[i] == dm1_v1_fmtowns_dm_music_defaults[i]);
    }

    /* PLAYER_COLOR @ 0x291b8: 8 bytes. */
    if (fseek(fp, 0x200 + 0x291b8, SEEK_SET) != 0) { fclose(fp); puts("SKIP: seek"); return; }
    if (fread(buf, 1, 8, fp) != 8) { fclose(fp); puts("SKIP: read"); return; }
    for (int i = 0; i < 8; ++i) {
        assert(buf[i] == dm1_v1_fmtowns_player_color[i]);
    }

    /* SPELL_MULT @ 0x243a0: 8 bytes. */
    if (fseek(fp, 0x200 + 0x243a0, SEEK_SET) != 0) { fclose(fp); puts("SKIP: seek"); return; }
    if (fread(buf, 1, 8, fp) != 8) { fclose(fp); puts("SKIP: read"); return; }
    for (int i = 0; i < 8; ++i) {
        assert(buf[i] == dm1_v1_fmtowns_spell_mult[i]);
    }

    fclose(fp);
    puts("PASS: real EDM.EXP music/spell tables match shipped constants");
}

int main(void) {
    test_level_songs_byte_exact();
    test_title_asset_ids();
    test_icon_pal();
    test_dm_music();
    test_spell_costs();
    test_level_song_lookup();
    test_player_color_and_spell_mult();
    test_real_data_round_trip();
    puts("All dm1_v1_fmtowns_music_tables tests passed.");
    return 0;
}
