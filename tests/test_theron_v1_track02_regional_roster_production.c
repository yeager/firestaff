#include "theron_v1_champions.h"
#include "theron_v1_track02.h"
#include "theron_v1_track02_jp_roster_receipt.h"
#include "theron_v1_track02_us_roster_receipt.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint8_t *read_file(const char *path, size_t *out_size) {
    FILE *file = fopen(path, "rb");
    long size;
    uint8_t *bytes;
    if (!file || fseek(file, 0L, SEEK_END) != 0 ||
        (size = ftell(file)) <= 0L || fseek(file, 0L, SEEK_SET) != 0) {
        if (file) fclose(file);
        return NULL;
    }
    bytes = (uint8_t *)malloc((size_t)size);
    if (!bytes || fread(bytes, 1u, (size_t)size, file) != (size_t)size) {
        free(bytes);
        fclose(file);
        return NULL;
    }
    fclose(file);
    *out_size = (size_t)size;
    return bytes;
}

static int verify_variant(const char *path, const char *md5_hex, int jp) {
    static const char *const names[] = {"THERON", "MARA", "LINOS", "HEXA"};
    Theron_V1_Party party;
    size_t size = 0u;
    uint8_t *bytes = read_file(path, &size);
    uint8_t final_byte;
    int ok;
    if (!bytes) return 0;

    theron_v1_party_init(&party, 1);
    if (party.champion_count != 0) {
        free(bytes);
        return 0;
    }
    memset(&party, 0, sizeof(party));
    party.champion_count = 4;
    for (int i = 0; i < 4; ++i)
        snprintf(party.champions[i].name,
                 sizeof(party.champions[i].name), "%s", names[i]);
    ok = jp ? theron_v1_party_refresh_jp_source_records(
                  &party, bytes, size, md5_hex)
            : theron_v1_party_refresh_us_source_records(
                  &party, bytes, size, md5_hex);
    if (!ok || party.champion_count != 4 ||
        party.champions[0].health != 175 ||
        party.champions[0].stamina != 1500 ||
        party.champions[1].mana != 200 ||
        party.champions[2].ninja_level != 9 ||
        party.champions[3].strength != 50 ||
        party.champions[0].portrait_index != THERON_PORTRAIT_UNAVAILABLE) {
        free(bytes);
        return 0;
    }

    final_byte = bytes[size - 1u];
    bytes[size - 1u] ^= 0x01u;
    if (jp) {
        Theron_Track02JpRosterReceipt records[THERON_TRACK02_JP_ROSTER_COUNT];
        memset(records, 0xa5, sizeof(records));
        ok = !theron_v1_track02_jp_roster_read(
            bytes, size, md5_hex, records) && records[0].valid == 0;
    } else {
        Theron_Track02UsRosterReceipt records[THERON_TRACK02_US_ROSTER_COUNT];
        memset(records, 0xa5, sizeof(records));
        ok = !theron_v1_track02_us_roster_read(
            bytes, size, md5_hex, records) && records[0].valid == 0;
    }
    bytes[size - 1u] = final_byte;
    free(bytes);
    return ok;
}

int main(void) {
    const char *home = getenv("HOME");
    char us[512];
    char jp[512];
    if (!home || !home[0]) return 77;
    snprintf(us, sizeof(us), "%s/.firestaff/data/theron/TQUS02.bin", home);
    snprintf(jp, sizeof(jp), "%s/.firestaff/data/theron/TQJP02.bin", home);
    {
        FILE *us_file = fopen(us, "rb");
        FILE *jp_file = fopen(jp, "rb");
        if (!us_file || !jp_file) {
            if (us_file) fclose(us_file);
            if (jp_file) fclose(jp_file);
            puts("SKIP: authentic US and JP Track 02 files are not staged");
            return 77;
        }
        fclose(us_file);
        fclose(jp_file);
    }
    if (!verify_variant(us, THERON_TRACK02_MD5_US_BIN, 0) ||
        !verify_variant(jp, THERON_TRACK02_MD5_JP_BIN, 1)) {
        fputs("FAIL: regional production roster bind\n", stderr);
        return 1;
    }
    puts("PASS: production party is empty until US/JP source records bind");
    return 0;
}
