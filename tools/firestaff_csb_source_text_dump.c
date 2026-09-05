/* Dump localization candidates from source-owned CSB runtime tables/dungeon.
 *
 * This tool scans the same original archive/disk/CD inputs as Firestaff and
 * reads C699/M564 plus every decoded DB2 record through csb_v1_boot. DB2 also
 * contains structured champion records, so its output is an audited candidate
 * corpus: only fields proven player-visible by a presentation consumer belong
 * in the canonical POT. It never extracts members to disk or substitutes data.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "firestaff_cp932.h"

#include "csb_v1_boot.h"
#include "asset_find_by_hash.h"
#include "csb_v1_graphics_atari_st_loader_pc34_compat.h"
#include "firestaff_pak_decode.h"
#include "memory_dungeon_dat_pc34_compat.h"

enum {
    CSB_FMTOWNS_M564_OBJECT_NAMES = 694,
    CSB_FMTOWNS_C699_ACTION_NAMES = 699,
    CSB_FMTOWNS_SOURCE_RECORD_MAX = 65535
};

static int atari_sibling_path(const char *source, const char *member,
                              char *out, size_t out_size) {
    const char *separator = NULL;
    const char *cursor;
    size_t prefix;
    for (cursor = source ? strstr(source, "::") : NULL; cursor;
         cursor = strstr(cursor + 2, "::")) separator = cursor;
    if (!separator || !member || !out) return 0;
    prefix = (size_t)(separator + 2 - source);
    if (prefix + strlen(member) >= out_size) return 0;
    memcpy(out, source, prefix);
    strcpy(out + prefix, member);
    return 1;
}

static uint32_t source_fnv1a(const uint8_t *bytes, size_t byte_count) {
    uint32_t hash = 2166136261u;
    size_t index;
    for (index = 0u; bytes && index < byte_count; ++index) {
        hash ^= bytes[index];
        hash *= 16777619u;
    }
    return hash;
}

static int load_atari_source_tables(CSB_V1_BootProfile *profile,
                                    const char *source_root,
                                    uint32_t *out_executable_fnv1a,
                                    size_t *out_executable_size) {
    uint8_t decoded[CSB_FMTOWNS_SOURCE_RECORD_MAX];
    CSB_AtariStLoader graphics;
    size_t decoded_size = 0u;
    uint8_t *executable = NULL;
    size_t executable_size = 0u;
    char executable_path[ASSET_PATH_MAX];
    size_t start;
    size_t match_start = 0u;
    size_t match_size = 0u;
    unsigned int matches = 0u;
    const uint8_t *action_source = NULL;
    size_t action_source_size = 0u;
    int ok = 0;

    csb_atari_st_graphics_loader_init(&graphics);

    if (!profile || profile->variant_id != CSB_V1_VARIANT_ST21_EN ||
        (strcmp(profile->graphics_md5, "ebf6a57af3f27782e358c0490bfd2f2e") != 0 &&
         strcmp(profile->graphics_md5, "e0ce7ac5160ca5540e90cf09ab9fad49") != 0)) {
        return 0;
    }
    if (!csb_atari_st_graphics_loader_open(&graphics,
                                           profile->graphics_path) ||
        graphics.item_count != 563u ||
        (int)(decoded_size = graphics.items[556u].decompressed_size) <= 0 ||
        csb_atari_st_graphics_loader_read_item(
            &graphics, 556u, decoded, sizeof(decoded)) != (int)decoded_size ||
        !csb_v1_runtime_load_object_names_m564(&profile->runtime, decoded,
                                                decoded_size)) {
        goto cleanup;
    }
    fprintf(stderr, "Atari ST M564 GRAPHICS.DAT item 556 bytes=%zu\n",
            decoded_size);

    /* S20E/S21E STARTUP2.C F0750 expands C560_GRAPHIC_GLOBAL_VARIABLES
     * over the contiguous G0485..G0505 block. Unlike A20/F31, these Atari
     * builds declare G0490 uninitialised in the executable: its authentic
     * bytes therefore live in GRAPHICS.DAT item 560, not START.PAK DATA.
     * Keep the decoded item in this stack buffer and locate the bounded
     * 44-row subtable below. */
    decoded_size = graphics.items[560u].decompressed_size;
    if (decoded_size == 0u || decoded_size > sizeof(decoded) ||
        csb_atari_st_graphics_loader_read_item(
            &graphics, 560u, decoded, sizeof(decoded)) != (int)decoded_size) {
        fprintf(stderr, "Atari ST C560 source table decode failed\n");
        goto cleanup;
    }
    action_source = decoded;
    action_source_size = decoded_size;
    fprintf(stderr, "Atari ST C560 GRAPHICS.DAT item 560 bytes=%zu\n",
            decoded_size);

    /* S20E/S21E MENU.C owns G0490 as a compiled 300-byte array, not as
     * GRAPHICS.DAT C699 (that loader exists only for later 3.x media). Read
     * the selected disk's FTLCODE sibling and admit the sole 44-row table
     * having the source sentinel shape N, BLOCK, CHOP, X ... FUSE. */
    executable_path[0] = '\0';
    if (!asset_find_by_md5(source_root,
                           "18abdf771f37e8953bf95ba2f462469d",
                           executable_path, sizeof(executable_path), 8)) {
        static const char *const executable_members[] = {
            "FTLCODE", "CHAOS.FTL", "START.PAK", NULL
        };
        unsigned int candidate;
        for (candidate = 0u; executable_members[candidate]; ++candidate) {
            if (atari_sibling_path(profile->graphics_path,
                                   executable_members[candidate],
                                   executable_path,
                                   sizeof(executable_path)) &&
                asset_read_virtual_path_alloc(executable_path, &executable,
                                               &executable_size)) break;
        }
    } else if (!(strstr(executable_path, "::")
                     ? asset_read_virtual_path_alloc(executable_path,
                                                      &executable,
                                                      &executable_size)
                     : asset_read_path_alloc(executable_path, &executable,
                                              &executable_size))) {
        executable = NULL;
    }
    if (!executable) {
        fprintf(stderr, "Atari ST hash-authenticated FTLCODE is unavailable\n");
        goto cleanup;
    }
    fprintf(stderr, "Atari ST executable candidate=%s bytes=%zu\n",
            executable_path, executable_size);
    if (strstr(executable_path, "START.PAK")) {
        FirestaffPakDecoded pak = {0};
        FirestaffPakHeader pak_header;
        memset(&pak_header, 0, sizeof(pak_header));
        if (FirestaffPak_ReadHeader(executable, executable_size,
                                    &pak_header) != 0) goto cleanup;
        fprintf(stderr, "Atari ST START.PAK header words=%u text=%u data=%u "
                        "symbols=%u\n", (unsigned)pak_header.file_size_words,
                (unsigned)pak_header.text_size, (unsigned)pak_header.data_size,
                (unsigned)pak_header.symbol_table_size);
        /* FirestaffPak_Decode follows STARTGD.C and returns the complete
         * F0913 word-count image. G0490 is initialized DATA, so rewriting
         * the outer text-size field is both unnecessary and incorrect. */
        if (FirestaffPak_Decode(executable, executable_size, &pak) != 0 ||
            !pak.text || pak.text_size == 0u) {
            FirestaffPak_Free(&pak);
            fprintf(stderr, "Atari ST START.PAK F0913 decode failed\n");
            goto cleanup;
        }
        free(executable);
        executable = pak.text;
        executable_size = pak.text_size;
        fprintf(stderr, "Atari ST START.PAK decoded bytes=%zu\n",
                executable_size);
    }
    for (start = 0u; start + 2u < action_source_size; ++start) {
        size_t row_start[44];
        size_t pos = start;
        unsigned int row;
        if (action_source[start] != 'N' || action_source[start + 1u] != 0u)
            continue;
        for (row = 0u; row < 44u; ++row) {
            row_start[row] = pos;
            while (pos < action_source_size && action_source[pos] != 0u &&
                   pos - start < 300u) ++pos;
            if (pos >= action_source_size || pos - start >= 300u) break;
            ++pos;
        }
        if (row != 44u || strcmp((const char *)action_source + row_start[1],
                                 "BLOCK") != 0 ||
            strcmp((const char *)action_source + row_start[2], "CHOP") != 0 ||
            strcmp((const char *)action_source + row_start[3], "X") != 0 ||
            strcmp((const char *)action_source + row_start[43], "FUSE") != 0)
            continue;
        ++matches;
        match_start = start;
        match_size = pos - start;
    }
    if (matches != 1u ||
        !csb_v1_runtime_load_action_names_c699(
            &profile->runtime, action_source + match_start, match_size)) {
        fprintf(stderr, "Atari ST C560 action table matches=%u\n", matches);
        goto cleanup;
    }
    fprintf(stderr, "Atari ST G0490 C560 offset=0x%zx bytes=%zu\n",
            match_start, match_size);
    if (out_executable_fnv1a)
        *out_executable_fnv1a = source_fnv1a(executable, executable_size);
    if (out_executable_size) *out_executable_size = executable_size;
    ok = 1;
