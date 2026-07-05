/*
 * save_browser_m12.c — Save Game Browser for the Firestaff launcher.
 *
 * Scans a data directory for Firestaff save files plus known
 * original/CSBWin save basenames, reads their binary headers to extract
 * party metadata (champion names, dungeon level), and exposes a
 * navigable list with load/delete actions.
 *
 * Depends on: memory_savegame_pc34_compat.h (SaveGameHeader, F0786).
 */

#include "save_browser_m12.h"
#include "csb_v1_runtime_pc34_compat.h"
#include "csb_v1_save_load_pc34_compat.h"
#include "dm1_v1_original_save_pc34_handoff.h"
#include "dm1_v1_save_load.h"
#include "dm2_v1_new_game.h"
#include "memory_savegame_pc34_compat.h"
#include "memory_savegame_pc34_native_export_pc34_compat.h"
#include "memory_champion_state_pc34_compat.h"
#include "memory_tick_orchestrator_pc34_compat.h"
#include "nexus_v1_save.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

static const char* path_basename(const char* path) {
    const char* slash;
    const char* backslash;
    if (!path) return "";
    slash = strrchr(path, '/');
    backslash = strrchr(path, '\\');
    if (backslash && (!slash || backslash > slash)) slash = backslash;
    return slash ? slash + 1 : path;
}

static int file_exists(const char* path) {
    struct stat st;
    return path && stat(path, &st) == 0 && S_ISREG(st.st_mode);
}

static int copy_file_bytes(const char* srcPath, const char* dstPath) {
    FILE* src;
    FILE* dst;
    unsigned char buf[8192];
    size_t n;
    int ok = 0;

    if (!srcPath || !dstPath || strcmp(srcPath, dstPath) == 0) return -1;
    src = fopen(srcPath, "rb");
    if (!src) return -1;
    dst = fopen(dstPath, "wb");
    if (!dst) {
        fclose(src);
        return -1;
    }
    while ((n = fread(buf, 1, sizeof(buf), src)) > 0) {
        if (fwrite(buf, 1, n, dst) != n) {
            ok = -1;
            break;
        }
    }
    if (ferror(src)) ok = -1;
    if (fclose(dst) != 0) ok = -1;
    fclose(src);
    return ok;
}

static void build_pc34_export_basename(const char* srcName,
                                       char* out,
                                       int outSize) {
    size_t len;
    if (!out || outSize <= 0) return;
    out[0] = '\0';
    if (!srcName || !*srcName) {
        snprintf(out, (size_t)outSize, "firestaff-dm1-pc34.sav");
        return;
    }
    len = strlen(srcName);
    if (len > 4u && strcmp(srcName + len - 4u, ".sav") == 0) {
        snprintf(out, (size_t)outSize, "%.*s-pc34.sav",
                 (int)(len - 4u), srcName);
    } else {
        snprintf(out, (size_t)outSize, "%s-pc34.sav", srcName);
    }
}

/* ------------------------------------------------------------------ */
/*  Internal helpers                                                   */
/* ------------------------------------------------------------------ */

static int ascii_lower(int c) {
    return (c >= 'A' && c <= 'Z') ? c + ('a' - 'A') : c;
}

static int ascii_equal_ci(const char* a, const char* b) {
    if (!a || !b) return 0;
    while (*a && *b) {
        if (ascii_lower((unsigned char)*a) !=
            ascii_lower((unsigned char)*b)) {
            return 0;
        }
        ++a;
        ++b;
    }
    return *a == '\0' && *b == '\0';
}

static int is_csb_original_save_basename(const char* name) {
    return ascii_equal_ci(name, "CSBGAME.DAT") ||
           ascii_equal_ci(name, "CSBGAME.BAK") ||
           ascii_equal_ci(name, "DMSAVE.DAT") ||
           ascii_equal_ci(name, "DMSAVE.BAK");
}

static int nexus_save_slot_from_basename(const char* name,
                                         unsigned char* outSlot) {
    int slot;
    if (!name ||
        ascii_lower((unsigned char)name[0]) != 'n' ||
        ascii_lower((unsigned char)name[1]) != 'e' ||
        ascii_lower((unsigned char)name[2]) != 'x' ||
        ascii_lower((unsigned char)name[3]) != 'u' ||
        ascii_lower((unsigned char)name[4]) != 's' ||
        name[5] != '_' ||
        ascii_lower((unsigned char)name[6]) != 's' ||
        ascii_lower((unsigned char)name[7]) != 'a' ||
        ascii_lower((unsigned char)name[8]) != 'v' ||
        ascii_lower((unsigned char)name[9]) != 'e' ||
        name[10] != '_' ||
        name[11] < '0' || name[11] > '9' ||
        name[12] < '0' || name[12] > '9' ||
        name[13] != '.' ||
        ascii_lower((unsigned char)name[14]) != 'd' ||
        ascii_lower((unsigned char)name[15]) != 'a' ||
        ascii_lower((unsigned char)name[16]) != 't' ||
        name[17] != '\0') {
        return 0;
    }
    slot = (name[11] - '0') * 10 + (name[12] - '0');
    if (slot < 0 || slot >= NEXUS_SAVE_MAX_SLOTS) {
        return 0;
    }
    if (outSlot) {
        *outSlot = (unsigned char)slot;
    }
    return 1;
}

static int dm2_sksave_slot_from_basename(const char* name,
                                         unsigned char* outSlot,
                                         int* outLastSession) {
    int slot;
    if (outLastSession) {
        *outLastSession = 0;
    }
    if (ascii_equal_ci(name, "SKSave.dat") ||
        ascii_equal_ci(name, "SKSave.bak")) {
        if (outSlot) {
            *outSlot = 0u;
        }
        if (outLastSession) {
            *outLastSession = 1;
        }
        return 1;
    }
    if (!name ||
        ascii_lower((unsigned char)name[0]) != 's' ||
        ascii_lower((unsigned char)name[1]) != 'k' ||
        ascii_lower((unsigned char)name[2]) != 's' ||
        ascii_lower((unsigned char)name[3]) != 'a' ||
        ascii_lower((unsigned char)name[4]) != 'v' ||
        ascii_lower((unsigned char)name[5]) != 'e' ||
        name[6] < '0' || name[6] > '9' ||
        name[7] < '0' || name[7] > '9' ||
        name[8] != '.' ||
        ascii_lower((unsigned char)name[9]) != 'd' ||
        ascii_lower((unsigned char)name[10]) != 'a' ||
        ascii_lower((unsigned char)name[11]) != 't' ||
        name[12] != '\0') {
        return 0;
    }
    slot = (name[6] - '0') * 10 + (name[7] - '0');
    if (slot < 0 || slot >= DM2_SLOT_MAX) {
        return 0;
    }
    if (outSlot) {
        *outSlot = (unsigned char)slot;
    }
    return 1;
}

