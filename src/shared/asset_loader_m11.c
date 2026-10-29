/*
 * asset_loader_m11.c — GRAPHICS.DAT asset loader and blitter for M11.
 *
 * Uses the M10 IMG3 pipeline to decompress graphic entries from
 * GRAPHICS.DAT into unpacked 1-byte-per-pixel buffers.
 *
 * The IMG3 expand path works on packed nibbles (4-bit, two pixels per
 * byte, high nibble first). After expansion, we unpack to 1 byte per
 * pixel for trivial framebuffer blitting.
 */

#include "asset_loader_m11.h"
#include "memory_graphics_dat_pc34_compat.h"
#include "memory_graphics_dat_state_pc34_compat.h"
#include "memory_graphics_dat_select_pc34_compat.h"
#include "memory_graphics_dat_header_pc34_compat.h"
#include "memory_frontend_pc34_compat.h"
#include "graphics_dat_entry_classify_pc34_compat.h"
#include "dm1_v1_legacy_graphics_dat.h"
#include "dm1_v1_fmtowns_pic_library.h"
#include "dm1_v1_iigs_graphics_dat.h"
#include "dm1_v1_amiga_graphics_dat.h"
#include "dm1_v1_atari_st_graphics_dat.h"
#include "csb_v1_amiga_graphics_dat.h"
#include "csb_v1_fmtowns_graphics_dat.h"
#include "csb_v1_graphics_atari_st_loader_pc34_compat.h"
#include "asset_find_by_hash.h"

#include <stdlib.h>
#include <stdint.h>
#include <limits.h>
#include <string.h>
#include <stdio.h>

/* IMG3 global state used by the decompressor */
extern unsigned short G2157_;
extern unsigned char* G2159_puc_Bitmap_Source;
extern unsigned char* G2160_puc_Bitmap_Destination;

/* --- lifecycle --- */

int M11_AssetLoader_Init(M11_AssetLoader* loader, const char* graphicsDatPath) {
    struct MemoryGraphicsDatState_Compat* fileState;
    struct MemoryGraphicsDatRuntimeState_Compat* runtimeState;

    if (!loader || !graphicsDatPath || graphicsDatPath[0] == '\0') {
        return 0;
    }
    /* A scanner may hand us a verified archive member.  The legacy
     * F0477 file-state path deliberately models a seekable FILE*, whereas
     * the matching FromBuffer initialization path owns identical source
     * bytes in RAM.  Use that path for virtual members; never extract an
     * archive member to disk just to make it seekable. */
    if (strstr(graphicsDatPath, "::")) {
        uint8_t* data = NULL;
        size_t byteCount = 0u;
        int ok;
        if (!asset_read_path_alloc(graphicsDatPath, &data, &byteCount) ||
            !data || byteCount == 0u || byteCount > (size_t)LONG_MAX) {
            free(data);
            return 0;
        }
        ok = M11_AssetLoader_InitFromBuffer(loader, data, (long)byteCount);
        free(data);
        if (ok) {
            snprintf(loader->graphicsDatPath, sizeof(loader->graphicsDatPath),
                     "%s", graphicsDatPath);
        }
        return ok;
    }
    memset(loader, 0, sizeof(*loader));
    snprintf(loader->graphicsDatPath, sizeof(loader->graphicsDatPath), "%s", graphicsDatPath);

    fileState = (struct MemoryGraphicsDatState_Compat*)calloc(
        1, sizeof(struct MemoryGraphicsDatState_Compat));
    runtimeState = (struct MemoryGraphicsDatRuntimeState_Compat*)calloc(
        1, sizeof(struct MemoryGraphicsDatRuntimeState_Compat));
    if (!fileState || !runtimeState) {
        free(fileState);
        free(runtimeState);
        return 0;
    }

    if (!F0479_MEMORY_InitializeGraphicsDatState_Compat(
            graphicsDatPath, fileState, runtimeState)) {
        free(fileState);
        free(runtimeState);
        return 0;
    }

    /* Reopen the file for subsequent graphic loads — the header loading
     * path closes the file handle after reading the header. */
    if (!F0477_MEMORY_OpenGraphicsDat_CPSDF_Compat(graphicsDatPath, fileState)) {
        F0479_MEMORY_FreeGraphicsDatState_Compat(runtimeState);
        free(fileState);
        free(runtimeState);
        return 0;
    }

    loader->fileState = fileState;
    loader->runtimeState = runtimeState;
    loader->graphicCount = runtimeState->graphicCount;
    loader->initialized = 1;
    return 1;
}

int M11_AssetLoader_InitDecodedOnly(M11_AssetLoader* loader,
                                    const char* sourcePath) {
    if (!loader || !sourcePath || sourcePath[0] == '\0') {
        return 0;
    }
    memset(loader, 0, sizeof(*loader));
    snprintf(loader->graphicsDatPath, sizeof(loader->graphicsDatPath), "%s",
             sourcePath);
    loader->initialized = 1;
    return 1;
}

int M11_AssetLoader_InitFromBuffer(M11_AssetLoader* loader,
                                   const unsigned char *data, long size) {
    struct MemoryGraphicsDatState_Compat* fileState;
    struct MemoryGraphicsDatRuntimeState_Compat* runtimeState;
    unsigned char *ownedBuffer;

    if (!loader || !data || size <= 0) {
        return 0;
    }
    memset(loader, 0, sizeof(*loader));
    snprintf(loader->graphicsDatPath, sizeof(loader->graphicsDatPath),
             "(in-memory buffer, %ld bytes)", size);

    fileState = (struct MemoryGraphicsDatState_Compat*)calloc(
        1, sizeof(struct MemoryGraphicsDatState_Compat));
    runtimeState = (struct MemoryGraphicsDatRuntimeState_Compat*)calloc(
        1, sizeof(struct MemoryGraphicsDatRuntimeState_Compat));
    if (!fileState || !runtimeState) {
        free(fileState);
        free(runtimeState);
        return 0;
    }
    ownedBuffer = (unsigned char *)malloc((size_t)size);
    if (!ownedBuffer) {
        free(fileState);
        free(runtimeState);
        return 0;
    }
    memcpy(ownedBuffer, data, (size_t)size);

    if (!F0479_MEMORY_InitializeGraphicsDatState_FromBuffer_Compat(
            ownedBuffer, size, fileState, runtimeState)) {
        free(ownedBuffer);
        free(fileState);
        free(runtimeState);
        return 0;
    }

    if (!F0477_MEMORY_OpenGraphicsDat_FromBuffer_Compat(ownedBuffer, size, fileState)) {
        F0479_MEMORY_FreeGraphicsDatState_Compat(runtimeState);
        free(ownedBuffer);
        free(fileState);
        free(runtimeState);
        return 0;
    }

    loader->fileState = fileState;
    loader->runtimeState = runtimeState;
    loader->ownedBuffer = ownedBuffer;
    loader->graphicCount = runtimeState->graphicCount;
    loader->initialized = 1;
    {
        DM1_V1_AmigaGraphicsReceipt amigaReceipt;
        loader->dm1Amiga36 =
            dm1_v1_amiga_graphics_receipt(data, (size_t)size,
                                          &amigaReceipt) == 0 &&
            amigaReceipt.version == DM1_AMIGA_VER_3_6;
    }
    return 1;
}

int M11_AssetLoader_InitDm1LegacyFromBuffer(M11_AssetLoader* loader,
                                             const unsigned char *data,
                                             long size, int big_endian) {
    unsigned char *copy;
    if (!loader || !data || size <= 0 ||
        !dm1_v1_legacy_graphics_probe(data, (size_t)size, big_endian)) return 0;
    copy = (unsigned char *)malloc((size_t)size);
    if (!copy) return 0;
    memcpy(copy, data, (size_t)size);
    memset(loader, 0, sizeof(*loader));
    loader->legacyData = copy;
    loader->legacyDataSize = size;
    loader->legacyBigEndian = big_endian ? 1 : 0;
    loader->legacyDm1 = 1;
    loader->graphicCount = 575u;
    loader->initialized = 1;
    snprintf(loader->graphicsDatPath, sizeof(loader->graphicsDatPath),
             "(DM1 legacy %s, %ld bytes)", big_endian ? "Amiga" : "FM Towns", size);
    return 1;
}

