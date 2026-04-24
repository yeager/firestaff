/*
 * title_dat_loader_v1.c — Pass 56.
 *
 * Bounded loader/player support for the DM PC v3.4 TITLE mapfile.
 * Grounding source: Greatstone TITLE bank says 59 items in order:
 *   AN, BR, P8, PL, EN, 36xDL, PL, EN, 15xDL, DO.
 * The player intentionally proves control/item progression only; pixel decode
 * of EN/DL payloads is a remaining frontend-integration gap.
 */

#include "title_dat_loader_v1.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void set_err(char* dst, size_t cap, const char* msg) {
    size_t n;
    if (!dst || cap == 0) return;
    n = strlen(msg);
    if (n >= cap) n = cap - 1;
    memcpy(dst, msg, n);
    dst[n] = '\0';
}

static unsigned int be16(const unsigned char* p) {
    return ((unsigned int)p[0] << 8) | (unsigned int)p[1];
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

static int is_known_tag(const unsigned char* p) {
    return (p[0] == 'A' && p[1] == 'N') ||
           (p[0] == 'B' && p[1] == 'R') ||
           (p[0] == 'P' && p[1] == '8') ||
           (p[0] == 'P' && p[1] == 'L') ||
           (p[0] == 'E' && p[1] == 'N') ||
           (p[0] == 'D' && p[1] == 'L') ||
           (p[0] == 'D' && p[1] == 'O');
}

static V1_TitleItemType type_for_tag(const char tag[3]) {
    if (tag[0] == 'A' && tag[1] == 'N') return V1_TITLE_ITEM_AN;
    if (tag[0] == 'B' && tag[1] == 'R') return V1_TITLE_ITEM_BR;
    if (tag[0] == 'P' && tag[1] == '8') return V1_TITLE_ITEM_P8;
    if (tag[0] == 'P' && tag[1] == 'L') return V1_TITLE_ITEM_PL;
    if (tag[0] == 'E' && tag[1] == 'N') return V1_TITLE_ITEM_EN;
    if (tag[0] == 'D' && tag[1] == 'L') return V1_TITLE_ITEM_DL;
    if (tag[0] == 'D' && tag[1] == 'O') return V1_TITLE_ITEM_DO;
    return V1_TITLE_ITEM_UNKNOWN;
}

const char* V1_Title_TypeLabel(V1_TitleItemType type) {
    switch (type) {
        case V1_TITLE_ITEM_AN: return "ANimation";
        case V1_TITLE_ITEM_BR: return "BReak";
        case V1_TITLE_ITEM_P8: return "EGA Palette";
        case V1_TITLE_ITEM_PL: return "PaLette";
        case V1_TITLE_ITEM_EN: return "ENcoded image";
        case V1_TITLE_ITEM_DL: return "Delta Layer";
        case V1_TITLE_ITEM_DO: return "DOne";
        default: return "UNKNOWN";
    }
}

static void classify_record(V1_TitleRecord* r, const unsigned char* payload) {
    r->type = type_for_tag(r->tag);
    switch (r->type) {
        case V1_TITLE_ITEM_AN:
            if (r->payloadBytes >= 10u) {
                r->width = be16(payload + 4);
                r->height = be16(payload + 6);
            }
            break;
        case V1_TITLE_ITEM_EN:
            if (r->payloadBytes >= 6u) {
                r->width = be16(payload + 2);
                r->height = be16(payload + 4);
            }
            break;
        case V1_TITLE_ITEM_DL:
            if (r->payloadBytes >= 8u) {
                r->width = be16(payload + 4);
                r->height = be16(payload + 6);
            }
            break;
        default:
            break;
    }
}

int V1_Title_ParseManifest(const char* titleDatPath,
                           V1_TitleManifest* outManifest,
                           char* errMsg,
                           size_t errMsgBytes) {
    FILE* f;
    long bytes;
    unsigned char* data;
    unsigned int positions[V1_TITLE_DAT_ITEM_COUNT];
    unsigned int posCount = 0;
    unsigned int i;
    unsigned int frameOrdinal = 0;
    unsigned int paletteOrdinal = 0;

    if (!titleDatPath || !outManifest) {
        set_err(errMsg, errMsgBytes, "null argument");
        return 0;
    }
    memset(outManifest, 0, sizeof(*outManifest));

    f = fopen(titleDatPath, "rb");
    if (!f) {
        set_err(errMsg, errMsgBytes, "cannot open TITLE");
        return 0;
    }
    bytes = file_size(f);
    if (bytes <= 0 || bytes > 65535L) {
        fclose(f);
        set_err(errMsg, errMsgBytes, "unexpected TITLE file size");
        return 0;
    }
    data = (unsigned char*)malloc((size_t)bytes);
    if (!data) {
        fclose(f);
        set_err(errMsg, errMsgBytes, "out of memory");
        return 0;
    }
    if (fread(data, 1, (size_t)bytes, f) != (size_t)bytes) {
        free(data);
        fclose(f);
        set_err(errMsg, errMsgBytes, "TITLE read failed");
        return 0;
    }
    fclose(f);

    for (i = 0; i + 3u < (unsigned int)bytes; ++i) {
        if (is_known_tag(data + i)) {
            if (posCount >= V1_TITLE_DAT_ITEM_COUNT) {
                free(data);
                set_err(errMsg, errMsgBytes, "too many TITLE records");
                return 0;
            }
            positions[posCount++] = i;
        }
    }
    if (posCount != V1_TITLE_DAT_ITEM_COUNT) {
        free(data);
        set_err(errMsg, errMsgBytes, "TITLE record count mismatch");
        return 0;
    }

    outManifest->fileBytes = (unsigned long)bytes;
    outManifest->itemCount = posCount;

    for (i = 0; i < posCount; ++i) {
        unsigned int off = positions[i];
        unsigned int next = (i + 1u < posCount) ? positions[i + 1u] : (unsigned int)bytes;
        V1_TitleRecord* r = &outManifest->records[i];
        if (off + 4u > (unsigned int)bytes || next < off + 4u) {
            free(data);
            set_err(errMsg, errMsgBytes, "invalid TITLE record boundary");
            return 0;
        }
        r->index = i;
        r->tag[0] = (char)data[off + 0u];
        r->tag[1] = (char)data[off + 1u];
        r->tag[2] = '\0';
        r->fileOffset = off;
        r->declaredBytes = be16(data + off + 2u);
        r->payloadBytes = next - off - 4u;
        classify_record(r, data + off + 4u);

        switch (r->type) {
            case V1_TITLE_ITEM_AN: outManifest->animationCount++; break;
            case V1_TITLE_ITEM_BR: outManifest->breakCount++; break;
            case V1_TITLE_ITEM_P8: outManifest->egaPaletteCount++; break;
            case V1_TITLE_ITEM_PL:
                outManifest->paletteCount++;
                paletteOrdinal++;
                r->paletteOrdinal = paletteOrdinal;
                break;
            case V1_TITLE_ITEM_EN:
            case V1_TITLE_ITEM_DL:
                if (r->width == 320u && r->height == 200u) {
                    frameOrdinal++;
                    r->frameOrdinal = frameOrdinal;
                    r->paletteOrdinal = paletteOrdinal;
                    outManifest->frameCount = frameOrdinal;
                }
                if (r->type == V1_TITLE_ITEM_EN) outManifest->encodedImageCount++;
                else outManifest->deltaLayerCount++;
                break;
            case V1_TITLE_ITEM_DO: outManifest->doneCount++; break;
            default:
                free(data);
                set_err(errMsg, errMsgBytes, "unknown TITLE record type");
                return 0;
        }
    }

    free(data);
    return 1;
}

void V1_TitlePlayer_Init(V1_TitlePlayer* player,
                         const V1_TitleManifest* manifest) {
    if (!player) return;
    memset(player, 0, sizeof(*player));
    player->manifest = manifest;
}

const V1_TitleRecord* V1_TitlePlayer_NextFrame(V1_TitlePlayer* player) {
    if (!player || !player->manifest || player->done) return NULL;
    while (player->cursor < player->manifest->itemCount) {
        const V1_TitleRecord* r = &player->manifest->records[player->cursor++];
        if (r->type == V1_TITLE_ITEM_PL) {
            player->paletteOrdinal = r->paletteOrdinal;
        } else if (r->type == V1_TITLE_ITEM_DO) {
            player->done = 1;
            return NULL;
        } else if (r->type == V1_TITLE_ITEM_EN || r->type == V1_TITLE_ITEM_DL) {
            player->frameOrdinal = r->frameOrdinal;
            return r;
        }
    }
    player->done = 1;
    return NULL;
}
