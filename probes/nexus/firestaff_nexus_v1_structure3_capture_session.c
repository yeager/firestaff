/* Launches an external, instrumented Saturn capture producer for one
 * canonical Structure3 face. Firestaff writes only a source-bound target;
 * the producer alone writes raw evidence lanes and its own manifest. */

#include "nexus_v1_engine.h"

#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define MANIFEST_MAX_BYTES (64U * 1024U)

static int parse_nonnegative(const char *text, int *out_value)
{
    char *end = NULL;
    long value;

    if (!text || !out_value) return 0;
    errno = 0;
    value = strtol(text, &end, 0);
    if (errno != 0 || end == text || *end != '\0' || value < 0 ||
        value > 0x7fffffffL) return 0;
    *out_value = (int)value;
    return 1;
}

static int directory_is_empty(const char *path)
{
    DIR *directory;
    struct dirent *entry;
    int empty = 1;

    directory = opendir(path);
    if (!directory) return 0;
    while ((entry = readdir(directory)) != NULL) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            empty = 0;
            break;
        }
    }
    closedir(directory);
    return empty;
}

static int prepare_empty_directory(const char *path)
{
    struct stat status;

    if (mkdir(path, 0700) == 0) return 1;
    if (errno != EEXIST || stat(path, &status) != 0 || !S_ISDIR(status.st_mode))
        return 0;
    return directory_is_empty(path);
}

static int join_path(char *out_path, size_t out_size, const char *directory,
                     const char *name)
{
    return snprintf(out_path, out_size, "%s/%s", directory, name) <
        (int)out_size;
}

static char *read_text_file(const char *path, size_t *out_size)
{
    FILE *file;
    long size;
    char *text;

    if (out_size) *out_size = 0U;
    file = fopen(path, "rb");
    if (!file || fseek(file, 0L, SEEK_END) != 0 || (size = ftell(file)) <= 0L ||
        (size_t)size > MANIFEST_MAX_BYTES || fseek(file, 0L, SEEK_SET) != 0) {
        if (file) fclose(file);
        return NULL;
    }
    text = (char *)malloc((size_t)size + 1U);
    if (!text || fread(text, 1U, (size_t)size, file) != (size_t)size ||
        fclose(file) != 0) {
        free(text);
        return NULL;
    }
    text[size] = '\0';
    if (out_size) *out_size = (size_t)size;
    return text;
}

static int raw_span_sizes_match(const Nexus_V1_DgnStructure3CaptureManifestReceipt *manifest,
                                const char *const paths[6])
{
    const uint32_t declared[6] = {
        manifest->texture_span_bytes, manifest->palette_state_bytes,
        manifest->vdp1_state_bytes, manifest->transform_state_bytes,
        manifest->normal_culling_state_bytes, manifest->vdp1_command_bytes
    };
    struct stat status;
    int lane;

    for (lane = 0; lane < 6; ++lane) {
        if (stat(paths[lane], &status) != 0 || !S_ISREG(status.st_mode) ||
            status.st_size <= 0 || (uint64_t)status.st_size != declared[lane])
            return 0;
    }
    return 1;
}

static int set_capture_environment(const char *data_dir, const char *target_path,
                                   const char *manifest_path,
                                   const char *const paths[6])
{
    static const char *const names[6] = {
        "FIRESTAFF_NEXUS_STRUCTURE3_TEXTURE_SPAN",
        "FIRESTAFF_NEXUS_STRUCTURE3_PALETTE_STATE",
        "FIRESTAFF_NEXUS_STRUCTURE3_VDP1_STATE",
        "FIRESTAFF_NEXUS_STRUCTURE3_TRANSFORM_STATE",
        "FIRESTAFF_NEXUS_STRUCTURE3_NORMAL_CULLING_STATE",
        "FIRESTAFF_NEXUS_STRUCTURE3_VDP1_COMMAND"
    };
    int lane;

    if (setenv("FIRESTAFF_NEXUS_DATA_DIR", data_dir, 1) != 0 ||
        setenv("FIRESTAFF_NEXUS_STRUCTURE3_TARGET", target_path, 1) != 0 ||
        setenv("FIRESTAFF_NEXUS_STRUCTURE3_MANIFEST", manifest_path, 1) != 0 ||
        setenv("FIRESTAFF_NEXUS_STRUCTURE3_NO_DRAW_ONLY", "1", 1) != 0)
        return 0;
    for (lane = 0; lane < 6; ++lane) {
        if (setenv(names[lane], paths[lane], 1) != 0) return 0;
    }
    return 1;
}

