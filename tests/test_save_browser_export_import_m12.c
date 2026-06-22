#include "save_browser_m12.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

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
    if (mkdir(path, 0755) == 0 || errno == EEXIST) return 1;
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

    cleanup(root);
    if (failures) {
        printf("test_save_browser_export_import_m12: FAIL %d\n", failures);
        return 1;
    }
    puts("test_save_browser_export_import_m12: PASS");
    return 0;
}
