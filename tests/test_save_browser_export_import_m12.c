#include "save_browser_m12.h"
#include "memory_savegame_pc34_native_export_pc34_compat.h"
#include "memory_champion_state_pc34_compat.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#ifdef _WIN32
#include <direct.h>
#endif

static int failures = 0;

static void check(int ok, const char* name) {
    if (!ok) {
        ++failures;
        printf("FAIL %s\n", name);
    } else {
        printf("PASS %s\n", name);
    }
}

static int mkdir_one(const char* path) {
#ifdef _WIN32
    if (_mkdir(path) == 0 || errno == EEXIST) return 1;
#else
    if (mkdir(path, 0755) == 0 || errno == EEXIST) return 1;
#endif
    return 0;
}

static int write_bytes(const char* path, const char* bytes) {
    FILE* fp = fopen(path, "wb");
    size_t len = strlen(bytes);
    if (!fp) return 0;
    if (fwrite(bytes, 1, len, fp) != len) {
        fclose(fp);
        return 0;
    }
    fclose(fp);
    return 1;
}

static int write_blob(const char* path, const unsigned char* bytes, int len) {
    FILE* fp;
    if (!path || !bytes || len <= 0) return 0;
    fp = fopen(path, "wb");
    if (!fp) return 0;
    if (fwrite(bytes, 1, (size_t)len, fp) != (size_t)len) {
        fclose(fp);
        return 0;
    }
    fclose(fp);
    return 1;
}

static int write_pc34_native_dm1_save(const char* path) {
    struct SaveGame_Compat state;
    struct PartyState_Compat party;
    unsigned char buf[SAVEGAME_PC34_MAX_FILE_SIZE];
    int written = 0;
    int rc;

    memset(&state, 0, sizeof(state));
    memset(&party, 0, sizeof(party));
    party.championCount = 1;
    party.mapIndex = 4;
    party.mapX = 7;
    party.mapY = 9;
    party.direction = 2;
    party.activeChampionIndex = 0;
    party.champions[0].present = 1;
    memcpy(party.champions[0].name, "TEST    ", 8);
    state.party = &party;

    rc = F0795_SAVEGAME_ExportPC34_Compat(
        &state, 0x44534D31u, buf, (int)sizeof(buf), &written);
    return rc == SAVEGAME_PC34_OK && write_blob(path, buf, written);
}

static int strip_pc34_manifest(unsigned char* buf, int len) {
    struct PC34SaveHeaderCopy {
        unsigned char noise[SAVEGAME_PC34_DM_SAVE_HEADER_SIZE / 2];
        unsigned char meta[SAVEGAME_PC34_DM_SAVE_HEADER_SIZE / 2];
    } hdr;
    unsigned char* metaHalf;
    uint16_t key;

    if (!buf || len < SAVEGAME_PC34_DM_SAVE_HEADER_SIZE) return 0;
    memcpy(&hdr, buf, sizeof(hdr));
    key = (uint16_t)((unsigned)hdr.noise
                       [SAVEGAME_PC34_DM_HEADER_DECRYPTION_KEY_INDEX * 2]
                     | ((unsigned)hdr.noise
                       [SAVEGAME_PC34_DM_HEADER_DECRYPTION_KEY_INDEX * 2 + 1]
                        << 8));
    metaHalf = hdr.meta;
    (void)F0798_SAVEGAME_PC34CPSCObfuscate_Compat(
        (uint16_t*)metaHalf,
        SAVEGAME_PC34_DM_SAVE_HEADER_HALF_WORDS, key);
    memset(metaHalf + SAVEGAME_PC34_MANIFEST_OFFSET * 2, 0,
           SAVEGAME_PC34_MANIFEST_SIZE);
    (void)F0798_SAVEGAME_PC34CPSCObfuscate_Compat(
        (uint16_t*)metaHalf,
        SAVEGAME_PC34_DM_SAVE_HEADER_HALF_WORDS, key);
    memcpy(buf, &hdr, sizeof(hdr));
    return F0799_SAVEGAME_PC34PeekManifest_Compat(
               buf, len, NULL, NULL, NULL) ==
           SAVEGAME_PC34_MANIFEST_ERR_NOT_PRESENT;
}

