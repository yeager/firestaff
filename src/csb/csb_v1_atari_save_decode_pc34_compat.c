#include "csb_v1_atari_save_decode_pc34_compat.h"

#include <string.h>

enum {
    CSB_BLOCK1_BYTES = 256,
    CSB_BLOCK2_BYTES = 256,
    CSB_BLOCK3_BYTES = 128,
    CSB_ITEM16_BYTES = 16,
    CSB_CHARACTER_BYTES = 3328,
    CSB_TIMER_BYTES = 10,
    CSB_TIMER_QUEUE_BYTES = 2,
    CSB_BLOCK1_BLOCK2_KEY_OFFSET = 58,
    CSB_BLOCK2_BLOCK3_KEY_WORD = 28,
    CSB_BLOCK2_ITEM16_KEY_WORD = 29,
    CSB_BLOCK2_CHARACTER_KEY_WORD = 30,
    CSB_BLOCK2_TIMER_KEY_WORD = 31,
    CSB_BLOCK2_TIMER_QUEUE_KEY_WORD = 32,
    CSB_BLOCK2_BLOCK3_CHECKSUM_WORD = 44,
    CSB_BLOCK2_ITEM16_CHECKSUM_WORD = 45,
    CSB_BLOCK2_CHARACTER_CHECKSUM_WORD = 46,
    CSB_BLOCK2_TIMER_CHECKSUM_WORD = 47,
    CSB_BLOCK2_TIMER_QUEUE_CHECKSUM_WORD = 48,
    CSB_BLOCK3_TIMER_COUNT_WORD = 12,
    CSB_BLOCK3_ITEM16_COUNT_WORD = 15,
    CSB_BLOCK3_TIMER_CAPACITY_WORD = 14,
    CSB_BLOCK3_ITEM16_CAPACITY_WORD = 23
};

static uint16_t read_be16(const uint8_t *p)
{
    return (uint16_t)(((uint16_t)p[0] << 8) | p[1]);
}

static uint16_t block1_checksum(const uint8_t *block)
{
    uint16_t sum = 0;
    size_t i;
    for (i = 0; i < CSB_BLOCK1_BYTES; i += 8) {
        sum = (uint16_t)(sum + read_be16(block + i));
        sum ^= read_be16(block + i + 2);
        sum = (uint16_t)(sum - read_be16(block + i + 4));
        sum ^= read_be16(block + i + 6);
    }
    return sum;
}

/* DMWeb's word-oriented save encryption, preserving the source endian. */
static uint16_t decrypt_words(const uint8_t *input, size_t size,
                              uint16_t key, uint16_t *output)
{
    uint16_t temporary = key;
    uint16_t checksum = key;
    size_t count = size / 2u;
    size_t i;
    for (i = 0; i < count; ++i) {
        uint16_t encrypted = read_be16(input + i * 2u);
        checksum = (uint16_t)(checksum + encrypted);
        uint16_t plain = (uint16_t)(encrypted ^ temporary);
        if (output) output[i] = plain;
        checksum = (uint16_t)(checksum + plain);
        temporary = (uint16_t)(temporary + count - i);
    }
    return checksum;
}

static int add_size(size_t *offset, size_t count, size_t unit, size_t limit)
{
    if (count > (limit - *offset) / unit) return 0;
    *offset += count * unit;
    return 1;
}

const char *csb_v1_atari_save_decode_source_evidence_pc34_compat(void)
{
    return
        "DMWeb Saved Game Files: CSB MINI.DAT is a native saved game\n"
        "DMWeb Saved Game Files: big-endian encryption/checksum algorithm\n"
        "DMWeb GAMEBLOCK1/GAMEBLOCK2: CSB block offsets and capacities\n"
        "ReDMCSB LOADSAVE.C F0435: source save data owns dungeon handoff\n";
}

