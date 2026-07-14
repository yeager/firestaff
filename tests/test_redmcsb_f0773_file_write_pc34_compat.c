#include "redmcsb_f0773_file_write_pc34_compat.h"

#include <stddef.h>
#include <stdint.h>

struct WriteScript {
    struct RedmcsbF0773FileWriteResult returned[3];
    uint16_t requested[3];
    int16_t handles[3];
    const unsigned char *buffers[3];
    size_t calls;
};

static struct RedmcsbF0773FileWriteResult ScriptedWrite(
    void *context,
    int16_t file_handle,
    const unsigned char *buffer,
    uint16_t byte_count)
{
    struct WriteScript *script = context;
    size_t call = script->calls++;

    script->requested[call] = byte_count;
    script->handles[call] = file_handle;
    script->buffers[call] = buffer;
    return script->returned[call];
}

int main(void)
{
    unsigned char buffer[65537];
    struct WriteScript script = {
        {{32768U, 0}, {32768U, 0}, {1U, 0}}, {0U}, {0}, {0}, 0U};

    if (!RedmcsbF0773FileWritePc34Compat(
            &script, ScriptedWrite, -7, 65537UL, buffer)) {
        return 1;
    }
    if (script.calls != 3U || script.requested[0] != 32768U ||
        script.requested[1] != 32768U || script.requested[2] != 1U ||
        script.handles[0] != -7 || script.handles[1] != -7 ||
        script.handles[2] != -7 || script.buffers[0] != buffer ||
        script.buffers[1] != buffer + 32768U ||
        script.buffers[2] != buffer + 65536U) {
        return 2;
    }

    script = (struct WriteScript){{{9U, 1}, {0U, 0}, {0U, 0}},
                                  {0U}, {0}, {0}, 0U};
    if (RedmcsbF0773FileWritePc34Compat(
            &script, ScriptedWrite, 3, 9UL, buffer) || script.calls != 1U ||
        script.requested[0] != 9U) {
        return 3;
    }

    script = (struct WriteScript){{{8U, 0}, {0U, 0}, {0U, 0}},
                                  {0U}, {0}, {0}, 0U};
    if (RedmcsbF0773FileWritePc34Compat(
            &script, ScriptedWrite, 4, 9UL, buffer) || script.calls != 1U ||
        script.requested[0] != 9U) {
        return 4;
    }

    script = (struct WriteScript){{{0U, 0}, {0U, 0}, {0U, 0}},
                                  {0U}, {0}, {0}, 0U};
    if (!RedmcsbF0773FileWritePc34Compat(
            &script, ScriptedWrite, 4, 0UL, buffer) || script.calls != 0U) {
        return 5;
    }

    return 0;
}
