#include "nexus_v1_sound.h"
#include "nexus_v1_iso_reader.h"
#include "asset_find_by_hash.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int failures;

static const char *const slev_md5[16] = {
    "59c01cbdd224152a6176687cdebeea9e", "b3b14a73db311b7cf1bf417e858e5350",
    "a77b9cae611a01fbc6a68958f9252b48", "6600745773cd7058d3125747ead5b612",
    "1d678eaff22d0827899a8ffa7377f06b", "9166ae024df53462b7b47ca81db56fc4",
    "7c737f7532677babfc28848198a8288d", "d607ef9b6ee52c0730b92ed22843c2da",
    "5f1bfbd324648ac6872c73d3d282cd6d", "f588811de05a2099633f7dba5a0ca956",
    "e346ee3c7db858cbd872ca947eb79309", "a7deb8241dfa633804793cf07e841a84",
    "fc4028c61279d7d10fc771b040486c85", "2a988cf44049e59e647976d93126dedb",
    "8116d450f4f60af67e8a86b559beb5ab", "5b71918f112e10b3d8c40092565cce53"
};

static const char *const sal_md5[16] = {
    "ea8493341fd8ad4f20335629e6dbdbbc", "ea8493341fd8ad4f20335629e6dbdbbc",
    "729a66977e1661808d104059ff21e95e", "5c357157d68b2878881e1e0a293d3058",
    "9d8d8b793801234b8f4b0e64e1135afc", "db21b7945b65ccfbb7a4246b1f5dca7b",
    "2d7698144c64996536e8240ee7bfea08", "9b31400c2b3c7468b8f88c1fd09c8bca",
    "0e5caba79b2e31963739784f6941f3c5", "f311e79dd6e4be376c0466ea34a27b10",
    "4b655b6cf8c6caebe99dd0b3b55d39c0", "7a9509b7d777f1468ecf987107f1aed0",
    "59e70afc5cf607c6d268811cbed961cd", "14a1f88abc0363d7a96b2a267d89e7a4",
    "1c12a4f3d3dfc9892cdf54955abbca62", "d8cfb5da08d5fc8d86834d81d8997eac"
};

static const char *const map_md5[16] = {
    "232afa942754027ecf49702703c72e83", "232afa942754027ecf49702703c72e83",
    "e724a7b953a6ee9d4bb7d5c2114d5310", "91be9e82471be25036889b6801e7fcd3",
    "64f95657b2745acdbed9d938ba5dfd9e", "95be564a755500e2605b6c83f742f37f",
    "8d0a168e11ebeea2c424a81a474c9d17", "2c8def9015004a9955706c7f41d319be",
    "4e5bee7797d2b3b06a54bb55e6809e90", "47af8003ec0900979fa939288cc1b549",
    "89f984a9eb3be797c37515766e658c12", "130bf4977263076710aaf722c3078f0c",
    "5cdd004b21437268ef51bdc6be33988d", "1f3a1f6ddae837f8140063a637d5fbbc",
    "fd3b5d9894265d0753aee0e0ddb02500", "9757c71fe8afad9ad3be58543640270d"
};

#define CHECK(condition, message) do { \
    if (!(condition)) { \
        fprintf(stderr, "FAIL: %s\\n", message); \
        failures++; \
    } \
} while (0)

static unsigned char *read_file(const char *path, int *out_size) {
    FILE *file;
    long size;
    unsigned char *data;

    *out_size = 0;
    file = fopen(path, "rb");
    if (!file || fseek(file, 0, SEEK_END) != 0 ||
        (size = ftell(file)) <= 0 || fseek(file, 0, SEEK_SET) != 0) {
        if (file) fclose(file);
        return NULL;
    }
    data = (unsigned char *)malloc((size_t)size);
    if (!data || fread(data, 1, (size_t)size, file) != (size_t)size) {
        free(data);
        fclose(file);
        return NULL;
    }
    fclose(file);
    *out_size = (int)size;
    return data;
}

/* The retail Saturn release is normally staged as a CUE plus raw BIN
 * tracks, not as a loose ISO tree.  Test the same bounded CUE/ISO reader as
 * the product instead of silently treating an extracted copy as the only
 * real-media layout.  Loose original files are still accepted for a
 * preservation dump that already exposes them. */
static unsigned char *read_retail_member(const char *data_dir,
                                         Nexus_ISOReader *iso,
                                         const char *member_name,
                                         int *out_size) {
    char path[1024];
    const Nexus_ISOFile *member;
    unsigned char *data;

    if (out_size) *out_size = 0;
    if (!data_dir || !member_name || !out_size) return NULL;
    snprintf(path, sizeof(path), "%s/%s", data_dir, member_name);
    data = read_file(path, out_size);
    if (data) return data;

    if (!iso || !iso->valid) return NULL;
    member = nexus_iso_find(iso, member_name);
    if (!member || member->size == 0U || member->size > (uint32_t)INT_MAX) {
        return NULL;
    }
    data = (unsigned char *)malloc(member->size);
    if (!data || nexus_iso_read_file(iso, member, data, (int)member->size) !=
                     (int)member->size) {
        free(data);
        return NULL;
    }
    *out_size = (int)member->size;
    return data;
}