cleanup:
    csb_atari_st_graphics_loader_close(&graphics);
    free(executable);
    return ok;
}

static int load_fmtowns_source_tables(CSB_V1_BootProfile *profile) {
    uint8_t decoded[CSB_FMTOWNS_SOURCE_RECORD_MAX];
    size_t decoded_size = 0u;
    if (!profile || !profile->fmtowns_graphics_bytes ||
        profile->fmtowns_graphics_size == 0u) return 0;
    if (csb_v1_graphics_decode_raw_entry_pc34(
            profile->fmtowns_graphics_bytes, profile->fmtowns_graphics_size,
            CSB_FMTOWNS_M564_OBJECT_NAMES, decoded, sizeof(decoded),
            &decoded_size) != 0 ||
        !(profile->variant_id == CSB_V1_VARIANT_FMTOWNS_JA
          ? csb_v1_runtime_load_object_names_m564_f31j(
                &profile->runtime, decoded, decoded_size)
          : csb_v1_runtime_load_object_names_m564(
                &profile->runtime, decoded, decoded_size))) {
        fprintf(stderr, "FM Towns M564 decode failed (decoded=%zu)\n",
                decoded_size);
        return 0;
    }
    fprintf(stderr, "FM Towns M564 GRAPHICS.DAT item 694 bytes=%zu\n",
            decoded_size);
    /* F31 does not compile MENU.C's F0620 GRAPHICS.DAT loader. FMTOWNS.H
     * aliases G0490 to the executable-owned DYNA_BUTTONS pool instead. Find
     * that pool in the already hash-admitted CHTWE/CHTWJ member by its source
     * structure: 44 NUL rows, N at index 0 and X controls at 3 and 26. */
    {
        size_t start;
        int found = 0;
        for (start = 0u; start + 2u < profile->fmtowns_executable_size; ++start) {
            size_t cursor = start;
            size_t row_start[44];
            unsigned int row;
            if (profile->fmtowns_executable_bytes[start] != 'N' ||
                profile->fmtowns_executable_bytes[start + 1u] != 0u) continue;
            for (row = 0u; row < 44u; ++row) {
                row_start[row] = cursor;
                while (cursor < profile->fmtowns_executable_size &&
                       profile->fmtowns_executable_bytes[cursor] != 0u &&
                       cursor - start < 360u) ++cursor;
                if (cursor >= profile->fmtowns_executable_size ||
                    cursor - start >= 360u) break;
                ++cursor;
            }
            if (row != 44u || cursor - start > 360u ||
                strcmp((const char *)profile->fmtowns_executable_bytes +
                           row_start[3], "X") != 0 ||
                strcmp((const char *)profile->fmtowns_executable_bytes +
                           row_start[26], "X") != 0) continue;
            decoded_size = cursor - start;
            if (!csb_v1_runtime_load_action_names_c699(
                    &profile->runtime,
                    profile->fmtowns_executable_bytes + start, decoded_size)) {
                continue;
            }
            if ((profile->variant_id == CSB_V1_VARIANT_FMTOWNS_EN &&
                 start != 0x29f50u) ||
                (profile->variant_id == CSB_V1_VARIANT_FMTOWNS_JA &&
                 start != 0x2a0ecu)) {
                memset(profile->runtime.action_names, 0,
                       sizeof(profile->runtime.action_names));
                profile->runtime.action_name_table_valid = 0;
                continue;
            }
            fprintf(stderr, "FM Towns DYNA_BUTTONS source offset=0x%zx "
                            "bytes=%zu\n", start, decoded_size);
            found = 1;
            break;
        }
        if (!found) {
            fprintf(stderr, "FM Towns executable lacks authenticated "
                            "DYNA_BUTTONS rows\n");
            return 0;
        }
    }
    return 1;
}

