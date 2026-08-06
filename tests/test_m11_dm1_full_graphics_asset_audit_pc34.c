#include "asset_loader_m11.h"
#include "graphics_dat_entry_classify_pc34_compat.h"
#include "memory_graphics_dat_state_pc34_compat.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *graphics_path(void)
{
    const char *path = getenv("FIRESTAFF_DM1_GRAPHICS_DAT");
    static char from_dir[1024];
    static char from_data_dir[1024];
    const char *data_dir;
    FILE *probe;

    if (path && path[0]) {
        return path;
    }
    data_dir = getenv("FIRESTAFF_DM1_DATA_DIR");
    if (!data_dir || !data_dir[0]) {
        return NULL;
    }
    (void)snprintf(from_dir, sizeof(from_dir), "%s/GRAPHICS.DAT", data_dir);
    probe = fopen(from_dir, "rb");
    if (probe) {
        fclose(probe);
        return from_dir;
    }
    /* The scanner and packaged DOS archives commonly expose the install
     * root, while PC34 keeps the two runtime files under DATA/. Accept both
     * layouts so the real-corpus audit cannot fail on directory shape. */
    (void)snprintf(from_data_dir, sizeof(from_data_dir),
                   "%s/DATA/GRAPHICS.DAT", data_dir);
    return from_data_dir;
}

static uint64_t fnv1a_update(uint64_t hash, const unsigned char *bytes,
                             size_t count)
{
    size_t i;
    for (i = 0u; i < count; ++i) {
        hash ^= (uint64_t)bytes[i];
        hash *= UINT64_C(1099511628211);
    }
    return hash;
}

int main(void)
{
    const char *path = graphics_path();
    M11_AssetLoader loader;
    const struct MemoryGraphicsDatRuntimeState_Compat *runtime;
    uint64_t pixel_digest = UINT64_C(1469598103934665603);
    unsigned int bitmap_safe = 0u;
    unsigned int bitmap_suspicious = 0u;
    unsigned int special = 0u;
    unsigned int empty = 0u;
    unsigned int zero_sized = 0u;
    unsigned int i;

    if (!path) {
        puts("SKIP: set FIRESTAFF_DM1_GRAPHICS_DAT or FIRESTAFF_DM1_DATA_DIR");
        return 0;
    }
    if (!M11_AssetLoader_Init(&loader, path)) {
        fprintf(stderr, "DM1 PC34 GRAPHICS.DAT could not be opened: %s\n", path);
        return 1;
    }
    runtime = (const struct MemoryGraphicsDatRuntimeState_Compat *)loader.runtimeState;
    if (!runtime || !runtime->initialized || runtime->graphicCount == 0u) {
        fprintf(stderr, "DM1 PC34 GRAPHICS.DAT has no initialized record table\n");
        M11_AssetLoader_Shutdown(&loader);
        return 1;
    }

    /* DM1 PC 3.4 has 713 records.  Keep this assertion tied to the canonical
     * corpus: another PC34-compatible source can still be audited while
     * reporting a useful failure instead of silently calling it canonical. */
    if (runtime->graphicCount != 713u) {
        fprintf(stderr, "unexpected DM1 PC34 record count: %u (expected 713)\n",
                (unsigned)runtime->graphicCount);
        M11_AssetLoader_Shutdown(&loader);
        return 1;
    }

    for (i = 0u; i < runtime->graphicCount; ++i) {
        struct GraphicsDatEntryClassificationResult_Compat classification;
        unsigned short width = 0u;
        unsigned short height = 0u;
        const M11_AssetSlot *slot;

        if (!F9012_RUNTIME_ClassifyGraphicsDatEntry_Compat(
                runtime, i, &classification)) {
            fprintf(stderr, "record %u could not be classified\n", i);
            M11_AssetLoader_Shutdown(&loader);
            return 1;
        }
        if (i == 696u &&
            classification.kind != GRAPHICS_DAT_ENTRY_SPECIAL_NON_BITMAP) {
            fprintf(stderr,
                    "C696_GRAPHIC_LAYOUT entered the bitmap classifier kind=%d\n",
                    (int)classification.kind);
            M11_AssetLoader_Shutdown(&loader);
            return 1;
        }
        if (classification.shouldUseBitmapPath) {
            if (!M11_AssetLoader_QuerySize(&loader, i, &width, &height) ||
                width == 0u || height == 0u) {
                fprintf(stderr, "bitmap record %u has invalid dimensions kind=%d comp=%u dec=%u\n",
                        i, (int)classification.kind,
                        (unsigned)runtime->compressedByteCounts[i],
                        (unsigned)runtime->decompressedByteCounts[i]);
                M11_AssetLoader_Shutdown(&loader);
                return 1;
            }
            slot = M11_AssetLoader_Load(&loader, i);
            if (!slot || !slot->pixels || slot->width != width ||
                slot->height != height) {
                fprintf(stderr, "bitmap record %u did not decode (%ux%u)\n",
                        i, (unsigned)width, (unsigned)height);
                M11_AssetLoader_Shutdown(&loader);
                return 1;
            }
            pixel_digest = fnv1a_update(
                pixel_digest, slot->pixels,
                (size_t)slot->width * (size_t)slot->height);
            pixel_digest = fnv1a_update(
                pixel_digest, (const unsigned char *)&slot->width,
                sizeof(slot->width));
            pixel_digest = fnv1a_update(
                pixel_digest, (const unsigned char *)&slot->height,
                sizeof(slot->height));
            if (classification.kind == GRAPHICS_DAT_ENTRY_BITMAP_SUSPICIOUS) {
                ++bitmap_suspicious;
            } else {
                ++bitmap_safe;
            }
        } else {
            if (M11_AssetLoader_Load(&loader, i) != NULL) {
                fprintf(stderr, "non-bitmap record %u entered bitmap cache\n", i);
                M11_AssetLoader_Shutdown(&loader);
                return 1;
            }
            switch (classification.kind) {
            case GRAPHICS_DAT_ENTRY_SPECIAL_NON_BITMAP:
                ++special;
                break;
            case GRAPHICS_DAT_ENTRY_EMPTY:
                ++empty;
                break;
            case GRAPHICS_DAT_ENTRY_ZERO_SIZED_PLACEHOLDER:
                ++zero_sized;
                break;
            default:
                fprintf(stderr, "record %u has invalid non-bitmap kind %d\n",
                        i, (int)classification.kind);
                M11_AssetLoader_Shutdown(&loader);
                return 1;
            }
        }
    }

    M11_AssetLoader_Shutdown(&loader);
    if (bitmap_safe != 543u || bitmap_suspicious != 0u || special != 35u ||
        empty != 4u || zero_sized != 131u) {
        fprintf(stderr,
                "unexpected DM1 PC34 class census bitmap=%u suspicious=%u special=%u empty=%u zero-sized=%u\n",
                bitmap_safe, bitmap_suspicious, special, empty, zero_sized);
        return 1;
    }
    printf("ok: DM1 PC34 audited 713 records; bitmap=%u suspicious=%u special=%u empty=%u zero-sized=%u pixel-digest=%016llx\n",
           bitmap_safe, bitmap_suspicious, special, empty, zero_sized,
           (unsigned long long)pixel_digest);
    return 0;
}
