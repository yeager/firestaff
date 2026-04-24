/*
 * title_dat_loader_v1.c — Pass 56.
 *
 * Bounded loader/player support for the DM PC v3.4 TITLE mapfile.
 * Grounding source: Greatstone TITLE bank says 59 items in order:
 *   AN, BR, P8, PL, EN, 36xDL, PL, EN, 15xDL, DO.
 * Pass 57 adds a bounded IMG1 EN/DL renderer for the original 320x200
 * TITLE canvas.  Timing/cadence is still not claimed beyond the source
 * per-item duration attributes; frontend wall-clock binding remains separate.
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

static unsigned char* read_file_bytes(const char* path, long* outBytes,
                                      char* errMsg, size_t errMsgBytes) {
    FILE* f;
    long bytes;
    unsigned char* data;

    if (outBytes) *outBytes = 0;
    f = fopen(path, "rb");
    if (!f) {
        set_err(errMsg, errMsgBytes, "cannot open TITLE");
        return NULL;
    }
    bytes = file_size(f);
    if (bytes <= 0 || bytes > 65535L) {
        fclose(f);
        set_err(errMsg, errMsgBytes, "unexpected TITLE file size");
        return NULL;
    }
    data = (unsigned char*)malloc((size_t)bytes);
    if (!data) {
        fclose(f);
        set_err(errMsg, errMsgBytes, "out of memory");
        return NULL;
    }
    if (fread(data, 1, (size_t)bytes, f) != (size_t)bytes) {
        free(data);
        fclose(f);
        set_err(errMsg, errMsgBytes, "TITLE read failed");
        return NULL;
    }
    fclose(f);
    if (outBytes) *outBytes = bytes;
    return data;
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

typedef struct NibbleReader {
    const unsigned char* data;
    unsigned int bytes;
    unsigned int nibblePos;
} NibbleReader;

static int nibble_read(NibbleReader* nr, unsigned int* out) {
    unsigned int b;
    if (!nr || !out || nr->nibblePos >= nr->bytes * 2u) return 0;
    b = nr->data[nr->nibblePos / 2u];
    *out = (nr->nibblePos & 1u) ? (b & 0x0fu) : ((b >> 4) & 0x0fu);
    nr->nibblePos++;
    return 1;
}

static int nibble_read_byte(NibbleReader* nr, unsigned int* out) {
    unsigned int hi, lo;
    if (!nibble_read(nr, &hi) || !nibble_read(nr, &lo)) return 0;
    *out = (hi << 4) | lo;
    return 1;
}

static int nibble_read_word(NibbleReader* nr, unsigned int* out) {
    unsigned int hi, lo;
    if (!nibble_read_byte(nr, &hi) || !nibble_read_byte(nr, &lo)) return 0;
    *out = (hi << 8) | lo;
    return 1;
}

static int img1_put(uint8_t* canvas, unsigned int total, unsigned int* pos,
                    unsigned int color, int transparent) {
    if (!canvas || !pos || *pos >= total) return 0;
    if (!transparent) canvas[*pos] = (uint8_t)(color & 0x0fu);
    *pos += 1u;
    return 1;
}

static int img1_repeat_color(uint8_t* canvas, unsigned int total, unsigned int* pos,
                             unsigned int count, unsigned int color) {
    unsigned int i;
    for (i = 0; i < count; ++i) {
        if (!img1_put(canvas, total, pos, color, 0)) return 0;
    }
    return 1;
}

static int img1_copy_previous_line(uint8_t* canvas, unsigned int width,
                                   unsigned int total, unsigned int* pos,
                                   unsigned int count) {
    unsigned int i;
    for (i = 0; i < count; ++i) {
        unsigned int src;
        if (!canvas || !pos || *pos >= total || *pos < width) return 0;
        src = *pos - width;
        canvas[*pos] = canvas[src];
        *pos += 1u;
    }
    return 1;
}

static int img1_literal_run(NibbleReader* nr, uint8_t* canvas, unsigned int total,
                            unsigned int* pos, unsigned int count) {
    unsigned int i, color;
    for (i = 0; i < count; ++i) {
        if (!nibble_read(nr, &color)) return 0;
        if (!img1_put(canvas, total, pos, color, 0)) return 0;
    }
    return 1;
}

static int img1_skip_transparent(unsigned int total, unsigned int* pos,
                                 unsigned int count) {
    if (!pos || count > total || *pos > total - count) return 0;
    *pos += count;
    return 1;
}

static int decode_img1_to_canvas(const unsigned char* img, unsigned int imgBytes,
                                 int delta, uint8_t* canvas,
                                 unsigned int width, unsigned int height,
                                 char* errMsg, size_t errMsgBytes) {
    NibbleReader nr;
    unsigned int pos = 0;
    unsigned int total = width * height;
    if (!img || !canvas || width == 0u || height == 0u || total != 64000u) {
        set_err(errMsg, errMsgBytes, "invalid IMG1 decode arguments");
        return 0;
    }
    nr.data = img;
    nr.bytes = imgBytes;
    nr.nibblePos = 0;
    while (pos < total) {
        unsigned int n1, n2, v;
        if (!nibble_read(&nr, &n1) || !nibble_read(&nr, &n2)) {
            set_err(errMsg, errMsgBytes, "IMG1 stream ended before canvas was filled");
            return 0;
        }
        if (n1 <= 7u) {
            if (!img1_repeat_color(canvas, total, &pos, n1 + 1u, n2)) goto overflow;
        } else if (n1 == 8u) {
            if (!nibble_read_byte(&nr, &v) || !img1_repeat_color(canvas, total, &pos, v + 1u, n2)) goto overflow;
        } else if (n1 == 0xcu) {
            if (!nibble_read_word(&nr, &v) || !img1_repeat_color(canvas, total, &pos, v + 1u, n2)) goto overflow;
        } else if (n1 == 0xbu) {
            if (!nibble_read_byte(&nr, &v) || !img1_copy_previous_line(canvas, width, total, &pos, v + 1u) ||
                !img1_put(canvas, total, &pos, n2, 0)) goto overflow;
        } else if (n1 == 0xfu) {
            if (!nibble_read_word(&nr, &v) || !img1_copy_previous_line(canvas, width, total, &pos, v + 1u) ||
                !img1_put(canvas, total, &pos, n2, 0)) goto overflow;
        } else if (n1 == 9u) {
            if (!nibble_read_byte(&nr, &v)) goto overflow;
            if ((v & 1u) == 0u) {
                if (!img1_put(canvas, total, &pos, n2, 0) || !img1_literal_run(&nr, canvas, total, &pos, v)) goto overflow;
            } else {
                if (!img1_literal_run(&nr, canvas, total, &pos, v + 1u)) goto overflow;
            }
        } else if (n1 == 0xdu) {
            if (!nibble_read_word(&nr, &v)) goto overflow;
            if ((v & 1u) == 0u) {
                if (!img1_put(canvas, total, &pos, n2, 0) || !img1_literal_run(&nr, canvas, total, &pos, v)) goto overflow;
            } else {
                if (!img1_literal_run(&nr, canvas, total, &pos, v + 1u)) goto overflow;
            }
        } else if (n1 == 0xau) {
            if (!delta) goto transparent_in_base;
            if (!img1_skip_transparent(total, &pos, n2 + 1u)) goto overflow;
        } else if (n1 == 0xeu) {
            if (!delta) goto transparent_in_base;
            if (n2 <= 0xcu) {
                if (!img1_skip_transparent(total, &pos, n2 + 17u)) goto overflow;
            } else if (n2 == 0xdu) {
                if (!nibble_read_byte(&nr, &v) || !img1_skip_transparent(total, &pos, v + 1u)) goto overflow;
            } else if (n2 == 0xeu) {
                if (!nibble_read_byte(&nr, &v) || !img1_skip_transparent(total, &pos, v + 257u)) goto overflow;
            } else {
                if (!nibble_read_word(&nr, &v) || !img1_skip_transparent(total, &pos, v + 1u)) goto overflow;
            }
        }
    }
    return 1;
overflow:
    set_err(errMsg, errMsgBytes, "IMG1 decode overran canvas or compressed stream");
    return 0;
transparent_in_base:
    set_err(errMsg, errMsgBytes, "IMG1 base frame unexpectedly used transparent opcode");
    return 0;
}

static void parse_palette(const unsigned char* plPayload, unsigned int payloadBytes,
                          V1_TitlePalette* palette) {
    unsigned int i;
    if (!plPayload || payloadBytes < 68u || !palette) return;
    for (i = 0; i < 16u; ++i) {
        const unsigned char* p = plPayload + 4u + i * 4u;
        unsigned int color = p[0] & 0x0fu;
        palette->rgba[color][0] = (uint8_t)((p[1] & 0x0fu) * 17u);
        palette->rgba[color][1] = (uint8_t)((p[2] & 0x0fu) * 17u);
        palette->rgba[color][2] = (uint8_t)((p[3] & 0x0fu) * 17u);
        palette->rgba[color][3] = 255u;
    }
}

int V1_Title_RenderFrames(const char* titleDatPath,
                          V1_TitleFrameCallback callback,
                          void* userData,
                          char* errMsg,
                          size_t errMsgBytes) {
    V1_TitleManifest manifest;
    V1_TitlePalette palettes[3];
    uint8_t canvas[320u * 200u];
    unsigned char* data;
    long bytes;
    unsigned int i;
    char parseErr[256];

    if (!titleDatPath || !callback) {
        set_err(errMsg, errMsgBytes, "null argument");
        return 0;
    }
    if (!V1_Title_ParseManifest(titleDatPath, &manifest, parseErr, sizeof(parseErr))) {
        set_err(errMsg, errMsgBytes, parseErr);
        return 0;
    }
    data = read_file_bytes(titleDatPath, &bytes, errMsg, errMsgBytes);
    if (!data) return 0;

    memset(palettes, 0, sizeof(palettes));
    memset(canvas, 0, sizeof(canvas));
    for (i = 0; i < manifest.itemCount; ++i) {
        const V1_TitleRecord* r = &manifest.records[i];
        const unsigned char* payload = data + r->fileOffset + 4u;
        if (r->type == V1_TITLE_ITEM_PL) {
            if (r->paletteOrdinal < 3u) parse_palette(payload, r->payloadBytes, &palettes[r->paletteOrdinal]);
        } else if (r->type == V1_TITLE_ITEM_EN || r->type == V1_TITLE_ITEM_DL) {
            V1_TitleRenderFrame frame;
            unsigned int imgOffset = (r->type == V1_TITLE_ITEM_DL) ? 8u : 6u;
            int isDelta = (r->type == V1_TITLE_ITEM_DL);
            unsigned int imgBytes;
            if (r->width != 320u || r->height != 200u || r->payloadBytes <= imgOffset) {
                free(data);
                set_err(errMsg, errMsgBytes, "unsupported TITLE frame dimensions or size");
                return 0;
            }
            imgBytes = r->payloadBytes - imgOffset;
            if (r->type == V1_TITLE_ITEM_DL && be16(payload + 2u) != 0xff81u) {
                free(data);
                set_err(errMsg, errMsgBytes, "DL record missing FF81 signature");
                return 0;
            }
            if (!decode_img1_to_canvas(payload + imgOffset, imgBytes, isDelta, canvas,
                                       r->width, r->height, errMsg, errMsgBytes)) {
                free(data);
                return 0;
            }
            memset(&frame, 0, sizeof(frame));
            frame.record = r;
            frame.frameOrdinal = r->frameOrdinal;
            frame.paletteOrdinal = r->paletteOrdinal;
            frame.durationFrames = be16(payload);
            frame.width = r->width;
            frame.height = r->height;
            frame.colorIndices = canvas;
            frame.palette = (r->paletteOrdinal < 3u) ? &palettes[r->paletteOrdinal] : NULL;
            if (!callback(&frame, userData, errMsg, errMsgBytes)) {
                free(data);
                return 0;
            }
        }
    }
    free(data);
    return 1;
}
