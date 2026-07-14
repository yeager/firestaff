#include "redmcsb_f0909_f0910_swoosh_sound_pc34_compat.h"

#include <stddef.h>

void redmcsb_f0909_play_swoosh_sound_pc34_compat(
    const redmcsb_f0909_f0910_swoosh_sound_host_pc34_compat *host)
{
    host->begin_io(host->host_context, host->sound1_left_channel);
    host->begin_io(host->host_context, host->sound1_right_channel);
    host->set_control_command(host->host_context, host->sound1_control,
                              host->command_start);
    host->do_io(host->host_context, host->sound1_control);
}

void redmcsb_f0910_release_swoosh_sound_pc34_compat(
    const redmcsb_f0909_f0910_swoosh_sound_host_pc34_compat *host)
{
    host->set_control_command(host->host_context, host->sound1_control,
                              host->command_finish);
    host->set_control_flag(host->host_context, host->sound1_control,
                           host->flag_sync_cycle);
    host->begin_io(host->host_context, host->sound1_control);
    host->wait_io(host->host_context, host->sound1_left_channel);
    host->wait_io(host->host_context, host->sound1_right_channel);
    host->clear_control_flag(host->host_context, host->sound1_control,
                             host->flag_sync_cycle);
    host->set_control_command(host->host_context, host->sound1_control,
                              host->command_stop);
    host->do_io(host->host_context, host->sound1_control);
}

void redmcsb_f0910_release_swoosh_sound_and_buffer_pc34_compat(
    const redmcsb_f0909_f0910_swoosh_sound_host_pc34_compat *host,
    redmcsb_f0909_f0910_free_memory_pc34_compat free_memory,
    void **swoosh_sound_data_location,
    long swoosh_sound_data_byte_count)
{
    redmcsb_f0910_release_swoosh_sound_pc34_compat(host);
    free_memory(host->host_context, *swoosh_sound_data_location,
                swoosh_sound_data_byte_count);
    *swoosh_sound_data_location = NULL;
}

const char *redmcsb_f0909_f0910_swoosh_sound_source_evidence_pc34(void)
{
    return "ReDMCSB Toolchains/Common/Source/SWSHSND.C:26-48 "
           "(MEDIA429_A20ED_A20E_A20F_A21E_A31E) and :207-227 "
           "(MEDIA618_A31E): F0909 begins left then right, sets CMD_START, "
           "and DoIOs control; F0910 sets ADCMD_FINISH and ADIOF_SYNCCYCLE, "
           "begins control, waits left then right, clears the flag, sets "
           "CMD_STOP, and DoIOs control. The former branch then FreeMems and "
           "nulls SwooshSoundData.";
}
