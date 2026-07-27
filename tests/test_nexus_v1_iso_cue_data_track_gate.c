#include "nexus_v1_iso_reader.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <direct.h>
#include <process.h>
#define mkdir(path, mode) _mkdir(path)
#define getpid() _getpid()
#else
#include <sys/stat.h>
#include <unistd.h>
#endif

static int failures;

#define CHECK(label, condition) do { \
    if (!(condition)) { fprintf(stderr, "FAIL: %s\\n", label); ++failures; } \
} while (0)

static void le32(uint8_t *p, uint32_t value)
{
    p[0] = (uint8_t)value; p[1] = (uint8_t)(value >> 8);
    p[2] = (uint8_t)(value >> 16); p[3] = (uint8_t)(value >> 24);
}

static int write_record(uint8_t *p, uint32_t lba, const char *name)
{
    size_t name_len = strlen(name);
    int length = 33 + (int)name_len;
    if (name_len == 0U || name_len > 32U) return 0;
    memset(p, 0, (size_t)length);
    p[0] = (uint8_t)length;
    le32(p + 2, lba);
    le32(p + 10, 1);
    p[32] = (uint8_t)name_len;
    memcpy(p + 33, name, name_len);
    return length;
}

static int write_nexus_iso(const char *path)
{
    uint8_t sector[NEXUS_ISO_DATA_SIZE];
    FILE *file = fopen(path, "wb");
    if (!file) return 0;
    for (int i = 0; i <= 22; ++i) {
        memset(sector, 0, sizeof(sector));
        if (i == 16) {
            sector[0] = 1;
            memcpy(sector + 1, "CD001", 5);
            sector[156] = 34;
            le32(sector + 158, 20);
            le32(sector + 166, NEXUS_ISO_DATA_SIZE);
            sector[181] = 2;
            sector[188] = 1;
        } else if (i == 20) {
            int offset = write_record(sector, 21, "DM.BIN;1");
            if (!offset || !write_record(sector + offset, 22, "LEV00.DGN;1")) {
                fclose(file);
                return 0;
            }
        }
        if (fwrite(sector, 1U, sizeof(sector), file) != sizeof(sector)) {
            fclose(file);
            return 0;
        }
    }
    return fclose(file) == 0;
}

static int write_text(const char *path, const char *text)
{
    FILE *file = fopen(path, "wb");
    if (!file) return 0;
    if (fwrite(text, 1U, strlen(text), file) != strlen(text)) {
        fclose(file);
        return 0;
    }
    return fclose(file) == 0;
}

int main(void)
{
    char root[256], audio[320], data[320], cue[320];
    char nested[320], nested_data[384], nested_cue[320];
    Nexus_ISOReader reader;
    FILE *audio_file;

    snprintf(root, sizeof(root), "/tmp/firestaff-nexus-cue-%ld", (long)getpid());
    CHECK("temporary root", mkdir(root, 0700) == 0);
    snprintf(audio, sizeof(audio), "%s/audio.bin", root);
    snprintf(data, sizeof(data), "%s/data.bin", root);
    snprintf(cue, sizeof(cue), "%s/disc.cue", root);
    audio_file = fopen(audio, "wb");
    CHECK("audio decoy", audio_file && fputc(0, audio_file) != EOF && fclose(audio_file) == 0);
    CHECK("data ISO", write_nexus_iso(data));
    CHECK("lowercase multi-file cue", write_text(cue,
          "file \"audio.bin\" binary\n  track 01 audio\n"
          "file \"data.bin\" binary\n  track 02 mode1/2048\n"));
    memset(&reader, 0, sizeof(reader));
    CHECK("cue chooses data after audio", nexus_iso_open_cue(&reader, cue) == 2 &&
          nexus_iso_is_nexus(&reader) && strstr(reader.path, "data.bin") != NULL);
    nexus_iso_close(&reader);

    snprintf(nested, sizeof(nested), "%s/nested", root);
    snprintf(nested_data, sizeof(nested_data), "%s/data.bin", nested);
    snprintf(nested_cue, sizeof(nested_cue), "%s/windows.cue", root);
    CHECK("nested directory", mkdir(nested, 0700) == 0);
    CHECK("nested data ISO", write_nexus_iso(nested_data));
    CHECK("Windows separator cue", write_text(nested_cue,
          "FILE \"nested\\\\data.bin\" BINARY\n  TRACK 01 MODE1/2048\n"));
    memset(&reader, 0, sizeof(reader));
    CHECK("cue normalizes Windows separator", nexus_iso_open_cue(&reader, nested_cue) == 2 &&
          nexus_iso_is_nexus(&reader));
    nexus_iso_close(&reader);

    remove(nested_cue); remove(nested_data); rmdir(nested);
    remove(cue); remove(data); remove(audio); rmdir(root);
    if (failures) return 1;
    puts("Nexus CUE data-track gate: PASS");
    return 0;
}