int M11_AssetLoader_InitDm1LegacyFromFile(M11_AssetLoader* loader,
                                           const char *graphicsDatPath,
                                           int big_endian) {
    uint8_t *data = NULL;
    size_t byte_count = 0u;
    int ok;
    if (!loader || !graphicsDatPath || !graphicsDatPath[0] ||
        !asset_read_path_alloc(graphicsDatPath, &data, &byte_count) ||
        !data || byte_count == 0u || byte_count > (size_t)LONG_MAX) {
        free(data);
        return 0;
    }
    ok = M11_AssetLoader_InitDm1LegacyFromBuffer(loader, data,
                                                  (long)byte_count, big_endian);
    free(data);
    if (ok) {
        snprintf(loader->graphicsDatPath, sizeof(loader->graphicsDatPath),
                 "%s", graphicsDatPath);
    }
    return ok;
}

int M11_AssetLoader_InitDm1FmtownsFromBuffer(M11_AssetLoader* loader,
                                             const unsigned char *data,
                                             long size) {
    dm1_v1_fmtowns_pic_library_view_t view;
    unsigned char *copy;
    unsigned char *legacyCopy;

    if (!loader || !data || size <= 0 ||
        dm1_v1_fmtowns_pic_library_open_pc34(data, (size_t)size, &view) !=
            DM1_V1_FMTOWNS_PIC_LIB_OK || view.asset_count != 575u ||
        !dm1_v1_fmtowns_pic_library_tables_are_mirror_pc34(&view)) {
        return 0;
    }
    copy = (unsigned char *)malloc((size_t)size);
    legacyCopy = (unsigned char *)malloc((size_t)size);
    if (!copy || !legacyCopy) {
        free(copy);
        free(legacyCopy);
        return 0;
    }
    memcpy(copy, data, (size_t)size);
    memcpy(legacyCopy, data, (size_t)size);
    /* A platform handoff can replace an already initialized loader. Keep
     * its cached surfaces and native-buffer ownership from leaking. The
     * replacement bytes are copied first, so `data` may safely alias the
     * old loader's storage. */
    M11_AssetLoader_Shutdown(loader);
    memset(loader, 0, sizeof(*loader));
    loader->dm1FmtownsData = copy;
    loader->dm1FmtownsDataSize = size;
    loader->dm1Fmtowns = 1;
    /* Several renderer and gameplay paths use this flag for classic DM1
     * semantics. Load/QuerySize dispatch the FM Towns format before the
     * generic legacy decoder, so its bytes cannot be decoded as IMG2. */
    loader->legacyData = legacyCopy;
    loader->legacyDataSize = size;
    loader->legacyDm1 = 1;
    loader->graphicCount = view.asset_count;
    loader->initialized = 1;
    snprintf(loader->graphicsDatPath, sizeof(loader->graphicsDatPath),
             "(DM1 FM Towns picture library, %ld bytes)", size);
    return 1;
}

/* Bound the resident original GS bank independently of a caller's long size.
 * The two native editions use 398713 bytes; this is a resource cap, not an
 * edition admission check. The explicit profile below also authenticates
 * the shared original G20/G21 graphics bank; executable ownership remains
 * the responsibility of a future complete platform profile. */
#define M11_DM1_IIGS_MAX_BANK_BYTES (1024U * 1024U)

int M11_AssetLoader_InitDm1IigsFromBuffer(M11_AssetLoader* loader,
                                         const unsigned char *data, long size) {
    unsigned char *copy;
    if (!loader || !data || size<=0 ||
        (unsigned long)size>M11_DM1_IIGS_MAX_BANK_BYTES ||
        size!=398713L ||
        !asset_memory_matches_md5(data,(size_t)size,
                                   "1045a36952e64eb2bc7b4c7b2965b112") ||
        !dm1_v1_iigs_graphics_probe(data,(size_t)size)) return 0;
    copy=(unsigned char *)malloc((size_t)size);
    if (!copy) return 0;
    memcpy(copy,data,(size_t)size);
    memset(loader,0,sizeof(*loader));
    loader->iigsData=copy;
    loader->iigsDataSize=size;
    loader->iigsDm1=1;
    loader->graphicCount=575U;
    loader->initialized=1;
    snprintf(loader->graphicsDatPath,sizeof(loader->graphicsDatPath),
             "(DM1 Apple IIGS, %ld bytes)",size);
    return 1;
}

int M11_AssetLoader_InitDm1IigsFromFile(M11_AssetLoader* loader,
                                       const char *graphicsDatPath) {
    uint8_t *data=NULL;
    size_t byte_count=0U;
    int ok;
    if (!loader || !graphicsDatPath || !graphicsDatPath[0] ||
        !asset_read_path_alloc(graphicsDatPath,&data,&byte_count) ||
        !data || byte_count==0U || byte_count>M11_DM1_IIGS_MAX_BANK_BYTES) {
        free(data);
        return 0;
    }
    ok=M11_AssetLoader_InitDm1IigsFromBuffer(loader,data,(long)byte_count);
    free(data);
    if (ok) snprintf(loader->graphicsDatPath,sizeof(loader->graphicsDatPath),
                     "%s",graphicsDatPath);
    return ok;
}

int M11_AssetLoader_InitDm1AtariStFromBuffer(M11_AssetLoader* loader,
                                             const unsigned char *data,
                                             long size) {
    DM1_V1_AtariStGraphicsDat probe;
    unsigned char *copy;
    if (!loader || !data || size <= 0 ||
        !dm1_v1_atari_st_graphics_open(data, (size_t)size, &probe)) return 0;
    copy = (unsigned char *)malloc((size_t)size);
    if (!copy) return 0;
    memcpy(copy, data, (size_t)size);
    memset(loader, 0, sizeof(*loader));
    loader->atariStData = copy;
    loader->atariStDataSize = size;
    loader->atariStDm1 = 1;
    loader->graphicCount = DM1_V1_ATARI_ST_GRAPHICS_COUNT;
    loader->initialized = 1;
    snprintf(loader->graphicsDatPath, sizeof(loader->graphicsDatPath),
             "(DM1 Atari ST DMCSB1, %ld bytes)", size);
    return 1;
}

int M11_AssetLoader_InitDm1AtariStFromFile(M11_AssetLoader* loader,
                                           const char *graphicsDatPath) {
    uint8_t *data = NULL;
    size_t byte_count = 0u;
    int ok;
    if (!loader || !graphicsDatPath || !graphicsDatPath[0] ||
        !asset_read_path_alloc(graphicsDatPath, &data, &byte_count) ||
        !data || byte_count == 0u || byte_count > (size_t)LONG_MAX) {
        free(data);
        return 0;
    }
    ok = M11_AssetLoader_InitDm1AtariStFromBuffer(loader, data,
                                                   (long)byte_count);
    free(data);
    if (ok) {
        snprintf(loader->graphicsDatPath, sizeof(loader->graphicsDatPath),
                 "%s", graphicsDatPath);
    }
    return ok;
}

