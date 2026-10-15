#include "dm1_v1_i34e_title_anim_pc34.h"
#include "firestaff_x68k_media_receipt.h"
#include "lzexe_unpack_v091.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void put_be16(uint8_t *p, unsigned value) {
    p[0] = (uint8_t)(value >> 8);
    p[1] = (uint8_t)value;
}

static void put_record(uint8_t *p, const char id[2], unsigned bytes,
                       unsigned attr) {
    p[0] = (uint8_t)id[0]; p[1] = (uint8_t)id[1];
    put_be16(p + 2, bytes); put_be16(p + 4, attr);
}

static void test_stream(void) {
    uint8_t data[14 + 6 + 6 + 6 + 66 + 6 + 504 + 6];
    size_t pos = 0u;
    size_t en_delay_offset;
    unsigned i;
    DM1_V1_I34ETitleAnimPc34 state;
    DM1_V1_I34ETitleAnimFramePc34 frame;
    memset(data, 0, sizeof(data));
    data[0] = 'A'; data[1] = 'N'; data[2] = 8;
    put_be16(data + 6, 320); put_be16(data + 8, 200); put_be16(data + 10, 4);
    pos = 14u;
    put_record(data + pos, "BR", 0, 1); pos += 6u;
    put_record(data + pos, "P8", 0, 0); pos += 6u;
    put_record(data + pos, "PL", 66, 0); pos += 6u;
    put_be16(data + pos, 16u);
    for (i = 0u; i < 16u; ++i) {
        data[pos + 2u + i * 4u] = (uint8_t)i;
        data[pos + 3u + i * 4u] = (uint8_t)i;
        data[pos + 4u + i * 4u] = (uint8_t)(15u - i);
        data[pos + 5u + i * 4u] = 3u;
    }
    pos += 66u;
    put_record(data + pos, "EN", 504, 7);
    en_delay_offset = pos + 4u;
    pos += 6u;
    put_be16(data + pos, 320); put_be16(data + pos + 2, 200); pos += 4u;
    for (i = 0u; i < 250u; ++i) {
        data[pos++] = 0x83u;
        data[pos++] = 255u;
    }
    put_record(data + pos, "DO", 0, 0); pos += 6u;
    assert(pos == sizeof(data));
    assert(dm1_v1_i34e_title_anim_pc34_init(&state, data, sizeof(data)));
    assert(dm1_v1_i34e_title_anim_pc34_step(&state, &frame) == 1);
    assert(frame.frame_index == 0u && frame.source_counter_units == 7u);
    assert(frame.break_allowed && frame.pixels[0] == 3u &&
           frame.pixels[63999] == 3u);
    assert(frame.palette[4].r == 19u && frame.palette[4].g == 47u &&
           frame.palette[4].b == 15u);
    assert(dm1_v1_i34e_title_anim_pc34_step(&state, &frame) == 0);
    assert(state.finished);
    put_be16(data + en_delay_offset, 1u);
    assert(dm1_v1_i34e_title_anim_pc34_init(&state, data, sizeof(data)));
    assert(dm1_v1_i34e_title_anim_pc34_step(&state, &frame) == 1);
    assert(frame.source_counter_units == 1u);
    assert(!dm1_v1_i34e_title_anim_pc34_init(&state, data, 13u));
}

static void test_source_ordered_erase(void) {
    uint8_t pixels[64000];
    DM1_V1_I34ETitleErasePc34 state;
    unsigned frames = 1u;
    memset(pixels, 7, sizeof(pixels));
    dm1_v1_i34e_title_erase_pc34_init(&state);
    assert(state.valid && state.lfsr == 1u);
    assert(dm1_v1_i34e_title_erase_pc34_step(&state, pixels,
                                              sizeof(pixels), 1024u) == 1);
    assert(pixels[1] == 0u && pixels[0] == 7u);
    do {
        int result = dm1_v1_i34e_title_erase_pc34_step(
            &state, pixels, sizeof(pixels), 1024u);
        ++frames;
        assert(result == 0 || result == 1);
        if (result == 0) break;
    } while (frames < 100u);
    assert(state.finished && state.lfsr == 1u);
    assert(state.lfsr_steps == 65535u && state.pixels_cleared == 64000u);
    assert(frames == 64u);
    assert(pixels[0] == 0u && pixels[63999] == 0u);
    assert(dm1_v1_i34e_title_erase_pc34_step(&state, pixels,
                                              sizeof(pixels), 1024u) == 0);
    assert(dm1_v1_i34e_title_erase_pc34_step(&state, pixels,
                                              sizeof(pixels) - 1u, 1u) == -1);
}

