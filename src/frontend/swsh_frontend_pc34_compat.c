#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#include "swsh_frontend_pc34_compat.h"
#include <stdlib.h>
#include <string.h>

typedef struct SWSH_BitStream {
        const unsigned char* data;
        unsigned int dataBytes;
        unsigned int pos;
        unsigned int buf;
        unsigned int count;
} SWSH_BitStream;

static unsigned int swsh_le16(const unsigned char* p) {
        return (unsigned int)p[0] | ((unsigned int)p[1] << 8);
}

static void swsh_put_pixel(unsigned char* bitmap,
                           unsigned int pixelOffset,
                           unsigned int color) {
        unsigned char* b = bitmap + (pixelOffset >> 1);
        if ((pixelOffset & 1u) == 0u) {
                *b = (unsigned char)((*b & 0x0fu) | ((color & 0x0fu) << 4));
        } else {
                *b = (unsigned char)((*b & 0xf0u) | (color & 0x0fu));
        }
}

static unsigned int swsh_get_pixel(const unsigned char* bitmap,
                                   unsigned int pixelOffset) {
        unsigned char b = bitmap[pixelOffset >> 1];
        return (pixelOffset & 1u) == 0u ? ((unsigned int)b >> 4) & 0x0fu
                                        : (unsigned int)b & 0x0fu;
}

static int swsh_fill_pixels(unsigned char* bitmap,
                            unsigned int pixelOffset,
                            unsigned int pixelCount,
                            unsigned int color,
                            unsigned int totalPixels) {
        unsigned int i;
        if (pixelOffset > totalPixels || pixelCount > totalPixels - pixelOffset) return 0;
        for (i = 0u; i < pixelCount; ++i) {
                swsh_put_pixel(bitmap, pixelOffset + i, color);
        }
        return 1;
}

static int swsh_copy_raw_nibbles(const unsigned char* source,
                                 unsigned int sourceBytes,
                                 unsigned int sourceOffset,
                                 unsigned char* bitmap,
                                 unsigned int pixelOffset,
                                 unsigned int pixelCount,
                                 unsigned int totalPixels) {
        unsigned int i;
        if (pixelOffset > totalPixels || pixelCount > totalPixels - pixelOffset) return 0;
        if (sourceOffset > sourceBytes || ((pixelCount + 1u) >> 1) > sourceBytes - sourceOffset) return 0;
        for (i = 0u; i < pixelCount; ++i) {
                unsigned char b = source[sourceOffset + (i >> 1)];
                unsigned int color = (i & 1u) == 0u ? ((unsigned int)b >> 4) & 0x0fu
                                                    : (unsigned int)b & 0x0fu;
                swsh_put_pixel(bitmap, pixelOffset + i, color);
        }
        return 1;
}

static int swsh_copy_previous_line(unsigned char* bitmap,
                                   unsigned int pixelOffset,
                                   unsigned int pixelCount,
                                   unsigned int width,
                                   unsigned int totalPixels) {
        unsigned int i;
        if (pixelOffset < width) return 0;
        if (pixelOffset > totalPixels || pixelCount > totalPixels - pixelOffset) return 0;
        for (i = 0u; i < pixelCount; ++i) {
                swsh_put_pixel(bitmap,
                               pixelOffset + i,
                               swsh_get_pixel(bitmap, pixelOffset - width + i));
        }
        return 1;
}

