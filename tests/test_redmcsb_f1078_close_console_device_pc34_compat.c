#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "redmcsb_f1078_close_console_device_pc34_compat.h"

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#error "This test requires C11 or later."
#endif

int main(void)
{
    const char *evidence =
        redmcsb_f1078_close_console_device_source_evidence_pc34();
    (void)evidence;

    redmcsb_f1078_close_console_device_pc34_compat();

    assert(strstr(evidence, "AMIGINIT.C:4") != NULL);
    assert(strstr(evidence,
                  "MEDIA754_A36M_A31E_A31M_A33M_A35E_A35M_AU1E_AU2E_AU2F_"
                  "AU2G_AU3E") != NULL);
    assert(strstr(evidence, "AMIGINIT.C:182-198") != NULL);
    assert(strstr(evidence, "F1078_CloseConsoleDevice") != NULL);
    assert(strstr(evidence,
                  "CloseDevice(G3156_ps_IOStdReq_Console)") != NULL);
    assert(strstr(evidence, "G3154_ps_Device_Console") != NULL);
    assert(strstr(evidence,
                  "DeleteStdIO(G3156_ps_IOStdReq_Console)") != NULL);
    assert(strstr(evidence, "G3156_ps_IOStdReq_Console") != NULL);
    assert(strstr(evidence, "DeletePort(G3155_ps_MsgPort)") != NULL);
    assert(strstr(evidence, "G3155_ps_MsgPort") != NULL);
    assert(strstr(evidence, "AMIGINIT.C:363-379") != NULL);
    assert(strstr(evidence, "F1089_CloseAmigaStuff") != NULL);
    assert(strstr(evidence, "F1082_CloseNIL") != NULL);
    assert(strstr(evidence, "EXETYPE is C03_GAME, C07_HINT") != NULL);
    assert(strstr(evidence, "defined(MEDIA762_AU1E) with C06_CEDT") != NULL);
    assert(strstr(evidence, "No PC 3.4 branch") != NULL);
    assert(strstr(evidence, "portable host behavior") != NULL);

    puts("ok: ReDMCSB F1078 console-device teardown boundary");
    return 0;
}
