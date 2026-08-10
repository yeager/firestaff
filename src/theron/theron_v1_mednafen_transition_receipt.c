#include "theron_v1_mednafen_transition_receipt.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define THERON_US_TRACK02_MD5 "f23601102138f87c33025877767ebf76"
#define THERON_SYSTEM_CARD_MD5 "ff1a674273fe3540ccef576376407d1d"

static int read_line(FILE *file, char *line, size_t capacity) {
    size_t length;
    if (!fgets(line, capacity, file)) return 0;
    length = strlen(line);
    while (length && (line[length - 1u] == '\n' ||
                      line[length - 1u] == '\r')) {
        line[--length] = '\0';
    }
    return length > 0u;
}

static int parse_u64(const char *text, uint64_t *out) {
    char *end = NULL;
    unsigned long long value;
    if (!text || !out || !text[0]) return 0;
    errno = 0;
    value = strtoull(text, &end, 10);
    if (errno || end == text || *end != '\0') return 0;
    *out = (uint64_t)value;
    return 1;
}

static int set_count(const char *key, const char *value,
                     Theron_V1MednafenTransitionReceipt *receipt,
                     unsigned int *seen) {
    uint64_t parsed;
    uint64_t *slot = NULL;
    unsigned int bit = 0u;

#define COUNT_FIELD(name, member, flag) \
    if (strcmp(key, name) == 0) { slot = &receipt->member; bit = flag; }
    COUNT_FIELD("input_transactions", input_transactions, 1u << 0)
    else COUNT_FIELD("cd_irq_callbacks", cd_irq_callbacks, 1u << 1)
    else COUNT_FIELD("raw_sector_spans", raw_sector_spans, 1u << 2)
    else COUNT_FIELD("scsi_read_commands", scsi_read_commands, 1u << 3)
    else COUNT_FIELD("scsi_read_sector_bindings", scsi_read_sector_bindings, 1u << 4)
    else COUNT_FIELD("byte_exact_origin_ram_receipts", byte_exact_origin_ram_receipts, 1u << 5)
    else COUNT_FIELD("authenticated_cd_ram_receipts", authenticated_cd_ram_receipts, 1u << 6)
    else COUNT_FIELD("game_main_ram_e009_dispatches", game_main_ram_e009_dispatches, 1u << 7)
    else COUNT_FIELD("main_ram_consumer_reads", main_ram_consumer_reads, 1u << 8)
    else COUNT_FIELD("main_ram_target_reads", main_ram_target_reads, 1u << 9)
    else COUNT_FIELD("main_ram_target_writes", main_ram_target_writes, 1u << 10)
    else COUNT_FIELD("spawn_consumer_reads", spawn_consumer_reads, 1u << 11)
    else COUNT_FIELD("spawn_entry_b0e5_samples", spawn_entry_b0e5_samples, 1u << 12)
    else COUNT_FIELD("rng_consumer_samples", rng_consumer_samples, 1u << 13)
    else COUNT_FIELD("vdc_vram_snapshot_bytes", vdc_vram_snapshot_bytes, 1u << 14)
    else COUNT_FIELD("vce_palette_snapshot_bytes", vce_palette_snapshot_bytes, 1u << 15)
    else return 1; /* Unknown forward-compatible receipt field. */
#undef COUNT_FIELD

    if (!parse_u64(value, &parsed) || (*seen & bit)) return 0;
    *slot = parsed;
    *seen |= bit;
    return 1;
}