static int dm2_sksave_root_from_path(const char* path,
                                     char* outRoot,
                                     size_t outRootCap,
                                     unsigned char* outSlot,
                                     int* outLastSession) {
    const char* slash;
    const char* base;
    size_t len;

    if (!path || !outRoot || outRootCap == 0u) {
        return 0;
    }
    slash = strrchr(path, '/');
#ifdef _WIN32
    {
        const char* backslash = strrchr(path, '\\');
        if (!slash || (backslash && backslash > slash)) {
            slash = backslash;
        }
    }
#endif
    base = slash ? slash + 1 : path;
    if (!dm2_sksave_slot_from_basename(base, outSlot, outLastSession)) {
        return 0;
    }
    if (!slash) {
        if (outRootCap < 2u) {
            return 0;
        }
        outRoot[0] = '.';
        outRoot[1] = '\0';
        return 1;
    }
    len = (size_t)(slash - path);
    if (len == 0u || len >= outRootCap) {
        return 0;
    }
    memcpy(outRoot, path, len);
    outRoot[len] = '\0';
    return 1;
}

static int validate_csb_original_save_import_path(const char* path) {
    CSB_V1_RuntimeProfile runtime;
    int rc;

    if (!path || !*path) return 0;
    csb_v1_runtime_init(&runtime, NULL);
    rc = csb_v1_runtime_load_game_from_path(&runtime, path);
    csb_v1_runtime_cleanup(&runtime);
    return rc == CSB_V1_LOAD_OK;
}

/* Check if filename matches a launcher-visible save candidate. */
static int is_save_file(const char* name) {
    size_t len;
    if (!name) return 0;
    if (is_csb_original_save_basename(name)) return 1;
    if (nexus_save_slot_from_basename(name, NULL)) return 1;
    if (dm2_sksave_slot_from_basename(name, NULL, NULL)) return 1;
    len = strlen(name);
    if (len < 15) return 0; /* "firestaff-.sav" minimum */
    if (strncmp(name, "firestaff-", 10) != 0) return 0;
    if (strcmp(name + len - 4, ".sav") != 0) return 0;
    return 1;
}

/* Extract game ID from filename: firestaff-{id}-quicksave.sav → {id}
 * or firestaff-{id}.sav → {id}. */
static void extract_game_id(const char* filename, char* outId, int outSize) {
    const char* start;
    const char* end;
    int len;

    outId[0] = '\0';
    if (is_csb_original_save_basename(filename)) {
        snprintf(outId, (size_t)outSize, "csb");
        return;
    }
    if (nexus_save_slot_from_basename(filename, NULL)) {
        snprintf(outId, (size_t)outSize, "nexus");
        return;
    }
    if (dm2_sksave_slot_from_basename(filename, NULL, NULL)) {
        snprintf(outId, (size_t)outSize, "dm2");
        return;
    }
    if (strncmp(filename, "firestaff-", 10) != 0) return;
    start = filename + 10;

    if (strncmp(start, "dm1-", 4) == 0 ||
        strncmp(start, "dm1.sav", 7) == 0) {
        snprintf(outId, (size_t)outSize, "dm1");
        return;
    }
    if (strncmp(start, "csb-", 4) == 0 ||
        strncmp(start, "csb.sav", 7) == 0) {
        snprintf(outId, (size_t)outSize, "csb");
        return;
    }
    if (strncmp(start, "dm2-", 4) == 0 ||
        strncmp(start, "dm2.sav", 7) == 0) {
        snprintf(outId, (size_t)outSize, "dm2");
        return;
    }

    /* Try stripping -quicksave.sav first, then .sav */
    end = strstr(start, "-quicksave.sav");
    if (!end) {
        end = strstr(start, ".sav");
    }
    if (!end) return;

    len = (int)(end - start);
    if (len <= 0 || len >= outSize) return;
    memcpy(outId, start, (size_t)len);
    outId[len] = '\0';
}

static uint16_t expected_game_code_for_id(const char* gameId) {
    if (!gameId) return 0;
    if (strcmp(gameId, "dm1") == 0) return SAVEGAME_PC34_GAME_CODE_DM1;
    if (strcmp(gameId, "csb") == 0) return SAVEGAME_PC34_GAME_CODE_CSB;
    if (strcmp(gameId, "dm2") == 0) return SAVEGAME_PC34_GAME_CODE_DM2;
    if (strcmp(gameId, "nexus") == 0) return SAVEGAME_PC34_GAME_CODE_NEXUS;
    if (strcmp(gameId, "theron") == 0) return SAVEGAME_PC34_GAME_CODE_THERON;
    return 0;
}

