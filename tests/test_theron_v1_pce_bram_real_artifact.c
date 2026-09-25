#include "theron_v1_srm_classifier.h"
#include "theron_v1_startup_save_resume.h"
#include "theron_v1_track02_campaign_mask_source.h"
#include "theron_v1_track02_retrieval_text_source.h"
#include "theron_v1_world.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RAW_SECTOR_BYTES 2352u
#define USER_SECTOR_BYTES 2048u
#define USER_DATA_OFFSET 16u
#define SAVE_MANAGER_RAW_SECTOR_INDEX 1196u
#define SAVE_MANAGER_LOAD_ADDRESS 0x4000u
#define SAVE_MANAGER_READ_PC 0x413du
#define SAVE_MANAGER_WRITE_ARGS_PC 0x4182u
#define STAGE2_HANDOFF_RAW_SECTOR_INDEX 1224u
#define JP_STAGE2_HANDOFF_RAW_SECTOR_INDEX 1223u
#define STAGE2_HANDOFF_USER_OFFSET 151u
#define STAGE2_SAVE_SUPPORT_USER_OFFSET 0x3d1fu
#define STAGE2_SELECTED_SLOT_USER_OFFSET 0x3d74u
#define US_DUNGEON_RESTORE_USER_OFFSET 0x248u
#define JP_DUNGEON_RESTORE_USER_OFFSET 0x245u

static uint8_t *load_track02_user_data(const char *path, size_t *out_size) {
    FILE *file = fopen(path, "rb");
    long raw_size;
    size_t sectors;
    size_t index;
    uint8_t *raw;
    uint8_t *user_data;
    if (!file || fseek(file, 0, SEEK_END) != 0 ||
        (raw_size = ftell(file)) <= 0 || fseek(file, 0, SEEK_SET) != 0) {
        if (file) fclose(file);
        return NULL;
    }
    if ((size_t)raw_size % RAW_SECTOR_BYTES != 0u) {
        fclose(file);
        return NULL;
    }
    raw = (uint8_t *)malloc((size_t)raw_size);
    if (!raw || fread(raw, 1u, (size_t)raw_size, file) != (size_t)raw_size) {
        free(raw);
        fclose(file);
        return NULL;
    }
    fclose(file);
    sectors = (size_t)raw_size / RAW_SECTOR_BYTES;
    user_data = (uint8_t *)malloc(sectors * USER_SECTOR_BYTES);
    if (!user_data) {
        free(raw);
        return NULL;
    }
    for (index = 0u; index < sectors; ++index) {
        memcpy(user_data + index * USER_SECTOR_BYTES,
               raw + index * RAW_SECTOR_BYTES + USER_DATA_OFFSET,
               USER_SECTOR_BYTES);
    }
    free(raw);
    *out_size = sectors * USER_SECTOR_BYTES;
    return user_data;
}

static int load_exact(const char *path, uint8_t *bytes, size_t size) {
    FILE *file = fopen(path, "rb");
    int ok;
    if (!file) return 0;
    ok = fread(bytes, 1u, size, file) == size && fgetc(file) == EOF &&
        !ferror(file);
    fclose(file);
    return ok;
}

static uint8_t *load_file(const char *path, size_t *out_size) {
    FILE *file = fopen(path, "rb");
    long size;
    uint8_t *bytes;
    if (!file || fseek(file, 0L, SEEK_END) != 0 ||
        (size = ftell(file)) <= 0L || fseek(file, 0L, SEEK_SET) != 0) {
        if (file) fclose(file);
        return NULL;
    }
    bytes = (uint8_t *)malloc((size_t)size);
    if (!bytes || fread(bytes, 1u, (size_t)size, file) != (size_t)size) {
        free(bytes);
        fclose(file);
        return NULL;
    }
    fclose(file);
    *out_size = (size_t)size;
    return bytes;
}

static int set_test_env(const char *name, const char *value) {
#if defined(_WIN32) || defined(_WIN64)
    return _putenv_s(name, value ? value : "") == 0;
#else
    return value ? setenv(name, value, 1) == 0 : unsetenv(name) == 0;
#endif
}

