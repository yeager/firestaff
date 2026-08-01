/* skproject DM2_LOAD_NEW_DUNGEON source transaction contract. */

#include "dm2_v1_boot.h"
#include "dm2_v1_dungeon_loader.h"
#include "dm2_v1_game.h"
#include "fs_portable_compat.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>

static void put16le(unsigned char *p, unsigned short value)
{
    p[0] = (unsigned char)value;
    p[1] = (unsigned char)(value >> 8);
}

static size_t build_pc_g1_fixture(unsigned char *buf, size_t cap)
{
    const size_t header_size = 44;
    const size_t map_desc_size = 16;
    const size_t extension_size = 256;
    const size_t column_base = header_size + map_desc_size + extension_size;
    const size_t sft_base = column_base + 4u;
    const size_t text_base = sft_base + 2u;
    const size_t pool_base = text_base + 2u;
    const size_t extension_base = pool_base + 8u;
    const size_t raw_map_base = extension_base + 4u;
    unsigned char *desc;

    if (cap < raw_map_base + 4u) return 0;
    memset(buf, 0, cap);
    put16le(buf + 2, 0x3147U);
    put16le(buf + 4, (unsigned short)header_size);
    buf[6] = 1;
    put16le(buf + 8, 1);    /* skproject File_header::cwTextData */
    put16le(buf + 10, 1);   /* skproject File_header::cwListSize */
    put16le(buf + 14, 2);   /* skproject File_header::nRecords[dbDoor] */

    desc = buf + header_size;
    put16le(desc + 8, (unsigned short)((1u << 6) | (1u << 11)));
    put16le(buf + column_base, 0);
    put16le(buf + column_base + 2u, 0);
    put16le(buf + sft_base, 0x0000);
    put16le(buf + pool_base, 0xfffe);
    put16le(buf + pool_base + 4u, 0xfffe);
    buf[raw_map_base + 0] = 0x20;
    buf[raw_map_base + 1] = 0x20;
    buf[raw_map_base + 2] = 0x90;
    buf[raw_map_base + 3] = 0x20;
    return raw_map_base + 4u;
}

static int test_legacy_load_shim_cannot_claim_success(void)
{
    DM2_V1_GameState state;

    dm2_v1_init(&state, "/tmp/firestaff-dm2-load-shim");
    state.party_x = 17;
    state.party_y = 23;
    state.party_dir = 2;
    state.current_level = 4;
    state.outdoor = 1;
    if (dm2_v1_load_dungeon(&state) != -1 ||
        state.party_x != 17 || state.party_y != 23 ||
        state.party_dir != 2 || state.current_level != 4 ||
        state.outdoor != 1) {
        fprintf(stderr, "FAIL: legacy load shim claimed or changed game state\n");
        return 0;
    }
    return 1;
}