static void classify_pc34_manifest(M12_SaveBrowserEntry* entry) {
    FILE* fp;
    unsigned char* buf;
    size_t n;
    int rc;
    uint16_t version = 0;
    uint16_t gameCode = 0;

    entry->expectedGameCode = expected_game_code_for_id(entry->gameId);
    entry->manifestGameCode = 0;
    entry->manifestStatus = SAVE_BROWSER_MANIFEST_UNKNOWN;

    if (entry->fileSize <= 0 ||
        entry->fileSize > (long)SAVEGAME_PC34_MAX_FILE_SIZE) {
        return;
    }
    fp = fopen(entry->fullPath, "rb");
    if (!fp) return;
    buf = (unsigned char*)malloc((size_t)entry->fileSize);
    if (!buf) {
        fclose(fp);
        return;
    }
    n = fread(buf, 1, (size_t)entry->fileSize, fp);
    fclose(fp);
    if (n != (size_t)entry->fileSize) {
        free(buf);
        return;
    }

    /* LSV-02 lives in the PC 3.4 DM_SAVE_HEADER AdditionalData
     * area; F0799/F0800 own the ReDMCSB header obfuscation details. */
    rc = F0799_SAVEGAME_PC34PeekManifest_Compat(
        buf, (int)n, &version, &gameCode, NULL);
    if (rc == SAVEGAME_PC34_MANIFEST_OK) {
        entry->manifestGameCode = gameCode;
        entry->manifestStatus = SAVE_BROWSER_MANIFEST_PRESENT;
        if (entry->expectedGameCode != 0) {
            rc = F0800_SAVEGAME_PC34ValidateGameCode_Compat(
                buf, (int)n, entry->expectedGameCode, 1);
            entry->manifestStatus =
                (rc == SAVEGAME_PC34_MANIFEST_OK)
                    ? SAVE_BROWSER_MANIFEST_MATCH
                    : SAVE_BROWSER_MANIFEST_WRONG_GAME;
        }
    } else if (rc == SAVEGAME_PC34_MANIFEST_ERR_NOT_PRESENT) {
        entry->manifestStatus = SAVE_BROWSER_MANIFEST_NOT_PRESENT;
    } else if (rc == SAVEGAME_PC34_MANIFEST_ERR_BAD_VERSION ||
               rc == SAVEGAME_PC34_MANIFEST_ERR_BODY_TRUNCATED) {
        entry->manifestStatus = SAVE_BROWSER_MANIFEST_UNSUPPORTED;
    }
    (void)version;
    free(buf);
}

static void format_champion_name(const unsigned char packed[8],
                                 char* out, int outSize);
static void format_csb_champion_name(const char packed[16],
                                     char* out, int outSize);

static void format_dm2_champion_name(const char packed[8],
                                     char* out, int outSize) {
    int i;
    int end;
    if (!out || outSize <= 0) return;
    end = 8;
    while (end > 0 &&
           (packed[end - 1] == ' ' || packed[end - 1] == '\0')) {
        --end;
    }
    if (end == 0 || end >= outSize) {
        out[0] = '\0';
        return;
    }
    for (i = 0; i < end; ++i) {
        out[i] = packed[i];
    }
    out[end] = '\0';
}

static int try_parse_dm2_session_entry(M12_SaveBrowserEntry* entry) {
    DM2_V1_SessionState session;
    char saveRoot[512];
    char nameBuf[32];
    unsigned char slot = 0u;
    int lastSession = 0;
    int offset;
    int i;

    if (!entry ||
        (entry->expectedGameCode != 0 &&
         entry->expectedGameCode != SAVEGAME_PC34_GAME_CODE_DM2)) {
        return 0;
    }
    if (!dm2_sksave_root_from_path(entry->fullPath,
                                   saveRoot,
                                   sizeof(saveRoot),
                                   &slot,
                                   &lastSession)) {
        return 0;
    }

    /*
     * skproject SKWINSPX/src/v4/skfileop.cpp READ_SAVEGAMES_FILENAMES
     * treats SKSAVE digit slots as the launcher-visible save surface after
     * the 0xBEEF/0xDEAD header check.  This bounded Firestaff path reads
     * the modeled DM2_V1_SessionState payload; broader original SKSAVE
     * dungeon DB pools remain owned by the later full importer.
     */
    memset(&session, 0, sizeof(session));
    if ((lastSession
            ? dm2_v1_session_load_last_session(saveRoot, &session)
            : dm2_v1_session_load_slot(saveRoot, slot, &session)) != 0 ||
        !dm2_v1_session_validate(&session)) {
        return 0;
    }

    snprintf(entry->gameId, sizeof(entry->gameId), "dm2");
    entry->expectedGameCode = SAVEGAME_PC34_GAME_CODE_DM2;
    entry->valid = 1;
    entry->mapLevel = (int)session.party_level;
    entry->championCount = (int)session.champion_count;
    entry->champions[0] = '\0';
    offset = 0;
    for (i = 0; i < session.champion_count && i < 4; ++i) {
        const DM2_ChampionRecord* rec =
            (const DM2_ChampionRecord*)session.champion_data[i];
        if (!rec || rec->max_hp == 0) {
            continue;
        }
        format_dm2_champion_name(rec->first_name,
                                 nameBuf,
                                 (int)sizeof(nameBuf));
        if (nameBuf[0] == '\0') {
            snprintf(nameBuf, sizeof(nameBuf), "CHAMPION %d", i + 1);
        }
        if (offset > 0) {
            offset += snprintf(entry->champions + offset,
                               sizeof(entry->champions) - (size_t)offset,
                               ", ");
        }
        offset += snprintf(entry->champions + offset,
                           sizeof(entry->champions) - (size_t)offset,
                           "%s", nameBuf);
    }

    if (entry->championCount > 0 && entry->champions[0] != '\0') {
        if (lastSession) {
            snprintf(entry->label, SAVE_BROWSER_LABEL_MAX,
                     "%s  L%d  [%s]  (DM2 last session)",
                     entry->gameId, entry->mapLevel, entry->champions);
        } else {
        snprintf(entry->label, SAVE_BROWSER_LABEL_MAX,
                 "%s  L%d  [%s]  (DM2 slot %u)",
                 entry->gameId, entry->mapLevel, entry->champions,
                 (unsigned)slot);
        }
    } else {
        if (lastSession) {
            snprintf(entry->label, SAVE_BROWSER_LABEL_MAX,
                     "%s  L%d  (DM2 last session)",
                     entry->gameId, entry->mapLevel);
        } else {
        snprintf(entry->label, SAVE_BROWSER_LABEL_MAX,
                 "%s  L%d  (DM2 slot %u)",
                 entry->gameId, entry->mapLevel, (unsigned)slot);
        }
    }
    return 1;
}

