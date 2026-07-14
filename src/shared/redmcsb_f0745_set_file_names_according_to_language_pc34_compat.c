#include "redmcsb_f0745_set_file_names_according_to_language_pc34_compat.h"

static void redmcsb_f0745_replace_tilde_in_file_name_pc34_compat(
    char *file_name,
    char replacement)
{
    char *cursor = file_name;

    while (*cursor != '\0') {
        if (*cursor == '~') {
            if (replacement != '\0') {
                *cursor = replacement;
            } else {
                char *shift_cursor = cursor;

                while ((*shift_cursor = *(shift_cursor + 1)) != '\0') {
                    shift_cursor++;
                }
                continue;
            }
        }
        cursor++;
    }
}

void redmcsb_f0745_set_file_names_according_to_language_pc34_compat(
    redmcsb_f0745_file_names_pc34_compat *file_names,
    redmcsb_f0745_language_pc34_compat language)
{
    char replacement;

    switch (language) {
    case REDMCSB_F0745_LANGUAGE_ENGLISH_PC34_COMPAT:
        replacement = '\0';
        break;
    case REDMCSB_F0745_LANGUAGE_FRENCH_PC34_COMPAT:
        replacement = 'F';
        break;
    case REDMCSB_F0745_LANGUAGE_GERMAN_PC34_COMPAT:
        replacement = 'G';
        break;
    default:
        return;
    }

    /* ReDMCSB FILENAME.C:84-105, PC 3.4 MEDIA736_I34M route. */
    redmcsb_f0745_replace_tilde_in_file_name_pc34_compat(
        file_names->dungeon_file_name, replacement);
    redmcsb_f0745_replace_tilde_in_file_name_pc34_compat(
        file_names->expansion_set_dungeon_file_name, replacement);
    redmcsb_f0745_replace_tilde_in_file_name_pc34_compat(
        file_names->bonus_dungeon_file_name, replacement);
    redmcsb_f0745_replace_tilde_in_file_name_pc34_compat(
        file_names->saved_game_file_name, replacement);
    redmcsb_f0745_replace_tilde_in_file_name_pc34_compat(
        file_names->saved_game_backup_file_name, replacement);
}

const char *redmcsb_f0745_set_file_names_according_to_language_source_evidence_pc34(void)
{
    return "ReDMCSB WIP20210206 FILENAME.C:59-82 defines F0744's in-place "
           "tilde replacement/removal loop; FILENAME.C:84-105 maps "
           "C0_ENGLISH/C1_FRENCH/C2_GERMAN to NUL/'F'/'G' and applies it "
           "to dungeon, expansion, bonus, save, and save-backup filenames.";
}
