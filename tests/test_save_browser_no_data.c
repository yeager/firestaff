#include "save_browser_m12.h"

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static int expect(int cond, const char* msg) {
    if (!cond) {
        fprintf(stderr, "FAIL: %s\n", msg);
        return 0;
    }
    return 1;
}

static int write_fixture_file(const char* dir, const char* name) {
    char path[512];
    FILE* fp;

    snprintf(path, sizeof(path), "%s/%s", dir, name);
    fp = fopen(path, "wb");
    if (!fp) return 0;
    fputs("not a valid Firestaff save\n", fp);
    return fclose(fp) == 0;
}

static void remove_fixture_file(const char* dir, const char* name) {
    char path[512];

    snprintf(path, sizeof(path), "%s/%s", dir, name);
    unlink(path);
}

int main(void) {
    char tmpTemplate[] = "/tmp/firestaff-save-browser-XXXXXX";
    char missingPath[512];
    M12_SaveBrowserState state;
    const M12_SaveBrowserEntry* selected;

    memset(&state, 0x5a, sizeof(state));
    snprintf(missingPath, sizeof(missingPath), "%s/missing", tmpTemplate);
    if (!expect(M12_SaveBrowser_Scan(&state, missingPath) == 0,
                "missing save root should scan as empty")) return 1;
    if (!expect(state.entryCount == 0, "missing root should clear entry count")) return 1;
    if (!expect(M12_SaveBrowser_GetSelected(&state) == NULL,
                "missing root should leave no selected entry")) return 1;

    if (!mkdtemp(tmpTemplate)) {
        perror("mkdtemp");
        return 1;
    }

    memset(&state, 0x5a, sizeof(state));
    if (!expect(M12_SaveBrowser_Scan(&state, tmpTemplate) == 0,
                "empty save directory should scan as empty")) return 1;
    if (!expect(state.entryCount == 0, "empty directory should have no entries")) return 1;
    if (!expect(state.selectedIndex == 0, "empty directory should reset selection")) return 1;
    if (!expect(state.scrollOffset == 0, "empty directory should reset scroll")) return 1;
    if (!expect(state.confirmDelete == 0, "empty directory should reset delete state")) return 1;

    if (!expect(write_fixture_file(tmpTemplate, "firestaff-beta.sav"),
                "should write matching save fixture")) return 1;
    if (!expect(write_fixture_file(tmpTemplate, "firestaff-alpha-quicksave.sav"),
                "should write matching quicksave fixture")) return 1;
    if (!expect(write_fixture_file(tmpTemplate, "firestaff-.sav"),
                "should write empty-id malformed fixture")) return 1;
    if (!expect(write_fixture_file(tmpTemplate, "Firestaff-gamma.sav"),
                "should write case-mismatched fixture")) return 1;
    if (!expect(write_fixture_file(tmpTemplate, "firestaff-delta.SAV"),
                "should write extension-mismatched fixture")) return 1;
    if (!expect(write_fixture_file(tmpTemplate, "firestaff-epsilon.sav.tmp"),
                "should write suffix-mismatched fixture")) return 1;

    if (!expect(M12_SaveBrowser_Scan(&state, tmpTemplate) == 2,
                "only firestaff-*.sav names with non-empty ids should be listed")) return 1;
    if (!expect(state.entryCount == 2, "filtered listing should contain two entries")) return 1;
    if (!expect(strcmp(state.entries[0].filename,
                       "firestaff-alpha-quicksave.sav") == 0,
                "equal-time entries should sort by filename")) return 1;
    if (!expect(strcmp(state.entries[0].gameId, "alpha") == 0,
                "quicksave id should strip firestaff prefix and quicksave suffix")) return 1;
    if (!expect(strcmp(state.entries[1].filename, "firestaff-beta.sav") == 0,
                "second sorted entry should be beta save")) return 1;
    if (!expect(strcmp(state.entries[1].gameId, "beta") == 0,
                "plain save id should strip firestaff prefix and .sav suffix")) return 1;
    if (!expect(state.entries[0].valid == 0 && state.entries[1].valid == 0,
                "corrupt fixture saves should be listed but marked invalid")) return 1;

    selected = M12_SaveBrowser_GetSelected(&state);
    if (!expect(selected == &state.entries[0],
                "selected entry should point at first sorted save")) return 1;

    remove_fixture_file(tmpTemplate, "firestaff-beta.sav");
    remove_fixture_file(tmpTemplate, "firestaff-alpha-quicksave.sav");
    remove_fixture_file(tmpTemplate, "firestaff-.sav");
    remove_fixture_file(tmpTemplate, "Firestaff-gamma.sav");
    remove_fixture_file(tmpTemplate, "firestaff-delta.SAV");
    remove_fixture_file(tmpTemplate, "firestaff-epsilon.sav.tmp");
    rmdir(tmpTemplate);

    puts("ok: save browser no-data scan handles missing, empty, and malformed save roots");
    return 0;
}
