#include "csb_v1_atari_save_runtime_handoff_pc34_compat.h"
#include "csb_v1_save_load_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if defined(_WIN32) || defined(_WIN64)
#include <direct.h>
#else
#include <sys/stat.h>
#endif

static int make_directory(const char *path)
{
#if defined(_WIN32) || defined(_WIN64)
    return _mkdir(path);
#else
    return mkdir(path, 0700);
#endif
}

static int read_file(const char *path, uint8_t **out, size_t *out_size)
{
    FILE *fp; long n; uint8_t *p = NULL;
    if (!path || !(fp = fopen(path, "rb"))) return 0;
    if (fseek(fp, 0, SEEK_END) || (n = ftell(fp)) < 1 || fseek(fp, 0, SEEK_SET) ||
        !(p = (uint8_t *)malloc((size_t)n)) || fread(p, 1u, (size_t)n, fp) != (size_t)n) {
        free(p); fclose(fp); return 0;
    }
    fclose(fp); *out = p; *out_size = (size_t)n; return 1;
}

static int write_file(const char *path, const uint8_t *bytes, size_t size)
{
    FILE *fp;
    size_t written;
    int close_result;
    if (!path || !bytes || size == 0u || !(fp = fopen(path, "wb"))) return 0;
    written = fwrite(bytes, 1u, size, fp);
    close_result = fclose(fp);
    return written == size && close_result == 0;
}

