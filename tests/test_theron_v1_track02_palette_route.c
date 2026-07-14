#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "theron_v1_dungeon_handoff.h"
#include "theron_v1_track02_palette_route.h"

static int failures;

#define CHECK(condition) do { \
    if (!(condition)) { \
        fprintf(stderr, "failed: %s (%s:%d)\n", #condition, __FILE__, __LINE__); \
        ++failures; \
    } \
} while (0)

static unsigned char *read_file(const char *path, size_t *out_bytes) {
    FILE *file;
    long bytes;
    unsigned char *data;

    if (!path || !out_bytes || !(file = fopen(path, "rb"))) return NULL;
    if (fseek(file, 0L, SEEK_END) != 0 || (bytes = ftell(file)) <= 0 ||
        fseek(file, 0L, SEEK_SET) != 0) {
        fclose(file);
        return NULL;
    }
    data = malloc((size_t)bytes);
    if (!data || fread(data, 1u, (size_t)bytes, file) != (size_t)bytes) {
        free(data);
        fclose(file);
        return NULL;
    }
    fclose(file);
    *out_bytes = (size_t)bytes;
    return data;
}

int main(void) {
    Theron_V1Track02PaletteTraceReceipt trace = {1, 1, 1};
    Theron_V1Track02PaletteStoreReceipt stores[] = {
        {1, 0x4120u, 0x0402u, 0x06u, THERON_V1_TRACK02_PALETTE_STORE_INDEX},
        {1, 0x4123u, 0x0403u, 0x2au, THERON_V1_TRACK02_PALETTE_STORE_LOW},
        {1, 0x4126u, 0x0404u, 0x01u, THERON_V1_TRACK02_PALETTE_STORE_HIGH}
    };
    Theron_V1Track02PaletteRouteFacts facts = {0};
    Theron_V1Track02PaletteRouteReceipt receipt;
    unsigned char fake_sector[THERON_V1_TRACK02_RAW_SECTOR_BYTES] = {0};
    unsigned char *real_track02;
    size_t real_track02_bytes;
    const char *real_path = getenv("FIRESTAFF_THERON_TRACK02_RAW");
    const char *real_md5 = getenv("FIRESTAFF_THERON_TRACK02_RAW_MD5");

    facts.raw_track02 = fake_sector;
    facts.raw_track02_bytes = sizeof(fake_sector);
    facts.raw_track02_md5 = "f23601102138f87c33025877767ebf76";
    facts.trace_receipt = &trace;
    facts.stores = stores;
    facts.store_count = sizeof(stores) / sizeof(stores[0]);
    CHECK(!theron_v1_track02_palette_route_validate(&facts, &receipt));
    CHECK(!receipt.accepted);
    CHECK(!receipt.render_allowed);
    CHECK(receipt.status == NULL);

    real_track02 = read_file(real_path, &real_track02_bytes);
    if (real_track02 && real_md5) {
        facts.raw_track02 = real_track02;
        facts.raw_track02_bytes = real_track02_bytes;
        facts.raw_track02_md5 = real_md5;
        CHECK(theron_v1_track02_palette_route_validate(&facts, &receipt));
        CHECK(receipt.accepted);
        CHECK(receipt.real_cd_verified);
        CHECK(!receipt.render_allowed);
        CHECK(strcmp(receipt.status,
                     "authenticated_direct_vce_store_route_render_blocked") == 0);

        trace.dynamic_cd_read_authenticated = 0;
        CHECK(!theron_v1_track02_palette_route_validate(&facts, &receipt));
        CHECK(!receipt.accepted);
        trace.dynamic_cd_read_authenticated = 1;
        stores[2].address = 0x0406u;
        CHECK(!theron_v1_track02_palette_route_validate(&facts, &receipt));
        CHECK(!receipt.accepted);
        stores[2].address = 0x0404u;
        free(real_track02);
    }

    return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
