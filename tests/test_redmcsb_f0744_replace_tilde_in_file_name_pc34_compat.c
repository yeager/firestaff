#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "redmcsb_f0744_replace_tilde_in_file_name_pc34_compat.h"

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#error "This test requires C11 or later."
#endif

_Static_assert(sizeof(char) == 1, "F0744 operates on individual characters");

int main(void)
{
    char replacement_name[] = "DF0:DMGAME~.DAT";
    char adjacent_tildes[] = "A~~B~.DAT";
    char only_tildes[] = "~~~";
    char unmarked_name[] = "DUNGEON.DAT";

    redmcsb_f0744_replace_tilde_in_file_name_pc34_compat(
        replacement_name, 'F');
    assert(strcmp(replacement_name, "DF0:DMGAMEF.DAT") == 0);

    redmcsb_f0744_replace_tilde_in_file_name_pc34_compat(
        adjacent_tildes, '\0');
    assert(strcmp(adjacent_tildes, "AB.DAT") == 0);

    redmcsb_f0744_replace_tilde_in_file_name_pc34_compat(only_tildes, '\0');
    assert(strcmp(only_tildes, "") == 0);

    redmcsb_f0744_replace_tilde_in_file_name_pc34_compat(unmarked_name, 'G');
    assert(strcmp(unmarked_name, "DUNGEON.DAT") == 0);

    assert(strstr(redmcsb_f0744_replace_tilde_in_file_name_source_evidence_pc34(),
                  "FILENAME.C:60-81") != NULL);

    puts("ok: ReDMCSB F0744 PC 3.4 tilde filename mutation");
    return 0;
}