static int swsh_expand_logo_to_bitmap_bounded(const unsigned char* graphic,
                                              unsigned int graphicBytes,
                                              unsigned char* bitmap,
                                              unsigned int* outPixelsWritten) {
        unsigned int width;
        unsigned int height;
        unsigned int evenWidth;
        unsigned int totalPixels;
        unsigned int pos = 4u;
        unsigned int dst = 0u;

        if (outPixelsWritten) *outPixelsWritten = 0u;
        if (!graphic || !bitmap || graphicBytes < 4u) return 0;
        width = swsh_le16(graphic);
        height = swsh_le16(graphic + 2u);
        if (width == 0u || height == 0u || width > 320u || height > 200u) return 0;
        evenWidth = (width + 1u) & ~1u;
        if (height > 0xffffffffu / evenWidth) return 0;
        totalPixels = evenWidth * height;

        /* ReDMCSB PC/I34E source lock:
         * - SWSH.C:2999-3002 expands G8187_Graphic_FTLLogo and copies
         *   320-pixel rows to the screen.
         * - IMAGE2.C:120-337 is the C01_SWOOSH IMG2 decoder; for the
         *   320x200 logo the same-stride branch at IMAGE2.C:294-336 is used.
         * - IMAGE4.C:7-154 defines the packed-nibble write/fill/raw-copy
         *   helpers mirrored below. */
        memset(bitmap, 0, (totalPixels + 1u) >> 1);
        while (dst < totalPixels) {
                unsigned int command;
                unsigned int color;
                unsigned int count;
                if (pos >= graphicBytes) return 0;
                command = graphic[pos++];
                color = command & 0x0fu;
                if ((command & 0x80u) == 0u) {
                        count = (command >> 4) + 1u;
                        if (!swsh_fill_pixels(bitmap, dst, count, color, totalPixels)) return 0;
                        dst += count;
                        continue;
                }
                if ((command & 0x40u) == 0u) {
                        if (pos >= graphicBytes) return 0;
                        count = (unsigned int)graphic[pos++] + 1u;
                } else {
                        if (pos + 1u >= graphicBytes) return 0;
                        count = (((unsigned int)graphic[pos] << 8) | (unsigned int)graphic[pos + 1u]) + 1u;
                        pos += 2u;
                }
                switch (command & 0x30u) {
                case 0x00u:
                        if (!swsh_fill_pixels(bitmap, dst, count, color, totalPixels)) return 0;
                        dst += count;
                        break;
                case 0x10u:
                        if ((count & 1u) != 0u) {
                                if (!swsh_fill_pixels(bitmap, dst, 1u, color, totalPixels)) return 0;
                                dst++;
                                count--;
                        }
                        if (!swsh_copy_raw_nibbles(graphic, graphicBytes, pos, bitmap, dst, count, totalPixels)) return 0;
                        pos += count >> 1;
                        dst += count;
                        break;
                case 0x30u:
                        if (!swsh_copy_previous_line(bitmap, dst, count, evenWidth, totalPixels)) return 0;
                        dst += count;
                        if (!swsh_fill_pixels(bitmap, dst, 1u, color, totalPixels)) return 0;
                        dst++;
                        break;
                default:
                        return 0;
                }
        }
        if (outPixelsWritten) *outPixelsWritten = dst;
        return 1;
}

