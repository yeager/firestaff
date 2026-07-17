#include <stdio.h>
#include <string.h>

#if !defined(_WIN32)
#include <dirent.h>
#include <sys/stat.h>
#endif

#include "theron_v1_track02_mednafen_trace_converter.h"

static int theron_v1_mednafen_trace_probe_discover(const char *directory) {
#if defined(_WIN32)
    (void)directory;
    printf("SKIP: capture-root discovery is unavailable on this platform\n");
    return 0;
#else
    enum { MAX_ENTRIES = 128 };
    DIR *root;
    struct dirent *entry;
    struct stat root_status;
    unsigned int entries = 0;
    unsigned int matches = 0;

    if (!directory || !directory[0] || lstat(directory, &root_status) != 0) {
        printf("SKIP: capture-root unavailable\n");
        return 0;
    }
    if (S_ISLNK(root_status.st_mode) || !S_ISDIR(root_status.st_mode) ||
        !(root = opendir(directory))) {
        fprintf(stderr, "REJECTED: capture-root must be a direct directory\n");
        return 1;
    }
    while (entries < MAX_ENTRIES && (entry = readdir(root)) != NULL) {
        char path[THERON_V1_TRACK02_MEDNAFEN_TRACE_PATH_CAPACITY];
        struct stat entry_status;
        Theron_V1Track02MednafenTraceConvertReceipt receipt;
        int path_length;

        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) continue;
        ++entries;
        path_length = snprintf(path, sizeof(path), "%s/%s", directory, entry->d_name);
        if (path_length < 0 || (size_t)path_length >= sizeof(path) ||
            lstat(path, &entry_status) != 0 ||
            !S_ISREG(entry_status.st_mode) ||
            !theron_v1_track02_mednafen_trace_inspect_file(path, &receipt) ||
            receipt.status != THERON_V1_TRACK02_MEDNAFEN_TRACE_INSPECTED) continue;
        ++matches;
        printf("READY: strict Mednafen export path=%s MD5=%s; emulator not launched\n",
               receipt.source_trace_path, receipt.source_trace_md5);
    }
    closedir(root);
    if (!matches) printf("SKIP: no strict Mednafen export in capture-root\n");
    return 0;
#endif
}

int main(int argc, char **argv) {
    Theron_V1Track02MednafenTraceConvertRequest request;
    Theron_V1Track02MednafenTraceConvertReceipt receipt;

    if (argc == 1) {
        printf("SKIP: explicit Mednafen export path required\n");
        return 0;
    }
    if (argc == 3 && !strcmp(argv[1], "--inspect")) {
        if (!theron_v1_track02_mednafen_trace_inspect_file(argv[2], &receipt)) {
            fprintf(stderr, "trace inspection rejected\n");
            return 1;
        }
        if (receipt.status == THERON_V1_TRACK02_MEDNAFEN_TRACE_INSPECTED) {
            printf("READY: strict Mednafen export MD5=%s; emulator not launched\n",
                   receipt.source_trace_md5);
            return 0;
        }
        if (receipt.status == THERON_V1_TRACK02_MEDNAFEN_TRACE_UNAVAILABLE) {
            printf("SKIP: external Mednafen export unavailable\n");
            return 0;
        }
        fprintf(stderr, "REJECTED: unsupported Mednafen export\n");
        return 1;
    }
    if (argc == 3 && !strcmp(argv[1], "--discover")) {
        return theron_v1_mednafen_trace_probe_discover(argv[2]);
    }
    if (argc != 5 || strcmp(argv[1], "--convert")) {
        fprintf(stderr,
                "usage: %s --inspect <mednafen-export>\n"
                "       %s --discover <capture-root>\n"
                "       %s --convert <mednafen-export> <expected-md5> <event-log>\n",
                argv[0],
                argv[0],
                argv[0]);
        return 2;
    }
    request.source_trace_path = argv[2];
    request.expected_source_trace_md5 = argv[3];
    request.event_log_path = argv[4];
    if (!theron_v1_track02_mednafen_trace_convert_file(&request, &receipt)) {
        fprintf(stderr, "trace conversion rejected\n");
        return 1;
    }
    if (receipt.status == THERON_V1_TRACK02_MEDNAFEN_TRACE_CONVERTED) {
        printf("READY: strict HuC6280 event log written; emulator not launched\n");
        return 0;
    }
    if (receipt.status == THERON_V1_TRACK02_MEDNAFEN_TRACE_UNAVAILABLE) {
        printf("SKIP: external Mednafen export unavailable\n");
        return 0;
    }
    fprintf(stderr, "REJECTED: unsupported Mednafen export or MD5 mismatch\n");
    return 1;
}
