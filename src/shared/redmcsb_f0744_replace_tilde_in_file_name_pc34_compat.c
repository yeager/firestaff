#include "redmcsb_f0744_replace_tilde_in_file_name_pc34_compat.h"

void redmcsb_f0744_replace_tilde_in_file_name_pc34_compat(
    char *file_name,
    char replacement_character)
{
    char *string;
    char character;

    /* ReDMCSB FILENAME.C:60-81, PC 3.4 MEDIA736_I34M_A36M_A31M_A33M_A35M. */
    while ((character = *file_name) != '\0') {
        if (character == '~') {
            if (replacement_character != '\0') {
                *file_name = replacement_character;
            } else {
                string = file_name;
                while ((*string = *(string + 1)) != '\0') {
                    string++;
                }
                continue;
            }
        }
        file_name++;
    }
}

const char *redmcsb_f0744_replace_tilde_in_file_name_source_evidence_pc34(void)
{
    return "ReDMCSB FILENAME.C:60-81 defines F0744 as an in-place scan: "
           "nonzero language characters replace '~', while a zero character "
           "shifts the suffix left and rechecks the same position.";
}
