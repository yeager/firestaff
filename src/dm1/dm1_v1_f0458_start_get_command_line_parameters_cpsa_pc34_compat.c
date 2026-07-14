#include "firestaff/dm1/v1/f0458_start_get_command_line_parameters_cpsa_pc34_compat.h"

#include <string.h>

int16_t DM1_V1_F0458_START_GetCommandLineParameters_CPSA_Pc34Compat(
    const unsigned char *base_page,
    size_t base_page_size,
    char *command_line_parameters,
    size_t command_line_parameters_size)
{
    size_t command_line_length;
    size_t source_size;

    if (!base_page || !command_line_parameters ||
        base_page_size <= DM1_V1_F0458_BASE_PAGE_COMMAND_TAIL_OFFSET_PC34) {
        return -1;
    }

    command_line_length =
        base_page[DM1_V1_F0458_BASE_PAGE_COMMAND_TAIL_OFFSET_PC34];
    if (command_line_length > DM1_V1_F0458_MAX_COMMAND_TAIL_LENGTH_PC34) {
        return -1;
    }

    source_size = DM1_V1_F0458_BASE_PAGE_COMMAND_TAIL_OFFSET_PC34 + 1U +
                  command_line_length;
    if (base_page_size < source_size ||
        command_line_parameters_size < command_line_length + 2U) {
        return -1;
    }

    memcpy(command_line_parameters,
           base_page + DM1_V1_F0458_BASE_PAGE_COMMAND_TAIL_OFFSET_PC34 + 1U,
           command_line_length);
    command_line_parameters[command_line_length + 1U] = '\0';
    return (int16_t)command_line_length;
}