static int write_pc34_vanilla_dm1_save(const char* path) {
    struct SaveGame_Compat state;
    struct PartyState_Compat party;
    unsigned char buf[SAVEGAME_PC34_MAX_FILE_SIZE];
    int written = 0;
    int rc;

    memset(&state, 0, sizeof(state));
    memset(&party, 0, sizeof(party));
    party.championCount = 1;
    party.mapIndex = 6;
    party.mapX = 11;
    party.mapY = 13;
    party.direction = 1;
    party.activeChampionIndex = 0;
    party.champions[0].present = 1;
    memcpy(party.champions[0].name, "LEGACY  ", 8);
    state.party = &party;

    rc = F0795_SAVEGAME_ExportPC34_Compat(
        &state, 0x56414E31u, buf, (int)sizeof(buf), &written);
    return rc == SAVEGAME_PC34_OK &&
           strip_pc34_manifest(buf, written) &&
           write_blob(path, buf, written);
}

static int read_bytes(const char* path, char* out, size_t outBytes) {
    FILE* fp = fopen(path, "rb");
    size_t n;
    if (!fp || outBytes == 0) return 0;
    n = fread(out, 1, outBytes - 1u, fp);
    out[n] = '\0';
    fclose(fp);
    return 1;
}

static void cleanup(const char* root) {
    char path[512];
    snprintf(path, sizeof(path), "%s/data/firestaff-dm1-slot.sav", root);
    unlink(path);
    snprintf(path, sizeof(path), "%s/data/firestaff-dm1.sav", root);
    unlink(path);
    snprintf(path, sizeof(path), "%s/data/firestaff-dm1-quicksave.sav", root);
    unlink(path);
    snprintf(path, sizeof(path), "%s/data/firestaff-csb.sav", root);
    unlink(path);
    snprintf(path, sizeof(path), "%s/backup/firestaff-dm1-slot.sav", root);
    unlink(path);
    snprintf(path, sizeof(path), "%s/backup/not-a-save.dat", root);
    unlink(path);
    snprintf(path, sizeof(path), "%s/data", root);
    rmdir(path);
    snprintf(path, sizeof(path), "%s/backup", root);
    rmdir(path);
    rmdir(root);
}

static const M12_SaveBrowserEntry* find_entry(
    const M12_SaveBrowserState* state, const char* filename) {
    int i;
    for (i = 0; i < state->entryCount; ++i) {
        if (strcmp(state->entries[i].filename, filename) == 0) {
            return &state->entries[i];
        }
    }
    return NULL;
}

