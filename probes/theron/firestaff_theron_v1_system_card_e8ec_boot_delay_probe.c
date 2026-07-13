/* Real-media receipt for the System Card 3.0 boot delay at $e8ec.
 *
 * Opt in with explicit paths only:
 *   FIRESTAFF_THERON_SYSCARD3_PCE=/absolute/path/syscard3.pce
 *   FIRESTAFF_THERON_19_TRACK_CUE=/absolute/path/TQUS.cue
 *
 * The probe verifies original bytes and CUE declarations. It deliberately
 * does not execute a loader, interpret CD state, or substitute missing media.
 */

#include "asset_status_m12.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SYSCARD3_MD5 "ff1a674273fe3540ccef576376407d1d"
#define SYSCARD3_BYTES 0x40200u
#define SYSCARD3_HEADER_BYTES 0x0200u
#define SYSCARD3_E000_WINDOW_OFFSET 0x08e3u

static const unsigned char g_boot_delay_bytes[] = {
    0xa9u, 0x02u, 0x0cu, 0x04u, 0x18u, 0xa0u, 0x0au, 0xa2u, 0x3bu,
    0xcau, 0xd0u, 0xfdu, 0x88u, 0xd0u, 0xf8u, 0xadu, 0x04u, 0x18u,
    0x29u, 0xfdu, 0x8du, 0x04u, 0x18u, 0xa2u, 0x77u, 0xcau, 0xd0u,
    0xfdu, 0x60u
};

static int g_failures;

static void check(int condition, const char *label) {
    if (!condition) {
        ++g_failures;
        printf("FAIL %s\n", label);
    } else {
        printf("PASS %s\n", label);
    }
}

static unsigned char *read_file(const char *path, size_t *out_size) {
    FILE *file = NULL;
    long length;
    unsigned char *bytes = NULL;

    if (!path || !out_size || !(file = fopen(path, "rb")) ||
        fseek(file, 0L, SEEK_END) != 0 || (length = ftell(file)) <= 0 ||
        fseek(file, 0L, SEEK_SET) != 0 ||
        !(bytes = (unsigned char *)malloc((size_t)length)) ||
        fread(bytes, 1u, (size_t)length, file) != (size_t)length) {
        if (file) fclose(file);
        free(bytes);
        return NULL;
    }
    fclose(file);
    *out_size = (size_t)length;
    return bytes;
}

static int inspect_19_track_cue(const char *path) {
    FILE *file;
    char line[1024];
    unsigned int track;
    unsigned int expected_track = 1u;
    unsigned int track_count = 0u;
    int track02_mode1_2352 = 0;

    if (!path || !(file = fopen(path, "rb"))) return 0;
    while (fgets(line, sizeof(line), file) != NULL) {
        char mode[32];
        if (sscanf(line, " TRACK %u %31s", &track, mode) == 2) {
            if (track != expected_track) {
                fclose(file);
                return 0;
            }
            ++expected_track;
            ++track_count;
            if (track == 2u && strcmp(mode, "MODE1/2352") == 0) {
                track02_mode1_2352 = 1;
            }
        }
    }
    fclose(file);
    return track_count == 19u && track02_mode1_2352;
}

int main(void) {
    const char *system_card_path = getenv("FIRESTAFF_THERON_SYSCARD3_PCE");
    const char *cue_path = getenv("FIRESTAFF_THERON_19_TRACK_CUE");
    unsigned char *system_card;
    size_t system_card_size;
    char actual_md5[33];
    const size_t delay_offset = SYSCARD3_HEADER_BYTES +
        SYSCARD3_E000_WINDOW_OFFSET;

    if (!system_card_path || !cue_path) {
        printf("SKIP set FIRESTAFF_THERON_SYSCARD3_PCE and "
               "FIRESTAFF_THERON_19_TRACK_CUE\n");
        return 0;
    }
    system_card = read_file(system_card_path, &system_card_size);
    check(system_card && system_card_size == SYSCARD3_BYTES &&
              m12_file_md5_hex(system_card_path, actual_md5) &&
              strcmp(actual_md5, SYSCARD3_MD5) == 0,
          "authenticated System Card 3.0 container");
    check(system_card && delay_offset + sizeof(g_boot_delay_bytes) <=
              system_card_size &&
              memcmp(system_card + delay_offset, g_boot_delay_bytes,
                     sizeof(g_boot_delay_bytes)) == 0,
          "$e8e3 sets $1804 then enters fixed $e8ec delay");
    check(inspect_19_track_cue(cue_path),
          "explicit 19-track CUE declares sequential tracks and Track 02 MODE1/2352");
    printf("receipt: e8ec_entry=fixed_tsb_1804_and_counter_delay "
           "cd_state_unproven=1\n");
    free(system_card);
    return g_failures ? 1 : 0;
}
