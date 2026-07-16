#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

static int path_exists(const char* path) {
    return path && access(path, R_OK) == 0;
}

static int join_path(char* out, size_t out_size, const char* a, const char* b) {
    int n;
    if (!out || !a || !b || out_size == 0) return 0;
    n = snprintf(out, out_size, "%s/%s", a, b);
    return n > 0 && (size_t)n < out_size;
}

static int dirname_of(const char* path, char* out, size_t out_size) {
    const char* slash;
    size_t len;
    if (!path || !out || out_size == 0) return 0;
    slash = strrchr(path, '/');
    if (!slash) {
        if (out_size < 2) return 0;
        out[0] = '.';
        out[1] = '\0';
        return 1;
    }
    len = (size_t)(slash - path);
    if (len == 0) len = 1;
    if (len >= out_size) return 0;
    memcpy(out, path, len);
    out[len] = '\0';
    return 1;
}

static int find_script(const char* argv0, char* out, size_t out_size) {
    char cwd[PATH_MAX];
    char exe_dir[PATH_MAX];
    char candidate[PATH_MAX];

    if (getcwd(cwd, sizeof(cwd)) &&
        join_path(candidate, sizeof(candidate), cwd,
                  "scripts/firestaff_artpack_studio.py") &&
        path_exists(candidate)) {
        snprintf(out, out_size, "%s", candidate);
        return 1;
    }

    if (dirname_of(argv0, exe_dir, sizeof(exe_dir)) &&
        join_path(candidate, sizeof(candidate), exe_dir,
                  "scripts/firestaff_artpack_studio.py") &&
        path_exists(candidate)) {
        snprintf(out, out_size, "%s", candidate);
        return 1;
    }
    if (dirname_of(argv0, exe_dir, sizeof(exe_dir)) &&
        join_path(candidate, sizeof(candidate), exe_dir,
                  "../scripts/firestaff_artpack_studio.py") &&
        path_exists(candidate)) {
        snprintf(out, out_size, "%s", candidate);
        return 1;
    }
    if (dirname_of(argv0, exe_dir, sizeof(exe_dir)) &&
        join_path(candidate, sizeof(candidate), exe_dir,
                  "../Resources/scripts/firestaff_artpack_studio.py") &&
        path_exists(candidate)) {
        snprintf(out, out_size, "%s", candidate);
        return 1;
    }
    if (dirname_of(argv0, exe_dir, sizeof(exe_dir)) &&
        join_path(candidate, sizeof(candidate), exe_dir,
                  "../share/firestaff/scripts/firestaff_artpack_studio.py") &&
        path_exists(candidate)) {
        snprintf(out, out_size, "%s", candidate);
        return 1;
    }
    if (path_exists("/usr/share/firestaff/scripts/firestaff_artpack_studio.py")) {
        snprintf(out, out_size, "%s",
                 "/usr/share/firestaff/scripts/firestaff_artpack_studio.py");
        return 1;
    }
    return 0;
}

int main(int argc, char** argv) {
    char script[PATH_MAX];
    char** child_argv;

    if (!find_script(argc > 0 ? argv[0] : "firestaff_artpack_studio",
                     script, sizeof(script))) {
        fprintf(stderr,
                "firestaff_artpack_studio: cannot find "
                "scripts/firestaff_artpack_studio.py\n");
        return 2;
    }

    child_argv = (char**)calloc((size_t)argc + 3u, sizeof(char*));
    if (!child_argv) {
        fprintf(stderr, "firestaff_artpack_studio: allocation failed: %s\n",
                strerror(errno));
        return 2;
    }
    child_argv[0] = (char*)"python3";
    child_argv[1] = script;
    for (int i = 1; i < argc; ++i) {
        child_argv[i + 1] = argv[i];
    }
    child_argv[argc + 1] = NULL;

    execvp("python3", child_argv);
    fprintf(stderr, "firestaff_artpack_studio: failed to exec python3: %s\n",
            strerror(errno));
    free(child_argv);
    return 127;
}