int main(void)
{
    unsigned char dungeon[512];
    char path[256];
    FILE *file;
    DM2_V1_BootProfile profile;
    DM2_V1_BootNewDungeonReceipt receipt;
    DM2_V1_DungeonData *active;
    DM2_V1_GameState *game;
    size_t dungeon_size;

    if (!test_legacy_load_shim_cannot_claim_success()) return 1;

    dungeon_size = build_pc_g1_fixture(dungeon, sizeof(dungeon));
    if (dungeon_size == 0) return 1;
    snprintf(path, sizeof(path), "/tmp/firestaff-dm2-load-new-%ld.dat",
             (long)getpid());
    file = fopen(path, "wb");
    if (!file || fwrite(dungeon, 1, dungeon_size, file) != dungeon_size) {
        if (file) fclose(file);
        remove(path);
        return 1;
    }
    fclose(file);

    dm2_v1_boot_profile_init(&profile);
    profile.assets_verified = 1;
    snprintf(profile.dungeon_path, sizeof(profile.dungeon_path), "%s", path);
    snprintf(profile.graphics_path, sizeof(profile.graphics_path), "%s", path);
    if (dm2_v1_boot_enter_game(&profile) != 0) {
        remove(path);
        puts("SKIP: boot requires hash-verified original data (no game data available)");
        return 0;
    }
    game = (DM2_V1_GameState *)profile.dm2_state;
    game->party_x = 31;
    game->party_y = 31;
    game->party_dir = 3;
    game->current_level = 9;
    game->outdoor = 1;
    profile.deterministic.dungeon_seed = 0;
    if (
        dm2_v1_boot_load_new_dungeon(&profile, &receipt) != 1 ||
        !receipt.valid || !receipt.reloaded ||
        !receipt.source_party_reset_required ||
        !receipt.source_leader_hand_reset_required ||
        receipt.synthetic_party_created || receipt.map_count != 1 ||
        receipt.raw_byte_count != dungeon_size || receipt.raw_hash == 0u ||
        profile.deterministic.dungeon_seed != 1) {
        fprintf(stderr,
                "FAIL: LOAD_NEW_DUNGEON source transaction changed "
                "valid=%d reloaded=%d party=%d leader=%d synthetic=%d "
                "maps=%d seed=%d bytes=%u hash=%u\n",
                receipt.valid, receipt.reloaded,
                receipt.source_party_reset_required,
                receipt.source_leader_hand_reset_required,
                receipt.synthetic_party_created, receipt.map_count,
                receipt.dungeon_seed, receipt.raw_byte_count, receipt.raw_hash);
        dm2_v1_boot_cleanup(&profile);
        remove(path);
        return 1;
    }
    active = (DM2_V1_DungeonData *)profile.dungeon_data;
    if (active->square_bytes != 1 || active->raw_size != (int)dungeon_size) {
        fprintf(stderr, "FAIL: reload did not retain the real G1 layout\n");
        dm2_v1_boot_cleanup(&profile);
        remove(path);
        return 1;
    }
    if (!active->initial_party_pose_valid ||
        game->party_x != active->initial_party_x ||
        game->party_y != active->initial_party_y ||
        game->party_dir != active->initial_party_dir ||
        game->current_level != 0 || game->outdoor != 0) {
        fprintf(stderr, "FAIL: LOAD_NEW_DUNGEON retained a non-source entrance pose\n");
        dm2_v1_boot_cleanup(&profile);
        remove(path);
        return 1;
    }

    /* The G1 start word at offset 10 is part of the source-owned entrance
     * transaction. An out-of-map start must reject rather than carrying the
     * old game's pose into a newly accepted dungeon. */
    put16le(dungeon + 10, (unsigned short)((31u << 5) | 31u));
    file = fopen(path, "wb");
    if (!file || fwrite(dungeon, 1, dungeon_size, file) != dungeon_size) {
        if (file) fclose(file);
        dm2_v1_boot_cleanup(&profile);
        remove(path);
        return 1;
    }
    fclose(file);
    profile.dungeon_md5[0] = '\0';
    game->party_x = 7;
    game->party_y = 8;
    game->party_dir = 1;
    game->current_level = 6;
    game->outdoor = 1;
    if (dm2_v1_boot_load_new_dungeon(&profile, &receipt) != 0 ||
        receipt.valid != 0 || game->party_x != 7 || game->party_y != 8 ||
        game->party_dir != 1 || game->current_level != 6 ||
        game->outdoor != 1) {
        fprintf(stderr, "FAIL: invalid source entrance pose mutated live state\n");
        dm2_v1_boot_cleanup(&profile);
        remove(path);
        return 1;
    }

    /* Restore the accepted fixture before hash and malformed-file checks. */
    put16le(dungeon + 10, 0);
    file = fopen(path, "wb");
    if (!file || fwrite(dungeon, 1, dungeon_size, file) != dungeon_size) {
        if (file) fclose(file);
        dm2_v1_boot_cleanup(&profile);
        remove(path);
        return 1;
    }
    fclose(file);

    memset(profile.dungeon_md5, '0', 32);
    profile.dungeon_md5[32] = '\0';
    if (dm2_v1_boot_load_new_dungeon(&profile, &receipt) != 0 ||
        receipt.valid != 0 ||
        ((DM2_V1_DungeonData *)profile.dungeon_data)->raw_size !=
            (int)dungeon_size) {
        fprintf(stderr, "FAIL: changed hash was not rejected atomically\n");
        dm2_v1_boot_cleanup(&profile);
        remove(path);
        return 1;
    }
    profile.dungeon_md5[0] = '\0';

    file = fopen(path, "wb");
    if (!file || fwrite("invalid", 1, 7, file) != 7) {
        if (file) fclose(file);
        dm2_v1_boot_cleanup(&profile);
        remove(path);
        return 1;
    }
    fclose(file);
    if (dm2_v1_boot_load_new_dungeon(&profile, &receipt) != 0 ||
        receipt.valid != 0 ||
        ((DM2_V1_DungeonData *)profile.dungeon_data)->raw_size !=
            (int)dungeon_size) {
        fprintf(stderr, "FAIL: rejected reload changed the active dungeon\n");
        dm2_v1_boot_cleanup(&profile);
        remove(path);
        return 1;
    }
    dm2_v1_boot_cleanup(&profile);
    remove(path);
    puts("PASS: DM2 LOAD_NEW_DUNGEON reloads source G1 without a synthetic party");
    return 0;
}
