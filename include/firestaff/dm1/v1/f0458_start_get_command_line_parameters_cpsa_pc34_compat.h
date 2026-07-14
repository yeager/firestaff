#ifndef FIRESTAFF_DM1_V1_F0458_START_GET_COMMAND_LINE_PARAMETERS_CPSA_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_F0458_START_GET_COMMAND_LINE_PARAMETERS_CPSA_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

/*
 * ReDMCSB STARTUP2.C:439-455 reads the TOS base-page command tail at
 * offset 128. It copies the length-prefixed bytes and terminates at
 * destination[length + 1]; it does not parse options or configuration.
 */
enum {
    DM1_V1_F0458_BASE_PAGE_COMMAND_TAIL_OFFSET_PC34 = 128,
    DM1_V1_F0458_MAX_COMMAND_TAIL_LENGTH_PC34 = 127
};

/*
 * Returns the reference command-tail length, or -1 when either bounded
 * buffer cannot represent the original operation.
 */
int16_t DM1_V1_F0458_START_GetCommandLineParameters_CPSA_Pc34Compat(
    const unsigned char *base_page,
    size_t base_page_size,
    char *command_line_parameters,
    size_t command_line_parameters_size);

#endif