int main(int argc, char **argv) {
    Theron_V1PceBramReceipt receipt;
    Theron_V1PceBramBodyReceipt body;
    Theron_V1PceBramBodyReceipt rejected_body;
    Theron_V1PceBramRecordReceipt record;
    Theron_V1PceBramRecordReceipt rejected_record;
    Theron_Track02CampaignMaskSource source;
    Theron_Track02RetrievalTextSource retrieval;
    Theron_V1StartupSaveResume startup_save;
    Theron_V1StartupContinueAvailability availability;
    Theron_V1_World world;
    uint8_t *user_data;
    uint8_t main_ram[8192];
    uint8_t save_manager_code[8192];
    size_t user_data_size = 0u;
    uint32_t saved_seeds[THERON_DUNGEON_COUNT];
    Theron_V1_Party saved_party;
    static const uint16_t field_bases[6] = {
        0x2a2cu, 0x2a7cu, 0x2accu, 0x2b1cu, 0x2b6cu, 0x2bbcu
    };
    static const uint8_t original_read_args[] = {
        0xa9, 0xa6, 0x85, 0xf8, 0xa9, 0x41, 0x85, 0xf9,
        0xa9, 0x13, 0x85, 0xfa, 0xa9, 0x53, 0x85, 0xfb,
        0xa9, 0x99, 0x85, 0xfc, 0xa9, 0x01, 0x85, 0xfd,
        0xa9, 0x00, 0x85, 0xfe, 0xa9, 0x00, 0x85, 0xff,
        0x20, 0x4e, 0xe0
    };
    static const uint8_t original_write_args[] = {
        0xa9, 0xa6, 0x85, 0xf8, 0xa9, 0x41, 0x85, 0xf9,
        0xa9, 0x7a, 0x85, 0xfa, 0xa9, 0x51, 0x85, 0xfb,
        0xa9, 0x99, 0x85, 0xfc, 0xa9, 0x01, 0x85, 0xfd,
        0xa9, 0x00, 0x85, 0xfe, 0xa9, 0x00, 0x85, 0xff,
        0x20, 0x51, 0xe0
    };
    static const uint8_t stage2_selected_slot_handoff[] = {
        0xad, 0x8a, 0x27, 0xae, 0x8b, 0x27, 0xac, 0x8c, 0x27,
        0xc8, 0x9c, 0xff, 0x26, 0x73, 0xff, 0x26, 0x00, 0x27,
        0x00, 0x11, 0x4c, 0x00, 0x38
    };
    static const uint8_t dungeon_selected_slot_store[] = {
        0x8d, 0xb7, 0x42, 0x8c, 0xb8, 0x42
    };
    static const uint8_t dungeon_save_body_restore[] = {
        0xad, 0x7c, 0x26, 0xf0, 0x06, 0x29, 0x7f, 0xc9,
        0x07, 0x90, 0x01, 0x60, 0x82, 0xc2,
        0xbd, 0x7d, 0x26, 0x99, 0x78, 0x29, 0x99, 0x80,
        0x29, 0xbd, 0x7e, 0x26, 0x99, 0x7c, 0x29, 0x99,
        0x84, 0x29, 0x98, 0x18, 0x69, 0x10, 0xa8, 0xe8,
        0xe8, 0xe0, 0x06, 0x90, 0xe3, 0x82, 0xc2,
        0xbd, 0x83, 0x26, 0x99, 0xf4, 0x29, 0x99, 0x10,
        0x2a, 0xc8, 0xc8, 0xc8, 0xc8, 0xe8, 0xe0, 0x07,
        0x90, 0xee, 0x82, 0xc2,
        0xbd, 0x8a, 0x26, 0x99, 0x2c, 0x2a,
        0xbd, 0x9e, 0x26, 0x99, 0x7c, 0x2a,
        0xbd, 0xb2, 0x26, 0x99, 0xcc, 0x2a,
        0xbd, 0xc6, 0x26, 0x99, 0x1c, 0x2b,
        0xbd, 0xda, 0x26, 0x99, 0x6c, 0x2b,
        0xbd, 0xee, 0x26, 0x99, 0xbc, 0x2b,
        0xc8, 0xc8, 0xc8, 0xc8, 0xe8, 0xe0, 0x14, 0x90,
        0xd3, 0x60
    };
    static const uint8_t jp_dungeon_save_body_restore[] = {
        0xad, 0x7c, 0x26, 0xf0, 0x06, 0x29, 0x7f, 0xc9,
        0x07, 0x90, 0x01, 0x60, 0x82, 0xc2,
        0xbd, 0x7d, 0x26, 0x99, 0x77, 0x29, 0x99, 0x7f,
        0x29, 0xbd, 0x7e, 0x26, 0x99, 0x7b, 0x29, 0x99,
        0x83, 0x29, 0x98, 0x18, 0x69, 0x10, 0xa8, 0xe8,
        0xe8, 0xe0, 0x06, 0x90, 0xe3, 0x82, 0xc2,
        0xbd, 0x83, 0x26, 0x99, 0xf3, 0x29, 0x99, 0x0f,
        0x2a, 0xc8, 0xc8, 0xc8, 0xc8, 0xe8, 0xe0, 0x07,
        0x90, 0xee, 0x82, 0xc2,
        0xbd, 0x8a, 0x26, 0x99, 0x2b, 0x2a,
        0xbd, 0x9e, 0x26, 0x99, 0x7b, 0x2a,
        0xbd, 0xb2, 0x26, 0x99, 0xcb, 0x2a,
        0xbd, 0xc6, 0x26, 0x99, 0x1b, 0x2b,
        0xbd, 0xda, 0x26, 0x99, 0x6b, 0x2b,
        0xbd, 0xee, 0x26, 0x99, 0xbb, 0x2b,
        0xc8, 0xc8, 0xc8, 0xc8, 0xe8, 0xe0, 0x14, 0x90,
        0xd3, 0x60
    };
    static const size_t us_dungeon_store_sectors[7] = {
        260u, 388u, 516u, 644u, 772u, 900u, 1028u
    };
    static const size_t jp_dungeon_store_sectors[7] = {
        259u, 387u, 515u, 643u, 771u, 899u, 1027u
    };
    static const uint8_t us_max_vitals_clamp[] = {
        0xbd, 0x80, 0x29, 0xdd, 0x78, 0x29, 0xbd, 0x84, 0x29,
        0xfd, 0x7c, 0x29, 0xb0, 0x0c, 0xbd, 0x80, 0x29, 0x9d,
        0x78, 0x29, 0xbd, 0x84, 0x29, 0x9d, 0x7c, 0x29
    };
    static const uint8_t jp_max_vitals_clamp[] = {
        0xbd, 0x7f, 0x29, 0xdd, 0x77, 0x29, 0xbd, 0x83, 0x29,
        0xfd, 0x7b, 0x29, 0xb0, 0x0c, 0xbd, 0x7f, 0x29, 0x9d,
        0x77, 0x29, 0xbd, 0x83, 0x29, 0x9d, 0x7b, 0x29
    };
    static const uint8_t us_max_attribute_compare[] = {
        0xbd, 0xf4, 0x29, 0xdd, 0x10, 0x2a, 0xb0, 0x02, 0xa0,
        0x0c, 0x8a, 0x18, 0x69, 0x04, 0xc9, 0x1c, 0x90, 0xed
    };
    static const uint8_t jp_max_attribute_compare[] = {
        0xbd, 0xf3, 0x29, 0xdd, 0x0f, 0x2a, 0xb0, 0x02, 0xa0,
        0x0c, 0x8a, 0x18, 0x69, 0x04, 0xc9, 0x1c, 0x90, 0xed
    };
    static const uint8_t us_skill_experience_read[] = {
        0xb9, 0xcc, 0x2a, 0x85, 0xb1, 0xb9, 0x1c, 0x2b, 0x85, 0xb2,
        0xb9, 0x6c, 0x2b, 0x85, 0xb3, 0xb9, 0xbc, 0x2b, 0x85, 0xb4
    };
    static const uint8_t jp_skill_experience_read[] = {
        0xb9, 0xcb, 0x2a, 0x85, 0xb1, 0xb9, 0x1b, 0x2b, 0x85, 0xb2,
        0xb9, 0x6b, 0x2b, 0x85, 0xb3, 0xb9, 0xbb, 0x2b, 0x85, 0xb4
    };
    static const uint8_t us_skill_temporary_add[] = {
        0xb9, 0x2c, 0x2a, 0x18, 0x65, 0xb1, 0x85, 0xb1, 0xb9,
        0x7c, 0x2a, 0x65, 0xb2, 0x85, 0xb2, 0xa5, 0xb3, 0x69,
        0x00, 0x85, 0xb3, 0xa5, 0xb4, 0x69, 0x00, 0x85, 0xb4, 0x60
    };
    static const uint8_t jp_skill_temporary_add[] = {
        0xb9, 0x2b, 0x2a, 0x18, 0x65, 0xb1, 0x85, 0xb1, 0xb9,
        0x7b, 0x2a, 0x65, 0xb2, 0x85, 0xb2, 0xa5, 0xb3, 0x69,
        0x00, 0x85, 0xb3, 0xa5, 0xb4, 0x69, 0x00, 0x85, 0xb4, 0x60
    };
    static const uint8_t us_skill_temporary_update[] = {
        0x79, 0x2c, 0x2a, 0x99, 0x2c, 0x2a, 0xb9, 0x7c,
        0x2a, 0x69, 0x00, 0x99, 0x7c, 0x2a
    };
    static const uint8_t jp_skill_temporary_update[] = {
        0x79, 0x2b, 0x2a, 0x99, 0x2b, 0x2a, 0xb9, 0x7b,
        0x2a, 0x69, 0x00, 0x99, 0x7b, 0x2a
    };
    static const uint8_t us_skill_experience_update[] = {
        0xb9, 0xcc, 0x2a, 0x18, 0x65, 0xb8, 0x99, 0xcc, 0x2a,
        0xb9, 0x1c, 0x2b, 0x65, 0xb9, 0x99, 0x1c, 0x2b,
        0xb9, 0x6c, 0x2b, 0x69, 0x00, 0x99, 0x6c, 0x2b,
        0xb9, 0xbc, 0x2b, 0x69, 0x00, 0x99, 0xbc, 0x2b, 0x60
    };
    static const uint8_t jp_skill_experience_update[] = {
        0xb9, 0xcb, 0x2a, 0x18, 0x65, 0xb8, 0x99, 0xcb, 0x2a,
        0xb9, 0x1b, 0x2b, 0x65, 0xb9, 0x99, 0x1b, 0x2b,
        0xb9, 0x6b, 0x2b, 0x69, 0x00, 0x99, 0x6b, 0x2b,
        0xb9, 0xbb, 0x2b, 0x69, 0x00, 0x99, 0xbb, 0x2b, 0x60
    };
    static const uint8_t dms_slot_offset_lookup[] = {
        0xac, 0xb8, 0x42, 0xb9, 0xf8, 0xdd, 0x85, 0xfe,
        0xb9, 0xfb, 0xdd, 0x85, 0xff, 0x20, 0x4e, 0xe0
    };
    static const uint8_t dms_slot_offset_table[] = {
        0x00, 0x88, 0x10, 0x00, 0x00, 0x01
    };
    static const uint8_t original_body_refresh_prefix[] = {
        0x68, 0x0d, 0x7c, 0x26, 0x8d, 0x7c, 0x26,
        0x82, 0xc2, 0xb9, 0x80, 0x29, 0x9d, 0x7d, 0x26,
        0xb9, 0x84, 0x29, 0x9d, 0x7e, 0x26, 0x98, 0x18,
        0x69, 0x10, 0xa8, 0xe8, 0xe8, 0xe0, 0x06, 0x90, 0xe9
    };
    static const uint8_t original_body_refresh_seven[] = {
        0x82, 0xc2, 0xb9, 0x10, 0x2a, 0x9d, 0x83, 0x26,
        0xc8, 0xc8, 0xc8, 0xc8, 0xe8, 0xe0, 0x07, 0x90, 0xf1
    };
    static const uint8_t original_body_refresh_columns[] = {
        0x82, 0xc2, 0xb9, 0x2c, 0x2a, 0x9d, 0x8a, 0x26,
        0xb9, 0x7c, 0x2a, 0x9d, 0x9e, 0x26,
        0xb9, 0xcc, 0x2a, 0x9d, 0xb2, 0x26,
        0xb9, 0x1c, 0x2b, 0x9d, 0xc6, 0x26,
        0xb9, 0x6c, 0x2b, 0x9d, 0xda, 0x26,
        0xb9, 0xbc, 0x2b, 0x9d, 0xee, 0x26,
        0xc8, 0xc8, 0xc8, 0xc8, 0xe8, 0xe0, 0x14, 0x90, 0xd3
    };
    static const uint8_t stage2_full_record_read[] = {
        0xa9, 0x72, 0x85, 0xf8, 0xa9, 0x7c, 0x85, 0xf9,
        0xa9, 0x49, 0x85, 0xfa, 0xa9, 0x7e, 0x85, 0xfb,
        0xa9, 0x99, 0x85, 0xfc, 0xa9, 0x01, 0x85, 0xfd,
        0xa9, 0x00, 0x85, 0xfe, 0xa9, 0x00, 0x85, 0xff,
        0x20, 0x4e, 0xe0, 0xa9, 0x72, 0x85, 0xf8,
        0xa9, 0x7c, 0x85, 0xf9, 0x20, 0x54, 0xe0
    };
    static const uint8_t stage2_selected_slot_pointer[] = {
        0xa9, 0x49, 0x85, 0x00, 0xa9, 0x7e, 0x85, 0x01,
        0xae, 0x8c, 0x27, 0xf0, 0x10, 0x18, 0xa5, 0x00,
        0x69, 0x88, 0x85, 0x00, 0xa5, 0x01, 0x69, 0x00,
        0x85, 0x01, 0xca, 0xd0, 0xf0, 0xa5, 0x00,
        0x8d, 0xad, 0x7d, 0x8d, 0xaf, 0x7d, 0xa5, 0x01,
        0x8d, 0xae, 0x7d, 0x8d, 0xb0, 0x7d, 0xee, 0xaf,
        0x7d, 0xd0, 0x03, 0xee, 0xb0, 0x7d
    };
    static const uint8_t stage2_slot_read[] = {
        0xa9, 0x72, 0x85, 0xf8, 0xa9, 0x7c, 0x85, 0xf9,
        0xa9, 0x49, 0x85, 0xfa, 0xa9, 0x7e, 0x85, 0xfb,
        0xa9, 0x88, 0x85, 0xfc, 0xa9, 0x00, 0x85, 0xfd,
        0x20, 0xbd, 0x7d, 0x20, 0x4e, 0xe0
    };
    static const uint8_t stage2_slot_clear[] = {
        0x62, 0x92, 0x00, 0x73, 0x49, 0x7e, 0x4a, 0x7e,
        0x87, 0x00, 0xad, 0x8d, 0x27, 0xf0, 0x04, 0xa9,
        0x80, 0x92, 0x00, 0x60
    };
    static const uint8_t jp_stage2_slot_read[] = {
        0xa9, 0x74, 0x85, 0xf8, 0xa9, 0x7c, 0x85, 0xf9,
        0xa9, 0x4b, 0x85, 0xfa, 0xa9, 0x7e, 0x85, 0xfb,
        0xa9, 0x88, 0x85, 0xfc, 0xa9, 0x00, 0x85, 0xfd,
        0x20, 0xbf, 0x7d, 0x20, 0x4e, 0xe0
    };
    static const uint8_t jp_stage2_slot_clear[] = {
        0x62, 0x92, 0x00, 0x73, 0x4b, 0x7e, 0x4c, 0x7e,
        0x87, 0x00, 0xad, 0x8d, 0x27, 0xf0, 0x04, 0xa9,
        0x80, 0x92, 0x00, 0x60
    };
    size_t field;
    size_t index;
    if (argc != 7 && argc != 8) return 2;
    if (theron_v1_pce_bram_campaign_byte_restorable(0u) ||
        !theron_v1_pce_bram_campaign_byte_restorable(1u) ||
        !theron_v1_pce_bram_campaign_byte_restorable(6u) ||
        theron_v1_pce_bram_campaign_byte_restorable(7u) ||
        /* The authentic completion dispatcher records these later bits,
         * but the separately authenticated dungeon restore consumer rejects
         * every masked value >= 7. Keep that original incompatibility
         * visible instead of silently broadening Continue in Firestaff. */
        theron_v1_pce_bram_campaign_byte_restorable(0x08u) ||
        theron_v1_pce_bram_campaign_byte_restorable(0x10u) ||
        theron_v1_pce_bram_campaign_byte_restorable(0x20u) ||
        !theron_v1_pce_bram_campaign_byte_restorable(0x80u) ||
        !theron_v1_pce_bram_campaign_byte_restorable(0x86u) ||
        theron_v1_pce_bram_campaign_byte_restorable(0x87u)) {
        fputs("original campaign-byte admission boundary drifted\n", stderr);
        return 1;
    }
    if (theron_v1_pce_bram_classify_path(argv[1], &receipt) != THERON_V1_PCE_BRAM_READY ||
        receipt.size_bytes != THERON_V1_PCE_BRAM_BYTES || !receipt.hubm_header_seen ||
        !receipt.theron_save_disk_marker_seen || receipt.theron_save_disk_marker_offset != 0x16u ||
        !receipt.save_record_layout_proven ||
        receipt.save_record_offset != 0x10u ||
        receipt.save_record_bytes != THERON_V1_PCE_BRAM_RECORD_BYTES ||
        receipt.save_data_offset != 0x20u ||
        receipt.save_data_bytes != THERON_V1_PCE_BRAM_DATA_BYTES ||
        receipt.save_slot_bytes != THERON_V1_PCE_BRAM_SLOT_BYTES ||
        receipt.save_slot_count != THERON_V1_PCE_BRAM_SLOT_COUNT ||
        receipt.save_trailing_bytes != 1u ||
        !receipt.save_slot_tail_unconsumed_padding ||
        !receipt.selected_slot_layout_proven ||
        receipt.selected_slot_index != 0u ||
        receipt.selected_slot_offset != 0x20u ||
        !receipt.save_body_layout_proven || receipt.save_body_offset != 0x20u ||
        receipt.save_body_bytes != 0x86u ||
        receipt.serialized_campaign_byte_offset != 0x20u ||
        receipt.serialized_campaign_byte != 0x01u ||
        !receipt.save_body_campaign_valid ||
        receipt.bytes_fnv1a == 0u) {
        fprintf(stderr, "progressed real Theron PC Engine SRAM was not admitted\n");
        return 1;
    }
    if (theron_v1_pce_bram_classify_path(argv[2], &receipt) != THERON_V1_PCE_BRAM_WRONG_SIZE ||
        receipt.hubm_header_seen || receipt.theron_save_disk_marker_seen) {
        fprintf(stderr, "text dump was not rejected by exact-size gate\n");
        return 1;
    }
    if (!theron_v1_pce_bram_decode_original_body_path(argv[1], &body) ||
        !body.layout_verified || !body.semantics_verified ||
        body.body_fnv1a != 0xb37e696eu ||
        body.ram_267c_campaign_byte != 0x01u ||
        memcmp(body.ram_267d_2682,
               "\xaf\x00\xdc\x05\x32\x00", 6u) != 0 ||
        memcmp(body.ram_2683_2689,
               "\x50\x32\x28\x28\x2d\x28\x2d", 7u) != 0 ||
        body.ram_268a_2701[2][0] != 0xe8u ||
        body.ram_268a_2701[3][0] != 0x03u) {
        fputs("authentic Backup RAM body did not match original writer layout\n",
              stderr);
        return 1;
    }
    if (body.theron_max_health != 175u ||
        body.theron_max_stamina != 1500u ||
        body.theron_max_mana != 50u ||
        memcmp(body.theron_max_attributes,
               "\x50\x32\x28\x28\x2d\x28\x2d", 7u) != 0) {
        fputs("authentic Theron maximum vitals/attributes were not decoded\n",
              stderr);
        return 1;
    }
    for (index = 0u; index < 20u; ++index) {
        uint16_t temporary =
            (uint16_t)body.ram_268a_2701[0][index] |
            ((uint16_t)body.ram_268a_2701[1][index] << 8);
        uint32_t experience =
            (uint32_t)body.ram_268a_2701[2][index] |
            ((uint32_t)body.ram_268a_2701[3][index] << 8) |
            ((uint32_t)body.ram_268a_2701[4][index] << 16) |
            ((uint32_t)body.ram_268a_2701[5][index] << 24);
        if (body.theron_skill_temporary_experience[index] != temporary ||
            body.theron_skill_experience[index] != experience) {
            fputs("authentic Theron skill experience was not decoded\n",
                  stderr);
            return 1;
        }
    }
    if (theron_v1_pce_bram_decode_original_body_path(argv[2], &rejected_body) ||
        rejected_body.layout_verified) {
        fputs("non-BRAM input reached the original body decoder\n", stderr);
        return 1;
    }
    if (!theron_v1_pce_bram_decode_original_record_path(argv[1], &record) ||
        !record.layout_verified || !record.slot_tail_unconsumed_padding ||
        record.data_fnv1a != 0x0ce6b7bau ||
        memcmp(record.slots[0], &body.ram_267c_campaign_byte, 1u) != 0 ||
        memcmp(record.slots[0] + 1u, body.ram_267d_2682, 6u) != 0 ||
        record.slots[0][0x86u] != 0u || record.slots[0][0x87u] != 0u ||
        memcmp(record.slots[1], record.slots[2],
               THERON_V1_PCE_BRAM_SLOT_BYTES) != 0 ||
        record.selected_slot_index != 0u) {
        fputs("complete authentic DMS-SG.001 record layout did not match\n",
              stderr);
        return 1;
    }
    if (theron_v1_pce_bram_decode_original_record_path(argv[2],
                                                        &rejected_record) ||
        rejected_record.layout_verified) {
        fputs("non-BRAM input reached the complete original record decoder\n",
              stderr);
        return 1;
    }
    if (!load_exact(argv[4], main_ram, sizeof(main_ram))) {
        fputs("same-session authentic main RAM capture is unavailable\n", stderr);
        return 1;
    }
    if (!load_exact(argv[5], save_manager_code,
                    sizeof(save_manager_code))) {
        fputs("authentic DMS-SG.001 code page is unavailable\n", stderr);
        return 1;
    }
    if (memcmp(save_manager_code + 0x1e5cu, dms_slot_offset_lookup,
               sizeof(dms_slot_offset_lookup)) != 0 ||
        memcmp(save_manager_code + 0x1df9u, dms_slot_offset_table,
               sizeof(dms_slot_offset_table)) != 0) {
        fputs("original selected-slot offset lookup drifted\n", stderr);
        return 1;
    }
    if (memcmp(save_manager_code + 0x1e73u,
               original_body_refresh_prefix,
               sizeof(original_body_refresh_prefix)) != 0 ||
        memcmp(save_manager_code + 0x1e93u,
               original_body_refresh_seven,
               sizeof(original_body_refresh_seven)) != 0 ||
        memcmp(save_manager_code + 0x1ea4u,
               original_body_refresh_columns,
               sizeof(original_body_refresh_columns)) != 0) {
        fputs("original body refresh read/write direction drifted\n", stderr);
        return 1;
    }
    for (index = 0u; index < 3u; ++index) {
        uint16_t base = (uint16_t)(0x2980u + index * 0x10u);
        if (body.ram_267d_2682[index * 2u] !=
                main_ram[base - 0x2000u] ||
            body.ram_267d_2682[index * 2u + 1u] !=
                main_ram[base + 4u - 0x2000u]) {
            fputs("original six-byte writer source does not match RAM\n", stderr);
            return 1;
        }
    }
    for (index = 0u; index < 7u; ++index) {
        if (body.ram_2683_2689[index] !=
            main_ram[0x2a10u + index * 4u - 0x2000u]) {
            fputs("original seven-byte writer source does not match RAM\n", stderr);
            return 1;
        }
    }
    for (field = 0u; field < 6u; ++field) {
        for (index = 0u; index < 20u; ++index) {
            if (body.ram_268a_2701[field][index] !=
                main_ram[field_bases[field] + index * 4u - 0x2000u]) {
                fputs("original 20-record writer source does not match RAM\n",
                      stderr);
                return 1;
            }
        }
    }

    user_data = load_track02_user_data(argv[3], &user_data_size);
    if (!user_data || !theron_v1_track02_decode_campaign_mask_source(
            user_data, user_data_size, 2, &source) ||
        !theron_v1_track02_decode_retrieval_text_source(
            user_data, user_data_size, 2, &retrieval)) {
        free(user_data);
        fputs("authentic US Track 02 campaign source was not admitted\n",
              stderr);
        return 1;
    }
    {
        size_t overlay = SAVE_MANAGER_RAW_SECTOR_INDEX * USER_SECTOR_BYTES;
        size_t read_offset = overlay + SAVE_MANAGER_READ_PC -
                             SAVE_MANAGER_LOAD_ADDRESS;
        size_t write_offset = overlay + SAVE_MANAGER_WRITE_ARGS_PC -
                              SAVE_MANAGER_LOAD_ADDRESS;
        if (read_offset + sizeof(original_read_args) > user_data_size ||
            write_offset + sizeof(original_write_args) > user_data_size ||
            memcmp(user_data + read_offset, original_read_args,
                   sizeof(original_read_args)) != 0 ||
            memcmp(user_data + write_offset, original_write_args,
                   sizeof(original_write_args)) != 0) {
            free(user_data);
            fputs("authentic save-manager $0199-byte read/write calls drifted\n",
                  stderr);
            return 1;
        }
        if (STAGE2_HANDOFF_RAW_SECTOR_INDEX * USER_SECTOR_BYTES +
                STAGE2_HANDOFF_USER_OFFSET +
                sizeof(stage2_selected_slot_handoff) > user_data_size ||
            memcmp(user_data +
                       STAGE2_HANDOFF_RAW_SECTOR_INDEX * USER_SECTOR_BYTES +
                       STAGE2_HANDOFF_USER_OFFSET,
                   stage2_selected_slot_handoff,
                   sizeof(stage2_selected_slot_handoff)) != 0) {
            free(user_data);
            fputs("original selected-slot Stage 2 handoff drifted\n", stderr);
            return 1;
        }
        if (STAGE2_HANDOFF_RAW_SECTOR_INDEX * USER_SECTOR_BYTES +
                    STAGE2_SAVE_SUPPORT_USER_OFFSET +
                    sizeof(stage2_full_record_read) > user_data_size ||
            memcmp(user_data +
                       STAGE2_HANDOFF_RAW_SECTOR_INDEX * USER_SECTOR_BYTES +
                       STAGE2_SAVE_SUPPORT_USER_OFFSET,
                   stage2_full_record_read,
                   sizeof(stage2_full_record_read)) != 0 ||
            STAGE2_HANDOFF_RAW_SECTOR_INDEX * USER_SECTOR_BYTES +
                    STAGE2_SELECTED_SLOT_USER_OFFSET +
                    sizeof(stage2_selected_slot_pointer) > user_data_size ||
            memcmp(user_data +
                       STAGE2_HANDOFF_RAW_SECTOR_INDEX * USER_SECTOR_BYTES +
                       STAGE2_SELECTED_SLOT_USER_OFFSET,
                   stage2_selected_slot_pointer,
                   sizeof(stage2_selected_slot_pointer)) != 0) {
            free(user_data);
            fputs("original Stage 2 full-record/slot selector drifted\n",
                  stderr);
            return 1;
        }
        if (STAGE2_HANDOFF_RAW_SECTOR_INDEX * USER_SECTOR_BYTES + 0x3ce1u +
                    sizeof(stage2_slot_read) > user_data_size ||
            memcmp(user_data +
                       STAGE2_HANDOFF_RAW_SECTOR_INDEX * USER_SECTOR_BYTES +
                       0x3ce1u,
                   stage2_slot_read, sizeof(stage2_slot_read)) != 0 ||
            memcmp(user_data +
                       STAGE2_HANDOFF_RAW_SECTOR_INDEX * USER_SECTOR_BYTES +
                       0x3da9u,
                   stage2_slot_clear, sizeof(stage2_slot_clear)) != 0) {
            free(user_data);
            fputs("original Stage 2 $88-byte slot transport drifted\n", stderr);
            return 1;
        }
        for (index = 0u; index < 7u; ++index) {
            size_t store_offset =
                us_dungeon_store_sectors[index] * USER_SECTOR_BYTES;
            if (store_offset + sizeof(dungeon_selected_slot_store) >
                    user_data_size ||
                memcmp(user_data + store_offset,
                       dungeon_selected_slot_store,
                       sizeof(dungeon_selected_slot_store)) != 0) {
                free(user_data);
                fputs("original dungeon selected-slot store drifted\n",
                      stderr);
                return 1;
            }
            if (store_offset + US_DUNGEON_RESTORE_USER_OFFSET +
                    sizeof(dungeon_save_body_restore) > user_data_size ||
                memcmp(user_data + store_offset +
                           US_DUNGEON_RESTORE_USER_OFFSET,
                       dungeon_save_body_restore,
                       sizeof(dungeon_save_body_restore)) != 0) {
                free(user_data);
                fputs("original dungeon save-body restore consumer drifted\n",
                      stderr);
                return 1;
            }
            if (memcmp(user_data + store_offset + 0x49edu,
                       us_max_vitals_clamp,
                       sizeof(us_max_vitals_clamp)) != 0 ||
                memcmp(user_data + store_offset + 0x5b22u,
                       us_max_attribute_compare,
                       sizeof(us_max_attribute_compare)) != 0 ||
                memcmp(user_data + store_offset + 0x4ebau,
                       us_skill_experience_read,
                       sizeof(us_skill_experience_read)) != 0 ||
                memcmp(user_data + store_offset + 0x4f92u,
                       us_skill_temporary_add,
                       sizeof(us_skill_temporary_add)) != 0 ||
                memcmp(user_data + store_offset + 0x504du,
                       us_skill_temporary_update,
                       sizeof(us_skill_temporary_update)) != 0 ||
                memcmp(user_data + store_offset + 0x5247u,
                       us_skill_experience_update,
                       sizeof(us_skill_experience_update)) != 0) {
                free(user_data);
                fputs("original US Theron field consumers drifted\n", stderr);
                return 1;
            }
        }
    }
    free(user_data);
    user_data = load_track02_user_data(argv[6], &user_data_size);
    if (!user_data) {
        fputs("authentic JP Track 02 could not be read\n", stderr);
        return 1;
    }
    if (JP_STAGE2_HANDOFF_RAW_SECTOR_INDEX * USER_SECTOR_BYTES + 0x3ce3u +
                sizeof(jp_stage2_slot_read) > user_data_size ||
        memcmp(user_data +
                   JP_STAGE2_HANDOFF_RAW_SECTOR_INDEX * USER_SECTOR_BYTES +
                   0x3ce3u,
               jp_stage2_slot_read, sizeof(jp_stage2_slot_read)) != 0 ||
        memcmp(user_data +
                   JP_STAGE2_HANDOFF_RAW_SECTOR_INDEX * USER_SECTOR_BYTES +
                   0x3dabu,
               jp_stage2_slot_clear, sizeof(jp_stage2_slot_clear)) != 0) {
        free(user_data);
        fputs("original JP Stage 2 $88-byte slot transport drifted\n", stderr);
        return 1;
    }
    for (index = 0u; index < 7u; ++index) {
        size_t restore_offset =
            jp_dungeon_store_sectors[index] * USER_SECTOR_BYTES +
            JP_DUNGEON_RESTORE_USER_OFFSET;
        if (restore_offset + sizeof(jp_dungeon_save_body_restore) >
                user_data_size ||
            memcmp(user_data + restore_offset,
                   jp_dungeon_save_body_restore,
                   sizeof(jp_dungeon_save_body_restore)) != 0) {
            free(user_data);
            fputs("original JP dungeon save-body restore consumer drifted\n",
                  stderr);
            return 1;
        }
        {
            size_t store_offset =
                jp_dungeon_store_sectors[index] * USER_SECTOR_BYTES;
            if (memcmp(user_data + store_offset + 0x49dau,
                       jp_max_vitals_clamp,
                       sizeof(jp_max_vitals_clamp)) != 0 ||
                memcmp(user_data + store_offset + 0x5b0fu,
                       jp_max_attribute_compare,
                       sizeof(jp_max_attribute_compare)) != 0 ||
                memcmp(user_data + store_offset + 0x4ea7u,
                       jp_skill_experience_read,
                       sizeof(jp_skill_experience_read)) != 0 ||
                memcmp(user_data + store_offset + 0x4f7fu,
                       jp_skill_temporary_add,
                       sizeof(jp_skill_temporary_add)) != 0 ||
                memcmp(user_data + store_offset + 0x503au,
                       jp_skill_temporary_update,
                       sizeof(jp_skill_temporary_update)) != 0 ||
                memcmp(user_data + store_offset + 0x5234u,
                       jp_skill_experience_update,
                       sizeof(jp_skill_experience_update)) != 0) {
                free(user_data);
                fputs("original JP Theron field consumers drifted\n", stderr);
                return 1;
            }
        }
    }
    free(user_data);
    theron_v1_world_init(&world);
    {
        size_t track02_raw_size = 0u;
        uint8_t *track02_raw = load_file(argv[3], &track02_raw_size);
        world.party.champion_count = THERON_MAX_CHAMPIONS;
        if (!track02_raw || !theron_v1_party_refresh_us_source_records(
                &world.party, track02_raw, track02_raw_size,
                THERON_TRACK02_MD5_US_BIN)) {
            free(track02_raw);
            fputs("authentic US roster did not initialize the test party\n",
                  stderr);
            return 1;
        }
        free(track02_raw);
    }
    if (!theron_v1_world_bind_track02_retrieval_text_source(
            &world, &retrieval, 2) ||
        !theron_v1_world_bind_track02_campaign_mask_source(&world, &source, 2)) {
        fputs("authentic campaign source did not bind to the world\n", stderr);
        return 1;
    }
    if (argc == 8) {
        Theron_V1PceBramReceipt empty_receipt;
        Theron_V1PceBramBodyReceipt empty_body;
        Theron_V1StartupSaveResume empty_snapshot;
        Theron_V1StartupContinueAvailability empty_availability;
        Theron_V1_World unchanged_world = world;
        char empty_continue_receipt[160];

        if (theron_v1_pce_bram_classify_path(argv[7], &empty_receipt) !=
                THERON_V1_PCE_BRAM_READY ||
            !empty_receipt.save_body_layout_proven ||
            empty_receipt.save_body_campaign_valid ||
            empty_receipt.serialized_campaign_byte != 0u ||
            !theron_v1_pce_bram_decode_original_body_path(argv[7],
                                                           &empty_body) ||
            !empty_body.layout_verified || !empty_body.semantics_verified ||
            empty_body.ram_267c_campaign_byte != 0u ||
            !set_test_env("FIRESTAFF_THERON_BRAM_PATH", argv[7]) ||
            !theron_v1_startup_save_resume_evaluate(NULL, &empty_snapshot) ||
            empty_snapshot.resume_claim != THERON_V1_STARTUP_RESUME_NONE ||
            empty_snapshot.srm_recognized_slots != 0 ||
            empty_snapshot.srm_first_decoded_slot != -1 ||
            !set_test_env("FIRESTAFF_THERON_BRAM_PATH", argv[1]) ||
            !theron_v1_startup_continue_availability_from_state(
                empty_snapshot.resume_claim,
                empty_snapshot.tqsv_active_slot,
                empty_snapshot.srm_first_decoded_slot,
                empty_snapshot.srm_progress_import_status,
                &empty_availability) ||
            empty_availability.has_srm_continue ||
            empty_availability.has_any_continue) {
            fputs("authentic empty JP Backup RAM was advertised as Continue\n",
                  stderr);
            return 1;
        }

        memset(&empty_snapshot, 0, sizeof(empty_snapshot));
        empty_snapshot.tqsv_active_slot = -1;
        empty_snapshot.srm_first_recognized_slot = -1;
        empty_snapshot.srm_first_decoded_slot = -1;
        if (theron_v1_startup_save_resume_apply_explicit_path(
                &empty_snapshot, argv[7], NULL) ||
            empty_snapshot.resume_claim != THERON_V1_STARTUP_RESUME_NONE ||
            theron_v1_startup_restore_pce_bram_campaign_path(
                &world, argv[7], &empty_receipt) ||
            theron_v1_startup_restore_pce_bram_theron_path(
                &world, argv[7], &empty_receipt, &empty_body) ||
            theron_v1_startup_continue_srm_apply(
                &world, argv[7], 0, empty_continue_receipt,
                sizeof(empty_continue_receipt)) ||
            world.progression.quest_items_collected !=
                unchanged_world.progression.quest_items_collected ||
            world.party.champions[0].health !=
                unchanged_world.party.champions[0].health) {
            fputs("empty JP Backup RAM changed world state or accepted Continue\n",
                  stderr);
            return 1;
        }
    }
    if (!theron_v1_startup_save_resume_evaluate(NULL, &startup_save) ||
        startup_save.resume_claim != THERON_V1_STARTUP_RESUME_SRM ||
        startup_save.srm_first_decoded_slot != 0 ||
        strcmp(startup_save.srm_root, argv[1]) != 0) {
        fputs("startup did not discover authentic Backup RAM\n", stderr);
        return 1;
    }
    memset(&startup_save, 0, sizeof(startup_save));
    startup_save.tqsv_active_slot = -1;
    startup_save.srm_first_recognized_slot = -1;
    startup_save.srm_first_decoded_slot = -1;
    if (!theron_v1_startup_save_resume_apply_explicit_path(
            &startup_save, argv[1], NULL) ||
        startup_save.resume_claim != THERON_V1_STARTUP_RESUME_SRM ||
        startup_save.srm_first_decoded_slot != 0 ||
        startup_save.srm_progress_import_status !=
            THERON_V1_SRM_PROGRESS_IMPORT_OK ||
        strcmp(startup_save.srm_root, argv[1]) != 0 ||
        !theron_v1_startup_continue_availability_from_state(
            startup_save.resume_claim,
            startup_save.tqsv_active_slot,
            startup_save.srm_first_decoded_slot,
            startup_save.srm_progress_import_status,
            &availability) ||
        !availability.has_srm_continue || !availability.has_any_continue) {
        fputs("authentic Backup RAM was not offered to startup Continue\n",
              stderr);
        return 1;
    }
    {
        Theron_V1_World continued_world = world;
        char continue_receipt[160];
        if (!theron_v1_startup_continue_srm_apply(
                &continued_world,
                startup_save.srm_root,
                startup_save.srm_first_decoded_slot,
                continue_receipt,
                sizeof(continue_receipt)) ||
            strstr(continue_receipt, "original Backup RAM") == NULL ||
            continued_world.party.champions[0].max_health != 175 ||
            continued_world.party.champions[0].max_stamina != 1500 ||
            continued_world.party.champions[0].max_mana != 50 ||
            continued_world.object_count != 0 ||
            continued_world.timer_count != 0) {
            fputs("startup Continue did not consume authentic Backup RAM\n",
                  stderr);
            return 1;
        }
    }
    world.progression.current_dungeon = THERON_DUNGEON_5_SHADO;
    world.progression.current_level = 2u;
    world.progression.dungeon_playtime_seconds = 321u;
    world.object_count = 17;
    world.party.gold = 73u;
    memcpy(saved_seeds, world.progression.dungeon_seeds, sizeof(saved_seeds));
    saved_party = world.party;
    if (!theron_v1_startup_restore_pce_bram_campaign_path(
            &world, argv[1], &receipt) ||
        world.progression.quest_items_collected !=
            (uint8_t)(receipt.serialized_campaign_byte & 0x7fu) ||
        world.progression.current_dungeon != THERON_DUNGEON_5_SHADO ||
        world.progression.current_level != 2u ||
        world.progression.dungeon_playtime_seconds != 321u ||
        world.object_count != 17 ||
        memcmp(saved_seeds, world.progression.dungeon_seeds,
               sizeof(saved_seeds)) != 0 ||
        memcmp(&saved_party, &world.party, sizeof(saved_party)) != 0) {
        fputs("bounded real Backup RAM campaign restore changed unproven state\n",
              stderr);
        return 1;
    }
    {
        Theron_V1PceBramBodyReceipt restored_body;
        Theron_V1_Party before_party;
        uint32_t before_seeds[THERON_DUNGEON_COUNT];
        uint8_t before_inventory[THERON_INVENTORY_SLOTS];
        int16_t before_slots[THERON_EQUIP_SLOT_COUNT];

        if (strcmp(world.party.champions[0].name, "THERON") != 0) {
            fputs("authentic roster did not retain Theron in slot zero\n",
                  stderr);
            return 1;
        }
        world.party.champions[0].health = 1;
        world.party.champions[0].max_health = 2;
        world.party.champions[0].stamina = 3;
        world.party.champions[0].max_stamina = 4;
        world.party.champions[0].mana = 5;
        world.party.champions[0].max_mana = 6;
        world.party.champions[0].luck = 7;
        world.party.champions[0].strength = 8;
        world.party.champions[0].dexterity = 9;
        world.party.champions[0].wisdom = 10;
        world.party.champions[0].vitality = 11;
        world.party.champions[0].anti_magic = 12;
        world.party.champions[0].anti_fire = 13;
        memset(world.party.champions[0].skill_temporary_experience, 0,
               sizeof(world.party.champions[0].skill_temporary_experience));
        memset(world.party.champions[0].skill_experience, 0,
               sizeof(world.party.champions[0].skill_experience));
        before_party = world.party;
        memcpy(before_inventory, world.party.champions[0].inventory,
               sizeof(before_inventory));
        memcpy(before_slots, world.party.champions[0].slots,
               sizeof(before_slots));
        memcpy(before_seeds, world.progression.dungeon_seeds,
               sizeof(before_seeds));

        if (!theron_v1_startup_restore_pce_bram_theron_path(
                &world, argv[1], &receipt, &restored_body) ||
            !restored_body.semantics_verified ||
            world.party.champions[0].health != 175 ||
            world.party.champions[0].max_health != 175 ||
            world.party.champions[0].stamina != 1500 ||
            world.party.champions[0].max_stamina != 1500 ||
            world.party.champions[0].mana != 50 ||
            world.party.champions[0].max_mana != 50 ||
            world.party.champions[0].luck != 80 ||
            world.party.champions[0].strength != 50 ||
            world.party.champions[0].dexterity != 40 ||
            world.party.champions[0].wisdom != 40 ||
            world.party.champions[0].vitality != 45 ||
            world.party.champions[0].anti_magic != 40 ||
            world.party.champions[0].anti_fire != 45 ||
            memcmp(world.party.champions[0].skill_temporary_experience,
                   body.theron_skill_temporary_experience,
                   sizeof(body.theron_skill_temporary_experience)) != 0 ||
            memcmp(world.party.champions[0].skill_experience,
                   body.theron_skill_experience,
                   sizeof(body.theron_skill_experience)) != 0 ||
            memcmp(world.party.champions[0].inventory, before_inventory,
                   sizeof(before_inventory)) != 0 ||
            memcmp(world.party.champions[0].slots, before_slots,
                   sizeof(before_slots)) != 0 ||
            memcmp(&world.party.champions[1], &before_party.champions[1],
                   sizeof(world.party.champions) -
                       sizeof(world.party.champions[0])) != 0 ||
            memcmp(before_seeds, world.progression.dungeon_seeds,
                   sizeof(before_seeds)) != 0) {
            fputs("authentic Theron body was not transactionally restored\n",
                  stderr);
            return 1;
        }
        memcpy(world.party.champions[0].name, "NOT-THERON", 11u);
        before_party = world.party;
        if (theron_v1_startup_restore_pce_bram_theron_path(
                &world, argv[1], &receipt, &restored_body) ||
            memcmp(&world.party, &before_party, sizeof(before_party)) != 0) {
            fputs("non-Theron slot reached transactional body restore\n",
                  stderr);
            return 1;
        }
    }
    if (argc == 8) {
        puts("PASS: authentic empty JP Backup RAM is rejected as Continue");
    }
    puts("PASS: authentic three-slot Backup RAM record matches original read/write code and writer RAM");
    return 0;
}
