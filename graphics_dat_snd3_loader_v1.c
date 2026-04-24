/*
 * graphics_dat_snd3_loader_v1.c — Pass 51.
 *
 * Bounded loader/probe support for the DM PC v3.4 English GRAPHICS.DAT
 * SND3 in-game SFX bank.  No runtime playback integration here.
 */

#include "graphics_dat_snd3_loader_v1.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const unsigned short kSnd3Indices[V1_GRAPHICS_DAT_SND3_COUNT] = {
    671, 672, 673, 674, 675,
    677, 678, 679, 680, 681, 682, 683, 684, 685,
    687, 688, 689, 690, 691, 692, 693,
    701, 702, 703, 704, 705, 706, 707, 708, 709, 710, 711, 712
};

static const char* const kSnd3Labels[V1_GRAPHICS_DAT_SND3_COUNT] = {
    "Sound 00 (Falling item)",
    "Sound 01 (Switch)",
    "Sound 02 (Door)",
    "Sound 03 (Attack (Trolin - Stone Golem) - Touching Wall)",
    "Sound 04 (Exploding Fireball)",
    "Sound 05 (Falling and Dying)",
    "Sound 06 (Swallowing)",
    "Sound 07 (Champion Wounded 1)",
    "Sound 08 (Champion Wounded 2)",
    "Sound 09 (Champion Wounded 3)",
    "Sound 10 (Champion Wounded 4)",
    "Sound 11 (Exploding Spell)",
    "Sound 12 (Attack (Skeleton - Animated Armour - Party Slash))",
    "Sound 13 (Teleporting)",
    "Sound 14 (Running Into A Wall)",
    "Sound 15 (Attack (Pain Rat - Red Dragon))",
    "Sound 16 (Attack (Mummy - Ghost))",
    "Sound 17 (Attack (Screamer - Oitu))",
    "Sound 18 (Attack (Giant Scorpion))",
    "Sound 19 (Attack (Magenta Worm))",
    "Sound 20 (Attack (Giggler))",
    "Sound 21 (Move (Animated Armour))",
    "Sound 22 (Move (Giant Wasp - Couatl))",
    "Sound 23 (Move (Mummy - Trolin - Stone Golem - Giggler - Vexirk - Demon))",
    "Sound 24 (Blowing Horn Of Fear)",
    "Sound 25 (Move (Screamer - Rockpile - Magenta Worm - Pain Rat - Ruster - Giant Scorpion - Oitu))",
    "Sound 26 (Move (Swamp Slime - Water Elemental))",
    "Sound 27 (War Cry)",
    "Sound 28 (Attack (Rockpile))",
    "Sound 29 (Attack (Water Elemental))",
    "Sound 30 (Attack (Couatl))",
    "Sound 31 (Move (Red Dragon))",
    "Sound 32 (Move (Skeleton))"
};

const unsigned short* V1_GraphicsSnd3_ItemIndices(unsigned int* outCount) {
    if (outCount) *outCount = V1_GRAPHICS_DAT_SND3_COUNT;
    return kSnd3Indices;
}

static int v1_snd3_slot_for_index(unsigned int itemIndex) {
    unsigned int i;
    for (i = 0; i < V1_GRAPHICS_DAT_SND3_COUNT; ++i) {
        if (kSnd3Indices[i] == itemIndex) return (int)i;
    }
    return -1;
}

int V1_GraphicsSnd3_IsItemIndex(unsigned int itemIndex) {
    return v1_snd3_slot_for_index(itemIndex) >= 0;
}

const char* V1_GraphicsSnd3_ItemLabel(unsigned int itemIndex) {
    int slot = v1_snd3_slot_for_index(itemIndex);
    if (slot < 0) return "(not a DM PC v3.4 SND3 item)";
    return kSnd3Labels[slot];
}

static void set_err(char* dst, size_t cap, const char* msg) {
    size_t n;
    if (!dst || cap == 0) return;
    n = strlen(msg);
    if (n >= cap) n = cap - 1;
    memcpy(dst, msg, n);
    dst[n] = '\0';
}

static unsigned short le16(const unsigned char* p) {
    return (unsigned short)(p[0] | ((unsigned short)p[1] << 8));
}