int main(int argc, char **argv)
{
    Nexus_V1_Engine engine;
    Nexus_V1_DgnStructure2SourceReceipt source;
    Nexus_V1_DgnStructure3CaptureTargetReceipt target;
    Nexus_V1_DgnStructure3CaptureManifestReceipt manifest;
    const char *raw_paths[6];
    char target_path[1024];
    char manifest_path[1024];
    char raw_path_storage[6][1024];
    static const char *const raw_names[6] = {
        "texture_span.bin", "palette_state.bin", "vdp1_state.bin",
        "transform_state.bin", "normal_culling_state.bin", "vdp1_command.bin"
    };
    char *manifest_text = NULL;
    size_t manifest_size = 0U;
    pid_t child;
    int status;
    int level_index;
    int entry_index;
    int face_ordinal;
    int lane;
    int candidate_ready = 0;

    if (argc < 7 || !parse_nonnegative(argv[2], &level_index) ||
        !parse_nonnegative(argv[3], &entry_index) ||
        !parse_nonnegative(argv[4], &face_ordinal) || level_index > 15) {
        fprintf(stderr, "usage: %s DATA_DIR LEVEL ENTRY FACE SESSION_DIR PRODUCER [PRODUCER_ARG ...]\n", argv[0]);
        return 2;
    }
    if (!prepare_empty_directory(argv[5]) ||
        !join_path(target_path, sizeof(target_path), argv[5], "structure3-target.txt") ||
        !join_path(manifest_path, sizeof(manifest_path), argv[5], "structure3-manifest.txt")) {
        fprintf(stderr, "session directory must be new or empty\n");
        return 1;
    }
    for (lane = 0; lane < 6; ++lane) {
        if (!join_path(raw_path_storage[lane], sizeof(raw_path_storage[lane]), argv[5],
                       raw_names[lane])) {
            fprintf(stderr, "session path too long\n");
            return 1;
        }
        raw_paths[lane] = raw_path_storage[lane];
    }
    if (nexus_v1_init(&engine, argv[1]) != 0 ||
        nexus_v1_load_level(&engine, level_index) != 0 ||
        nexus_v1_current_level_structure2_source_receipt(&engine, &source) != 0 ||
        !source.canonical_hash_verified || !source.loaded_bytes_bound ||
        !nexus_v1_dgn_structure3_capture_target_build(
            &engine.current_level, engine.current_level_dgn_data,
            engine.current_level_dgn_size, level_index, source.canonical_hash_verified,
            (uint32_t)entry_index, (uint32_t)face_ordinal, &target) ||
        !nexus_v1_dgn_structure3_capture_target_write(target_path, &target)) {
        fprintf(stderr, "canonical source-bound target unavailable\n");
        nexus_v1_shutdown(&engine);
        return 1;
    }
    child = fork();
    if (child == 0) {
        if (!set_capture_environment(argv[1], target_path, manifest_path, raw_paths))
            _exit(126);
        execvp(argv[6], &argv[6]);
        _exit(127);
    }
    if (child < 0 || waitpid(child, &status, 0) < 0 || !WIFEXITED(status) ||
        WEXITSTATUS(status) != 0) {
        fprintf(stderr, "external capture producer did not complete\n");
        nexus_v1_shutdown(&engine);
        return 1;
    }
    manifest_text = read_text_file(manifest_path, &manifest_size);
    if (manifest_text && nexus_v1_dgn_structure3_capture_manifest_parse(
            manifest_text, manifest_size, &manifest) &&
        nexus_v1_dgn_structure3_capture_target_matches_manifest(&target, &manifest) &&
        raw_span_sizes_match(&manifest, raw_paths)) candidate_ready = 1;
    printf("canonical_dgn_bound=%d\n", source.canonical_hash_verified);
    printf("capture_producer_executed=1\n");
    printf("target_manifest_matches=%d\n", candidate_ready ? 1 : 0);
    printf("external_attestation_required=1\n");
    printf("runtime_import_permitted=0\n");
    printf("no_draw_only=1\n");
    free(manifest_text);
    nexus_v1_shutdown(&engine);
    return candidate_ready ? 0 : 1;
}