static void print_po_string(const char* text) {
    const unsigned char* cursor = (const unsigned char*)text;
    putchar('"');
    while (cursor && *cursor) {
        if (*cursor == '\\' || *cursor == '"') putchar('\\');
        if (*cursor == '\n') {
            fputs("\\n", stdout);
        } else {
            putchar((int)*cursor);
        }
        ++cursor;
    }
    putchar('"');
}

static int g_source_is_shift_jis;

static int print_entry(const char* source, const char* owner) {
    /* A source byte can expand to three UTF-8 bytes (half-width katakana).
     * Use the admitted runtime text bound, not an object-name-sized buffer. */
    char utf8[3u * CSB_V1_RUNTIME_TEXT_MESSAGE_MAX_CHARS];
    const char *presented = source;
    if (!source || source[0] == '\0') return 0;
    if (g_source_is_shift_jis) {
        if (firestaff_cp932_to_utf8(source, strlen(source), utf8,
                                   sizeof(utf8)) < 0) {
            fprintf(stderr, "Cannot convert authenticated %s from CP932 to UTF-8\n",
                    owner);
            return -1;
        }
        presented = utf8;
    }
    printf("#. Source-owned %s\nmsgid ", owner);
    print_po_string(presented);
    fputs("\nmsgstr \"\"\n\n", stdout);
    return 1;
}

