#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "memory_dungeon_dat_pc34_compat.h"

/*
 * M10 Phase 7 probe: decode text strings from DUNGEON.DAT.
 * Verifies DM 5-bit packed text encoding (3 codes per word).
 * Usage: probe <DUNGEON.DAT path> <output_dir>
 */

int main(int argc, char* argv[]) {
        struct DungeonDatState_Compat state;
        struct DungeonThings_Compat things;
        struct DungeonTextTable_Compat textTable;
        FILE* report;
        FILE* invariants;
        char path_buf[512];
        int i;
        int fail_count = 0;

        if (argc < 3) {
                fprintf(stderr, "Usage: %s <DUNGEON.DAT> <output_dir>\n", argv[0]);
                return 1;
        }

        if (!F0500_DUNGEON_LoadDatHeader_Compat(argv[1], &state)) {
                fprintf(stderr, "FAIL: could not load header\n");
                return 1;
        }
        if (!F0504_DUNGEON_LoadThingData_Compat(argv[1], &state, &things)) {
                fprintf(stderr, "FAIL: could not load thing data\n");
                F0500_DUNGEON_FreeDatHeader_Compat(&state);
                return 1;
        }

        /* Decode text table */
        int textOk = F0506_DUNGEON_DecodeTextTable_Compat(
                things.textData, things.textDataWordCount, &textTable);

        /* ---- Report ---- */
        snprintf(path_buf, sizeof(path_buf), "%s/dungeon_text_probe.md", argv[2]);
        report = fopen(path_buf, "w");
        if (!report) { fprintf(stderr, "FAIL: cannot write report\n"); goto cleanup; }

        fprintf(report, "# DUNGEON.DAT Text Strings Probe\n\n");
        fprintf(report, "## Layout\n\n");
        fprintf(report, "- File size: %ld bytes\n", state.fileSize);
        fprintf(report, "- TextDataWordCount: %d (%d bytes)\n",
                things.textDataWordCount, things.textDataWordCount * 2);
        fprintf(report, "- TextString thing count: %d\n", things.textStringCount);
        fprintf(report, "- Text table decoded strings: %d\n\n", textTable.count);

        /* Show all decoded strings */
        fprintf(report, "## Decoded Text Strings\n\n");
        fprintf(report, "| Index | Length | Text (first 60 chars) |\n");
        fprintf(report, "|-------|--------|----------------------|\n");
        for (i = 0; i < textTable.count && i < 200; i++) {
                const char* s = textTable.strings[i] ? textTable.strings[i] : "";
                int len = (int)strlen(s);
                char preview[64];
                int j, k = 0;
                for (j = 0; j < len && k < 59; j++) {
                        if (s[j] == '\n') { preview[k++] = '|'; }
                        else { preview[k++] = s[j]; }
                }
                preview[k] = '\0';
                fprintf(report, "| %d | %d | %s |\n", i, len, preview);
        }

        /* Show TextString thing → text mapping */
        fprintf(report, "\n## TextString Things → Text\n\n");
        for (i = 0; i < things.textStringCount && i < 50; i++) {
                struct DungeonTextString_Compat* ts = &things.textStrings[i];
                char buf[DUNGEON_TEXT_MAX_STRING_LEN];
                int decLen = F0507_DUNGEON_DecodeTextAtOffset_Compat(
                        things.textData, things.textDataWordCount,
                        ts->textDataWordOffset, buf, sizeof(buf));
                /* Replace newlines for display */
                int j;
                for (j = 0; j < decLen; j++) { if (buf[j] == '\n') buf[j] = '|'; }
                fprintf(report, "- TS[%d] vis=%d woff=%d: \"%s\"\n",
                        i, ts->visible, ts->textDataWordOffset, buf);
        }

        fclose(report);

        /* ---- Invariant checks ---- */
        snprintf(path_buf, sizeof(path_buf), "%s/dungeon_text_invariants.md", argv[2]);
        invariants = fopen(path_buf, "w");
        if (!invariants) { fprintf(stderr, "FAIL: cannot write invariants\n"); goto cleanup; }

        fprintf(invariants, "# Dungeon Text Invariants\n\n");

#define CHECK(cond, msg) do { \
        if (cond) { fprintf(invariants, "- PASS: %s\n", msg); } \
        else { fprintf(invariants, "- FAIL: %s\n", msg); fail_count++; } \
} while(0)

        /* 1. Text table decoded successfully */
        CHECK(textOk && textTable.count > 0, "Text table decoded with > 0 strings");

        /* 2. All decoded strings contain only printable ASCII */
        {
                int allPrintable = 1;
                for (i = 0; i < textTable.count; i++) {
                        const char* s = textTable.strings[i];
                        if (!s) continue;
                        int j;
                        for (j = 0; s[j]; j++) {
                                unsigned char c = (unsigned char)s[j];
                                if (c != '\n' && (c < 0x20 || c > 0x7E)) {
                                        allPrintable = 0;
                                        break;
                                }
                        }
                        if (!allPrintable) break;
                }
                CHECK(allPrintable, "All decoded strings contain printable ASCII + newline");
        }

        /* 3. Known inscriptions found */
        {
                int foundHall = 0, foundTest = 0, foundNone = 0;
                for (i = 0; i < textTable.count; i++) {
                        const char* s = textTable.strings[i];
                        if (!s) continue;
                        if (strstr(s, "HALL OF")) foundHall = 1;
                        if (strstr(s, "CHAMPIONS")) foundHall = 1;
                        if (strstr(s, "TEST YOUR")) foundTest = 1;
                        if (strstr(s, "STRENGTH")) foundTest = 1;
                        if (strstr(s, "NONE SHALL")) foundNone = 1;
                        if (strstr(s, "PASS")) foundNone = 1;
                }
                char msg[128];
                snprintf(msg, sizeof(msg),
                        "Known DM inscriptions: HALL=%d TEST=%d NONE=%d (need all 3)",
                        foundHall, foundTest, foundNone);
                CHECK(foundHall && foundTest && foundNone, msg);
        }

        /* 4. String count is reasonable (DM has ~120+ text entries) */
        {
                char msg[128];
                snprintf(msg, sizeof(msg),
                        "String count %d in reasonable range (50-500)", textTable.count);
                CHECK(textTable.count >= 50 && textTable.count <= 500, msg);
        }

        /* 5. All TextString thing offsets < textDataWordCount */
        {
                int allValid = 1;
                for (i = 0; i < things.textStringCount; i++) {
                        if (things.textStrings[i].textDataWordOffset >= (unsigned short)things.textDataWordCount) {
                                allValid = 0;
                                break;
                        }
                }
                CHECK(allValid, "All TextString word offsets < textDataWordCount");
        }

        /* 6. Average string length is reasonable (5-60 chars) */
        {
                int totalLen = 0;
                int nonEmpty = 0;
                for (i = 0; i < textTable.count; i++) {
                        if (textTable.strings[i] && textTable.strings[i][0]) {
                                totalLen += (int)strlen(textTable.strings[i]);
                                nonEmpty++;
                        }
                }
                double avgLen = nonEmpty > 0 ? (double)totalLen / nonEmpty : 0;
                char msg[128];
                snprintf(msg, sizeof(msg),
                        "Average non-empty string length %.1f in range 5-60", avgLen);
                CHECK(avgLen >= 5.0 && avgLen <= 60.0, msg);
        }

        /* 7. File checksum: last 2 bytes are at expected position */
        {
                int totalColumns = 0;
                for (i = 0; i < (int)state.header.mapCount; i++)
                        totalColumns += state.maps[i].width;
                long thingBytes = 0;
                for (i = 0; i < 16; i++)
                        thingBytes += (long)things.thingCounts[i] * (long)s_thingDataByteCount[i];
                long expectedNoChecksum = DUNGEON_HEADER_SIZE +
                        (long)state.header.mapCount * DUNGEON_MAP_DESC_SIZE +
                        (long)totalColumns * 2 +
                        (long)state.header.squareFirstThingCount * 2 +
                        (long)things.textDataWordCount * 2 +
                        thingBytes +
                        (long)state.header.rawMapDataByteCount;
                long checksumSize = state.fileSize - expectedNoChecksum;
                char msg[128];
                snprintf(msg, sizeof(msg),
                        "Checksum at end: expected=%ld, filesize=%ld, checksum_bytes=%ld",
                        expectedNoChecksum, state.fileSize, checksumSize);
                CHECK(checksumSize == 2, msg);
        }

        fprintf(invariants, "\nStatus: %s\n", fail_count == 0 ? "PASS" : "FAIL");
        fclose(invariants);

        printf("Dungeon text probe: %d strings decoded, %s\n",
               textTable.count, fail_count == 0 ? "PASS" : "FAIL");

cleanup:
        F0506_DUNGEON_FreeTextTable_Compat(&textTable);
        F0504_DUNGEON_FreeThingData_Compat(&things);
        F0500_DUNGEON_FreeDatHeader_Compat(&state);
        return fail_count == 0 ? 0 : 1;
}
