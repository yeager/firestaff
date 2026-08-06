#include "theron_v1_world.h"
#include "theron_v1_track02.h"
#include "theron_v1_track02_level_data_blocks.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *resolve_path(const char *env_name, const char *name,
                                char *fallback, size_t fallback_size) {
    const char *value = getenv(env_name);
    const char *home = getenv("HOME");
    if (value && value[0]) return value;
    if (!home || !home[0] || !fallback || fallback_size == 0u) return NULL;
    if (snprintf(fallback, fallback_size, "%s/.firestaff/data/theron/%s",
                 home, name) < 0) return NULL;
    return fallback;
}

static int load_user_data(const char *path, uint8_t **out_data,
                          size_t *out_size) {
    FILE *file = NULL;
    long file_size;
    size_t sectors;
    uint8_t *data;

    if (!path || !out_data || !out_size ||
        !(file = fopen(path, "rb")) || fseek(file, 0L, SEEK_END) != 0 ||
        (file_size = ftell(file)) <= 0L || (file_size % 2352L) != 0L ||
        fseek(file, 0L, SEEK_SET) != 0) {
        if (file) fclose(file);
        return 0;
    }
    sectors = (size_t)file_size / 2352u;
    data = calloc(sectors, 2048u);
    if (!data) {
        fclose(file);
        return 0;
    }
    for (size_t sector = 0u; sector < sectors; ++sector) {
        if (fseek(file, (long)(sector * 2352u + 16u), SEEK_SET) != 0 ||
            fread(data + sector * 2048u, 1u, 2048u, file) != 2048u) {
            free(data);
            fclose(file);
            return 0;
        }
    }
    fclose(file);
    *out_data = data;
    *out_size = sectors * 2048u;
    return 1;
}

static void verify_variant(const char *env_name, const char *file_name,
                           int variant, const char *label) {
    char fallback[512];
    const char *path = resolve_path(env_name, file_name, fallback,
                                    sizeof(fallback));
    uint8_t *user_data = NULL;
    size_t user_data_size = 0u;
    Theron_V1_World world;
    Theron_RuntimeMediaIdentity identity = {0};

    if (!path || !load_user_data(path, &user_data, &user_data_size)) {
        printf("SKIP: %s Track 02 runtime level receipt data unavailable\n",
               label);
        return;
    }
    theron_v1_world_init(&world);
    world.runtime_media.restored = 1;
    identity.ready = 1;
    identity.track02_variant = variant;
    identity.bank_stride = 0x0400u;
    assert(theron_v1_world_runtime_media_set_identity(&world, &identity));

    for (unsigned int level = 0u; level < THERON_TRACK02_LEVEL_COUNT; ++level) {
        assert(theron_v1_world_runtime_media_bind_level_data_block(
            &world, user_data, user_data_size, variant, level));
        assert(world.runtime_media.later_level_data.ready);
        assert(world.runtime_media.later_level_data.no_semantic_promotion);
        assert(world.runtime_media.later_level_data.track02_variant == variant);
        assert(world.runtime_media.later_level_data.level == level);
        assert(world.runtime_media.later_level_data.compressed_bytes > 0u);
        assert(world.runtime_media.later_level_data.compressed_fnv1a != 0u);
        assert(world.runtime_media.later_level_data.shared_prologue_fnv1a != 0u);
    }

    {
        Theron_RuntimeLevelDataBlockReceipt retained =
            world.runtime_media.later_level_data;
        const uint8_t original = user_data[retained.block_ud_offset];
        user_data[retained.block_ud_offset] ^= 1u;
        assert(!theron_v1_world_runtime_media_bind_level_data_block(
            &world, user_data, user_data_size, variant, retained.level));
        user_data[retained.block_ud_offset] = original;
        assert(memcmp(&world.runtime_media.later_level_data, &retained,
                      sizeof(retained)) == 0);
    }
    free(user_data);
    printf("PASS: authentic %s Track 02 level receipts bound to runtime\n",
           label);
}

int main(void) {
    verify_variant("FIRESTAFF_THERON_TRACK02_RAW", "TQUS02.bin",
                   THERON_TRACK02_VARIANT_US_BIN, "US");
    verify_variant("FIRESTAFF_THERON_TRACK02_JP_RAW", "TQJP02.bin",
                   THERON_TRACK02_VARIANT_JP_BIN, "JP");
    puts("PASS: theron_v1_runtime_level_data_receipt");
    return 0;
}
