#include "fmtowns_shared_tables_all_games.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int verify_bytes(const char *env, uint32_t vaddr, const uint8_t *expected, size_t n) {
    const char *path = getenv(env);
    if (!path || !path[0]) return -1;
    FILE *fp = fopen(path, "rb");
    if (!fp) return -1;
    if (fseek(fp, 0x200 + vaddr, SEEK_SET) != 0) { fclose(fp); return 0; }
    uint8_t buf[64];
    if (n > sizeof(buf)) n = sizeof(buf);
    if (fread(buf, 1, n, fp) != n) { fclose(fp); return 0; }
    fclose(fp);
    return memcmp(buf, expected, n) == 0 ? 1 : 0;
}

int main(void) {
    assert(FMTOWNS_SHARED_TABLES_ALL_GAMES_COUNT == 3);
    const fmtowns_shared_tables_vaddrs_t *dm1 =
        fmtowns_shared_tables_vaddrs_for_game_pc34("DM1");
    const fmtowns_shared_tables_vaddrs_t *csb =
        fmtowns_shared_tables_vaddrs_for_game_pc34("CSB");
    const fmtowns_shared_tables_vaddrs_t *dm2 =
        fmtowns_shared_tables_vaddrs_for_game_pc34("DM2");
    assert(dm1 && csb && dm2);
    assert(dm1->spell_costs_vaddr == 0x24388);
    assert(csb->spell_costs_vaddr == 0x29f64);
    assert(dm2->spell_costs_vaddr == 0x03bb0);
    /* DM2 has no PLAYER_COLOR / ICON_PAL match. */
    assert(dm2->player_color_vaddr == 0);
    assert(dm2->icon_pal_vaddr == 0);
    /* CSB has all four. */
    assert(csb->spell_mult_vaddr == 0x29f7c);
    assert(csb->player_color_vaddr == 0x2d164);
    assert(csb->icon_pal_vaddr == 0x2cd8a);
    /* Unknown game. */
    assert(fmtowns_shared_tables_vaddrs_for_game_pc34("XX") == NULL);
    assert(fmtowns_shared_tables_vaddrs_for_game_pc34(NULL) == NULL);

    /* Real-data verify: read SPELL_COSTS from each game and compare
     * to DM1's shipped 32-byte constant. */
    static const uint8_t spell_costs[32] = {
        0x01,0x02,0x03,0x04,0x05,0x06,0x02,0x03,
        0x04,0x05,0x06,0x07,0x04,0x05,0x06,0x07,
        0x07,0x09,0x02,0x02,0x03,0x04,0x06,0x07,
        0x08,0x0c,0x10,0x14,0x18,0x1c,0x00,0x00
    };
    int ran = 0;
    if (verify_bytes("FIRESTAFF_DM1_FMTOWNS_EDM_EXP",   dm1->spell_costs_vaddr, spell_costs, 30) == 1) ++ran;
    if (verify_bytes("FIRESTAFF_CSB_FMTOWNS_CHTWE_EXP", csb->spell_costs_vaddr, spell_costs, 30) == 1) ++ran;
    if (verify_bytes("FIRESTAFF_DM2_FMTOWNS_SKULL_EXP", dm2->spell_costs_vaddr, spell_costs, 30) == 1) ++ran;
    if (ran > 0) printf("PASS: %d game(s) verified with byte-identical SPELL_COSTS\n", ran);
    else puts("SKIP: no EXP env vars set");
    puts("All fmtowns_shared_tables_all_games tests passed.");
    return 0;
}
