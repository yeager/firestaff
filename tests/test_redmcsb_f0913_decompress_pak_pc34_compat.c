#include "redmcsb_f0913_decompress_pak_pc34_compat.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
    unsigned int call_count;
    unsigned int seen_slot;
    redmcsb_f0913_prim_decompress_callback_pc34_compat seen_replacement;
    void *seen_context;
    bool result;
} redmcsb_f0913_hook_capture_pc34;

static void write_be16(uint8_t *bytes, uint16_t value)
{
    bytes[0] = (uint8_t)(value >> 8);
    bytes[1] = (uint8_t)value;
}

static __attribute__((unused)) bool capture_hook(
    void *context,
    unsigned int function_slot,
    redmcsb_f0913_prim_decompress_callback_pc34_compat replacement)
{
    redmcsb_f0913_hook_capture_pc34 *capture = context;

    capture->call_count++;
    capture->seen_slot = function_slot;
    capture->seen_replacement = replacement;
    capture->seen_context = context;
    return capture->result;
}

int main(void)
{
    uint8_t pak[REDMCSB_F0913_PAK_COMPRESSED_DATA_OFFSET_PC34 + 6u] = { 0 };
    uint16_t output[4] = { 0u, 0u, 0u, 0u };
    (void)output;
    redmcsb_f0913_hook_capture_pc34 capture = { 0u, 0u, NULL, NULL, true };
    (void)capture;

    write_be16(pak, REDMCSB_F0913_PAK_SIGNATURE_PC34);
    pak[7] = 4u;
    write_be16(pak + REDMCSB_F0913_PAK_WORD_TABLE_OFFSET_PC34, 0x1234u);
    write_be16(pak + REDMCSB_F0913_PAK_WORD_TABLE_OFFSET_PC34 + 254u,
               0x5678u);
    write_be16(pak + REDMCSB_F0913_PAK_WORD_TABLE_OFFSET_PC34 + 256u,
               0x9abcu);
    pak[REDMCSB_F0913_PAK_COMPRESSED_DATA_OFFSET_PC34 + 0u] = 0x00u;
    pak[REDMCSB_F0913_PAK_COMPRESSED_DATA_OFFSET_PC34 + 1u] = 0x7fu;
    pak[REDMCSB_F0913_PAK_COMPRESSED_DATA_OFFSET_PC34 + 2u] = 0x80u;
    pak[REDMCSB_F0913_PAK_COMPRESSED_DATA_OFFSET_PC34 + 3u] = 0x0fu;
    pak[REDMCSB_F0913_PAK_COMPRESSED_DATA_OFFSET_PC34 + 4u] = 0xbeu;
    pak[REDMCSB_F0913_PAK_COMPRESSED_DATA_OFFSET_PC34 + 5u] = 0xefu;

    assert(redmcsb_f0913_decompress_pak_pc34_compat(
               pak, sizeof(pak), output, 4u) ==
           REDMCSB_F0913_DECOMPRESS_PAK_OK_PC34);
    assert(output[0] == 0x1234u);
    assert(output[1] == 0x5678u);
    assert(output[2] == 0x9abcu);
    assert(output[3] == 0xbeefu);

    pak[0] = 0u;
    assert(redmcsb_f0913_decompress_pak_pc34_compat(
               pak, sizeof(pak), output, 4u) ==
           REDMCSB_F0913_DECOMPRESS_PAK_BAD_SIGNATURE_PC34);
    pak[0] = 0x52u;
    assert(redmcsb_f0913_decompress_pak_pc34_compat(
               pak, sizeof(pak) - 1u, output, 4u) ==
           REDMCSB_F0913_DECOMPRESS_PAK_HOST_TRUNCATED_INPUT_PC34);
    assert(redmcsb_f0913_decompress_pak_pc34_compat(
               pak, sizeof(pak), output, 3u) ==
           REDMCSB_F0913_DECOMPRESS_PAK_HOST_OUTPUT_TOO_SMALL_PC34);

    assert(redmcsb_f0913_install_prim_decompress_hook_pc34_compat(
        capture_hook, &capture));
    assert(capture.call_count == 1u);
    assert(capture.seen_slot ==
           REDMCSB_F0913_PRIM_DECOMPRESS_CODE_SEGMENT_SLOT_PC34);
    assert(capture.seen_replacement == redmcsb_f0913_decompress_pak_pc34_compat);
    assert(capture.seen_context == &capture);
    assert(!redmcsb_f0913_install_prim_decompress_hook_pc34_compat(NULL,
                                                                      &capture));
    return 0;
}
