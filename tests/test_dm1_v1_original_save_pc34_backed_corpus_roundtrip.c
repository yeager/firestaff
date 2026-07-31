#include "dm1_v1_original_save_classifier.h"
#include "m11_game_view.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <process.h>
#define dm1_test_getpid _getpid
#else
#include <unistd.h>
#define dm1_test_getpid getpid
#endif

#define CHECK(condition, message) do { \
    if (!(condition)) { \
        fprintf(stderr, "FAIL %s (line %d): %s\n", \
                message, __LINE__, #condition); \
        return 0; \
    } \
} while (0)

static int set_env_value(const char *name, const char *value)
{
#ifdef _WIN32
    return _putenv_s(name, value);
#else
    return setenv(name, value, 1);
#endif
}

static int clear_env_value(const char *name)
{
#ifdef _WIN32
    return _putenv_s(name, "");
#else
    return unsetenv(name);
#endif
}

static int read_file_bytes(const char *path, unsigned char **out_bytes,
                           size_t *out_size)
{
    FILE *file;
    long length;
    unsigned char *bytes;

    if (!path || !out_bytes || !out_size) return 0;
    *out_bytes = NULL;
    *out_size = 0u;
    file = fopen(path, "rb");
    if (!file || fseek(file, 0L, SEEK_END) != 0 ||
        (length = ftell(file)) <= 0 || fseek(file, 0L, SEEK_SET) != 0) {
        if (file) fclose(file);
        return 0;
    }
    bytes = (unsigned char *)malloc((size_t)length);
    if (!bytes || fread(bytes, 1u, (size_t)length, file) !=
                      (size_t)length) {
        free(bytes);
        fclose(file);
        return 0;
    }
    fclose(file);
    *out_bytes = bytes;
    *out_size = (size_t)length;
    return 1;
}