static int open_retail_iso_if_present(const char *data_dir,
                                      Nexus_ISOReader *out_iso) {
    static const char *const cue_names[] = {
        "Dungeon Master Nexus (Japan).cue",
        "Dungeon Master Nexus (English).cue",
        NULL
    };
    char path[1024];
    int i;

    if (!out_iso || !data_dir || !data_dir[0]) return 0;
    memset(out_iso, 0, sizeof(*out_iso));
    if (strlen(data_dir) >= 4U &&
        strcmp(data_dir + strlen(data_dir) - 4U, ".cue") == 0) {
        return nexus_iso_open_cue(out_iso, data_dir) > 0;
    }
    for (i = 0; cue_names[i]; ++i) {
        snprintf(path, sizeof(path), "%s/%s", data_dir, cue_names[i]);
        if (nexus_iso_open_cue(out_iso, path) > 0) return 1;
    }
    return 0;
}

static void test_retail_map_requires_ff_ff_terminator(void) {
    static const unsigned char sal_data[] = { 0 };
    unsigned char malformed_map[26];
    Nexus_SoundEngine sound;
    Nexus_SoundMapWindow window;

    memset(malformed_map, 0, sizeof(malformed_map));
    /* One observed-shaped record followed by a false FF 00 terminator. */
    malformed_map[0] = 0x20;
    malformed_map[1] = 0x00;
    malformed_map[2] = 0xb0;
    malformed_map[6] = 0x05;
    malformed_map[7] = 0x40;
    malformed_map[24] = 0xff;
    malformed_map[25] = 0x00;

    CHECK(nexus_sound_init(&sound) == 0,
          "malformed retail MAP sound engine initializes");
    CHECK(nexus_sound_load_canonical_level(&sound, 0, sal_data,
                                            (int)sizeof(sal_data),
                                            malformed_map,
                                            (int)sizeof(malformed_map), 1, 1) == 0,
          "malformed retail MAP loads as source bytes");
    CHECK(sound.map_record_table_supported == 0,
          "retail MAP rejects FF xx as a terminator");
    CHECK(nexus_sound_map_lookup_raw_selector(&sound, 0x20, &window) == -1,
          "malformed retail MAP cannot expose a partial window");
    nexus_sound_shutdown(&sound);
}

static void test_minimal_retail_map_has_no_legacy_header_requirement(void) {
    static const unsigned char sal_data[] = { 0x7f };
    static const unsigned char retail_map[] = {
        0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
        0xff, 0xff
    };
    Nexus_SoundEngine sound;

    CHECK(nexus_sound_init(&sound) == 0,
          "minimal retail MAP sound engine initializes");
    CHECK(nexus_sound_load_canonical_level(&sound, 0, sal_data,
                                            (int)sizeof(sal_data),
                                            retail_map,
                                            (int)sizeof(retail_map), 1, 1) == 0,
          "minimal retail MAP loads without a legacy header");
    CHECK(sound.map_record_table_supported && sound.map_record_count == 1,
          "minimal retail MAP exposes its byte-zero record");
    CHECK(sound.map_records[0].selector == 0x20 &&
          sound.map_records[0].data_id == 2 &&
          sound.map_records[0].sal_offset == 0 &&
          sound.map_records[0].sal_size == 1,
          "minimal retail MAP preserves its bounded opaque window");
    nexus_sound_shutdown(&sound);
}

