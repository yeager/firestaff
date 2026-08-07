#include "fmtowns_geometry_all_games.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void test_count(void) {
    assert(FMTOWNS_GEOMETRY_ALL_GAMES_COUNT == 3);
}

static void test_lookup(void) {
    const fmtowns_geometry_vaddr_t *p;
    p = fmtowns_geometry_vaddrs_for_game_pc34("DM1");
    assert(p != NULL);
    assert(p->icon_size_vaddr == 0x26c68);
    assert(p->char_geometry_vaddr == 0x26c8a);
    p = fmtowns_geometry_vaddrs_for_game_pc34("CSB");
    assert(p != NULL);
    assert(p->icon_size_vaddr == 0x2c938);
    assert(p->char_geometry_vaddr == 0x2c94c);
    p = fmtowns_geometry_vaddrs_for_game_pc34("DM2");
    assert(p != NULL);
    assert(p->icon_size_vaddr == 0x1de);
    assert(p->char_geometry_vaddr == 0x1f6);
    assert(fmtowns_geometry_vaddrs_for_game_pc34("XXX") == NULL);
    assert(fmtowns_geometry_vaddrs_for_game_pc34(NULL) == NULL);
}

static int verify_at(const char *env_var, uint32_t char_vaddr) {
    /* Return -1 skipped, 0 fail, 1 pass. */
    const char *path = getenv(env_var);
    if (!path || !path[0]) return -1;
    FILE *fp = fopen(path, "rb");
    if (!fp) return -1;
    if (fseek(fp, 0x200 + char_vaddr, SEEK_SET) != 0) { fclose(fp); return 0; }
    uint8_t w[14];
    if (fread(w, 1, 14, fp) != 14) { fclose(fp); return 0; }
    fclose(fp);
    /* Expected: 5,6,1,1,1,6,7 as u16 LE. */
    static const uint8_t expected[14] = {
        5, 0, 6, 0, 1, 0, 1, 0, 1, 0, 6, 0, 7, 0
    };
    return memcmp(w, expected, 14) == 0 ? 1 : 0;
}

static void test_real_data(void) {
    int r;
    int ran = 0;
    r = verify_at("FIRESTAFF_DM1_FMTOWNS_EDM_EXP",   0x26c8a);
    if (r == 1) ++ran; else if (r == 0) assert(0 && "DM1 char mismatch");
    r = verify_at("FIRESTAFF_CSB_FMTOWNS_CHTWE_EXP", 0x2c94c);
    if (r == 1) ++ran; else if (r == 0) assert(0 && "CSB char mismatch");
    r = verify_at("FIRESTAFF_DM2_FMTOWNS_SKULL_EXP", 0x1f6);
    if (r == 1) ++ran; else if (r == 0) assert(0 && "DM2 char mismatch");
    if (ran == 0) puts("SKIP: no EXP env vars set");
    else printf("PASS: %d game(s) verified with byte-exact CHAR geometry\n", ran);
}

int main(void) {
    test_count();
    test_lookup();
    test_real_data();
    puts("All fmtowns_geometry_all_games tests passed.");
    return 0;
}