int csb_v1_atari_save_decode_pc34_compat(const uint8_t *bytes,
                                         size_t size,
                                         CSB_V1_AtariSaveInfo *out_info)
{
    uint16_t block2[CSB_BLOCK2_BYTES / 2u];
    uint16_t block3[CSB_BLOCK3_BYTES / 2u];
    size_t offset;
    size_t item16_bytes;
    size_t timer_bytes;
    size_t queue_bytes;
    uint16_t key;
    uint16_t checksum;

    if (!bytes || !out_info) return CSB_V1_ATARI_SAVE_ERR_NULL;
    memset(out_info, 0, sizeof(*out_info));
    if (size < CSB_BLOCK1_BYTES + CSB_BLOCK2_BYTES + CSB_BLOCK3_BYTES) {
        return CSB_V1_ATARI_SAVE_ERR_TRUNCATED;
    }

    key = read_be16(bytes + CSB_BLOCK1_BLOCK2_KEY_OFFSET);
    (void)decrypt_words(bytes + CSB_BLOCK1_BYTES, CSB_BLOCK2_BYTES, key, block2);
    {
        uint16_t plaintext_sum = 0;
        size_t i;
        for (i = 0; i < 128u; ++i) plaintext_sum = (uint16_t)(plaintext_sum + block2[i]);
        if (block1_checksum(bytes) != plaintext_sum) {
            return CSB_V1_ATARI_SAVE_ERR_BLOCK2_CHECKSUM;
        }
    }

    checksum = decrypt_words(bytes + CSB_BLOCK1_BYTES + CSB_BLOCK2_BYTES,
                             CSB_BLOCK3_BYTES,
                             block2[CSB_BLOCK2_BLOCK3_KEY_WORD], block3);
    if (checksum != block2[CSB_BLOCK2_BLOCK3_CHECKSUM_WORD]) {
        return CSB_V1_ATARI_SAVE_ERR_BLOCK3_CHECKSUM;
    }

    if (block3[CSB_BLOCK3_ITEM16_CAPACITY_WORD] > 1024u ||
        block3[CSB_BLOCK3_TIMER_CAPACITY_WORD] > 4096u ||
        block3[CSB_BLOCK3_ITEM16_COUNT_WORD] > block3[CSB_BLOCK3_ITEM16_CAPACITY_WORD] ||
        block3[CSB_BLOCK3_TIMER_COUNT_WORD] > block3[CSB_BLOCK3_TIMER_CAPACITY_WORD]) {
        return CSB_V1_ATARI_SAVE_ERR_COUNTS;
    }

    item16_bytes = (size_t)block3[CSB_BLOCK3_ITEM16_CAPACITY_WORD] * CSB_ITEM16_BYTES;
    timer_bytes = (size_t)block3[CSB_BLOCK3_TIMER_CAPACITY_WORD] * CSB_TIMER_BYTES;
    queue_bytes = (size_t)block3[CSB_BLOCK3_TIMER_CAPACITY_WORD] * CSB_TIMER_QUEUE_BYTES;
    offset = CSB_BLOCK1_BYTES + CSB_BLOCK2_BYTES + CSB_BLOCK3_BYTES;
    if (!add_size(&offset, item16_bytes, 1u, size) ||
        !add_size(&offset, CSB_CHARACTER_BYTES, 1u, size) ||
        !add_size(&offset, timer_bytes, 1u, size) ||
        !add_size(&offset, queue_bytes, 1u, size)) {
        return CSB_V1_ATARI_SAVE_ERR_TRUNCATED;
    }

    /* Validate encrypted sections before exposing the unencrypted dungeon. */
    checksum = decrypt_words(bytes + CSB_BLOCK1_BYTES + CSB_BLOCK2_BYTES + CSB_BLOCK3_BYTES,
                             item16_bytes, block2[CSB_BLOCK2_ITEM16_KEY_WORD], NULL);
    if (checksum != block2[CSB_BLOCK2_ITEM16_CHECKSUM_WORD]) return CSB_V1_ATARI_SAVE_ERR_ITEM16_CHECKSUM;
    checksum = decrypt_words(bytes + CSB_BLOCK1_BYTES + CSB_BLOCK2_BYTES + CSB_BLOCK3_BYTES + item16_bytes,
                             CSB_CHARACTER_BYTES, block2[CSB_BLOCK2_CHARACTER_KEY_WORD], NULL);
    if (checksum != block2[CSB_BLOCK2_CHARACTER_CHECKSUM_WORD]) return CSB_V1_ATARI_SAVE_ERR_CHARACTER_CHECKSUM;
    checksum = decrypt_words(bytes + CSB_BLOCK1_BYTES + CSB_BLOCK2_BYTES + CSB_BLOCK3_BYTES + item16_bytes + CSB_CHARACTER_BYTES,
                             timer_bytes, block2[CSB_BLOCK2_TIMER_KEY_WORD], NULL);
    if (checksum != block2[CSB_BLOCK2_TIMER_CHECKSUM_WORD]) return CSB_V1_ATARI_SAVE_ERR_TIMER_CHECKSUM;
    checksum = decrypt_words(bytes + CSB_BLOCK1_BYTES + CSB_BLOCK2_BYTES + CSB_BLOCK3_BYTES + item16_bytes + CSB_CHARACTER_BYTES + timer_bytes,
                             queue_bytes, block2[CSB_BLOCK2_TIMER_QUEUE_KEY_WORD], NULL);
    if (checksum != block2[CSB_BLOCK2_TIMER_QUEUE_CHECKSUM_WORD]) return CSB_V1_ATARI_SAVE_ERR_TIMER_QUEUE_CHECKSUM;

    out_info->game_time = ((uint32_t)block3[0] << 16) | block3[1];
    out_info->random_seed = ((uint32_t)block3[2] << 16) | block3[3];
    out_info->leader_hand_thing = (int16_t)block3[4];
    out_info->champion_count = (int16_t)block3[5];
    out_info->party_x = (int16_t)block3[6];
    out_info->party_y = (int16_t)block3[7];
    out_info->party_direction = (int16_t)block3[8];
    out_info->party_map_index = (int16_t)block3[9];
    out_info->timer_count = (int16_t)block3[CSB_BLOCK3_TIMER_COUNT_WORD];
    out_info->item16_count = (int16_t)block3[CSB_BLOCK3_ITEM16_COUNT_WORD];
    out_info->timer_capacity = (int16_t)block3[CSB_BLOCK3_TIMER_CAPACITY_WORD];
    out_info->item16_capacity = (int16_t)block3[CSB_BLOCK3_ITEM16_CAPACITY_WORD];
    out_info->dungeon_offset = offset;
    out_info->dungeon_size = size - offset;
    return CSB_V1_ATARI_SAVE_OK;
}
