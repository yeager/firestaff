#include "save_byte_manifest_m12.h"
#include "dm1_v1_save_load.h"
#include "memory_tick_orchestrator_pc34_compat.h"

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

static void cleanup(const char* root) {
    char path[512];
    snprintf(path, sizeof(path), "%s/data/firestaff-dm1-slot.sav", root);
    unlink(path);
    snprintf(path, sizeof(path), "%s/data/firestaff-dm1-slot.sav.bak", root);
    unlink(path);
    snprintf(path, sizeof(path), "%s/export/firestaff-dm1-slot.sav", root);
    unlink(path);
    snprintf(path, sizeof(path), "%s/export/firestaff-dm1-slot.sav.manifest.json", root);
    unlink(path);
    snprintf(path, sizeof(path), "%s/import/firestaff-dm1-slot.sav", root);
    unlink(path);
    snprintf(path, sizeof(path), "%s/data", root);
    rmdir(path);
    snprintf(path, sizeof(path), "%s/export", root);
    rmdir(path);
    snprintf(path, sizeof(path), "%s/import", root);
    rmdir(path);
    rmdir(root);
}

static long file_size(const char* path) {
    struct stat st;
    if (stat(path, &st) != 0) return -1;
    return (long)st.st_size;
}

static int files_equal(const char* a, const char* b) {
    FILE* fa = fopen(a, "rb");
    FILE* fb = fopen(b, "rb");
    int ok = 1;
    if (!fa || !fb) {
        if (fa) fclose(fa);
        if (fb) fclose(fb);
        return 0;
    }
    for (;;) {
        unsigned char ba[4096];
        unsigned char bb[4096];
        size_t na = fread(ba, 1, sizeof(ba), fa);
        size_t nb = fread(bb, 1, sizeof(bb), fb);
        if (na != nb || memcmp(ba, bb, na) != 0) {
            ok = 0;
            break;
        }
        if (na < sizeof(ba)) {
            if (ferror(fa) || ferror(fb)) ok = 0;
            break;
        }
    }
    fclose(fa);
    fclose(fb);
    return ok;
}

static int flip_payload_byte(const char* path) {
    FILE* fp = fopen(path, "r+b");
    int c;
    if (!fp) return 0;
    if (fseek(fp, 80, SEEK_SET) != 0) {
        fclose(fp);
        return 0;
    }
    c = fgetc(fp);
    if (c == EOF) {
        fclose(fp);
        return 0;
    }
    if (fseek(fp, 80, SEEK_SET) != 0) {
        fclose(fp);
        return 0;
    }
    fputc(c ^ 0x40, fp);
    fclose(fp);
    return 1;
}

static void seed_dm1_world(struct GameWorld_Compat* world) {
    int i;
    F0881_WORLD_InitDefault_Compat(world, 0xA5A50123u);
    world->gameTick = 4321u;
    world->partyMapIndex = 1;
    world->party.mapIndex = 1;
    world->party.mapX = 6;
    world->party.mapY = 7;
    world->party.direction = DIR_EAST;
    world->party.championCount = 1;
    world->party.activeChampionIndex = 0;
    for (i = 0; i < CHAMPION_MAX_PARTY; ++i) {
        F0600_CHAMPION_InitEmpty_Compat(&world->party.champions[i]);
    }
    world->party.champions[0].present = 1;
    world->party.champions[0].portraitIndex = 3;
    memcpy(world->party.champions[0].name, "TIGGY   ", CHAMPION_NAME_LENGTH);
    world->party.champions[0].hp.current = 41;
    world->party.champions[0].hp.maximum = 60;
    world->party.champions[0].stamina.current = 66;
    world->party.champions[0].stamina.maximum = 80;
    world->party.champions[0].mana.current = 22;
    world->party.champions[0].mana.maximum = 45;
}

