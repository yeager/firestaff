#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dm1_v1_text_message_pc34_compat.h"
#include "firestaff_po_loader.h"

static int fail(const char* message) {
    fprintf(stderr, "FAIL: %s\n", message);
    return 1;
}

int main(void) {
    char catalogPath[1024];
    char expanded[DM1_V1_STRING_BUILD_BUFFER_SIZE];
    char formatted[DM1_V1_STRING_BUILD_BUFFER_SIZE];
    char sourcePath[1024];
    char* source;
    FILE* stream;
    long sourceSizeLong;
    size_t sourceSize;
    const char* translated;
    const char* translatedSkill;

    snprintf(catalogPath, sizeof(catalogPath),
             FIRESTAFF_SOURCE_DIR "/po/dm1.sv.po");
    if (fs_po_load(catalogPath) <= 0) return fail("load DM1 Swedish catalog");

    translated = fs_po_gettext_in_domain(
        "dm1", "{champion} NEEDS MORE PRACTICE WITH THIS {skill} SPELL.");
    if (!translated || strcmp(
            translated,
            "{champion} NEEDS MORE PRACTICE WITH THIS {skill} SPELL.") == 0) {
        return fail("resolve translated F0410 template in DM1 domain");
    }
    if (!dm1_v1_text_expand_l10n_template(
            translated, "HALK",
            (translatedSkill = fs_po_gettext_in_domain("dm1", "WIZARD")),
            expanded, sizeof(expanded))) {
        return fail("expand translated F0410 named fields");
    }
    if (!translatedSkill || strcmp(translatedSkill, "WIZARD") == 0) {
        return fail("resolve translated F0410 skill field in DM1 domain");
    }
    if (strstr(expanded, "HALK") == NULL ||
        strstr(expanded, translatedSkill) == NULL ||
        strstr(expanded, "WIZARD") != NULL) {
        return fail("localize the F0410 skill field while preserving champion name");
    }
    if (strcmp(fs_po_gettext_in_domain("dm1", "UNLISTED RETAIL TEXT"),
               "UNLISTED RETAIL TEXT") != 0) {
        return fail("preserve exact retail fallback for unknown text");
    }

    /* MENU.C F0407 has exactly two PC34 F0381 producers.  Resolve both in
     * the DM1 domain and drive the same C015 publisher used by Original and
     * Modern, retaining F0381's cyan/+70 contract after translation. */
    {
        static const char* const flipMsgids[] = {
            "IT COMES UP HEADS.", "IT COMES UP TAILS."
        };
        static const char* const flipSwedish[] = {
            "DET BLEV KRONA.", "DET BLEV KLAVE."
        };
        size_t flipIndex;
        for (flipIndex = 0u; flipIndex < 2u; ++flipIndex) {
            DM1_V1_TextMessageState messageState;
            const DM1_V1_MessageRow* row;
            translated = fs_po_gettext_in_domain("dm1", flipMsgids[flipIndex]);
            if (!translated || strcmp(translated, flipSwedish[flipIndex]) != 0) {
                return fail("resolve exact Swedish F0381 coin message");
            }
            dm1_v1_text_init(&messageState);
            if (!dm1_v1_text_print_message_after_replacements_f0381(
                    &messageState, 19, translated, "HALK")) {
                return fail("publish translated F0381 coin message");
            }
            row = dm1_v1_text_get_row(
                &messageState, DM1_V1_MESSAGE_AREA_ROW_COUNT - 1);
            if (!row || strcmp(row->text, flipSwedish[flipIndex]) != 0 ||
                row->color != DM1_V1_COLOR_CYAN || row->expirationTime != 89) {
                return fail("retain translated F0381 C015 presentation contract");
            }
        }
    }

    translated = fs_po_gettext_in_domain("dm1", "%s -> MOUSE HAND");
    snprintf(formatted, sizeof(formatted), translated, "FACKLA");
    if (strcmp(formatted, "FACKLA -> MUSHAND") != 0) {
        return fail("translate pickup format before interpolating item name");
    }
    translated = fs_po_gettext_in_domain("dm1", "%s — %d OF 4 SYMBOLS");
    snprintf(formatted, sizeof(formatted), translated, "FUL", 2);
    if (strcmp(formatted, "FUL — 2 AV 4 SYMBOLER") != 0) {
        return fail("translate rune format before interpolating source fields");
    }
    if (strcmp(fs_po_gettext_in_domain("dm1", "LEADER HAND FULL"),
               "LEDARENS HAND ÄR FULL") != 0 ||
        strcmp(fs_po_gettext_in_domain("dm1", "SPELL CAST"),
               "BESVÄRJELSE KASTAD") != 0) {
        return fail("translate explicitly admitted M11 DM1 status literals");
    }
    if (strcmp(fs_po_gettext_in_domain("dm1", "FOUNTAIN"), "FONTÄN") != 0 ||
        strcmp(fs_po_gettext_in_domain("dm1", "CONTAINER FILLED"),
               "BEHÅLLAREN ÄR FYLLD") != 0 ||
        strcmp(fs_po_gettext_in_domain("dm1", "FLASK FILLED WITH WATER"),
               "FLASKAN ÄR FYLLD MED VATTEN") != 0) {
        return fail("translate source-locked DM1 fountain feedback");
    }
    /* PANEL.C F0351 publishes retail rank/class/statistic labels both into
     * the source-font panel and the live eye readout.  Representative values
     * prove the Swedish catalog reaches that final DM1-domain boundary. */
    if (strcmp(fs_po_gettext_in_domain("dm1", "APPRENTICE"), "LÄRLING") != 0 ||
        strcmp(fs_po_gettext_in_domain("dm1", "ARCHMASTER"), "ARKIMÄSTARE") != 0 ||
        strcmp(fs_po_gettext_in_domain("dm1", "STRENGTH"), "STYRKA") != 0 ||
        strcmp(fs_po_gettext_in_domain("dm1", "ANTI-MAGIC"), "ANTIMAGI") != 0) {
        return fail("translate authentic F0351 rank and statistic labels");
    }
    if (strcmp(fs_po_gettext_in_domain("dm1", "GIANT SCORPION"),
               "JÄTTESKORPION") != 0 ||
        strcmp(fs_po_gettext_in_domain("dm1", "RUSTER"),
               "ROSTVARELSE") != 0 ||
        strcmp(fs_po_gettext_in_domain("dm1", "GIANT WASP"),
               "JÄTTEGETING") != 0 ||
        strcmp(fs_po_gettext_in_domain("dm1", "GREY LORD"),
               "GRÅ HERRE") != 0) {
        return fail("translate source-owned C00-C26 creature labels");
    }
    translated = fs_po_gettext_in_domain(
        "dm1",
        "THANK YOU MY\nFRIENDS.  YOU\nHAVE BANISHED\nCHAOS AND\n"
        "REJECTED THE\nFALSE PATH OF\nUNCOMPROMISING\nORDER.");
    if (!translated || strstr(translated, "TACK MINA\nVÄNNER") != translated ||
        strstr(translated, "ORUBBLIG\nORDNING.") == NULL) {
        return fail("translate authentic PC3.4 F0446 first endgame message");
    }
    translated = fs_po_gettext_in_domain(
        "dm1",
        "ONLY BY LEARNING\nTHE TRUTH AND\nSEEKING THE PATH\nOF BALANCE\n"
        "DID YOU GUESS THE\nTRUE NATURE OF\nTHE FIRESTAFF.");
    if (!translated || strstr(translated, "ENDAST GENOM ATT") != translated ||
        strstr(translated, "ELDSTAVENS SANNA\nNATUR.") == NULL) {
        return fail("translate authentic PC3.4 F0446 second endgame message");
    }

    /* Lock the runtime call sites as well as the PO lookup primitive.  These
     * strings used to be formatted first and consequently could never match
     * their percent-bearing catalog keys. */
    snprintf(sourcePath, sizeof(sourcePath),
             FIRESTAFF_SOURCE_DIR "/src/engine/m11_game_view.c");
    stream = fopen(sourcePath, "rb");
    if (!stream) return fail("open M11 runtime source lock");
    if (fseek(stream, 0L, SEEK_END) != 0) {
        fclose(stream);
        return fail("measure M11 runtime source lock");
    }
    sourceSizeLong = ftell(stream);
    if (sourceSizeLong < 0 || fseek(stream, 0L, SEEK_SET) != 0 ||
        (sourceSize = (size_t)sourceSizeLong) == 0u ||
        !(source = (char*)malloc(sourceSize + 1u))) {
        fclose(stream);
        return fail("allocate M11 runtime source lock");
    }
    sourceSize = fread(source, 1u, sourceSize, stream);
    fclose(stream);
    source[sourceSize] = '\0';
    if (!strstr(source,
                "m11_set_inspect_readoutf(state, \"PICKED UP\", \"%s -> MOUSE HAND\"") ||
        !strstr(source,
                "m11_set_inspect_readoutf(state, \"DROPPED\", \"%s FROM MOUSE HAND\"") ||
        !strstr(source,
                "m11_set_inspect_readoutf(state, \"RUNE ENTERED\",") ||
        !strstr(source, "M11_DM1_PRESENTED(\"LEADER HAND FULL\")") ||
        !strstr(source, "M11_DM1_PRESENTED(\"SPELL CAST\")") ||
        !strstr(source, "M11_DM1_PRESENTED(\"FOUNTAIN\")") ||
        !strstr(source, "M11_DM1_PRESENTED(\"FLASK FILLED WITH WATER\")") ||
        !strstr(source, "M11_DM1_PRESENTED(\"GIANT SCORPION\")") ||
        !strstr(source, "M11_DM1_PRESENTED(\"GREY LORD\")") ||
        !strstr(source,
                "fs_po_gettext_in_domain(\"dm1\", dm1PresentedNames[creatureType])") ||
        !strstr(source, "const char* sourceMessage = &decoded[2]") ||
        !strstr(source, "snprintf(presented, sizeof(presented), \"\\n%s\"")) {
        free(source);
        return fail("route formatted DM1 readouts through translation boundary");
    }
    free(source);

    puts("PASS: DM1 runtime l10n catalog, formatted presentation, named fields, and fallback");
    return 0;
}