int main(void) {
    char root[256];
    char dataDir[512];
    char backupDir[512];
    char savePath[512];
    char badPath[512];
    char outPath[512];
    char bytes[64];
    M12_SaveBrowserState state;

    snprintf(root, sizeof(root), "/tmp/firestaff_save_browser_export_import_%ld", (long)getpid());
    cleanup(root);
    snprintf(dataDir, sizeof(dataDir), "%s/data", root);
    snprintf(backupDir, sizeof(backupDir), "%s/backup", root);
    check(mkdir_one(root), "created root");
    check(mkdir_one(dataDir), "created data dir");
    check(mkdir_one(backupDir), "created backup dir");

    snprintf(savePath, sizeof(savePath), "%s/firestaff-dm1-slot.sav", dataDir);
    check(write_bytes(savePath, "SAVE-BYTES-01"), "wrote source save bytes");
    check(M12_SaveBrowser_Scan(&state, dataDir) == 1, "scan finds source save");
    check(strcmp(state.entries[0].filename, "firestaff-dm1-slot.sav") == 0,
          "scan records save basename");

    check(M12_SaveBrowser_ExportSelected(&state, backupDir, outPath, (int)sizeof(outPath)) == 0,
          "export selected save succeeds");
    check(strstr(outPath, "/backup/firestaff-dm1-slot.sav") != NULL,
          "export reports backup path");
    check(read_bytes(outPath, bytes, sizeof(bytes)) && strcmp(bytes, "SAVE-BYTES-01") == 0,
          "export preserves file bytes");

    check(unlink(savePath) == 0, "removed original before import");
    check(M12_SaveBrowser_ImportFile(dataDir, outPath, savePath, (int)sizeof(savePath)) == 0,
          "import copied save back to data dir");
    check(read_bytes(savePath, bytes, sizeof(bytes)) && strcmp(bytes, "SAVE-BYTES-01") == 0,
          "import preserves file bytes");
    check(M12_SaveBrowser_ImportFile(dataDir, outPath, NULL, 0) == -1,
          "duplicate import preserves existing destination");

    snprintf(badPath, sizeof(badPath), "%s/not-a-save.dat", backupDir);
    check(write_bytes(badPath, "NOPE"), "wrote bad import fixture");
    check(M12_SaveBrowser_ImportFile(dataDir, badPath, outPath, (int)sizeof(outPath)) == -1,
          "non firestaff save filename rejected");
    check(M12_SaveBrowser_ExportSelected(NULL, backupDir, outPath, (int)sizeof(outPath)) == -1,
          "NULL export state rejected");

    check(unlink(savePath) == 0, "removed restored save before manifest scan");
    snprintf(savePath, sizeof(savePath), "%s/firestaff-dm1.sav", dataDir);
    check(write_pc34_native_dm1_save(savePath), "wrote DM1 PC34 native manifest fixture");
    snprintf(savePath, sizeof(savePath), "%s/firestaff-dm1-quicksave.sav", dataDir);
    check(write_pc34_vanilla_dm1_save(savePath), "wrote DM1 PC34 vanilla fixture");
    snprintf(savePath, sizeof(savePath), "%s/firestaff-csb.sav", dataDir);
    check(write_pc34_native_dm1_save(savePath), "wrote wrong-game PC34 native manifest fixture");
    check(M12_SaveBrowser_Scan(&state, dataDir) == 3,
          "scan finds native manifest fixtures");
    {
        const M12_SaveBrowserEntry* dm1 =
            find_entry(&state, "firestaff-dm1.sav");
        const M12_SaveBrowserEntry* vanilla =
            find_entry(&state, "firestaff-dm1-quicksave.sav");
        const M12_SaveBrowserEntry* csb =
            find_entry(&state, "firestaff-csb.sav");
        check(dm1 != NULL, "DM1 native manifest entry present");
        check(vanilla != NULL, "DM1 vanilla PC34 entry present");
        check(csb != NULL, "CSB wrong-game manifest entry present");
        if (dm1) {
            check(dm1->expectedGameCode == SAVEGAME_PC34_GAME_CODE_DM1,
                  "DM1 filename maps to DM1 game code");
            check(dm1->manifestGameCode == SAVEGAME_PC34_GAME_CODE_DM1,
                  "DM1 native manifest reports DM1 game code");
            check(dm1->manifestStatus == SAVE_BROWSER_MANIFEST_MATCH,
                  "DM1 native manifest matches selected game");
            check(dm1->valid == 1, "DM1 native manifest is load-browser valid");
            check(strstr(dm1->label, "PC34 DM1 save") != NULL,
                  "DM1 native manifest label is specific");
            state.selectedIndex = (int)(dm1 - state.entries);
            check(M12_SaveBrowser_HandleInput(&state, 5) == 1,
                  "DM1 native manifest can request load handoff");
        }
        if (vanilla) {
            check(vanilla->expectedGameCode == SAVEGAME_PC34_GAME_CODE_DM1,
                  "DM1 vanilla filename maps to DM1 game code");
            check(vanilla->manifestGameCode == 0,
                  "DM1 vanilla PC34 save has no manifest game code");
            check(vanilla->manifestStatus == SAVE_BROWSER_MANIFEST_NOT_PRESENT,
                  "DM1 vanilla PC34 save reports manifest-not-present");
            check(vanilla->valid == 1,
                  "DM1 vanilla PC34 save remains load-browser valid");
            check(vanilla->mapLevel == 6,
                  "DM1 vanilla PC34 save imports map level via native importer");
            check(strstr(vanilla->label, "vanilla PC34 save") != NULL,
                  "DM1 vanilla PC34 label names legacy interop path");
            state.selectedIndex = (int)(vanilla - state.entries);
            check(M12_SaveBrowser_HandleInput(&state, 5) == 1,
                  "DM1 vanilla PC34 save can request load handoff");
        }
        if (csb) {
            check(csb->expectedGameCode == SAVEGAME_PC34_GAME_CODE_CSB,
                  "CSB filename maps to CSB game code");
            check(csb->manifestGameCode == SAVEGAME_PC34_GAME_CODE_DM1,
                  "CSB wrong-game fixture still reports DM1 manifest");
            check(csb->manifestStatus == SAVE_BROWSER_MANIFEST_WRONG_GAME,
                  "CSB wrong-game manifest is rejected before launch");
            check(csb->valid == 0, "CSB wrong-game manifest is not valid");
            check(strstr(csb->label, "wrong-game save is DM1") != NULL,
                  "CSB wrong-game manifest label names actual game");
            state.selectedIndex = (int)(csb - state.entries);
            check(M12_SaveBrowser_HandleInput(&state, 5) == 0,
                  "CSB wrong-game manifest cannot request load handoff");
        }
    }

    cleanup(root);
    if (failures) {
        printf("test_save_browser_export_import_m12: FAIL %d\n", failures);
        return 1;
    }
    puts("test_save_browser_export_import_m12: PASS");
    return 0;
}