int main(void) {
    char root[256];
    char dataDir[512];
    char exportDir[512];
    char importDir[512];
    char savePath[512];
    char manifestPath[512];
    char payloadPath[512];
    char importedPath[512];
    struct GameWorld_Compat world;
    struct GameWorld_Compat loaded;
    struct DM1SaveHeader hdr;
    M12_SaveByteManifest manifest;
    int rc;

    snprintf(root, sizeof(root), "/tmp/firestaff_save_byte_manifest_%ld",
             (long)getpid());
    cleanup(root);
    snprintf(dataDir, sizeof(dataDir), "%s/data", root);
    snprintf(exportDir, sizeof(exportDir), "%s/export", root);
    snprintf(importDir, sizeof(importDir), "%s/import", root);
    check(mkdir_one(root), "created root");
    check(mkdir_one(dataDir), "created data dir");
    check(mkdir_one(exportDir), "created export dir");
    check(mkdir_one(importDir), "created import dir");

    memset(&world, 0, sizeof(world));
    seed_dm1_world(&world);
    snprintf(savePath, sizeof(savePath), "%s/firestaff-dm1-slot.sav", dataDir);
    rc = DM1_SaveGameWithProfile(&world, savePath, 0xD1000001u, 1, 1,
                                 DM1_DefaultSaveProfileHash());
    check(rc == DM1_SAVE_OK, "created native DM1 save bytes");

    check(M12_SaveByteManifest_ExportGameSave("dm1", savePath, exportDir,
                                              manifestPath, (int)sizeof(manifestPath),
                                              payloadPath, (int)sizeof(payloadPath)) == 0,
          "exported DM1 save bytes plus manifest");
    check(strstr(manifestPath, "firestaff-dm1-slot.sav.manifest.json") != NULL,
          "manifest path uses save basename");
    check(strstr(payloadPath, "firestaff-dm1-slot.sav") != NULL,
          "payload path uses save basename");
    check(files_equal(savePath, payloadPath), "export payload preserves bytes");

    check(M12_SaveByteManifest_Read(manifestPath, &manifest) == 0,
          "manifest parses");
    check(manifest.manifestVersion == M12_SAVE_BYTE_MANIFEST_VERSION,
          "manifest version is current");
    check(manifest.runtimeSaveBytesIncluded == 1,
          "manifest records runtime save bytes included");
    check(strcmp(manifest.gameId, "dm1") == 0,
          "manifest game_id is dm1");
    check(strcmp(manifest.formatId, "FSDM1SV1") == 0,
          "manifest format_id is DM1 native save magic");
    check(strcmp(manifest.sourceFilename, "firestaff-dm1-slot.sav") == 0,
          "manifest source filename is basename only");
    check((long)manifest.byteCount == file_size(payloadPath),
          "manifest byte_count matches payload size");
    check(M12_SaveByteManifest_VerifyPayload(manifestPath, &manifest) == 0,
          "manifest verifies payload before import");

    check(M12_SaveByteManifest_ImportGameSave(importDir, manifestPath,
                                              importedPath,
                                              (int)sizeof(importedPath)) == 0,
          "manifest import copies verified payload");
    check(files_equal(savePath, importedPath), "import preserves DM1 save bytes");
    memset(&loaded, 0, sizeof(loaded));
    memset(&hdr, 0, sizeof(hdr));
    rc = DM1_LoadGame(importedPath, &loaded, &hdr);
    check(rc == DM1_SAVE_OK, "imported save loads through DM1 loader");
    check(hdr.gameID == 0xD1000001u, "imported header game id survives");
    check(hdr.partyMapX == 6 && hdr.partyMapY == 7,
          "imported party position survives");
    check(loaded.party.championCount == 1 &&
          memcmp(loaded.party.champions[0].name, "TIGGY   ",
                 CHAMPION_NAME_LENGTH) == 0,
          "imported champion identity survives");
    F0883_WORLD_Free_Compat(&loaded);

    check(M12_SaveByteManifest_ImportGameSave(importDir, manifestPath,
                                              importedPath,
                                              (int)sizeof(importedPath)) == -1,
          "duplicate import refuses overwrite");
    unlink(importedPath);
    check(flip_payload_byte(payloadPath), "corrupted exported payload byte");
    check(M12_SaveByteManifest_ImportGameSave(importDir, manifestPath,
                                              importedPath,
                                              (int)sizeof(importedPath)) == -1,
          "corrupt payload rejected by manifest gate");
    check(file_size(importedPath) < 0, "corrupt payload did not create import");
    check(M12_SaveByteManifest_ExportGameSave("csb", savePath, exportDir,
                                              NULL, 0, NULL, 0) == -1,
          "unsupported game id rejected until per-game gate exists");

    F0883_WORLD_Free_Compat(&world);
    cleanup(root);

    if (failures) {
        printf("test_save_byte_manifest_m12: FAIL %d\n", failures);
        return 1;
    }
    puts("test_save_byte_manifest_m12: PASS");
    return 0;
}
