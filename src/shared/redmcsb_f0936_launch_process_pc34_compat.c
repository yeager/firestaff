#include "redmcsb_f0936_launch_process_pc34_compat.h"

void redmcsb_f0936_launch_process_pc34_compat(
    const char *name,
    bool wait_process_completion,
    void *message,
    void *reply_port,
    const redmcsb_f0936_launch_process_ops_pc34_compat *ops,
    void *context)
{
    void *segment_list;
    void *process;

    segment_list = ops->load_seg(context, name);
    process = ops->create_proc(context, name, 0L, segment_list, 8000L);
    ops->put_msg(context, process, message);
    if (wait_process_completion) {
        ops->wait_port(context, reply_port);
        (void)ops->get_msg(context, reply_port);
        ops->unload_seg(context, segment_list);
    }
}

const char *redmcsb_f0936_launch_process_source_evidence_pc34(void)
{
    return "ReDMCSB EXEC.C:66-82 defines F0936_LaunchProcess: LoadSeg(name), "
           "CreateProc(name, 0L, segmentList, 8000L), PutMsg(process, "
           "G0723_ps_ExecMessage), then, only when wait is true, WaitPort, "
           "GetMsg, and UnLoadSeg. No F0936_ComputeChecksum exists in the "
           "supplied ReDMCSB source tree.";
}