static void test_default_pit_timing(void) {
    uint64_t deadline, next;
    /* Active-clock milliseconds either side of the first 54.9254 ms IRQ. */
    assert(dm1_v1_i34e_title_pc34_deadline_ms(0u, 54u, 1u) == 55u);
    assert(dm1_v1_i34e_title_pc34_deadline_ms(0u, 55u, 1u) == 110u);
    assert(dm1_v1_i34e_title_pc34_deadline_ms(0u, 56u, 1u) == 110u);
    /* This phase is exactly on an IRQ in rational PIT time. */
    assert(dm1_v1_i34e_title_pc34_deadline_ms(0u, 65536000u, 1u) == 65536055u);
    assert(dm1_v1_i34e_title_pc34_deadline_ms(1000u, 1000u, 0u) == 1000u);
    assert(dm1_v1_i34e_title_pc34_deadline_ms(1000u, 1000u, 1u) == 1055u);
    assert(dm1_v1_i34e_title_pc34_deadline_ms(1000u, 1000u, 3u) == 1055u);
    assert(dm1_v1_i34e_title_pc34_deadline_ms(1000u, 1000u, 4u) == 1110u);
    assert(dm1_v1_i34e_title_pc34_deadline_ms(0u, 0u, 65535u) ==
           (UINT64_C(21845) * 65536000u + 1193181u) / 1193182u);
    /* Counter assignment precedes decode. Overrun consumes the wait; the
     * next assignment discards the old negative counter, not future time. */
    deadline = dm1_v1_i34e_title_pc34_deadline_ms(0u, 0u, 1u);
    assert(100u >= deadline);
    next = dm1_v1_i34e_title_pc34_deadline_ms(0u, 100u, 1u);
    assert(next == 110u && next > 100u);
    /* m11_intro_active_ticks is wall time minus accumulated focus pauses.
     * A paused wall clock must not consume callbacks or shift phase. */
    assert(dm1_v1_i34e_title_pc34_deadline_ms(0u, 1040u - 1000u, 1u) ==
           dm1_v1_i34e_title_pc34_deadline_ms(0u, 40u, 1u));
    assert(1054u - 1000u < deadline && 1055u - 1000u >= deadline);
    assert(dm1_v1_i34e_title_pc34_deadline_ms(10u, 9u, 1u) == UINT64_MAX);
    assert(dm1_v1_i34e_title_pc34_deadline_ms(UINT64_MAX - 1u,
           UINT64_MAX - 1u, 1u) == UINT64_MAX);
    assert(dm1_v1_i34e_title_pc34_deadline_ms(0u, UINT64_MAX, 65535u) == UINT64_MAX);
}

/* The original launch table distinguishes TITLE's start erase from END's
 * end erase. ReDMCSB IBMIO.C G8102:2411-2418; ANIM.C main:1880-1898. */