void SWSH_Compat_ExpandLogoToBitmap(
const unsigned char* graphic SEPARATOR
unsigned char*       bitmap FINAL_SEPARATOR
{
        (void)swsh_expand_logo_to_bitmap_bounded(graphic, 65536u, bitmap, 0);
}

static int swsh_logo_shape_matches_source(const unsigned char* payload,
                                          unsigned int payloadBytes) {
        unsigned char packed[32000];
        unsigned int pixels = 0u;
        unsigned int i;
        unsigned int x;
        unsigned int y;
        unsigned int nonzero = 0u;
        unsigned int firstRow = 200u;
        unsigned int lastRow = 0u;
        unsigned int firstCol = 320u;
        unsigned int lastCol = 0u;

        if (!payload || payloadBytes < 4u) return 0;
        if (swsh_le16(payload) != 320u || swsh_le16(payload + 2u) != 200u) return 0;
        if (!swsh_expand_logo_to_bitmap_bounded(payload, payloadBytes, packed, &pixels)) return 0;
        if (pixels != 320u * 200u) return 0;
        for (i = 0u; i < 320u * 200u; ++i) {
                unsigned int color = swsh_get_pixel(packed, i);
                if (color == 0u) continue;
                x = i % 320u;
                y = i / 320u;
                nonzero++;
                if (y < firstRow) firstRow = y;
                if (y > lastRow) lastRow = y;
                if (x < firstCol) firstCol = x;
                if (x > lastCol) lastCol = x;
        }
        return nonzero >= 8000u && nonzero <= 12000u &&
               firstRow >= 45u && firstRow <= 60u &&
               lastRow >= 160u && lastRow <= 180u &&
               firstCol >= 15u && firstCol <= 35u &&
               lastCol >= 250u && lastCol <= 280u;
}

static const unsigned char* swsh_find_logo_in_unpacked_bytes(const unsigned char* data,
                                                             unsigned int dataBytes) {
        unsigned int i;
        if (!data || dataBytes < 4u) return 0;
        if ((unsigned int)(data[0] | (data[1] << 8)) == 320u &&
            (unsigned int)(data[2] | (data[3] << 8)) == 200u &&
            swsh_logo_shape_matches_source(data, dataBytes)) {
                return data;
        }
        for (i = 1u; i + 4u <= dataBytes; ++i) {
                if ((unsigned int)(data[i] | (data[i + 1u] << 8)) == 320u &&
                    (unsigned int)(data[i + 2u] | (data[i + 3u] << 8)) == 200u &&
                    swsh_logo_shape_matches_source(data + i, dataBytes - i)) {
                        return data + i;
                }
        }
        return 0;
}

static int swsh_lzexe_bit(SWSH_BitStream* s, int* outBit) {
        if (!s || !outBit) return 0;
        *outBit = (int)(s->buf & 1u);
        if (--s->count == 0u) {
                if (s->pos + 1u >= s->dataBytes) return 0;
                s->buf = swsh_le16(s->data + s->pos);
                s->pos += 2u;
                s->count = 16u;
        } else {
                s->buf >>= 1;
        }
        return 1;
}

static int swsh_grow_owned(unsigned char** bytes,
                           unsigned int* capacity,
                           unsigned int needed) {
        unsigned int newCapacity;
        unsigned char* grown;
        if (needed <= *capacity) return 1;
        newCapacity = *capacity ? *capacity : 4096u;
        while (newCapacity < needed) {
                if (newCapacity > 1024u * 1024u) return 0;
                newCapacity *= 2u;
        }
        grown = (unsigned char*)realloc(*bytes, newCapacity);
        if (!grown) return 0;
        *bytes = grown;
        *capacity = newCapacity;
        return 1;
}

static int swsh_lzexe_unpack_load_module(const unsigned char* data,
                                         unsigned int dataBytes,
                                         unsigned char** outBytes,
                                         unsigned int* outByteCount) {
        unsigned int h[16];
        unsigned int inf[8];
        unsigned int i;
        unsigned int infOffset;
        unsigned int compOffset;
        unsigned char* out = 0;
        unsigned int outCount = 0u;
        unsigned int outCapacity = 0u;
        SWSH_BitStream bits;

        if (outBytes) *outBytes = 0;
        if (outByteCount) *outByteCount = 0u;
        if (!data || dataBytes < 64u || !outBytes || !outByteCount) return 0;
        if (!((data[0] == 'M' && data[1] == 'Z') || (data[0] == 'Z' && data[1] == 'M'))) return 0;
        for (i = 0u; i < 16u; ++i) h[i] = swsh_le16(data + i * 2u);
        if (h[12] != 0x1cu || h[13] != 0u) return 0;
        if (h[12] + 4u > dataBytes || memcmp(data + h[12], "LZ91", 4u) != 0) return 0;
        infOffset = (h[11] + h[4]) << 4;
        if (infOffset + 16u > dataBytes) return 0;
        for (i = 0u; i < 8u; ++i) inf[i] = swsh_le16(data + infOffset + i * 2u);
        if (h[11] < inf[4]) return 0;
        compOffset = (h[11] - inf[4] + h[4]) << 4;
        if (compOffset + 2u > dataBytes) return 0;
        memset(&bits, 0, sizeof(bits));
        bits.data = data;
        bits.dataBytes = dataBytes;
        bits.pos = compOffset + 2u;
        bits.buf = swsh_le16(data + compOffset);
        bits.count = 16u;
        for (;;) {
                int bit0;
                int bit1;
                int len;
                int span;
                if (!swsh_lzexe_bit(&bits, &bit0)) goto fail;
                if (bit0) {
                        if (bits.pos >= dataBytes) goto fail;
                        if (!swsh_grow_owned(&out, &outCapacity, outCount + 1u)) goto fail;
                        out[outCount++] = data[bits.pos++];
                        continue;
                }
                if (!swsh_lzexe_bit(&bits, &bit1)) goto fail;
                if (!bit1) {
                        int b2;
                        int b3;
                        if (!swsh_lzexe_bit(&bits, &b2) || !swsh_lzexe_bit(&bits, &b3)) goto fail;
                        len = (b2 << 1) | b3;
                        len += 2;
                        if (bits.pos >= dataBytes) goto fail;
                        span = (int)data[bits.pos++] | -0x100;
                } else {
                        unsigned int spanByte;
                        unsigned int lenByte;
                        if (bits.pos + 1u >= dataBytes) goto fail;
                        spanByte = data[bits.pos++];
                        lenByte = data[bits.pos++];
                        span = (int)(spanByte | ((lenByte & ~7u) << 5)) | -0x2000;
                        len = (int)(lenByte & 7u) + 2;
                        if (len == 2) {
                                if (bits.pos >= dataBytes) goto fail;
                                len = data[bits.pos++];
                                if (len == 0) break;
                                if (len == 1) continue;
                                len++;
                        }
                }
                if (len < 0 || (int)outCount + span < 0) goto fail;
                if (!swsh_grow_owned(&out, &outCapacity, outCount + (unsigned int)len)) goto fail;
                for (i = 0u; i < (unsigned int)len; ++i) {
                        out[outCount] = out[(unsigned int)((int)outCount + span)];
                        outCount++;
                }
                if (outCount > 1024u * 1024u) goto fail;
        }
        *outBytes = out;
        *outByteCount = outCount;
        return 1;
fail:
        if (out) free(out);
        return 0;
}

int SWSH_Compat_FindLogoImagePayloadEx(const unsigned char* data,
                                       unsigned int dataBytes,
                                       SWSH_CompatLogoPayload* outPayload) {
        const unsigned char* payload;
        unsigned char* unpacked = 0;
        unsigned int unpackedBytes = 0u;
        if (!outPayload) return 0;
        memset(outPayload, 0, sizeof(*outPayload));
        payload = swsh_find_logo_in_unpacked_bytes(data, dataBytes);
        if (payload) {
                outPayload->payload = payload;
                return 1;
        }
        if (swsh_lzexe_unpack_load_module(data, dataBytes, &unpacked, &unpackedBytes)) {
                payload = swsh_find_logo_in_unpacked_bytes(unpacked, unpackedBytes);
                if (payload) {
                        outPayload->payload = payload;
                        outPayload->ownedBytes = unpacked;
                        outPayload->ownedByteCount = unpackedBytes;
                        return 1;
                }
                free(unpacked);
        }
        return 0;
}

const unsigned char* SWSH_Compat_FindLogoImagePayload(const unsigned char* data,
                                                       unsigned int dataBytes) {
        SWSH_CompatLogoPayload payload;
        if (!SWSH_Compat_FindLogoImagePayloadEx(data, dataBytes, &payload)) return 0;
        if (payload.ownedBytes) {
                free(payload.ownedBytes);
                return 0;
        }
        return payload.payload;
}

void SWSH_Compat_ReleaseLogoImagePayload(SWSH_CompatLogoPayload* payload) {
        if (!payload) return;
        if (payload->ownedBytes) free(payload->ownedBytes);
        memset(payload, 0, sizeof(*payload));
}

static unsigned char SWSH_Compat_SwooshComponentToRgb8(unsigned int component) {
        static const unsigned char sourceUsed[8] = {
                0u, 36u, 125u, 146u, 164u, 190u, 219u, 255u
        };
        return sourceUsed[component & 7u];
}

void SWSH_Compat_ConvertPcSwooshRgbWordToRgb8(unsigned int colorValue,
                                              unsigned char outRgb[3]) {
        unsigned int r3, g3, b3;
        if (!outRgb) return;
        /* ReDMCSB SWSH.C:281-307 only animates 777, 555, 222, 770, and 000.
         * The F20E PC port displays those source words through the DM PC
         * palette curve, not a linear Atari-3-bit to VGA-DAC ramp: 222 is
         * the dark-grey swoosh step (125), 555 is light grey (190), and
         * 777 is white. */
        r3 = (colorValue >> 8) & 7u;
        g3 = (colorValue >> 4) & 7u;
        b3 =  colorValue       & 7u;
        outRgb[0] = SWSH_Compat_SwooshComponentToRgb8(r3);
        outRgb[1] = SWSH_Compat_SwooshComponentToRgb8(g3);
        outRgb[2] = SWSH_Compat_SwooshComponentToRgb8(b3);
}


typedef struct SWSH_CompatPaletteCommand {
        unsigned short word;
        unsigned int sourceLine;
} SWSH_CompatPaletteCommand;

static const SWSH_CompatPaletteCommand g_swsh_palette_commands[] = {
        {0x1777u, 282u}, {0x0001u, 283u}, {0x2777u, 284u}, {0x0001u, 285u},
        {0x3777u, 286u}, {0x0002u, 287u}, {0x4777u, 288u}, {0x0002u, 289u},
        {0x5777u, 290u}, {0x0003u, 291u}, {0x6777u, 292u}, {0x0003u, 293u},
        {0x7777u, 294u}, {0x0003u, 295u}, {0x8777u, 296u}, {0x9777u, 297u},
        {0xA555u, 298u}, {0xF777u, 299u}, {0x0003u, 300u}, {0x8000u, 301u},
        {0xB777u, 302u}, {0xC555u, 303u}, {0xD222u, 304u}, {0x0003u, 305u},
        {0xF770u, 306u}, {0xE770u, 307u}
};

static const char* SWSH_Compat_SourceEventLine(SWSH_CompatSourceEventKind kind) {
        switch (kind) {
        case SWSH_COMPAT_SOURCE_EVENT_LOAD_LOGO_BITMAP:
                return "SWSH.C:2999-3002 expands SWSHGDAT.C G8187 through IMAGE2.C C01_SWOOSH and copies the 320-pixel rows";
        case SWSH_COMPAT_SOURCE_EVENT_START_SOUND:
                return "SWSH.C:10-14 starts Dosound() with V0901005_SoundCommands before palette animation";
        case SWSH_COMPAT_SOURCE_EVENT_SET_PALETTE_COLOR:
                return "SWSH.C:15-29 reads a palette word, extracts color index/value, and calls XBIOS Setcolor()";
        case SWSH_COMPAT_SOURCE_EVENT_WAIT_VBLANKS:
                return "SWSH.C:30-38 handles palette wait commands with XBIOS Vsync() loops";
        case SWSH_COMPAT_SOURCE_EVENT_RUN_START_PROGRAM:
                return "SWSH.C:39-47 runs START.PRG via GEMDOS Pexec() after the palette command terminator";
        }
        return "SWSH.C source event";
}

unsigned int SWSH_Compat_GetSourceAnimationStepCount(void) {
        return 2u + SWSH_COMPAT_SOURCE_PALETTE_COMMAND_COUNT + 1u;
}

int SWSH_Compat_GetSourceAnimationStep(unsigned int sourceStepOrdinal,
                                       SWSH_CompatSourceAnimationStep* outStep) {
        SWSH_CompatSourceAnimationStep step;
        unsigned int paletteOrdinal;
        unsigned int word;

        if (!outStep || sourceStepOrdinal == 0u || sourceStepOrdinal > SWSH_Compat_GetSourceAnimationStepCount()) return 0;
        memset(&step, 0, sizeof(step));
        step.sourceStepOrdinal = sourceStepOrdinal;

        if (sourceStepOrdinal == 1u) {
                step.kind = SWSH_COMPAT_SOURCE_EVENT_LOAD_LOGO_BITMAP;
        } else if (sourceStepOrdinal == 2u) {
                step.kind = SWSH_COMPAT_SOURCE_EVENT_START_SOUND;
        } else if (sourceStepOrdinal == SWSH_Compat_GetSourceAnimationStepCount()) {
                step.kind = SWSH_COMPAT_SOURCE_EVENT_RUN_START_PROGRAM;
        } else {
                paletteOrdinal = sourceStepOrdinal - 3u;
                word = (unsigned int)g_swsh_palette_commands[paletteOrdinal].word;
                step.sourceLine = g_swsh_palette_commands[paletteOrdinal].sourceLine;
                if (word < 8u) {
                        step.kind = SWSH_COMPAT_SOURCE_EVENT_WAIT_VBLANKS;
                        /* ReDMCSB SWSH.C:33-37 uses 68k DBF after each
                         * Vsync call, so a wait word of N performs N + 1
                         * vertical blanks before returning. */
                        step.vblankCount = word + 1u;
                } else {
                        step.kind = SWSH_COMPAT_SOURCE_EVENT_SET_PALETTE_COLOR;
                        step.colorIndex = (word >> 12) & 0x0fu;
                        step.colorValue = word & 0x0777u;
                }
        }
        step.sourceLineEvidence = SWSH_Compat_SourceEventLine(step.kind);
        *outStep = step;
        return 1;
}

SWSH_CompatSourceTiming SWSH_Compat_GetSourceTimingEvidence(void) {
        SWSH_CompatSourceTiming timing;
        memset(&timing, 0, sizeof(timing));
        timing.paletteCommandCount = SWSH_COMPAT_SOURCE_PALETTE_COMMAND_COUNT;
        timing.paletteColorSetCount = SWSH_COMPAT_SOURCE_PALETTE_COLOR_SET_COUNT;
        timing.paletteWaitCommandCount = SWSH_COMPAT_SOURCE_PALETTE_WAIT_COMMAND_COUNT;
        timing.paletteWaitVblankCount = SWSH_COMPAT_SOURCE_PALETTE_WAIT_VBLANK_COUNT;
        timing.soundRegisterWriteCount = SWSH_COMPAT_SOURCE_SOUND_REGISTER_WRITE_COUNT;
        timing.soundWaitCommandCount = SWSH_COMPAT_SOURCE_SOUND_WAIT_COMMAND_COUNT;
        timing.soundWaitVblankCount = SWSH_COMPAT_SOURCE_SOUND_WAIT_VBLANK_COUNT;
        timing.sourceFile = "ReDMCSB_WIP20210206/Toolchains/Common/Source/SWSH.C";
        timing.sourceFunction = "T0901000_PlaySoundAndAnimatePalette";
        timing.evidenceNote = "PC SWSH.C path: expand SWSHGDAT logo to screen, start Dosound() from V0901005_SoundCommands, process 26 V0901006_PaletteCommands as Setcolor()/Vsync events, then Pexec START.PRG.";
        return timing;
}

const char* SWSH_Compat_GetSourceAnimationEvidence(void) {
        return "ReDMCSB SWSH.C PC path: T0901006 expands the FTL logo bitmap to Physbase using SWSHGDAT.C, then T0901000 starts Dosound(), applies the V0901006 palette command sequence with Setcolor()/Vsync, and hands off to START.PRG via Pexec after the zero terminator.";
}