int theron_v1_mednafen_transition_receipt_parse_file(
    const char *path, Theron_V1MednafenTransitionReceipt *out) {
    Theron_V1MednafenTransitionReceipt receipt = {0};
    FILE *file;
    char line[1024];
    unsigned int seen_counts = 0u;
    unsigned int required_counts = (1u << 16) - 1u;

    if (!out) return 0;
    *out = receipt;
    if (!path || !path[0] ||
        !(file = fopen(path, "rb"))) {
        out->status = THERON_V1_MEDNAFEN_TRANSITION_UNAVAILABLE;
        return 1;
    }
    snprintf(receipt.source_trace_path, sizeof(receipt.source_trace_path),
             "%s", path);
    while (read_line(file, line, sizeof(line))) {
        char key[128];
        char value[768];
        int consumed = 0;
        if (sscanf(line, "%127[^=]=%767[^\n]%n", key, value, &consumed) != 2 ||
            line[consumed] != '\0') {
            fclose(file);
            receipt.status = THERON_V1_MEDNAFEN_TRANSITION_REJECTED;
            *out = receipt;
            return 0;
        }
        if (strcmp(key, "source") == 0) {
            if (receipt.source_header_verified ||
                strcmp(value, "authentic-mednafen-transition-receipt")) goto reject;
            receipt.source_header_verified = 1;
        } else if (strcmp(key, "mednafen_module") == 0) {
            if (receipt.pce_module_verified || strcmp(value, "pce")) goto reject;
            receipt.pce_module_verified = 1;
        } else if (strcmp(key, "track02_mode") == 0) {
            if (receipt.mode_verified || strcmp(value, "MODE1/2352")) goto reject;
            receipt.mode_verified = 1;
        } else if (strcmp(key, "track02_md5") == 0) {
            if (receipt.track02_md5_verified ||
                strcmp(value, THERON_US_TRACK02_MD5)) goto reject;
            snprintf(receipt.track02_md5, sizeof(receipt.track02_md5), "%s", value);
            receipt.track02_md5_verified = 1;
        } else if (strcmp(key, "system_card_md5") == 0) {
            if (receipt.system_card_md5_verified ||
                strcmp(value, THERON_SYSTEM_CARD_MD5)) goto reject;
            snprintf(receipt.system_card_md5, sizeof(receipt.system_card_md5), "%s", value);
            receipt.system_card_md5_verified = 1;
        } else if (strcmp(key, "transition") == 0) {
            if (receipt.transition_observed || strcmp(value, "observed")) goto reject;
            receipt.transition_observed = 1;
        } else if (!set_count(key, value, &receipt, &seen_counts)) {
            goto reject;
        }
    }
    fclose(file);
    if (!receipt.source_header_verified || !receipt.pce_module_verified ||
        !receipt.mode_verified || !receipt.track02_md5_verified ||
        !receipt.system_card_md5_verified || !receipt.transition_observed ||
        seen_counts != required_counts || receipt.input_transactions == 0u ||
        receipt.cd_irq_callbacks == 0u || receipt.raw_sector_spans == 0u ||
        receipt.scsi_read_commands == 0u ||
        receipt.scsi_read_sector_bindings == 0u ||
        receipt.byte_exact_origin_ram_receipts == 0u ||
        receipt.authenticated_cd_ram_receipts == 0u ||
        receipt.game_main_ram_e009_dispatches == 0u ||
        receipt.main_ram_consumer_reads == 0u ||
        receipt.vdc_vram_snapshot_bytes != 65536u ||
        receipt.vce_palette_snapshot_bytes != 1024u ||
        receipt.main_ram_target_reads != 0u ||
        receipt.main_ram_target_writes != 0u ||
        receipt.spawn_consumer_reads != 0u ||
        receipt.spawn_entry_b0e5_samples != 0u ||
        receipt.rng_consumer_samples != 0u) goto reject_after_close;
    receipt.status = THERON_V1_MEDNAFEN_TRANSITION_READY;
    receipt.transport_verified = 1;
    receipt.semantic_publication_allowed = 0;
    *out = receipt;
    return 1;

reject:
    fclose(file);
reject_after_close:
    receipt.status = THERON_V1_MEDNAFEN_TRANSITION_REJECTED;
    receipt.transport_verified = 0;
    receipt.semantic_publication_allowed = 0;
    *out = receipt;
    return 0;
}