int main(void) {
    const char *data_dir = getenv("FIRESTAFF_NEXUS_DATA_DIR");
    Nexus_ISOReader iso;
    int iso_opened;
    int level;
    int record_total = 0;

    if (!data_dir || !data_dir[0]) {
        fprintf(stderr, "SKIP: FIRESTAFF_NEXUS_DATA_DIR is not configured\\n");
        return 77;
    }

    iso_opened = open_retail_iso_if_present(data_dir, &iso);
    asset_scan_cache_batch_begin();

    for (level = 0; level < 16; ++level) {
        unsigned char *sal_data;
        unsigned char *map_data;
        int sal_size;
        int map_size;
        int record;
        Nexus_SoundEngine sound;
        Nexus_SfxRuntimeReceipt runtime_receipt;

        char sal_name[32];
        char map_name[32];
        char slev_name[32];
        char ignored_path[ASSET_PATH_MAX];
        snprintf(sal_name, sizeof(sal_name), "SNDLEV%02d.SAL", level);
        snprintf(map_name, sizeof(map_name), "SNDLEV%02d.MAP", level);
        snprintf(slev_name, sizeof(slev_name), "SLEV%02d.BIN", level);
        sal_data = read_retail_member(data_dir, iso_opened ? &iso : NULL,
                                      sal_name, &sal_size);
        map_data = read_retail_member(data_dir, iso_opened ? &iso : NULL,
                                      map_name, &map_size);
        CHECK(sal_data != NULL && map_data != NULL, "retail SAL/MAP pair opens");
        if (!sal_data || !map_data) {
            free(sal_data);
            free(map_data);
            continue;
        }

        CHECK(asset_find_by_md5(data_dir, sal_md5[level], ignored_path,
                                (int)sizeof(ignored_path), 8) &&
                  asset_find_by_md5(data_dir, map_md5[level], ignored_path,
                                    (int)sizeof(ignored_path), 8),
              "retail SAL/MAP pair matches the authenticated level identity");
        {
            CHECK(asset_find_by_md5(data_dir, slev_md5[level], ignored_path,
                                    (int)sizeof(ignored_path), 8),
                  "retail SLEV matches the authenticated level identity");
        }

        CHECK(nexus_sound_init(&sound) == 0, "sound engine initializes");
        CHECK(nexus_sound_load_canonical_level(&sound, level,
                                                sal_data, sal_size,
                                                map_data, map_size, 1, 1) == 0,
              "retail SAL/MAP pair loads");
        memset(&runtime_receipt, 0, sizeof(runtime_receipt));
        CHECK(nexus_sound_level_runtime_receipt(&sound,
                                                &runtime_receipt) == 0,
              "retail SAL/MAP runtime receipt emits");
        CHECK(runtime_receipt.sal_canonical_source_verified == 1 &&
              runtime_receipt.map_canonical_source_verified == 1 &&
              runtime_receipt.event_dispatch_source_verified == 0,
              "retail SAL/MAP metadata is ready while event dispatch stays gated");
        CHECK(sound.map_record_table_supported,
              "retail MAP has a terminated record table");
        CHECK(sound.map_record_count > 0 &&
              sound.map_record_count <= NEXUS_SFX_MAP_MAX_RECORDS,
              "retail MAP record count stays bounded");
        CHECK(sound.sal_tone_bank_directory_supported == 1 &&
              sound.sal_tone_entry_count > 4 &&
              sound.sal_tone_entry_count_decoded == sound.sal_tone_entry_count,
              "retail SAL DataID 0 tone directory and every entry decode");
        CHECK(sound.sal_tone_pcm8_count + sound.sal_tone_pcm16_count ==
                  sound.sal_tone_entry_count - 4 &&
              sound.sal_tone_sample_payload_bytes > 0,
              "retail SAL tone metadata preserves 8/16-bit source payload");
        CHECK(sound.sal_decode_ready == 1 &&
                  sound.sal_decoded_tone_count == sound.sal_tone_entry_count,
              "retail SAL tone bank decodes into the bounded PCM diagnostic cache");
        {
            int decoded_samples = 0;
            int tone;
            for (tone = 4; tone < sound.sal_tone_entry_count; ++tone) {
                if (sound.sal_decoded_sample_count[tone] > 0) {
                    CHECK(sound.sal_decoded_samples[tone] != NULL,
                          "decoded retail SAL memory source owns PCM samples");
                    CHECK(sound.sal_decoded_sample_count[tone] <= 65536,
                          "decoded retail SAL sample count stays bounded");
                    decoded_samples++;
                } else {
                    CHECK(sound.sal_decoded_samples[tone] == NULL,
                          "retail SAL noise source has no synthetic PCM payload");
                }
            }
            CHECK(decoded_samples == sound.sal_tone_memory_source_count,
                  "decoded retail SAL sample slots match source descriptors");
        }
        record_total += sound.map_record_count;
        CHECK(sound.map_out_of_bounds_record_count == 0,
              "every retail MAP window is inside its SAL bank");
        CHECK(sound.map_record_terminator_offset ==
              sound.map_record_count * 8,
              "retail MAP terminator follows complete eight-byte records");

        for (record = 0; record < sound.map_record_count; ++record) {
            Nexus_SoundMapWindow window;
            const Nexus_SoundMapWindow *stored = &sound.map_records[record];
            if (stored->data_id == 0) {
                CHECK(nexus_sound_map_lookup_raw_selector(&sound, stored->selector,
                                                           &window) == 0,
                      "retail DataID 0 selector resolves to a bounded MAP window");
                CHECK(window.selector == stored->selector &&
                      window.attribute == stored->attribute &&
                      window.sal_offset == stored->sal_offset &&
                      window.sal_size == stored->sal_size,
                      "retail DataID 0 route preserves MAP fields");
            }
        }
        {
            Nexus_SoundMapWindow window;
            CHECK(nexus_sound_map_lookup_raw_selector(&sound, 0xff, &window) == -1,
                  "missing selector does not synthesize a SAL window");
        }

        nexus_sound_shutdown(&sound);
        free(sal_data);
        free(map_data);
    }

    asset_scan_cache_batch_end();
    if (iso_opened) nexus_iso_close(&iso);

    CHECK(record_total == 154,
          "retail SNDLEV00-15 MAP corpus exposes 154 bounded records");

    test_retail_map_requires_ff_ff_terminator();
    test_minimal_retail_map_has_no_legacy_header_requirement();

    return failures ? 1 : 0;
}