static int try_parse_csb_runtime_entry(M12_SaveBrowserEntry* entry) {
    CSB_V1_RuntimeProfile runtime;
    CSB_V1_PartyState party;
    char nameBuf[32];
    int rc;
    int i;
    int offset;

    if (!entry ||
        (entry->expectedGameCode != 0 &&
         entry->expectedGameCode != SAVEGAME_PC34_GAME_CODE_CSB)) {
        return 0;
    }

    csb_v1_runtime_init(&runtime, NULL);
    /*
     * ReDMCSB LOADSAVE.C F0435 and CSBWin SaveGame.cpp both restore CSB
     * through the CSB namespace, not the DM1 save loader.  Keep the M12
     * browser on the same unified runtime path as quick-resume/M11:
     * Firestaff-native saves, verified CSBWin GAMEBLOCK1/body saves, and
     * raw CSBGAME roster handoffs are classified by one loader.
     */
    rc = csb_v1_runtime_load_game_from_path(&runtime, entry->fullPath);
    if (rc != CSB_V1_LOAD_OK) {
        csb_v1_runtime_cleanup(&runtime);
        return 0;
    }

    if (entry->expectedGameCode == 0 || entry->gameId[0] == '\0') {
        snprintf(entry->gameId, sizeof(entry->gameId), "csb");
        entry->expectedGameCode = SAVEGAME_PC34_GAME_CODE_CSB;
    }
    memset(&party, 0, sizeof(party));
    (void)csb_v1_runtime_get_party_state(&runtime, &party);
    entry->valid = 1;
    entry->mapLevel = runtime.current_level;
    entry->championCount = runtime.champion_count;
    if (entry->championCount <= 0 && party.ChampionCount > 0) {
        entry->championCount = party.ChampionCount;
    }
    entry->champions[0] = '\0';
    offset = 0;
    for (i = 0; i < party.ChampionCount && i < CSB_V1_MAX_CHAMPIONS; ++i) {
        format_csb_champion_name(party.Champions[i].Name,
                                 nameBuf, (int)sizeof(nameBuf));
        if (nameBuf[0] == '\0' &&
            party.Champions[i].MaximumHealth <= 0 &&
            party.Champions[i].CurrentHealth <= 0) {
            continue;
        }
        if (nameBuf[0] == '\0') {
            snprintf(nameBuf, sizeof(nameBuf), "CHAMPION %d", i + 1);
        }
        if (offset > 0) {
            offset += snprintf(entry->champions + offset,
                               sizeof(entry->champions) - (size_t)offset,
                               ", ");
        }
        offset += snprintf(entry->champions + offset,
                           sizeof(entry->champions) - (size_t)offset,
                           "%s", nameBuf);
    }

    if (entry->championCount > 0 && entry->champions[0] != '\0') {
        snprintf(entry->label, SAVE_BROWSER_LABEL_MAX,
                 "%s  L%d  [%s]  (CSB runtime save)",
                 entry->gameId, entry->mapLevel, entry->champions);
    } else {
        snprintf(entry->label, SAVE_BROWSER_LABEL_MAX,
                 "%s  L%d  (CSB runtime save)",
                 entry->gameId, entry->mapLevel);
    }

    csb_v1_runtime_cleanup(&runtime);
    return 1;
}

static int try_parse_nexus_fnxs_entry(M12_SaveBrowserEntry* entry) {
    Nexus_V1_SaveHeader header;
    unsigned char* champion_buf = NULL;
    unsigned char* world_buf = NULL;
    size_t champion_read = 0u;
    size_t world_read = 0u;
    size_t champion_cap;
    size_t world_cap;
    char diagnostic[256];
    Nexus_SaveResult result;

    if (!entry ||
        (entry->expectedGameCode != 0 &&
         entry->expectedGameCode != SAVEGAME_PC34_GAME_CODE_NEXUS)) {
        return 0;
    }

    champion_cap = nexus_v1_save_max_champion_pool_size();
    world_cap = nexus_v1_save_max_world_size();
    champion_buf = (unsigned char*)malloc(champion_cap);
    world_buf = (unsigned char*)malloc(world_cap);
    if (!champion_buf || !world_buf) {
        free(champion_buf);
        free(world_buf);
        return 0;
    }

    memset(&header, 0, sizeof(header));
    memset(diagnostic, 0, sizeof(diagnostic));
    /*
     * Firestaff Nexus saves are the FNXS container from
     * include/nexus_v1_save.h.  Validate the complete header/data/CRC
     * envelope here so M12 only offers saves that the M11 Nexus resume
     * path can hand to nexus_v1_load_full_from_path_with_runtime().
     * Original Saturn memory-card bytes remain intentionally out of
     * scope until that format is decoded.
     */
    result = nexus_v1_load_from_path(entry->fullPath,
                                     &header,
                                     champion_buf,
                                     champion_cap,
                                     &champion_read,
                                     world_buf,
                                     world_cap,
                                     &world_read,
                                     diagnostic,
                                     sizeof(diagnostic));
    free(champion_buf);
    free(world_buf);
    if (result != NEXUS_SAVE_OK) {
        return 0;
    }

    snprintf(entry->gameId, sizeof(entry->gameId), "nexus");
    entry->expectedGameCode = SAVEGAME_PC34_GAME_CODE_NEXUS;
    entry->valid = 1;
    entry->mapLevel = (int)header.current_level;
    entry->championCount = 0;
    entry->champions[0] = '\0';
    if (header.description[0] != '\0') {
        snprintf(entry->label, SAVE_BROWSER_LABEL_MAX,
                 "%s  L%d  X%d Y%d  (%.*s)",
                 entry->gameId,
                 entry->mapLevel,
                 (int)header.party_x,
                 (int)header.party_y,
                 (int)sizeof(header.description),
                 header.description);
    } else {
        snprintf(entry->label, SAVE_BROWSER_LABEL_MAX,
                 "%s  L%d  X%d Y%d  (FNXS save)",
                 entry->gameId,
                 entry->mapLevel,
                 (int)header.party_x,
                 (int)header.party_y);
    }
    (void)champion_read;
    (void)world_read;
    return 1;
}