static unsigned short be16(const unsigned char* p) {
    return (unsigned short)(((unsigned short)p[0] << 8) | p[1]);
}

static long file_size(FILE* f) {
    long cur = ftell(f);
    long end;
    if (cur < 0) return -1;
    if (fseek(f, 0, SEEK_END) != 0) return -1;
    end = ftell(f);
    if (fseek(f, cur, SEEK_SET) != 0) return -1;
    return end;
}

int V1_GraphicsSnd3_ParseManifest(const char* graphicsDatPath,
                                  V1_GraphicsSnd3Manifest* outManifest,
                                  char* errMsg,
                                  size_t errMsgBytes) {
    FILE* f;
    long fileBytes;
    unsigned char* hdr;
    unsigned short signature;
    unsigned short itemCount;
    const unsigned char* pComp;
    const unsigned char* pDecomp;
    const unsigned char* pAttrs;
    unsigned int i;
    unsigned long cur;

    if (!graphicsDatPath || !outManifest) {
        set_err(errMsg, errMsgBytes, "null argument");
        return 0;
    }
    memset(outManifest, 0, sizeof(*outManifest));

    f = fopen(graphicsDatPath, "rb");
    if (!f) {
        set_err(errMsg, errMsgBytes, "cannot open GRAPHICS.DAT");
        return 0;
    }

    fileBytes = file_size(f);
    if (fileBytes < (long)V1_GRAPHICS_DAT_HEADER_BYTES) {
        fclose(f);
        set_err(errMsg, errMsgBytes, "file too small to be DM PC v3.4 GRAPHICS.DAT");
        return 0;
    }

    hdr = (unsigned char*)malloc(V1_GRAPHICS_DAT_HEADER_BYTES);
    if (!hdr) {
        fclose(f);
        set_err(errMsg, errMsgBytes, "out of memory (header)");
        return 0;
    }
    if (fseek(f, 0, SEEK_SET) != 0 ||
        fread(hdr, 1, V1_GRAPHICS_DAT_HEADER_BYTES, f) != V1_GRAPHICS_DAT_HEADER_BYTES) {
        free(hdr);
        fclose(f);
        set_err(errMsg, errMsgBytes, "GRAPHICS.DAT header read failed");
        return 0;
    }
    fclose(f);

    signature = le16(hdr + 0);
    itemCount = le16(hdr + 2);
    if (signature != V1_GRAPHICS_DAT_SIGNATURE || itemCount != V1_GRAPHICS_DAT_ITEM_COUNT) {
        free(hdr);
        set_err(errMsg, errMsgBytes, "not DM PC v3.4 English GRAPHICS.DAT (signature/count mismatch)");
        return 0;
    }

    outManifest->signature = signature;
    outManifest->itemCount = itemCount;
    outManifest->headerBytes = V1_GRAPHICS_DAT_HEADER_BYTES;
    outManifest->fileBytes = (unsigned long)fileBytes;

    pComp = hdr + 4;
    pDecomp = pComp + 2u * V1_GRAPHICS_DAT_ITEM_COUNT;
    pAttrs = pDecomp + 2u * V1_GRAPHICS_DAT_ITEM_COUNT;

    cur = V1_GRAPHICS_DAT_HEADER_BYTES;
    for (i = 0; i < V1_GRAPHICS_DAT_ITEM_COUNT; ++i) {
        unsigned int comp = le16(pComp + 2u * i);
        int sndSlot = v1_snd3_slot_for_index(i);
        if (sndSlot >= 0) {
            V1_GraphicsSnd3Item* it = &outManifest->items[sndSlot];
            unsigned char first4[4];
            FILE* body;
            it->itemIndex = i;
            it->soundOrdinal = (unsigned int)sndSlot;
            it->fileOffset = cur;
            it->itemBytes = comp;
            it->decompressedBytes = le16(pDecomp + 2u * i);
            it->attr0 = le16(pAttrs + 4u * i + 0);
            it->attr1 = le16(pAttrs + 4u * i + 2);

            if (comp < 2 || cur + comp > (unsigned long)fileBytes) {
                free(hdr);
                set_err(errMsg, errMsgBytes, "SND3 item has invalid size/offset");
                return 0;
            }
            body = fopen(graphicsDatPath, "rb");
            if (!body) {
                free(hdr);
                set_err(errMsg, errMsgBytes, "cannot reopen GRAPHICS.DAT");
                return 0;
            }
            if (fseek(body, (long)cur, SEEK_SET) != 0 || fread(first4, 1, 4, body) != 4) {
                fclose(body);
                free(hdr);
                set_err(errMsg, errMsgBytes, "SND3 item prefix read failed");
                return 0;
            }
            fclose(body);
            it->declaredSampleCount = be16(first4);
            if (it->declaredSampleCount + 2u > it->itemBytes) {
                free(hdr);
                set_err(errMsg, errMsgBytes, "SND3 sample count exceeds item size");
                return 0;
            }
            it->trailingBytes = it->itemBytes - 2u - it->declaredSampleCount;
            if (it->trailingBytes > 3u) {
                free(hdr);
                set_err(errMsg, errMsgBytes, "SND3 trailing byte count exceeds dmweb 0..3 bound");
                return 0;
            }
        }
        cur += comp;
    }

    if (cur != (unsigned long)fileBytes) {
        free(hdr);
        set_err(errMsg, errMsgBytes, "GRAPHICS.DAT item size sum does not match file size");
        return 0;
    }

    free(hdr);
    return 1;
}