static int test_authentic_program(void) {
    const char *path = getenv("FIRESTAFF_DM1_I34E_IBMIO");
    uint8_t packed[8151];
    unsigned char *module = NULL;
    unsigned int module_bytes = 0u;
    char digest[65];
    FILE *file;
    unsigned int i;
    unsigned int title_matches = 0u, end_matches = 0u;
    static const char title[] = "title -ah -as";
    static const char end[] = "end -as -ae";
    if (!path || !path[0]) return 77;
    file = fopen(path, "rb");
    if (!file) return 1;
    if (fread(packed, 1u, sizeof(packed), file) != sizeof(packed) ||
        fgetc(file) != EOF) {
        fclose(file);
        return 1;
    }
    fclose(file);
    if (firestaff_x68k_media_receipt_sha256_hex(packed, sizeof(packed),
            digest, sizeof(digest)) != 0 ||
        strcmp(digest, "b3f7148643c95c595c9dafd88b0f00e84364bcdac19d63867549fce53759bad8") != 0 ||
        !V1_LZEXE_UnpackLoadModule091(packed, sizeof(packed),
                                    &module, &module_bytes)) return 1;
    for (i = 0u; i < module_bytes; ++i) {
        if (module_bytes - i >= sizeof(title) &&
            memcmp(module + i, title, sizeof(title)) == 0) ++title_matches;
        if (module_bytes - i >= sizeof(end) &&
            memcmp(module + i, end, sizeof(end)) == 0) ++end_matches;
    }
    free(module);
    if (title_matches != 1u || end_matches != 1u) {
        fprintf(stderr, "FAIL: original IBMIO launch flags title=%u end=%u\n",
                title_matches, end_matches);
        return 1;
    }
    puts("PASS: authenticated IBMIO selects TITLE -ah -as and END -as -ae");
    return 0;
}

static int test_authentic_media(void) {
    const char *path = getenv("FIRESTAFF_DM1_I34E_TITLE");
    FILE *file;
    uint8_t *data;
    long size;
    char digest[65];
    DM1_V1_I34ETitleAnimPc34 state;
    DM1_V1_I34ETitleAnimFramePc34 frame;
    unsigned frames = 0u;
    unsigned counter_units = 0u;
    int result;
    int sha_ok, hash_ok, init_ok;
    if (!path || !path[0]) return 77;
    file = fopen(path, "rb");
    if (!file) return 77;
    if (fseek(file, 0, SEEK_END) != 0 || (size = ftell(file)) <= 0 ||
        size > 1024L * 1024L || fseek(file, 0, SEEK_SET) != 0) {
        fclose(file); return 1;
    }
    data = (uint8_t *)malloc((size_t)size);
    if (!data || fread(data, 1u, (size_t)size, file) != (size_t)size) {
        free(data); fclose(file); return 1;
    }
    fclose(file);
    sha_ok = firestaff_x68k_media_receipt_sha256_hex(data, (size_t)size,
                                                       digest, sizeof(digest)) == 0;
    hash_ok = strcmp(digest, "adc7f1916eeef343849f23c047977d307495b29793b796a54aa427ba71dd3745") == 0;
    init_ok = dm1_v1_i34e_title_anim_pc34_init(&state, data, (size_t)size);
    if (!sha_ok || !hash_ok || !init_ok) {
        fprintf(stderr, "FAIL: authentic TITLE digest=%s size=%ld sha=%d hash=%d init=%d\n",
                digest, size, sha_ok, hash_ok, init_ok);
        free(data); return 1;
    }
    while ((result = dm1_v1_i34e_title_anim_pc34_step(&state, &frame)) == 1) {
        if (frame.source_counter_units != 1u) {
            free(data);
            return 1;
        }
        counter_units += frame.source_counter_units;
        ++frames;
    }
    free(data);
    if (result != 0 || frames != 53u || counter_units != 53u || !state.finished) {
        fprintf(stderr, "FAIL: authentic TITLE result=%d frames=%u offset=%zu size=%zu\n",
                result, frames, state.offset, state.size);
        return 1;
    }
    printf("PASS: authenticated I34E TITLE stream, %u frames\n", frames);
    return 0;
}

int main(void) {
    test_stream();
    test_default_pit_timing();
    test_source_ordered_erase();
    {
        int result = test_authentic_media();
        if (result == 77) {
            puts("SKIP: set FIRESTAFF_DM1_I34E_TITLE for authentic-media verification");
            return 77;
        }
        if (result != 0) return result;
    }
    {
        int result = test_authentic_program();
        if (result == 77) {
            puts("SKIP: set FIRESTAFF_DM1_I34E_IBMIO for original launch-flag verification");
            return 77;
        }
        if (result != 0) return result;
    }
    puts("PASS: I34E TITLE parser and bounded F8288 player");
    return 0;
}