static int try_parse_dm1_pc34_vanilla_entry(M12_SaveBrowserEntry* entry) {
    FILE* fp;
    unsigned char* buf;
    size_t n;
    int rc;
    struct SaveGame_Compat sg;
    struct PartyState_Compat party;
    DM1OriginalSavePC34HandoffReport originalReport;
    char nameBuf[16];
    int offset;
    int i;

    if (!entry ||
        entry->manifestStatus != SAVE_BROWSER_MANIFEST_NOT_PRESENT ||
        entry->expectedGameCode != SAVEGAME_PC34_GAME_CODE_DM1 ||
        entry->fileSize <= 0 ||
        entry->fileSize > (long)SAVEGAME_PC34_MAX_FILE_SIZE) {
        return 0;
    }

    memset(&sg, 0, sizeof(sg));
    memset(&party, 0, sizeof(party));
    sg.party = &party;
    rc = dm1_v1_original_save_pc34_handoff_file(
        entry->fullPath, &sg, &originalReport);
    if (rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK) {
        entry->valid = 1;
        entry->mapLevel = sg.party ? sg.party->mapIndex : -1;
        entry->championCount = sg.party ? sg.party->championCount : 0;
        entry->champions[0] = '\0';
        offset = 0;
        for (i = 0; sg.party && i < sg.party->championCount &&
                    i < CHAMPION_MAX_PARTY; ++i) {
            if (!sg.party->champions[i].present) continue;
            format_champion_name(sg.party->champions[i].name,
                                 nameBuf, (int)sizeof(nameBuf));
            if (nameBuf[0] == '\0') continue;
            if (offset > 0) {
                offset += snprintf(entry->champions + offset,
                                   sizeof(entry->champions) - (size_t)offset,
                                   ", ");
            }
            offset += snprintf(entry->champions + offset,
                               sizeof(entry->champions) - (size_t)offset,
                               "%s", nameBuf);
        }
        if (entry->mapLevel >= 0 && entry->championCount > 0) {
            snprintf(entry->label, SAVE_BROWSER_LABEL_MAX,
                     "%s  L%d  [%s]  (original PC34 save)",
                     entry->gameId, entry->mapLevel, entry->champions);
        } else if (entry->mapLevel >= 0) {
            snprintf(entry->label, SAVE_BROWSER_LABEL_MAX,
                     "%s  L%d  (original PC34 save)",
                     entry->gameId, entry->mapLevel);
        } else {
            snprintf(entry->label, SAVE_BROWSER_LABEL_MAX,
                     "%s (original PC34 save)", entry->gameId);
        }
        return 1;
    }

    fp = fopen(entry->fullPath, "rb");
    if (!fp) return 0;
    buf = (unsigned char*)malloc((size_t)entry->fileSize);
    if (!buf) {
        fclose(fp);
        return 0;
    }
    n = fread(buf, 1, (size_t)entry->fileSize, fp);
    fclose(fp);
    if (n != (size_t)entry->fileSize) {
        free(buf);
        return 0;
    }

    rc = F0800_SAVEGAME_PC34ValidateGameCode_Compat(
        buf, (int)n, SAVEGAME_PC34_GAME_CODE_DM1,
        /* requireManifest = */ 0);
    if (rc != SAVEGAME_PC34_MANIFEST_OK) {
        free(buf);
        return 0;
    }

    memset(&sg, 0, sizeof(sg));
    memset(&party, 0, sizeof(party));
    sg.party = &party;
    rc = F0796_SAVEGAME_ImportPC34_Compat(buf, (int)n, &sg,
                                          /* strictChecksums = */ 0);
    free(buf);
    if (rc != SAVEGAME_PC34_OK) {
        return 0;
    }

    entry->valid = 1;
    entry->mapLevel = sg.party ? sg.party->mapIndex : -1;
    entry->championCount = sg.party ? sg.party->championCount : 0;
    entry->champions[0] = '\0';
    if (entry->mapLevel >= 0) {
        snprintf(entry->label, SAVE_BROWSER_LABEL_MAX,
                 "%s  L%d  (vanilla PC34 save)",
                 entry->gameId, entry->mapLevel);
    } else {
        snprintf(entry->label, SAVE_BROWSER_LABEL_MAX,
                 "%s (vanilla PC34 save)", entry->gameId);
    }
    return 1;
}

static int try_parse_dm1_native_entry(M12_SaveBrowserEntry* entry) {
    struct GameWorld_Compat world;
    struct DM1SaveHeader hdr;
    FILE* fp;
    unsigned char magic[8];
    int rc;
    int i;
    int offset;
    char nameBuf[16];

    if (!entry || entry->expectedGameCode != SAVEGAME_PC34_GAME_CODE_DM1) {
        return 0;
    }
    fp = fopen(entry->fullPath, "rb");
    if (!fp) return 0;
    if (fread(magic, 1, sizeof(magic), fp) != sizeof(magic)) {
        fclose(fp);
        return 0;
    }
    fclose(fp);
    if (memcmp(magic, DM1_SAVE_MAGIC, sizeof(magic)) != 0) {
        return 0;
    }

    memset(&world, 0, sizeof(world));
    memset(&hdr, 0, sizeof(hdr));
    rc = DM1_LoadGame(entry->fullPath, &world, &hdr);
    if (rc != DM1_SAVE_OK) {
        return 0;
    }

    entry->valid = 1;
    entry->mapLevel = world.party.mapIndex;
    entry->championCount = world.party.championCount;
    entry->champions[0] = '\0';
    offset = 0;
    for (i = 0; i < world.party.championCount &&
                i < CHAMPION_MAX_PARTY; ++i) {
        if (!world.party.champions[i].present) continue;
        format_champion_name(world.party.champions[i].name,
                             nameBuf, (int)sizeof(nameBuf));
        if (nameBuf[0] == '\0') continue;
        if (offset > 0) {
            offset += snprintf(entry->champions + offset,
                               sizeof(entry->champions) - (size_t)offset,
                               ", ");
        }
        offset += snprintf(entry->champions + offset,
                           sizeof(entry->champions) - (size_t)offset,
                           "%s", nameBuf);
    }

    if (entry->championCount > 0 && entry->mapLevel >= 0) {
        snprintf(entry->label, SAVE_BROWSER_LABEL_MAX,
                 "%s  L%d  [%s]  (Firestaff native save)",
                 entry->gameId, entry->mapLevel, entry->champions);
    } else if (entry->mapLevel >= 0) {
        snprintf(entry->label, SAVE_BROWSER_LABEL_MAX,
                 "%s  L%d  (Firestaff native save)",
                 entry->gameId, entry->mapLevel);
    } else {
        snprintf(entry->label, SAVE_BROWSER_LABEL_MAX,
                 "%s (Firestaff native save)", entry->gameId);
    }
    F0883_WORLD_Free_Compat(&world);
    (void)hdr;
    return 1;
}