enum {
    CSB_SOURCE_TEXT_MAX_UNIQUE = 1024,
    CSB_SOURCE_TEXT_MAX_BYTES = CSB_V1_RUNTIME_TEXT_MESSAGE_MAX_CHARS
};

static int emit_unique(const char* source, const char* owner,
                       char seen[CSB_SOURCE_TEXT_MAX_UNIQUE]
                                [CSB_SOURCE_TEXT_MAX_BYTES],
                       unsigned int* seenCount) {
    unsigned int index;
    if (!source || source[0] == '\0' || !seenCount) return 0;
    for (index = 0; index < *seenCount; ++index) {
        if (strcmp(seen[index], source) == 0) return 0;
    }
    if (*seenCount >= CSB_SOURCE_TEXT_MAX_UNIQUE ||
        strlen(source) >= CSB_SOURCE_TEXT_MAX_BYTES) {
        return -1;
    }
    if (print_entry(source, owner) < 0) return -1;
    snprintf(seen[*seenCount], sizeof(seen[*seenCount]), "%s", source);
    ++*seenCount;
    return 1;
}

static int emit_dungeon_text_bank(const CSB_V1_BootProfile *profile,
                                  char seen[CSB_SOURCE_TEXT_MAX_UNIQUE]
                                           [CSB_SOURCE_TEXT_MAX_BYTES],
                                  unsigned int *seen_count)
{
    const CSB_V1_DungeonData *dungeon;
    unsigned int index;

    if (!profile || !seen_count ||
        !(dungeon = profile->runtime.dungeon_handle) || !dungeon->raw_data ||
        dungeon->text_data_base < 0 || dungeon->text_word_count <= 0)
        return -1;
    for (index = 0u; index < (unsigned int)dungeon->thing_type_counts[2]; ++index) {
        const uint8_t *record;
        int type;
        int size;
        unsigned int text_word;
        int offset;
        char decoded[CSB_SOURCE_TEXT_MAX_BYTES];

        record = csb_v1_dungeon_get_thing_record(
            dungeon, (uint16_t)((2u << 10) | index), &type, NULL, &size);
        if (!record || type != 2 || size < 4) return -1;
        text_word = (unsigned int)record[2] | ((unsigned int)record[3] << 8);
        offset = (int)((text_word >> 3) & 0x1fffu);
        if (offset < 0 || offset >= dungeon->text_word_count) return -1;
        if (F0507_DUNGEON_DecodeTextAtOffset_Compat(
                (const unsigned short *)(const void *)(
                    dungeon->raw_data + dungeon->text_data_base),
                dungeon->text_word_count, offset, decoded,
                (int)sizeof(decoded)) < 0)
            return -1;
        /* CEDT champion records share DB2 storage but append encoded sex,
         * attributes, skills and statistics after their player-facing name.
         * They are consumed structurally by CHAMPION.C, not printed as one
         * string.  This slice admits the independently consumed Utility Disk
         * instruction only; scroll/wall consumers need their own relation
         * walk before they can join the catalog. */
        if (strcmp(decoded,
                   "THERE IS ONLY ONE\nLEVEL HERE.      \n                 \n"
                   "FIRST CHOOSE YOUR\nCHAMPIONS...THEN\nSAVE THE GAME...\n"
                   "THEN RESTART WITH\nTHE UTILITY DISK.") == 0 &&
            emit_unique(decoded, "DUNGEON.DAT Utility instruction DB2",
                        seen, seen_count) < 0)
            return -1;
    }
    return 0;
}