int V1_GraphicsSnd3_DecodeItem(const char* graphicsDatPath,
                               const V1_GraphicsSnd3Manifest* manifest,
                               unsigned int itemIndex,
                               V1_GraphicsSnd3Buffer* outBuffer,
                               char* errMsg,
                               size_t errMsgBytes) {
    int slot;
    const V1_GraphicsSnd3Item* it;
    FILE* f;

    if (!graphicsDatPath || !manifest || !outBuffer) {
        set_err(errMsg, errMsgBytes, "null argument");
        return 0;
    }
    memset(outBuffer, 0, sizeof(*outBuffer));

    slot = v1_snd3_slot_for_index(itemIndex);
    if (slot < 0) {
        set_err(errMsg, errMsgBytes, "itemIndex is not a DM PC v3.4 SND3 item");
        return 0;
    }
    it = &manifest->items[slot];
    if (it->itemIndex != itemIndex || it->itemBytes < 2) {
        set_err(errMsg, errMsgBytes, "manifest missing requested SND3 item");
        return 0;
    }

    outBuffer->samples = (unsigned char*)malloc(it->declaredSampleCount ? it->declaredSampleCount : 1u);
    if (!outBuffer->samples) {
        set_err(errMsg, errMsgBytes, "out of memory (sample buffer)");
        return 0;
    }

    f = fopen(graphicsDatPath, "rb");
    if (!f) {
        V1_GraphicsSnd3_FreeBuffer(outBuffer);
        set_err(errMsg, errMsgBytes, "cannot open GRAPHICS.DAT");
        return 0;
    }
    if (fseek(f, (long)it->fileOffset + 2L, SEEK_SET) != 0 ||
        fread(outBuffer->samples, 1, it->declaredSampleCount, f) != it->declaredSampleCount) {
        fclose(f);
        V1_GraphicsSnd3_FreeBuffer(outBuffer);
        set_err(errMsg, errMsgBytes, "SND3 sample read failed");
        return 0;
    }
    fclose(f);

    outBuffer->itemIndex = itemIndex;
    outBuffer->soundOrdinal = it->soundOrdinal;
    outBuffer->declaredSampleCount = it->declaredSampleCount;
    outBuffer->decodedSampleCount = it->declaredSampleCount;
    outBuffer->sampleRateHz = V1_GRAPHICS_DAT_SND3_SAMPLE_RATE_HZ;
    return 1;
}

void V1_GraphicsSnd3_FreeBuffer(V1_GraphicsSnd3Buffer* buffer) {
    if (!buffer) return;
    free(buffer->samples);
    buffer->samples = 0;
    buffer->declaredSampleCount = 0;
    buffer->decodedSampleCount = 0;
}