/* Format a champion name from packed 8-byte field (may lack NUL). */
static void format_champion_name(const unsigned char packed[8],
                                 char* out, int outSize) {
    int i, end;
    if (outSize <= 0) return;

    /* Find last non-space, non-NUL character */
    end = 8;
    while (end > 0 && (packed[end - 1] == ' ' || packed[end - 1] == '\0'))
        end--;

    if (end == 0 || end >= outSize) {
        out[0] = '\0';
        return;
    }
    for (i = 0; i < end; i++)
        out[i] = (char)packed[i];
    out[end] = '\0';
}

static void format_csb_champion_name(const char packed[16],
                                     char* out, int outSize) {
    int i;
    int end;
    if (outSize <= 0) return;
    end = 16;
    while (end > 0 &&
           (packed[end - 1] == ' ' || packed[end - 1] == '\0')) {
        --end;
    }
    if (end == 0 || end >= outSize) {
        out[0] = '\0';
        return;
    }
    for (i = 0; i < end; ++i) {
        out[i] = packed[i];
    }
    out[end] = '\0';
}

/* Parse save file and fill entry metadata. Returns 1 on success. */
static int parse_save_entry(M12_SaveBrowserEntry* entry) {
    struct SaveGame_Compat sg;
    int rc, i;
    char nameBuf[16];
    int offset;

    classify_pc34_manifest(entry);

    memset(&sg, 0, sizeof(sg));
    rc = F0786_SAVEGAME_LoadFromFile_Compat(entry->fullPath, &sg);
    if (rc != SAVEGAME_OK) {
        if (try_parse_dm2_session_entry(entry)) {
            return 1;
        }
        if (try_parse_csb_runtime_entry(entry)) {
            return 1;
        }
        if (try_parse_nexus_fnxs_entry(entry)) {
            return 1;
        }
        if (try_parse_dm1_native_entry(entry)) {
            return 1;
        }
        if (try_parse_dm1_pc34_vanilla_entry(entry)) {
            return 1;
        }
        if (entry->manifestStatus == SAVE_BROWSER_MANIFEST_MATCH) {
            entry->valid = 1;
            entry->mapLevel = -1;
            entry->championCount = 0;
            entry->champions[0] = '\0';
            snprintf(entry->label, SAVE_BROWSER_LABEL_MAX,
                     "%s (PC34 %s save)", entry->gameId,
                     F0801_SAVEGAME_PC34GameCodeName_Compat(
                         entry->manifestGameCode));
            return 1;
        }
        if (entry->manifestStatus == SAVE_BROWSER_MANIFEST_WRONG_GAME) {
            entry->valid = 0;
            entry->mapLevel = -1;
            entry->championCount = 0;
            entry->champions[0] = '\0';
            snprintf(entry->label, SAVE_BROWSER_LABEL_MAX,
                     "%s (wrong-game save is %s)", entry->gameId,
                     F0801_SAVEGAME_PC34GameCodeName_Compat(
                         entry->manifestGameCode));
            return 0;
        }
        if (entry->manifestStatus == SAVE_BROWSER_MANIFEST_UNSUPPORTED) {
            entry->valid = 0;
            entry->mapLevel = -1;
            entry->championCount = 0;
            entry->champions[0] = '\0';
            snprintf(entry->label, SAVE_BROWSER_LABEL_MAX,
                     "%s (unsupported save manifest)", entry->gameId);
            return 0;
        }
        entry->valid = 0;
        entry->mapLevel = -1;
        entry->championCount = 0;
        entry->champions[0] = '\0';
        snprintf(entry->label, SAVE_BROWSER_LABEL_MAX,
                 "%s (corrupt/unreadable)", entry->gameId);
        return 0;
    }

    entry->valid = 1;

    /* Extract party info */
    if (sg.party) {
        entry->mapLevel = sg.party->mapIndex;
        entry->championCount = sg.party->championCount;

        /* Build champion name list */
        offset = 0;
        entry->champions[0] = '\0';
        for (i = 0; i < sg.party->championCount && i < CHAMPION_MAX_PARTY; i++) {
            if (!sg.party->champions[i].present) continue;
            format_champion_name(sg.party->champions[i].name,
                                 nameBuf, (int)sizeof(nameBuf));
            if (nameBuf[0] == '\0') continue;
            if (offset > 0) {
                offset += snprintf(entry->champions + offset,
                                   sizeof(entry->champions) - (size_t)offset,
                                   ", ");
            }
            offset += snprintf(entry->champions + offset,
                               sizeof(entry->champions) - (size_t)offset,
                               "%s", nameBuf);
        }
    } else {
        entry->mapLevel = -1;
        entry->championCount = 0;
        entry->champions[0] = '\0';
    }

    /* Build display label */
    if (entry->championCount > 0 && entry->mapLevel >= 0) {
        snprintf(entry->label, SAVE_BROWSER_LABEL_MAX,
                 "%s  L%d  [%s]", entry->gameId,
                 entry->mapLevel, entry->champions);
    } else if (entry->championCount > 0) {
        snprintf(entry->label, SAVE_BROWSER_LABEL_MAX,
                 "%s  [%s]", entry->gameId, entry->champions);
    } else {
        snprintf(entry->label, SAVE_BROWSER_LABEL_MAX,
                 "%s", entry->gameId);
    }

    /* Free allocated subsystem pointers (F0786 allocates them) */
    free(sg.party);
    free(sg.lastMovement);
    free(sg.pendingSensorEffects);
    free(sg.timeline);
    free(sg.combatScratch);
    free(sg.magic);
    free(sg.mutations);

    return 1;
}