int main(void)
{
    const char *path = getenv("FIRESTAFF_CSB_ATARI_MINI");
    const char *corpus_path = getenv("FIRESTAFF_CSB_ATARI_SAVE_CORPUS");
    const char *fmtowns_mini = getenv("FIRESTAFF_CSB_FMTOWNS_MINI");
    CSB_V1_RuntimeProfile runtime;
    CSB_V1_AtariSaveInfo info;
    uint8_t *bytes = NULL;
    size_t size = 0u;

    csb_v1_runtime_init(&runtime, NULL);
    if (csb_v1_atari_save_handoff_runtime_pc34_compat(&runtime, NULL, 0u, &info) !=
        CSB_V1_ATARI_RUNTIME_ERR_NULL) return 1;
    /* ReDMCSB LOADSAVE.C F0435 selects F31E/F31J's native save-header path.
     * Its CDATA/CJDATA MINI.DAT startup bytes are not Atari ST/Amiga
     * GAMEBLOCK saves, despite sharing the filename.  Do not let the
     * big-endian Atari decoder turn an unverified FM Towns layout into a
     * Resume candidate while that platform-specific handoff is still open. */
    if (fmtowns_mini && fmtowns_mini[0]) {
        if (csb_v1_runtime_can_load_resume_path(fmtowns_mini) ||
            csb_v1_runtime_load_game_from_path(&runtime, fmtowns_mini) ==
                CSB_V1_LOAD_OK ||
            runtime.dungeon_handle != NULL ||
            runtime.party_state.ChampionCount != 0) {
            csb_v1_runtime_cleanup(&runtime);
            return 1;
        }
        puts("PASS: FM Towns MINI.DAT stays outside the Atari/Amiga save handoff");
    }
    if (corpus_path && corpus_path[0]) {
        const char *written_path = "/tmp/CSBGAME2.DAT";
        const char *backup_path = "/tmp/CSBGAME2.BAK";
        const char *blocked_dir = "/tmp/CSBGAME3.DAT";
        const char *blocked_backup = "/tmp/CSBGAME3.BAK";
        CSB_V1_AtariSaveInfo written_info;
        uint8_t *written = NULL;
        uint8_t *backup = NULL;
        size_t written_size = 0u;
        size_t backup_size = 0u;
        if (!read_file(corpus_path, &bytes, &size) ||
            csb_v1_atari_save_decode_pc34_compat(bytes, size, &info) !=
                CSB_V1_ATARI_SAVE_OK ||
            !csb_v1_runtime_can_load_resume_path(corpus_path) ||
            csb_v1_runtime_load_game_from_path(&runtime, corpus_path) !=
                CSB_V1_LOAD_OK ||
            runtime.dungeon_handle == NULL ||
            runtime.current_level != info.party_map_index ||
            runtime.party_x != info.party_x || runtime.party_y != info.party_y ||
            runtime.party_dir != info.party_direction ||
            runtime.game_time != info.game_time ||
            runtime.party_state.ChampionCount != info.champion_count ||
            csb_v1_dungeon_get_current() != runtime.dungeon_handle ||
            strcmp(csb_v1_runtime_original_atari_save_source_path(&runtime),
                   corpus_path) != 0 ||
            !csb_v1_runtime_original_atari_save_source_current(&runtime) ||
            csb_v1_atari_save_handoff_runtime_pc34_compat(&runtime, bytes, size,
                                                           NULL) != 0) {
            free(bytes);
            csb_v1_runtime_cleanup(&runtime);
            return 1;
        }
        runtime.game_time += 3u;
        runtime.party_x = info.party_x;
        runtime.party_y = info.party_y;
        runtime.party_dir = info.party_direction;
        runtime.current_level = info.party_map_index;
        runtime.party_state.LeaderHandThing = (uint16_t)info.leader_hand_thing;
        runtime.party_state.Champions[0].CurrentHealth -= 1;
        snprintf(runtime.party_state.Champions[0].Name,
                 sizeof(runtime.party_state.Champions[0].Name), "SOURCE");
        remove(written_path);
        remove(backup_path);
        if (!write_file(written_path, bytes, size)) {
            free(bytes);
            csb_v1_runtime_cleanup(&runtime);
            return 1;
        }
        if (csb_v1_runtime_write_original_atari_save_to_path(
                &runtime, corpus_path, written_path) != 0 ||
            !read_file(written_path, &written, &written_size) ||
            !read_file(backup_path, &backup, &backup_size) ||
            backup_size != size || memcmp(backup, bytes, size) != 0 ||
            csb_v1_atari_save_decode_pc34_compat(written, written_size,
                                                  &written_info) !=
                CSB_V1_ATARI_SAVE_OK ||
            written_info.game_time != runtime.game_time ||
            written_info.random_seed != info.random_seed ||
            written_info.dungeon_size != info.dungeon_size ||
            memcmp(written + written_info.dungeon_offset,
                   bytes + info.dungeon_offset, info.dungeon_size) != 0) {
            free(written);
            free(backup);
            free(bytes);
            csb_v1_runtime_cleanup(&runtime);
            return 1;
        }
        {
            CSB_V1_PartyState written_party;
            if (csb_v1_atari_save_decode_party_pc34_compat(
                    written, written_size, &written_party, NULL) !=
                    CSB_V1_ATARI_SAVE_OK ||
                strcmp(written_party.Champions[0].Name, "SOURCE") != 0 ||
                written_party.Champions[0].CurrentHealth !=
                    runtime.party_state.Champions[0].CurrentHealth) {
                free(written);
                free(backup);
                free(bytes);
                csb_v1_runtime_cleanup(&runtime);
                return 1;
            }
        }
        /* CSBWin restores CSBGAMEx.BAK when the selected original slot is
         * unreadable.  Verify that the runtime admits the recovered save and
         * restores its canonical .DAT name only after Atari validation. */
        free(written);
        written = NULL;
        written_size = 0u;
        if (!write_file(backup_path, backup, backup_size) ||
            !write_file(written_path, (const uint8_t *)"bad", 3u) ||
            !csb_v1_runtime_can_load_resume_path(written_path) ||
            csb_v1_runtime_load_game_from_path(&runtime, written_path) !=
                CSB_V1_LOAD_OK ||
            strcmp(csb_v1_runtime_original_atari_save_source_path(&runtime),
                   written_path) != 0 ||
            !csb_v1_runtime_original_atari_save_source_current(&runtime) ||
            !read_file(written_path, &written, &written_size) ||
            written_size != backup_size ||
            memcmp(written, backup, backup_size) != 0) {
            free(written);
            free(backup);
            free(bytes);
            csb_v1_runtime_cleanup(&runtime);
            return 1;
        }
        /* LOADSAVE.C F0435 restores with M570_RenameFile rather than a
         * prerequisite delete.  A user can therefore have the authentic
         * CSBGAMEx.BAK while the selected .DAT is absent entirely; it must
         * remain a real resumable slot, not be rejected as a host-only
         * missing-path edge case. */
        free(written);
        written = NULL;
        written_size = 0u;
        remove(written_path);
        if (!write_file(backup_path, backup, backup_size) ||
            !csb_v1_runtime_can_load_resume_path(written_path) ||
            csb_v1_runtime_load_game_from_path(&runtime, written_path) !=
                CSB_V1_LOAD_OK ||
            strcmp(csb_v1_runtime_original_atari_save_source_path(&runtime),
                   written_path) != 0 ||
            !csb_v1_runtime_original_atari_save_source_current(&runtime) ||
            !read_file(written_path, &written, &written_size) ||
            written_size != backup_size ||
            memcmp(written, backup, backup_size) != 0) {
            free(written);
            free(backup);
            free(bytes);
            csb_v1_runtime_cleanup(&runtime);
            return 1;
        }
        /* F0435 only treats the backup as resumed after it has been renamed
         * back to the selected canonical slot.  A valid source backup must
         * not yield LOAD_OK if that replacement is impossible.  The MINI.DAT
         * bytes are authentic Atari CSB data; only the deliberately blocked
         * destination exercises the host filesystem failure boundary. */
        remove(blocked_backup);
        remove(blocked_dir);
        if (make_directory(blocked_dir) != 0 ||
            !write_file("/tmp/CSBGAME3.DAT/keep", (const uint8_t *)"x", 1u) ||
            !write_file(blocked_backup, backup, backup_size) ||
            csb_v1_runtime_load_game_from_path(&runtime, blocked_dir) ==
                CSB_V1_LOAD_OK ||
            !read_file(blocked_backup, &written, &written_size) ||
            written_size != backup_size ||
            memcmp(written, backup, backup_size) != 0) {
            free(written);
            free(backup);
            free(bytes);
            csb_v1_runtime_cleanup(&runtime);
            return 1;
        }
        free(written);
        written = NULL;
        remove("/tmp/CSBGAME3.DAT/keep");
        remove(blocked_dir);
        remove(blocked_backup);
        remove(written_path);
        remove(backup_path);
        free(written);
        free(backup);
        printf("PASS: original Atari CSB corpus loads through Resume/runtime (%d champions)\n",
               info.champion_count);
        free(bytes);
        csb_v1_runtime_cleanup(&runtime);
        return 0;
    }
    if (!path || !path[0]) { puts("SKIP: FIRESTAFF_CSB_ATARI_MINI is not set"); return 0; }
    if (!read_file(path, &bytes, &size) ||
        !csb_v1_runtime_can_load_resume_path(path) ||
        csb_v1_runtime_load_game_from_path(&runtime, path) != CSB_V1_LOAD_OK ||
        runtime.dungeon_handle == NULL || runtime.level_count != 11 || runtime.current_level != 4 ||
        runtime.party_x != 22 || runtime.party_y != 18 || runtime.party_dir != 2 ||
        runtime.game_time != 19u || runtime.party_state.PartyMapX != 22 ||
        runtime.party_state.ChampionCount != 1 ||
        strcmp(runtime.party_state.Champions[0].Name, "HALK") != 0 ||
        csb_v1_dungeon_get_current() != runtime.dungeon_handle ||
        strcmp(csb_v1_runtime_original_atari_save_source_path(&runtime), path) != 0 ||
        !csb_v1_runtime_original_atari_save_source_current(&runtime) ||
        csb_v1_atari_save_handoff_runtime_pc34_compat(&runtime, bytes, size, &info) != 0) {
        free(bytes); csb_v1_runtime_cleanup(&runtime); return 1;
    }
    free(bytes); csb_v1_runtime_cleanup(&runtime);
    puts("PASS: original MINI.DAT loads through Resume/runtime and direct handoff");
    return 0;
}