int main(int argc, char** argv) {
    CSB_V1_BootProfile profile;
    CSB_V1_BootStartupLaunch_PC34 launch;
    CSB_V1_BootProfile *selected = &profile;
    int fmtowns_variant = CSB_V1_VARIANT_UNKNOWN;
    int launch_owned = 0;
    uint32_t atari_executable_fnv1a = 0u;
    size_t atari_executable_size = 0u;
    char seen[CSB_SOURCE_TEXT_MAX_UNIQUE][CSB_SOURCE_TEXT_MAX_BYTES] = {{0}};
    unsigned int seenCount = 0u;
    unsigned int index;
    if (argc != 2 && argc != 3) {
        fprintf(stderr, "usage: %s <original-csb-data-root-or-archive> "
                        "[fm-towns-en|fm-towns-ja]\n",
                argc > 0 ? argv[0] : "firestaff_csb_source_text_dump");
        return 2;
    }
    if (argc == 3) {
        if (strcmp(argv[2], "fm-towns-en") == 0) {
            fmtowns_variant = CSB_V1_VARIANT_FMTOWNS_EN;
        } else if (strcmp(argv[2], "fm-towns-ja") == 0) {
            fmtowns_variant = CSB_V1_VARIANT_FMTOWNS_JA;
        } else {
            fprintf(stderr, "unknown source-media variant: %s\n", argv[2]);
            return 2;
        }
    }
    csb_v1_boot_profile_init(&profile);
    memset(&launch, 0, sizeof(launch));
    if (fmtowns_variant != CSB_V1_VARIANT_UNKNOWN) {
        if (!csb_v1_boot_startup_launch_alloc_with_variant_pc34(
                argv[1], NULL, NULL, NULL, NULL, fmtowns_variant, &launch)) {
            fprintf(stderr, "CSB FM Towns source media did not produce the "
                            "selected native runtime\n");
            return 1;
        }
        selected = launch.profile;
        launch_owned = 1;
    } else if (csb_v1_boot_scan_assets(&profile, argv[1]) != 0) {
        fprintf(stderr, "CSB source media did not produce a native runtime\n");
        csb_v1_boot_cleanup(&profile);
        return 1;
    }
    if (selected->variant_id == CSB_V1_VARIANT_FMTOWNS_EN ||
        selected->variant_id == CSB_V1_VARIANT_FMTOWNS_JA) {
        if (!load_fmtowns_source_tables(selected)) {
            if (launch_owned) csb_v1_boot_startup_launch_cleanup_pc34(&launch);
            else csb_v1_boot_cleanup(&profile);
            return 1;
        }
    } else if (csb_v1_boot_enter_game(selected) != 0) {
        fprintf(stderr, "CSB source media did not produce a native runtime\n");
        if (launch_owned) csb_v1_boot_startup_launch_cleanup_pc34(&launch);
        else csb_v1_boot_cleanup(&profile);
        return 1;
    }
    if (selected->variant_id == CSB_V1_VARIANT_ST21_EN &&
        !load_atari_source_tables(selected, argv[1], &atari_executable_fnv1a,
                                  &atari_executable_size)) {
        fprintf(stderr, "CSB Atari ST source tables did not authenticate\n");
        csb_v1_boot_cleanup(&profile);
        return 1;
    }
    if (!selected->runtime.action_name_table_valid ||
        !selected->runtime.object_name_table_valid) {
        fprintf(stderr, "CSB media lacks authenticated C699/M564 tables\n");
        if (launch_owned) csb_v1_boot_startup_launch_cleanup_pc34(&launch);
        else csb_v1_boot_cleanup(&profile);
        return 1;
    }
    g_source_is_shift_jis =
        selected->variant_id == CSB_V1_VARIANT_FMTOWNS_JA;
    printf("# Generated only from the CSB media selected by csb_v1_boot.\n"
           "# graphics-md5: %s\n# dungeon-md5: %s\n"
           "msgid \"\"\nmsgstr \"\"\n"
           "\"Project-Id-Version: firestaff-csb\\n\"\n"
           "\"Report-Msgid-Bugs-To: daniel@danielnylander.se\\n\"\n"
           "\"POT-Creation-Date: 1970-01-01 00:00+0000\\n\"\n"
           "\"PO-Revision-Date: 1970-01-01 00:00+0000\\n\"\n"
           "\"Last-Translator: Firestaff Localization Team <daniel@danielnylander.se>\\n\"\n"
           "\"Language-Team: Firestaff Localization Team <daniel@danielnylander.se>\\n\"\n"
           "\"Language: en\\n\"\n"
           "\"MIME-Version: 1.0\\n\"\n"
           "\"Content-Type: text/plain; charset=UTF-8\\n\"\n"
           "\"Content-Transfer-Encoding: 8bit\\n\"\n\n",
           selected->graphics_md5, selected->dungeon_md5);
    if (atari_executable_fnv1a)
        printf("#. Atari ST FTLCODE bytes=%zu fnv1a=%08x\n",
               atari_executable_size, (unsigned)atari_executable_fnv1a);
    for (index = 0; index < 44u; ++index) {
        if (emit_unique(selected->runtime.action_names[index],
                        "C699 action name", seen, &seenCount) < 0) {
            if (launch_owned) csb_v1_boot_startup_launch_cleanup_pc34(&launch);
            else csb_v1_boot_cleanup(&profile);
            return 1;
        }
    }
    for (index = 0; index < CSB_V1_OBJECT_NAME_COUNT; ++index) {
        if (emit_unique(selected->runtime.object_names[index],
                        "M564 object name", seen, &seenCount) < 0) {
            if (launch_owned) csb_v1_boot_startup_launch_cleanup_pc34(&launch);
            else csb_v1_boot_cleanup(&profile);
            return 1;
        }
    }
    if (emit_dungeon_text_bank(selected, seen, &seenCount) != 0) {
        fprintf(stderr, "CSB media lacks a complete source-owned DB2 text bank\n");
        if (launch_owned) csb_v1_boot_startup_launch_cleanup_pc34(&launch);
        else csb_v1_boot_cleanup(&profile);
        return 1;
    }
    if (launch_owned) csb_v1_boot_startup_launch_cleanup_pc34(&launch);
    else csb_v1_boot_cleanup(&profile);
    return 0;
}