/* Compare entries by modification time (newest first). */
static int compare_entries(const void* a, const void* b) {
    const M12_SaveBrowserEntry* ea = (const M12_SaveBrowserEntry*)a;
    const M12_SaveBrowserEntry* eb = (const M12_SaveBrowserEntry*)b;
    if (eb->fileModTime > ea->fileModTime) return 1;
    if (eb->fileModTime < ea->fileModTime) return -1;
    return strcmp(ea->filename, eb->filename);
}

static int save_browser_has_path(const M12_SaveBrowserState* state,
                                 const char* fullPath) {
    int i;
    if (!state || !fullPath) return 0;
    for (i = 0; i < state->entryCount; ++i) {
        if (strcmp(state->entries[i].fullPath, fullPath) == 0) {
            return 1;
        }
    }
    return 0;
}

int save_browser_scan_dir(M12_SaveBrowserState* state,
                                 const char* dirPath) {
    DIR* dir;
    struct dirent* ent;
    struct stat st;
    M12_SaveBrowserEntry* entry;
    char fullPath[512];
    int added = 0;
    int n;

    if (!state || !dirPath || dirPath[0] == '\0') return 0;
    dir = opendir(dirPath);
    if (!dir) return 0;

    while ((ent = readdir(dir)) != NULL) {
        if (state->entryCount >= SAVE_BROWSER_MAX_ENTRIES) break;
        if (!is_save_file(ent->d_name)) continue;

        n = snprintf(fullPath, sizeof(fullPath), "%s/%s",
                     dirPath, ent->d_name);
        if (n <= 0 || n >= (int)sizeof(fullPath)) continue;
        if ((size_t)n >= SAVE_BROWSER_FILENAME_MAX) continue;
        if (save_browser_has_path(state, fullPath)) continue;

        entry = &state->entries[state->entryCount];
        snprintf(entry->filename, SAVE_BROWSER_FILENAME_MAX,
                 "%s", ent->d_name);
        snprintf(entry->fullPath, SAVE_BROWSER_FILENAME_MAX,
                 "%s", fullPath);

        extract_game_id(ent->d_name, entry->gameId,
                        (int)sizeof(entry->gameId));

        if (stat(fullPath, &st) == 0) {
            entry->fileModTime = st.st_mtime;
            entry->fileSize = (long)st.st_size;
        } else {
            entry->fileModTime = 0;
            entry->fileSize = 0;
        }

        parse_save_entry(entry);
        ++state->entryCount;
        ++added;
    }
    closedir(dir);
    return added;
}

/* ------------------------------------------------------------------ */
/*  Public API                                                         */
/* ------------------------------------------------------------------ */

int M12_SaveBrowser_Scan(M12_SaveBrowserState* state, const char* dataDir) {
    static const char* const games[] = {
        "dm1", "csb", "dm2", "nexus", "theron"
    };
    char saveDir[512];
    int i;
    int n;

    if (!state || !dataDir) return 0;

    memset(state, 0, sizeof(*state));
    state->selectedIndex = 0;
    state->scrollOffset = 0;
    state->confirmDelete = 0;

    (void)save_browser_scan_dir(state, dataDir);

    /*
     * Runtime saves are not game data.  The launcher still receives the
     * data root, so bridge the common Firestaff layout where saves live
     * beside the data tree: ~/.firestaff/data/../saves/{game}/.  Also
     * accept an embedded data-root saves/{game}/ directory for imported
     * portable bundles.
     */
    for (i = 0; i < (int)(sizeof(games) / sizeof(games[0])); ++i) {
        if (state->entryCount >= SAVE_BROWSER_MAX_ENTRIES) break;
        n = snprintf(saveDir, sizeof(saveDir), "%s/../saves/%s",
                     dataDir, games[i]);
        if (n > 0 && n < (int)sizeof(saveDir)) {
            (void)save_browser_scan_dir(state, saveDir);
        }
        if (state->entryCount >= SAVE_BROWSER_MAX_ENTRIES) break;
        n = snprintf(saveDir, sizeof(saveDir), "%s/saves/%s",
                     dataDir, games[i]);
        if (n > 0 && n < (int)sizeof(saveDir)) {
            (void)save_browser_scan_dir(state, saveDir);
        }
    }

    /* Sort by modification time (newest first) */
    if (state->entryCount > 1) {
        qsort(state->entries, (size_t)state->entryCount,
              sizeof(M12_SaveBrowserEntry), compare_entries);
    }

    return state->entryCount;
}

int M12_SaveBrowser_HandleInput(M12_SaveBrowserState* state, int input) {
    if (!state || state->entryCount == 0) return 0;

    /* Cancel delete confirmation on any non-accept input */
    if (state->confirmDelete && input != 5 /* ACCEPT */) {
        state->confirmDelete = 0;
        return 0;
    }

    switch (input) {
    case 1: /* UP */
        if (state->selectedIndex > 0) {
            state->selectedIndex--;
            if (state->selectedIndex < state->scrollOffset)
                state->scrollOffset = state->selectedIndex;
        }
        break;

    case 2: /* DOWN */
        if (state->selectedIndex < state->entryCount - 1) {
            state->selectedIndex++;
            /* Scroll if needed (assume ~8 visible rows) */
            if (state->selectedIndex >= state->scrollOffset + 8)
                state->scrollOffset = state->selectedIndex - 7;
        }
        break;

    case 5: /* ACCEPT — load selected save */
        if (state->confirmDelete) {
            /* Confirm delete */
            M12_SaveBrowser_DeleteSelected(state);
            state->confirmDelete = 0;
            return 0;
        }
        return state->entries[state->selectedIndex].valid ? 1 : 0;

    case 7: /* ACTION — initiate delete */
        state->confirmDelete = 1;
        break;

    default:
        break;
    }

    return 0;
}