int M11_AssetLoader_InitCsbAtariStFromFile(M11_AssetLoader* loader,
                                           const char *graphicsDatPath) {
    if (!loader || !graphicsDatPath || !graphicsDatPath[0]) return 0;
    memset(loader, 0, sizeof(*loader));
    csb_atari_st_graphics_loader_init(&loader->csbAtariStGraphics);
    if (!csb_atari_st_graphics_loader_open(
            &loader->csbAtariStGraphics, graphicsDatPath) ||
        loader->csbAtariStGraphics.item_count != 563u ||
        !loader->csbAtariStGraphics.dat_bytes ||
        loader->csbAtariStGraphics.dat_byte_count == 0u) {
        csb_atari_st_graphics_loader_close(&loader->csbAtariStGraphics);
        memset(loader, 0, sizeof(*loader));
        return 0;
    }
    loader->csbAtariSt = 1;
    loader->graphicCount = loader->csbAtariStGraphics.item_count;
    loader->initialized = 1;
    snprintf(loader->graphicsDatPath, sizeof(loader->graphicsDatPath), "%s",
             graphicsDatPath);
    return 1;
}

int M11_AssetLoader_InitCsbAmigaFromFile(M11_AssetLoader* loader,
                                         const char *graphicsDatPath) {
    uint8_t *data = NULL;
    size_t byte_count = 0U;
    CSB_V1_AmigaGraphicsReceipt receipt;

    if (!loader || !graphicsDatPath || !graphicsDatPath[0] ||
        !(strstr(graphicsDatPath, "::")
              ? asset_read_virtual_path_alloc(graphicsDatPath, &data, &byte_count)
              : asset_read_path_alloc(graphicsDatPath, &data, &byte_count)) ||
        !data || byte_count == 0U || byte_count > (size_t)LONG_MAX) {
        free(data);
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    if (csb_v1_amiga_graphics_receipt(data, byte_count, &receipt) != 0 ||
        !receipt.is_amiga || receipt.item_count == 0u) {
        free(data); return 0;
    }
    memset(loader, 0, sizeof(*loader));
    loader->csbAmigaData = data;
    loader->csbAmigaDataSize = (long)byte_count;
    loader->csbAmiga = 1;
    loader->graphicCount = receipt.item_count;
    loader->initialized = 1;
    snprintf(loader->graphicsDatPath, sizeof(loader->graphicsDatPath), "%s",
             graphicsDatPath);
    return 1;
}

int M11_AssetLoader_InitCsbFmtownsFromFile(M11_AssetLoader* loader,
                                           const char *graphicsDatPath) {
    uint8_t *data = NULL;
    size_t byte_count = 0u;
    CSB_V1_FmtownsGraphicsReceipt receipt;

    if (!loader || !graphicsDatPath || !graphicsDatPath[0] ||
        !asset_read_path_alloc(graphicsDatPath, &data, &byte_count) ||
        !data || byte_count == 0u || byte_count > (size_t)LONG_MAX) {
        free(data);
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    if (csb_v1_fmtowns_graphics_receipt(data, byte_count, &receipt) != 0 ||
        !receipt.is_fmtowns || receipt.item_count == 0u) {
        free(data); return 0;
    }
    memset(loader, 0, sizeof(*loader));
    loader->csbFmtownsData = data;
    loader->csbFmtownsDataSize = (long)byte_count;
    loader->csbFmtowns = 1;
    loader->graphicCount = receipt.item_count;
    loader->initialized = 1;
    snprintf(loader->graphicsDatPath, sizeof(loader->graphicsDatPath), "%s",
             graphicsDatPath);
    return 1;
}

int M11_AssetLoader_InitCsbFmtownsFromBuffer(M11_AssetLoader* loader,
                                             const unsigned char *data,
                                             long size,
                                             const char *sourcePath) {
    CSB_V1_FmtownsGraphicsReceipt receipt;
    unsigned char *owned;

    if (!loader || !data || size <= 0 ||
        (size_t)size > CSB_FMTOWNS_GRAPHICS_MAX_SIZE) return 0;
    memset(&receipt, 0, sizeof(receipt));
    if (csb_v1_fmtowns_graphics_receipt(data, (size_t)size, &receipt) != 0 ||
        !receipt.is_fmtowns || receipt.item_count == 0u) return 0;
    owned = (unsigned char *)malloc((size_t)size);
    if (!owned) return 0;
    memcpy(owned, data, (size_t)size);
    memset(loader, 0, sizeof(*loader));
    loader->csbFmtownsData = owned;
    loader->csbFmtownsDataSize = size;
    loader->csbFmtowns = 1;
    loader->graphicCount = receipt.item_count;
    loader->initialized = 1;
    if (sourcePath && sourcePath[0]) {
        snprintf(loader->graphicsDatPath, sizeof(loader->graphicsDatPath),
                 "%s", sourcePath);
    }
    return 1;
}

void M11_AssetLoader_Shutdown(M11_AssetLoader* loader) {
    int i;
    if (!loader) {
        return;
    }
    free(loader->legacyData);
    free(loader->dm1FmtownsData);
    free(loader->iigsData);
    free(loader->ownedBuffer);
    free(loader->atariStData);
    csb_atari_st_graphics_loader_close(&loader->csbAtariStGraphics);
    free(loader->csbAmigaData);
    free(loader->csbFmtownsData);
    for (i = 0; i < M11_ASSET_CACHE_SLOTS; ++i) {
        if (loader->cache[i].loaded && loader->cache[i].pixels) {
            free(loader->cache[i].pixels);
        }
    }
    if (loader->runtimeState) {
        F0479_MEMORY_FreeGraphicsDatState_Compat(
            (struct MemoryGraphicsDatRuntimeState_Compat*)loader->runtimeState);
        free(loader->runtimeState);
    }
    if (loader->fileState) {
        struct MemoryGraphicsDatState_Compat* fs =
            (struct MemoryGraphicsDatState_Compat*)loader->fileState;
        F0478_MEMORY_CloseGraphicsDat_CPSDF_Compat(fs);
        free(fs);
    }
    memset(loader, 0, sizeof(*loader));
}

int M11_AssetLoader_IsReady(const M11_AssetLoader* loader) {
    return loader && loader->initialized;
}

int M11_AssetLoader_SetDm1Amiga36Language(M11_AssetLoader* loader,
                                         const char* languageCode) {
    int language = 0;
    if (!loader || !loader->initialized || !loader->dm1Amiga36 ||
        loader->cacheUsed != 0) return 0;
    if (languageCode && (strcmp(languageCode, "FR") == 0 ||
                         strcmp(languageCode, "fr") == 0)) language = 1;
    else if (languageCode && (strcmp(languageCode, "DE") == 0 ||
                              strcmp(languageCode, "de") == 0)) language = 2;
    loader->dm1Amiga36Language = language;
    return 1;
}

unsigned int M11_AssetLoader_ResolveDm1Amiga36GraphicIndex(
    unsigned int graphicIndex, int language) {
    static const unsigned short localized[10][3] = {
        {4u, 728u, 738u}, {6u, 729u, 739u}, {27u, 730u, 740u},
        {30u, 731u, 741u}, {31u, 732u, 742u}, {32u, 733u, 743u},
        {40u, 734u, 744u}, {694u, 735u, 745u},
        {699u, 736u, 746u}, {700u, 737u, 747u}
    };
    if (language < 1 || language > 2) return graphicIndex;
    for (size_t index = 0u; index < sizeof(localized) / sizeof(localized[0]);
         ++index) {
        if (graphicIndex == localized[index][0]) {
            return localized[index][language];
        }
    }
    return graphicIndex;
}

static unsigned int m11_resolve_dm1_amiga36_graphic_index(
    const M11_AssetLoader* loader, unsigned int graphicIndex) {
    unsigned int resolved;
    if (!loader || !loader->dm1Amiga36) return graphicIndex;
    resolved = M11_AssetLoader_ResolveDm1Amiga36GraphicIndex(
        graphicIndex, loader->dm1Amiga36Language);
    if (resolved != graphicIndex && getenv("FIRESTAFF_ASSET_TRACE")) {
        fprintf(stderr,
                "asset-language-remap game=dm1 platform=amiga-v36 requested=%u resolved=%u language=%s\n",
                graphicIndex, resolved,
                loader->dm1Amiga36Language == 1 ? "fr" : "de");
    }
    return resolved;
}

int M11_AssetLoader_QuerySize(const M11_AssetLoader* loader,
                              unsigned int graphicIndex,
                              unsigned short* outWidth,
                              unsigned short* outHeight) {
    const struct MemoryGraphicsDatRuntimeState_Compat* rt;
    if (!loader || !loader->initialized) {
        return 0;
    }
    graphicIndex = m11_resolve_dm1_amiga36_graphic_index(loader, graphicIndex);
    if (loader->iigsDm1) {
        if (graphicIndex>=575U) {
            if (outWidth) *outWidth=0;
            if (outHeight) *outHeight=0;
            return 0;
        }
        return dm1_v1_iigs_graphics_query(loader->iigsData,
            (size_t)loader->iigsDataSize,(uint16_t)graphicIndex,outWidth,outHeight);
    }
    if (loader->dm1Fmtowns) {
        dm1_v1_fmtowns_pic_library_view_t view;
        const uint8_t *asset = NULL;
        uint16_t assetSize = 0u;
        dm1_v1_fmtowns_pic_library_gfx_header_t header;
        if (graphicIndex >= loader->graphicCount ||
            dm1_v1_fmtowns_pic_library_open_pc34(
                loader->dm1FmtownsData, (size_t)loader->dm1FmtownsDataSize,
                &view) != DM1_V1_FMTOWNS_PIC_LIB_OK ||
            dm1_v1_fmtowns_pic_library_asset_bytes_pc34(
                &view, (uint16_t)graphicIndex, &asset, &assetSize) !=
                DM1_V1_FMTOWNS_PIC_LIB_OK ||
            dm1_v1_fmtowns_pic_library_parse_gfx_header_pc34(
                asset, assetSize, &header) != DM1_V1_FMTOWNS_PIC_LIB_OK) {
            return 0;
        }
        if (outWidth) *outWidth = header.width_pixels;
        if (outHeight) *outHeight = header.height_pixels;
        return 1;
    }
    if (loader->legacyDm1) {
        return dm1_v1_legacy_graphics_query(loader->legacyData,
            (size_t)loader->legacyDataSize, loader->legacyBigEndian,
            (uint16_t)graphicIndex, outWidth, outHeight);
    }
    if (loader->atariStDm1) {
        DM1_V1_AtariStGraphicsDat dat;
        uint8_t *item;
        uint16_t idx = (uint16_t)graphicIndex;
        if (graphicIndex >= DM1_V1_ATARI_ST_GRAPHICS_COUNT) return 0;
        if (!dm1_v1_atari_st_graphics_open(loader->atariStData,
                (size_t)loader->atariStDataSize, &dat)) return 0;
        if (dat.records[idx].expanded_size < 4u) return 0;
        item = (uint8_t *)malloc(dat.records[idx].expanded_size);
        if (!item) return 0;
        if (dm1_v1_atari_st_graphics_read(&dat, idx, item,
                dat.records[idx].expanded_size) < 4) {
            free(item); return 0;
        }
        if (outWidth) *outWidth = (uint16_t)(((uint16_t)item[0] << 8) | item[1]);
        if (outHeight) *outHeight = (uint16_t)(((uint16_t)item[2] << 8) | item[3]);
        free(item);
        return 1;
    }
    if (loader->csbAtariSt) {
        const CSB_AtariStLoader *dat = &loader->csbAtariStGraphics;
        uint8_t *item;
        uint8_t *pixels;
        uint16_t width = 0u, height = 0u;
        size_t capacity = 1024u * 1024u;
        int decoded;
        if (graphicIndex >= loader->graphicCount) return 0;
        item = (uint8_t *)malloc(dat->items[graphicIndex].decompressed_size);
        pixels = (uint8_t *)malloc(capacity);
        decoded = item && pixels &&
            csb_atari_st_graphics_loader_read_item(dat,
                (uint16_t)graphicIndex, item,
                dat->items[graphicIndex].decompressed_size) ==
                    (int)dat->items[graphicIndex].decompressed_size &&
            dm1_v1_legacy_graphics_decode_item(item,
                dat->items[graphicIndex].decompressed_size, 1, pixels,
                capacity, &width, &height);
        free(item);
        free(pixels);
        if (!decoded) return 0;
        if (outWidth) *outWidth = width;
        if (outHeight) *outHeight = height;
        return 1;
    }
    if (loader->csbAmiga) {
        uint16_t amigaWidth = 0u;
        uint16_t amigaHeight = 0u;
        unsigned char *amigaPixels;

        if (graphicIndex >= loader->graphicCount) return 0;
        /* ReDMCSB MEMORY.C F0474 is the owning IMG1 expansion path.  The
         * DMCSB2 table deliberately does not carry a width/height pair, so
         * query through the same bounded original decoder as Load() rather
         * than dereferencing the absent PC34 runtime metadata. */
        amigaPixels = (unsigned char *)malloc(1024u * 1024u);
        if (!amigaPixels || !csb_v1_amiga_graphics_decode_item(
                loader->csbAmigaData, (size_t)loader->csbAmigaDataSize,
                (uint16_t)graphicIndex, amigaPixels, 1024u * 1024u,
                &amigaWidth, &amigaHeight)) {
            free(amigaPixels);
            return 0;
        }
        free(amigaPixels);
        if (outWidth) *outWidth = amigaWidth;
        if (outHeight) *outHeight = amigaHeight;
        return 1;
    }
    if (loader->csbFmtowns) {
        uint8_t *pixels;
        CSB_V1_FmtownsItemDecodeReceipt receipt;

        if (graphicIndex >= loader->graphicCount) return 0;
        pixels = (uint8_t *)malloc(640u * 400u);
        if (!pixels || !csb_v1_fmtowns_graphics_decode_item(
                loader->csbFmtownsData, (size_t)loader->csbFmtownsDataSize,
                (uint16_t)graphicIndex, pixels, 640u * 400u, &receipt) ||
            !receipt.valid || !receipt.is_image) {
            free(pixels);
            return 0;
        }
        free(pixels);
        if (outWidth) *outWidth = receipt.width;
        if (outHeight) *outHeight = receipt.height;
        return 1;
    }
    rt = (const struct MemoryGraphicsDatRuntimeState_Compat*)loader->runtimeState;
    if (!rt || graphicIndex >= rt->graphicCount) {
        return 0;
    }
    if (outWidth) {
        *outWidth = rt->widthHeight[graphicIndex].Width;
    }
    if (outHeight) {
        *outHeight = rt->widthHeight[graphicIndex].Height;
    }
    return 1;
}

/* --- loading --- */

static M11_AssetSlot* m11_find_cached(M11_AssetLoader* loader,
                                      unsigned int graphicIndex) {
    int i;
    for (i = 0; i < loader->cacheUsed; ++i) {
        if (loader->cache[i].loaded &&
            loader->cache[i].graphicIndex == graphicIndex) {
            return &loader->cache[i];
        }
    }
    return NULL;
}

static M11_AssetSlot* m11_alloc_slot(M11_AssetLoader* loader) {
    if (loader->cacheUsed < M11_ASSET_CACHE_SLOTS) {
        return &loader->cache[loader->cacheUsed++];
    }
    /* Load() returns slot pointers that callers may keep until Shutdown.
     * Evicting here creates dangling pointers inside a single complex draw
     * once enough GRAPHICS.DAT entries have been touched. Treat a full cache
     * as a soft load miss instead. */
    return NULL;
}

int M11_AssetLoader_InstallDecodedPixels(M11_AssetLoader* loader,
                                         unsigned int graphicIndex,
                                         const unsigned char* pixels,
                                         unsigned short width,
                                         unsigned short height) {
    M11_AssetSlot* slot;
    unsigned char* copy;
    size_t pixel_count;

    if (!loader || !loader->initialized || !pixels || width == 0u ||
        height == 0u || (size_t)height > SIZE_MAX / (size_t)width) {
        return 0;
    }
    pixel_count = (size_t)width * (size_t)height;
    copy = (unsigned char*)malloc(pixel_count);
    if (!copy) return 0;
    memcpy(copy, pixels, pixel_count);
    slot = m11_find_cached(loader, graphicIndex);
    if (!slot) {
        slot = m11_alloc_slot(loader);
    }
    if (!slot) {
        free(copy);
        return 0;
    }
    free(slot->pixels);
    memset(slot, 0, sizeof(*slot));
    slot->loaded = 1;
    slot->graphicIndex = graphicIndex;
    slot->width = width;
    slot->height = height;
    slot->pixels = copy;
    return 1;
}

const M11_AssetSlot* M11_AssetLoader_Load(M11_AssetLoader* loader,
                                          unsigned int graphicIndex) {
    const struct MemoryGraphicsDatRuntimeState_Compat* rt;
    struct MemoryGraphicsDatState_Compat* fs;
    struct GraphicsDatEntryClassificationResult_Compat classResult;
    struct MemoryGraphicsDatSelection_Compat selection;
    struct MemoryGraphicsDatHeader_Compat tempHeader;
    unsigned short w, h;
    unsigned long packedSize;
    unsigned long expandedCapacity;
    unsigned long unpackedSize;
    unsigned char* compressedBuf = NULL;
    unsigned char* packedBitmap = NULL;
    unsigned char* unpackedPixels = NULL;
    M11_AssetSlot* slot;
    long offset;
    unsigned long ix, iy;
    unsigned long packedStride;
    unsigned long overrunFirst;
    unsigned long overrunLast;
    int traceAssets;

    if (!loader || !loader->initialized) {
        return NULL;
    }
    graphicIndex = m11_resolve_dm1_amiga36_graphic_index(loader, graphicIndex);
    traceAssets = (getenv("FIRESTAFF_ASSET_TRACE") != NULL);

    /* Check cache first */
    slot = m11_find_cached(loader, graphicIndex);
    if (slot) {
        return slot;
    }

    if (loader->iigsDm1) {
        uint16_t width=0U,height=0U;
        size_t capacity;
        unsigned char *pixels;
        int installed;
        if (graphicIndex>=575U || !dm1_v1_iigs_graphics_query(
                loader->iigsData,(size_t)loader->iigsDataSize,
                (uint16_t)graphicIndex,&width,&height)) return NULL;
        capacity=(size_t)width*height;
        pixels=(unsigned char *)malloc(capacity);
        if (!pixels) return NULL;
        if (!dm1_v1_iigs_graphics_decode(loader->iigsData,
                (size_t)loader->iigsDataSize,(uint16_t)graphicIndex,
                pixels,capacity,&width,&height)) {
            free(pixels);
            return NULL;
        }
        installed=M11_AssetLoader_InstallDecodedPixels(loader,graphicIndex,
                                                       pixels,width,height);
        free(pixels);
        return installed ? m11_find_cached(loader,graphicIndex) : NULL;
    }

    if (loader->dm1Fmtowns) {
        dm1_v1_fmtowns_pic_library_view_t view;
        const uint8_t *asset = NULL;
        uint16_t assetSize = 0u;
        dm1_v1_fmtowns_pic_library_gfx_header_t header;
        uint8_t *packed = NULL;
        uint8_t *pixels = NULL;
        size_t bytesWritten = 0u;
        size_t consumed = 0u;
        size_t pixelCount;
        unsigned int x, y;
        int installed = 0;

        if (graphicIndex >= loader->graphicCount ||
            dm1_v1_fmtowns_pic_library_open_pc34(
                loader->dm1FmtownsData, (size_t)loader->dm1FmtownsDataSize,
                &view) != DM1_V1_FMTOWNS_PIC_LIB_OK ||
            dm1_v1_fmtowns_pic_library_asset_bytes_pc34(
                &view, (uint16_t)graphicIndex, &asset, &assetSize) !=
                DM1_V1_FMTOWNS_PIC_LIB_OK ||
            dm1_v1_fmtowns_pic_library_parse_gfx_header_pc34(
                asset, assetSize, &header) != DM1_V1_FMTOWNS_PIC_LIB_OK ||
            header.width_pixels == 0u || header.height_pixels == 0u ||
            header.padded_width_pixels < header.width_pixels ||
            header.decoded_total_bytes == 0u ||
            (size_t)header.width_pixels > SIZE_MAX / header.height_pixels) {
            return NULL;
        }
        packed = (uint8_t *)malloc(header.decoded_total_bytes);
        pixelCount = (size_t)header.width_pixels * header.height_pixels;
        pixels = (uint8_t *)malloc(pixelCount);
        if (!packed || !pixels ||
            dm1_v1_fmtowns_pic_library_decode_asset_pc34(
                asset, assetSize, packed, header.decoded_total_bytes,
                &bytesWritten, &consumed, &header) !=
                DM1_V1_FMTOWNS_PIC_LIB_OK || consumed != assetSize ||
            bytesWritten != header.decoded_total_bytes) {
            free(packed);
            free(pixels);
            return NULL;
        }
        for (y = 0; y < header.height_pixels; ++y) {
            for (x = 0; x < header.width_pixels; ++x) {
                const uint8_t value = packed[(size_t)y * header.row_bytes + x / 2u];
                /* The renderer's packed 4-bpp convention is high nibble
                 * first: even X is the left pixel, odd X the right pixel.
                 * Reversing these produces visibly wrong Towns walls,
                 * alcoves and champion portraits despite valid RLE output. */
                pixels[(size_t)y * header.width_pixels + x] =
                    (uint8_t)((x & 1u) ? (value & 0x0fu) : (value >> 4));
            }
        }
        installed = M11_AssetLoader_InstallDecodedPixels(
            loader, graphicIndex, pixels, header.width_pixels,
            header.height_pixels);
        free(packed);
        free(pixels);
        return installed ? m11_find_cached(loader, graphicIndex) : NULL;
    }

    if (loader->legacyDm1) {
        if (!dm1_v1_legacy_graphics_is_bitmap_index((uint16_t)graphicIndex)) {
            return NULL;
        }
        uint16_t legacyWidth = 0u;
        uint16_t legacyHeight = 0u;
        size_t capacity = 1024u * 1024u;
        unsigned char *legacyPixels = (unsigned char *)malloc(capacity);
        int decoded = legacyPixels && dm1_v1_legacy_graphics_decode(
            loader->legacyData, (size_t)loader->legacyDataSize,
            loader->legacyBigEndian, (uint16_t)graphicIndex, legacyPixels,
            capacity, &legacyWidth, &legacyHeight);
        if (!decoded) {
            free(legacyPixels);
            return NULL;
        }
        if (!M11_AssetLoader_InstallDecodedPixels(loader, graphicIndex,
                                                   legacyPixels, legacyWidth,
                                                   legacyHeight)) {
            free(legacyPixels);
            return NULL;
        }
        free(legacyPixels);
        return m11_find_cached(loader, graphicIndex);
    }

    if (loader->atariStDm1) {
        if (!dm1_v1_legacy_graphics_is_bitmap_index((uint16_t)graphicIndex) ||
            graphicIndex >= DM1_V1_ATARI_ST_GRAPHICS_COUNT) {
            return NULL;
        }
        DM1_V1_AtariStGraphicsDat dat;
        uint16_t stW = 0u, stH = 0u;
        size_t stCap = 1024u * 1024u;
        unsigned char *stPixels = (unsigned char *)malloc(stCap);
        if (!stPixels) return NULL;
        if (!dm1_v1_atari_st_graphics_open(loader->atariStData,
                (size_t)loader->atariStDataSize, &dat) ||
            !dm1_v1_atari_st_graphics_decode(&dat, (uint16_t)graphicIndex,
                stPixels, stCap, &stW, &stH)) {
            free(stPixels);
            return NULL;
        }
        if (!M11_AssetLoader_InstallDecodedPixels(loader, graphicIndex,
                                                   stPixels, stW, stH)) {
            free(stPixels);
            return NULL;
        }
        free(stPixels);
        return m11_find_cached(loader, graphicIndex);
    }

    if (loader->csbAtariSt) {
        const CSB_AtariStLoader *dat = &loader->csbAtariStGraphics;
        uint8_t *item = NULL;
        uint8_t *pixels = NULL;
        uint16_t width = 0u, height = 0u;
        size_t capacity = 1024u * 1024u;
        size_t item_size;
        int decoded = 0;

        if (graphicIndex >= loader->graphicCount) return NULL;
        item_size = dat->items[graphicIndex].decompressed_size;
        item = (uint8_t *)malloc(item_size ? item_size : 1u);
        pixels = (uint8_t *)malloc(capacity);
        if (item && pixels &&
            csb_atari_st_graphics_loader_read_item(dat,
                (uint16_t)graphicIndex, item, item_size) == (int)item_size) {
            decoded = dm1_v1_legacy_graphics_decode_item(item, item_size, 1,
                pixels, capacity, &width, &height);
        }
        free(item);
        if (!decoded || !M11_AssetLoader_InstallDecodedPixels(
                loader, graphicIndex, pixels, width, height)) {
            free(pixels);
            return NULL;
        }
        free(pixels);
        return m11_find_cached(loader, graphicIndex);
    }

    if (loader->csbAmiga) {
        CSB_V1_AmigaGraphicsItem item;
        uint16_t amigaWidth = 0u, amigaHeight = 0u;
        unsigned char *amigaPixels;
        if (graphicIndex >= loader->graphicCount ||
            !csb_v1_amiga_graphics_item(loader->csbAmigaData,
                                         (size_t)loader->csbAmigaDataSize,
                                         (uint16_t)graphicIndex, &item) ||
            item.decompressedByteCount == 0u) return NULL;
        /* DMCSB2's table describes the IMG1 source record, not necessarily
         * the final unpacked pixel count. Decode into the same bounded
         * 1 MiB host buffer used by the other IMG1 loader paths. */
        amigaPixels = (unsigned char *)malloc(1024u * 1024u);
        if (!amigaPixels || !csb_v1_amiga_graphics_decode_item(
                loader->csbAmigaData, (size_t)loader->csbAmigaDataSize,
                (uint16_t)graphicIndex, amigaPixels,
                1024u * 1024u, &amigaWidth, &amigaHeight) ||
            !M11_AssetLoader_InstallDecodedPixels(loader, graphicIndex,
                                                    amigaPixels, amigaWidth,
                                                    amigaHeight)) {
            free(amigaPixels);
            return NULL;
        }
        free(amigaPixels);
        return m11_find_cached(loader, graphicIndex);
    }

    if (loader->csbFmtowns) {
        uint8_t *townsPixels;
        CSB_V1_FmtownsItemDecodeReceipt townsReceipt;

        if (graphicIndex >= loader->graphicCount) return NULL;
        townsPixels = (uint8_t *)malloc(640u * 400u);
        if (!townsPixels || !csb_v1_fmtowns_graphics_decode_item(
                loader->csbFmtownsData, (size_t)loader->csbFmtownsDataSize,
                (uint16_t)graphicIndex, townsPixels, 640u * 400u,
                &townsReceipt) || !townsReceipt.valid ||
            !townsReceipt.is_image ||
            !M11_AssetLoader_InstallDecodedPixels(
                loader, graphicIndex, townsPixels, townsReceipt.width,
                townsReceipt.height)) {
            free(townsPixels);
            return NULL;
        }
        free(townsPixels);
        return m11_find_cached(loader, graphicIndex);
    }

    rt = (const struct MemoryGraphicsDatRuntimeState_Compat*)loader->runtimeState;
    fs = (struct MemoryGraphicsDatState_Compat*)loader->fileState;

    /* runtimeState is NULL for a loader brought up by
     * M11_AssetLoader_InitDecodedOnly (the CSB Atari ST ST20/ST21 path),
     * which sets only initialized = 1. M11_AssetLoader_QuerySize already
     * guards this; without the same check here the first cold-cache lookup
     * dereferences NULL. */
    if (!rt) {
        return NULL;
    }

    if (graphicIndex >= rt->graphicCount) {
        return NULL;
    }

    /* Classify the entry */
    memset(&classResult, 0, sizeof(classResult));
    if (!F9012_RUNTIME_ClassifyGraphicsDatEntry_Compat(rt, graphicIndex, &classResult)) {
        return NULL;
    }
    if (!classResult.shouldUseBitmapPath) {
        return NULL; /* not a bitmap-safe entry */
    }

    w = rt->widthHeight[graphicIndex].Width;
    h = rt->widthHeight[graphicIndex].Height;
    if (w == 0 || h == 0) {
        return NULL;
    }
    if (traceAssets) {
        fprintf(stderr, "asset-load %u w=%u h=%u comp=%u dec=%u\n",
                graphicIndex, (unsigned)w, (unsigned)h,
                (unsigned)rt->compressedByteCounts[graphicIndex],
                (unsigned)rt->decompressedByteCounts[graphicIndex]);
    }

    /* Calculate buffer for the "fake header" approach:
     * We need to build a temporary header to select the graphic,
     * then read the compressed data from the file and expand it.
     *
     * Approach: use F0490_MEMORY_SelectGraphicFromHeader_Compat to
     * get the file offset and byte count, then read into a buffer,
     * then expand via the IMG3 pipeline.
     */
    memset(&tempHeader, 0, sizeof(tempHeader));
    tempHeader.format = rt->format;
    tempHeader.graphicCount = rt->graphicCount;
    tempHeader.compressedByteCounts = rt->compressedByteCounts;
    tempHeader.decompressedByteCounts = rt->decompressedByteCounts;
    tempHeader.widthHeight = rt->widthHeight;
    tempHeader.fileSize = rt->fileSize;

    memset(&selection, 0, sizeof(selection));
    if (!F0490_MEMORY_SelectGraphicFromHeader_Compat(
            &tempHeader, graphicIndex, &selection)) {
        return NULL;
    }

    offset = selection.offset;

    /* Allocate buffer for compressed data.  Even format-1 GRAPHICS.DAT keeps
     * the legacy 4-byte width/height prefix in each graphic's compressed
     * stream; the global header duplicates those dimensions for indexing. */
    compressedBuf = (unsigned char*)calloc(
        selection.compressedByteCount + 16, 1);
    if (!compressedBuf) {
        return NULL;
    }

    /* Read compressed data from GRAPHICS.DAT */
    if (!F0474_MEMORY_LoadGraphic_CPSDF_Compat(
            offset,
            selection.compressedByteCount,
            fs,
            compressedBuf)) {
        free(compressedBuf);
        return NULL;
    }

    if (loader->dm1Amiga36) {
        unsigned char *decodedPixels;
        uint16_t decodedWidth = 0u;
        uint16_t decodedHeight = 0u;
        size_t capacity = (size_t)w * (size_t)h;
        int decoded;
        if (h != 0u && capacity / (size_t)h != (size_t)w) {
            free(compressedBuf);
            return NULL;
        }
        decodedPixels = (unsigned char *)malloc(capacity);
        if (!decodedPixels) {
            free(compressedBuf);
            return NULL;
        }
        decoded = dm1_v1_amiga36_graphic_expand(
            compressedBuf, (size_t)selection.compressedByteCount,
            decodedPixels, capacity, &decodedWidth, &decodedHeight);
        free(compressedBuf);
        if (!decoded || decodedWidth != w || decodedHeight != h ||
            !M11_AssetLoader_InstallDecodedPixels(loader, graphicIndex,
                                                  decodedPixels, w, h)) {
            free(decodedPixels);
            return NULL;
        }
        free(decodedPixels);
        return m11_find_cached(loader, graphicIndex);
    }

    /* Packed bitmap: IMG3 writes one 4-bit pixel per nibble and pads each
     * scanline to an even pixel stride. Odd-width graphics (notably C010,
     * the 87x45 action/PASS strip) therefore use ceil(evenStride/2) bytes
     * per row, not a continuous width*height nibble stream. */
    packedStride = (unsigned long)(((unsigned int)w + 1u) & ~1u) / 2u;
    packedSize = packedStride * (unsigned long)h;
    /* 4 bytes header prefix + padded data */
    unpackedSize = (unsigned long)w * (unsigned long)h;
    expandedCapacity = packedSize;
    if (expandedCapacity < unpackedSize) {
        expandedCapacity = unpackedSize;
    }
    packedBitmap = (unsigned char*)calloc(expandedCapacity + 4096, 1);
    if (!packedBitmap) {
        free(compressedBuf);
        return NULL;
    }

    memset(packedBitmap + 4 + expandedCapacity, 0xCD, 4092);
    overrunFirst = 4092;
    overrunLast = 0;

    /* The M10 expand writes width/height at negative offsets from the
     * bitmap pointer: bitmap[-4..-3] = width, bitmap[-2..-1] = height.
     * So we pass packedBitmap+4 as the destination. */
    {
        unsigned char* bitmapBase = packedBitmap + 4;
        struct GraphicWidthHeight_Compat sizeInfo;
        sizeInfo.Width = w;
        sizeInfo.Height = h;
        F0488_MEMORY_ExpandGraphicToBitmap_Compat(compressedBuf, bitmapBase, &sizeInfo);
        {
            unsigned long gi;
            for (gi = 0; gi < 4092; ++gi) {
                if (packedBitmap[4 + expandedCapacity + gi] != 0xCD) {
                    if (overrunFirst == 4092) overrunFirst = gi;
                    overrunLast = gi;
                }
            }
            if (traceAssets && overrunFirst != 60) {
                fprintf(stderr, "asset-overrun %u first=+%lu last=+%lu\n",
                        graphicIndex, overrunFirst, overrunLast);
            }
        }
    }
    free(compressedBuf);

    /* Unpack from nibble-packed to 1-byte-per-pixel */
    unpackedPixels = (unsigned char*)calloc(unpackedSize, 1);
    if (!unpackedPixels) {
        free(packedBitmap);
        return NULL;
    }

    {
        unsigned char* bitmapBase = packedBitmap + 4;
        /* The packed format: each byte holds two pixels.
         * High nibble = left pixel (even X), low nibble = right pixel (odd X).
         * Each row is padded to an even pixel count; without respecting that
         * row stride, every row after the first in an odd-width graphic is
         * shifted by half a byte, visibly distorting text like PASS. */
        for (iy = 0; iy < (unsigned long)h; ++iy) {
            for (ix = 0; ix < (unsigned long)w; ++ix) {
                unsigned long byteIndex = iy * packedStride + (ix / 2u);
                unsigned char packed = bitmapBase[byteIndex];
                unsigned char pixel;
                if ((ix & 1u) == 0) {
                    pixel = (packed >> 4) & 0x0F;
                } else {
                    pixel = packed & 0x0F;
                }
                unpackedPixels[iy * (unsigned long)w + ix] = pixel;
            }
        }
    }
    free(packedBitmap);

    /* Store in cache */
    slot = m11_alloc_slot(loader);
    if (!slot) {
        free(unpackedPixels);
        return NULL;
    }
    slot->loaded = 1;
    slot->graphicIndex = graphicIndex;
    slot->width = w;
    slot->height = h;
    slot->pixels = unpackedPixels;
    if (traceAssets) {
        fprintf(stderr, "asset-done %u\n", graphicIndex);
    }
    return slot;
}

/* --- blitting --- */

void M11_AssetLoader_Blit(const M11_AssetSlot* slot,
                          unsigned char* framebuffer,
                          int fbWidth,
                          int fbHeight,
                          int dstX,
                          int dstY,
                          int transparentColor) {
    int srcX, srcY;
    int copyW, copyH;
    int startSrcX = 0, startSrcY = 0;

    if (!slot || !slot->loaded || !slot->pixels || !framebuffer) {
        return;
    }

    copyW = (int)slot->width;
    copyH = (int)slot->height;

    /* Clip left */
    if (dstX < 0) {
        startSrcX = -dstX;
        copyW -= startSrcX;
        dstX = 0;
    }
    /* Clip top */
    if (dstY < 0) {
        startSrcY = -dstY;
        copyH -= startSrcY;
        dstY = 0;
    }
    /* Clip right */
    if (dstX + copyW > fbWidth) {
        copyW = fbWidth - dstX;
    }
    /* Clip bottom */
    if (dstY + copyH > fbHeight) {
        copyH = fbHeight - dstY;
    }
    if (copyW <= 0 || copyH <= 0) {
        return;
    }

    for (srcY = 0; srcY < copyH; ++srcY) {
        const unsigned char* srcRow =
            slot->pixels + (startSrcY + srcY) * (int)slot->width + startSrcX;
        unsigned char* dstRow = framebuffer + (dstY + srcY) * fbWidth + dstX;
        if (transparentColor >= 0) {
            for (srcX = 0; srcX < copyW; ++srcX) {
                if (srcRow[srcX] != (unsigned char)transparentColor) {
                    dstRow[srcX] = srcRow[srcX];
                }
            }
        } else {
            memcpy(dstRow, srcRow, (size_t)copyW);
        }
    }
}

void M11_AssetLoader_BlitRegion(const M11_AssetSlot* slot,
                                int srcRX,
                                int srcRY,
                                int srcRW,
                                int srcRH,
                                unsigned char* framebuffer,
                                int fbWidth,
                                int fbHeight,
                                int dstX,
                                int dstY,
                                int transparentColor) {
    int y;
    int copyW, copyH;
    int startSrcX, startSrcY;

    if (!slot || !slot->loaded || !slot->pixels || !framebuffer) {
        return;
    }

    /* Clip source region to asset bounds */
    if (srcRX < 0) { srcRW += srcRX; srcRX = 0; }
    if (srcRY < 0) { srcRH += srcRY; srcRY = 0; }
    if (srcRX + srcRW > (int)slot->width) { srcRW = (int)slot->width - srcRX; }
    if (srcRY + srcRH > (int)slot->height) { srcRH = (int)slot->height - srcRY; }
    if (srcRW <= 0 || srcRH <= 0) {
        return;
    }

    startSrcX = srcRX;
    startSrcY = srcRY;
    copyW = srcRW;
    copyH = srcRH;

    /* Clip to destination */
    if (dstX < 0) { startSrcX -= dstX; copyW += dstX; dstX = 0; }
    if (dstY < 0) { startSrcY -= dstY; copyH += dstY; dstY = 0; }
    if (dstX + copyW > fbWidth) { copyW = fbWidth - dstX; }
    if (dstY + copyH > fbHeight) { copyH = fbHeight - dstY; }
    if (copyW <= 0 || copyH <= 0) {
        return;
    }

    for (y = 0; y < copyH; ++y) {
        const unsigned char* srcRow =
            slot->pixels + (startSrcY + y) * (int)slot->width + startSrcX;
        unsigned char* dstRow = framebuffer + (dstY + y) * fbWidth + dstX;
        if (transparentColor >= 0) {
            int x;
            for (x = 0; x < copyW; ++x) {
                if (srcRow[x] != (unsigned char)transparentColor) {
                    dstRow[x] = srcRow[x];
                }
            }
        } else {
            memcpy(dstRow, srcRow, (size_t)copyW);
        }
    }
}

void M11_AssetLoader_BlitScaled(const M11_AssetSlot* slot,
                                unsigned char* framebuffer,
                                int fbWidth,
                                int fbHeight,
                                int dstX,
                                int dstY,
                                int dstW,
                                int dstH,
                                int transparentColor) {
    int dy, dx;
    if (!slot || !slot->loaded || !slot->pixels || !framebuffer ||
        dstW <= 0 || dstH <= 0) {
        return;
    }
    for (dy = 0; dy < dstH; ++dy) {
        int sy = dy * (int)slot->height / dstH;
        int fbY = dstY + dy;
        if (fbY < 0 || fbY >= fbHeight) continue;
        for (dx = 0; dx < dstW; ++dx) {
            int sx = dx * (int)slot->width / dstW;
            int fbX = dstX + dx;
            unsigned char pixel;
            if (fbX < 0 || fbX >= fbWidth) continue;
            pixel = slot->pixels[sy * (int)slot->width + sx];
            if (transparentColor >= 0 && pixel == (unsigned char)transparentColor) {
                continue;
            }
            framebuffer[fbY * fbWidth + fbX] = pixel;
        }
    }
}

void M11_AssetLoader_BlitScaledMirror(const M11_AssetSlot* slot,
                                      unsigned char* framebuffer,
                                      int fbWidth,
                                      int fbHeight,
                                      int dstX,
                                      int dstY,
                                      int dstW,
                                      int dstH,
                                      int transparentColor) {
    int dy, dx;
    if (!slot || !slot->loaded || !slot->pixels || !framebuffer ||
        dstW <= 0 || dstH <= 0) {
        return;
    }
    for (dy = 0; dy < dstH; ++dy) {
        int sy = dy * (int)slot->height / dstH;
        int fbY = dstY + dy;
        if (fbY < 0 || fbY >= fbHeight) continue;
        for (dx = 0; dx < dstW; ++dx) {
            /* Mirror: sample from (width - 1 - sx) instead of sx */
            int sx = dx * (int)slot->width / dstW;
            int mirrorSx = (int)slot->width - 1 - sx;
            int fbX = dstX + dx;
            unsigned char pixel;
            if (fbX < 0 || fbX >= fbWidth) continue;
            if (mirrorSx < 0) mirrorSx = 0;
            pixel = slot->pixels[sy * (int)slot->width + mirrorSx];
            if (transparentColor >= 0 && pixel == (unsigned char)transparentColor) {
                continue;
            }
            framebuffer[fbY * fbWidth + fbX] = pixel;
        }
    }
}

/* ── Replacement-color blit variants ── */

static unsigned char m11_apply_replacement(unsigned char pixel,
                                           int replSrc9, int replDst9,
                                           int replSrc10, int replDst10) {
    if (replSrc9 >= 0 && pixel == (unsigned char)replSrc9)
        return (unsigned char)replDst9;
    if (replSrc10 >= 0 && pixel == (unsigned char)replSrc10)
        return (unsigned char)replDst10;
    return pixel;
}

void M11_AssetLoader_BlitScaledReplace(const M11_AssetSlot* slot,
                                      unsigned char* framebuffer,
                                      int fbWidth,
                                      int fbHeight,
                                      int dstX,
                                      int dstY,
                                      int dstW,
                                      int dstH,
                                      int transparentColor,
                                      int replSrc9,
                                      int replDst9,
                                      int replSrc10,
                                      int replDst10) {
    int dy, dx;
    if (!slot || !slot->loaded || !slot->pixels || !framebuffer ||
        dstW <= 0 || dstH <= 0) {
        return;
    }
    for (dy = 0; dy < dstH; ++dy) {
        int sy = dy * (int)slot->height / dstH;
        int fbY = dstY + dy;
        if (fbY < 0 || fbY >= fbHeight) continue;
        for (dx = 0; dx < dstW; ++dx) {
            int sx = dx * (int)slot->width / dstW;
            int fbX = dstX + dx;
            unsigned char pixel;
            if (fbX < 0 || fbX >= fbWidth) continue;
            pixel = slot->pixels[sy * (int)slot->width + sx];
            if (transparentColor >= 0 && pixel == (unsigned char)transparentColor)
                continue;
            pixel = m11_apply_replacement(pixel, replSrc9, replDst9,
                                          replSrc10, replDst10);
            framebuffer[fbY * fbWidth + fbX] = pixel;
        }
    }
}

void M11_AssetLoader_BlitScaledMirrorReplace(const M11_AssetSlot* slot,
                                             unsigned char* framebuffer,
                                             int fbWidth,
                                             int fbHeight,
                                             int dstX,
                                             int dstY,
                                             int dstW,
                                             int dstH,
                                             int transparentColor,
                                             int replSrc9,
                                             int replDst9,
                                             int replSrc10,
                                             int replDst10) {
    int dy, dx;
    if (!slot || !slot->loaded || !slot->pixels || !framebuffer ||
        dstW <= 0 || dstH <= 0) {
        return;
    }
    for (dy = 0; dy < dstH; ++dy) {
        int sy = dy * (int)slot->height / dstH;
        int fbY = dstY + dy;
        if (fbY < 0 || fbY >= fbHeight) continue;
        for (dx = 0; dx < dstW; ++dx) {
            int sx = dx * (int)slot->width / dstW;
            int mirrorSx = (int)slot->width - 1 - sx;
            int fbX = dstX + dx;
            unsigned char pixel;
            if (fbX < 0 || fbX >= fbWidth) continue;
            if (mirrorSx < 0) mirrorSx = 0;
            pixel = slot->pixels[sy * (int)slot->width + mirrorSx];
            if (transparentColor >= 0 && pixel == (unsigned char)transparentColor)
                continue;
            pixel = m11_apply_replacement(pixel, replSrc9, replDst9,
                                          replSrc10, replDst10);
            framebuffer[fbY * fbWidth + fbX] = pixel;
        }
    }
}


/* Blit a sub-rectangle from the source slot, scaled to destination.
 * ReDMCSB DUNVIEW.C:3916 uses this pattern for champion portrait
 * extraction from the C026 sprite sheet. */
void M11_AssetLoader_BlitSubRectScaled(const M11_AssetSlot* slot,
                                       unsigned char* framebuffer,
                                       int fbWidth,
                                       int fbHeight,
                                       int dstX,
                                       int dstY,
                                       int dstW,
                                       int dstH,
                                       int srcX,
                                       int srcY,
                                       int srcW,
                                       int srcH,
                                       int transparentColor) {
    int dy, dx;
    if (!slot || !slot->loaded || !slot->pixels || !framebuffer ||
        dstW <= 0 || dstH <= 0 || srcW <= 0 || srcH <= 0) {
        return;
    }
    for (dy = 0; dy < dstH; ++dy) {
        int sy = srcY + dy * srcH / dstH;
        int fbY = dstY + dy;
        if (fbY < 0 || fbY >= fbHeight) continue;
        if (sy < 0 || sy >= (int)slot->height) continue;
        for (dx = 0; dx < dstW; ++dx) {
            int sx = srcX + dx * srcW / dstW;
            int fbX = dstX + dx;
            unsigned char pixel;
            if (fbX < 0 || fbX >= fbWidth) continue;
            if (sx < 0 || sx >= (int)slot->width) continue;
            pixel = slot->pixels[sy * (int)slot->width + sx];
            if (transparentColor >= 0 && pixel == (unsigned char)transparentColor) {
                continue;
            }
            framebuffer[fbY * fbWidth + fbX] = pixel;
        }
    }
}