static int roundtrip_backed_save(const char *save_path, const char *data_dir,
                                 int ordinal)
{
    unsigned char *source_bytes = NULL;
    unsigned char *exported_bytes = NULL;
    size_t source_size = 0u;
    size_t exported_size = 0u;
    DM1OriginalSaveClassifyResult source_classify;
    DM1OriginalSaveClassifyResult exported_classify;
    M11_GameViewState source;
    M11_GameViewState reloaded;
    char quicksave_path[512];
    char quicksave_tail_path[528];
    char exported_path[512];
    int source_started = 0;
    int reloaded_started = 0;
    int ok = 0;

    memset(&source, 0, sizeof(source));
    memset(&reloaded, 0, sizeof(reloaded));
    snprintf(quicksave_path, sizeof(quicksave_path),
             "/tmp/firestaff-backed-pc34-%ld-%d.sav",
             (long)dm1_test_getpid(), ordinal);
    snprintf(exported_path, sizeof(exported_path),
             "/tmp/firestaff-backed-pc34-%ld-%d.dat",
             (long)dm1_test_getpid(), ordinal);
    snprintf(quicksave_tail_path, sizeof(quicksave_tail_path), "%s.pc34tail",
             quicksave_path);

    CHECK(read_file_bytes(save_path, &source_bytes, &source_size),
          "read corpus PC34 save");
    memset(&source_classify, 0, sizeof(source_classify));
    CHECK(dm1_v1_original_save_classify_bytes(source_bytes, source_size,
                                               &source_classify) &&
              source_classify.shape ==
                  DM1_ORIGINAL_SAVE_SHAPE_ORIGINAL_DM1_PC34 &&
              source_classify.pc34_importer_candidate &&
              source_classify.pc34_loader_part_envelope_candidate,
          "corpus save remains a checksum-qualified original PC34 envelope");
    CHECK(set_env_value("FIRESTAFF_QUICKSAVE_PATH", quicksave_path) == 0,
          "configure isolated corpus quicksave output");

    M11_GameView_Init(&source);
    source_started = 1;
    CHECK(M11_GameView_StartDm1(&source, data_dir),
          "start DM1 against original backing data");
    CHECK(M11_GameView_LoadDm1OriginalPc34SaveBytes(
              &source, source_bytes, source_size, save_path),
          "F0435 imports corpus save against original DUNGEON.DAT");
    CHECK(source.world.dungeon && source.world.things && source.world.ownsDungeon,
          "F0435 retains an owned original-backed dungeon");
    CHECK(M11_GameView_QuickSave(&source),
          "F0433 stages original-backed runtime without fallback world");
    CHECK(M11_GameView_ExportQuickSaveAsDM1PC34(quicksave_path, exported_path),
          "F0433 exports original-backed corpus runtime");
    CHECK(read_file_bytes(exported_path, &exported_bytes, &exported_size),
          "read F0433 corpus export");
    memset(&exported_classify, 0, sizeof(exported_classify));
    CHECK(dm1_v1_original_save_classify_bytes(exported_bytes, exported_size,
                                               &exported_classify) &&
              exported_classify.shape ==
                  DM1_ORIGINAL_SAVE_SHAPE_ORIGINAL_DM1_PC34 &&
              exported_classify.pc34_importer_candidate &&
              exported_classify.pc34_loader_part_envelope_candidate,
          "F0433 corpus export remains a checksum-qualified PC34 envelope");

    M11_GameView_Init(&reloaded);
    reloaded_started = 1;
    CHECK(M11_GameView_StartDm1(&reloaded, data_dir),
          "start second original DM1 backing");
    CHECK(M11_GameView_LoadDm1OriginalPc34SaveBytes(
              &reloaded, exported_bytes, exported_size, exported_path),
          "second F0435 import consumes F0433 corpus output");
    CHECK(reloaded.world.party.mapIndex == source.world.party.mapIndex &&
              reloaded.world.party.mapX == source.world.party.mapX &&
              reloaded.world.party.mapY == source.world.party.mapY &&
              reloaded.world.party.direction == source.world.party.direction &&
              reloaded.world.party.championCount ==
                  source.world.party.championCount &&
              reloaded.world.party.activeChampionIndex ==
                  source.world.party.activeChampionIndex &&
              reloaded.world.gameTick == source.world.gameTick &&
              reloaded.world.creatureAICount == source.world.creatureAICount &&
              reloaded.world.pc34ActiveGroupSourceCount ==
                  source.world.pc34ActiveGroupSourceCount &&
              reloaded.world.timeline.count == source.world.timeline.count,
          "backed corpus roundtrip preserves party, C03/C04 and active groups");
    ok = 1;

    if (reloaded_started) M11_GameView_Shutdown(&reloaded);
    if (source_started) M11_GameView_Shutdown(&source);
    (void)remove(quicksave_path);
    (void)remove(quicksave_tail_path);
    (void)remove(exported_path);
    (void)clear_env_value("FIRESTAFF_QUICKSAVE_PATH");
    free(exported_bytes);
    free(source_bytes);
    return ok;
}

int main(void)
{
    const char *corpus_root = getenv("FIRESTAFF_DM1_PC34_SAVE_CORPUS");
    const char *data_dir = getenv("FIRESTAFF_DM1_PC_DATA");
    DM1OriginalSaveCorpusManifest corpus;
    int completed = 0;
    int candidates = 0;

    if (!corpus_root || !corpus_root[0] || !data_dir || !data_dir[0]) {
        puts("SKIP backed original PC34 corpus: corpus root or data root unset");
        return 0;
    }
    memset(&corpus, 0, sizeof(corpus));
    if (!dm1_v1_original_save_classify_corpus_root(corpus_root, &corpus)) {
        fprintf(stderr, "FAIL classify original PC34 corpus: %s\n", corpus_root);
        return 1;
    }
    for (int i = 0; i < corpus.present_count; ++i) {
        const DM1OriginalSaveClassifyResult *candidate = &corpus.results[i];
        if (!candidate->pc34_loader_part_envelope_candidate) continue;
        ++candidates;
        if (!roundtrip_backed_save(corpus.paths[i], data_dir, candidates)) {
            return 1;
        }
        ++completed;
    }
    if (candidates == 0 || completed != candidates) {
        fprintf(stderr, "FAIL backed corpus candidates=%d completed=%d\n",
                candidates, completed);
        return 1;
    }
    printf("PASS backed original PC34 corpus roundtrip candidates=%d root=%s\n",
           completed, corpus_root);
    return 0;
}