int M12_SaveBrowser_DeleteSelected(M12_SaveBrowserState* state) {
    int idx, i;

    if (!state || state->entryCount == 0) return -1;
    idx = state->selectedIndex;
    if (idx < 0 || idx >= state->entryCount) return -1;

    /* Delete the file */
    if (remove(state->entries[idx].fullPath) != 0) {
        return -1;
    }

    /* Shift remaining entries down */
    for (i = idx; i < state->entryCount - 1; i++) {
        state->entries[i] = state->entries[i + 1];
    }
    state->entryCount--;

    /* Adjust selection */
    if (state->selectedIndex >= state->entryCount && state->entryCount > 0) {
        state->selectedIndex = state->entryCount - 1;
    }
    if (state->entryCount == 0) {
        state->selectedIndex = 0;
    }

    return 0;
}

int M12_SaveBrowser_ExportSelected(const M12_SaveBrowserState* state,
                                   const char* exportDir,
                                   char* outPath,
                                   int outPathSize) {
    const M12_SaveBrowserEntry* entry;
    char dst[512];
    if (outPath && outPathSize > 0) outPath[0] = '\0';
    entry = M12_SaveBrowser_GetSelected(state);
    if (!entry || !exportDir || !*exportDir || !file_exists(entry->fullPath)) {
        return -1;
    }
    snprintf(dst, sizeof(dst), "%s/%s", exportDir, entry->filename);
    if (copy_file_bytes(entry->fullPath, dst) != 0) {
        return -1;
    }
    if (outPath && outPathSize > 0) {
        snprintf(outPath, (size_t)outPathSize, "%s", dst);
    }
    return 0;
}

int M12_SaveBrowser_ExportSelectedAsDM1PC34(
    const M12_SaveBrowserState* state,
    const char* exportDir,
    char* outPath,
    int outPathSize) {
    const M12_SaveBrowserEntry* entry;
    struct GameWorld_Compat world;
    struct DM1SaveHeader hdr;
    char base[SAVE_BROWSER_FILENAME_MAX];
    char dst[512];
    uint32_t gameID;
    int rc;

    if (outPath && outPathSize > 0) outPath[0] = '\0';
    entry = M12_SaveBrowser_GetSelected(state);
    if (!entry || !exportDir || !*exportDir || !file_exists(entry->fullPath)) {
        return -1;
    }
    if (strcmp(entry->gameId, "dm1") != 0 || !entry->valid) {
        return -1;
    }

    memset(&world, 0, sizeof(world));
    memset(&hdr, 0, sizeof(hdr));
    rc = DM1_LoadGame(entry->fullPath, &world, &hdr);
    if (rc != DM1_SAVE_OK) {
        return -1;
    }

    build_pc34_export_basename(entry->filename, base, (int)sizeof(base));
    snprintf(dst, sizeof(dst), "%s/%s", exportDir, base);
    if (file_exists(dst)) {
        F0883_WORLD_Free_Compat(&world);
        return -1;
    }

    /*
     * ReDMCSB LOADSAVE.C F0433 writes the original GLOBAL_DATA,
     * ACTIVE_GROUP, PARTY, EVENTS, and TIMELINE save parts; F0435 reads
     * the same parts back. DM1_SaveGamePC34 owns that PC 3.4-shaped
     * write-back from Firestaff's bounded GameWorld_Compat state.
     */
    gameID = hdr.gameID != 0u ? hdr.gameID : 0x44534D31u;
    rc = DM1_SaveGamePC34(&world, dst, gameID);
    F0883_WORLD_Free_Compat(&world);
    if (rc != DM1_SAVE_OK) {
        remove(dst);
        return -1;
    }

    if (outPath && outPathSize > 0) {
        snprintf(outPath, (size_t)outPathSize, "%s", dst);
    }
    return 0;
}

int M12_SaveBrowser_ImportFile(const char* dataDir,
                               const char* importPath,
                               char* outPath,
                               int outPathSize) {
    const char* base;
    char dst[512];
    if (outPath && outPathSize > 0) outPath[0] = '\0';
    if (!dataDir || !*dataDir || !importPath || !file_exists(importPath)) {
        return -1;
    }
    base = path_basename(importPath);
    if (!is_save_file(base)) {
        return -1;
    }
    if (is_csb_original_save_basename(base) &&
        !validate_csb_original_save_import_path(importPath)) {
        return -1;
    }
    snprintf(dst, sizeof(dst), "%s/%s", dataDir, base);
    if (file_exists(dst)) {
        return -1;
    }
    if (copy_file_bytes(importPath, dst) != 0) {
        return -1;
    }
    if (outPath && outPathSize > 0) {
        snprintf(outPath, (size_t)outPathSize, "%s", dst);
    }
    return 0;
}

const M12_SaveBrowserEntry* M12_SaveBrowser_GetSelected(
    const M12_SaveBrowserState* state) {
    if (!state || state->entryCount == 0) return NULL;
    if (state->selectedIndex < 0 ||
        state->selectedIndex >= state->entryCount) return NULL;
    return &state->entries[state->selectedIndex];
}

void M12_SaveBrowser_Draw(const M12_SaveBrowserState* state,
                          unsigned char* fb, int fbWidth, int fbHeight) {
    int i, visible, yPos;
    const M12_SaveBrowserEntry* e;
    char timeBuf[32];
    struct tm* tm;

    (void)fb;
    (void)fbWidth;
    (void)fbHeight;

    if (!state) return;

    /* This is a text-mode rendering stub. The modern renderer
     * (menu_startup_render_modern_m12.c) will implement the actual
     * visual layout. For now, we just validate the data is accessible. */

    visible = 8; /* max visible rows */
    yPos = 0;

    for (i = state->scrollOffset;
         i < state->entryCount && yPos < visible; i++, yPos++) {
        e = &state->entries[i];
        tm = localtime(&e->fileModTime);
        if (tm) {
            strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M", tm);
        } else {
            snprintf(timeBuf, sizeof(timeBuf), "unknown");
        }

        /* In a real renderer, we'd draw:
         *   [>] label    date    size
         * with highlight on selectedIndex.
         * For now this function exists to satisfy the API contract. */
        (void)e;
        (void)timeBuf;
    }
}
