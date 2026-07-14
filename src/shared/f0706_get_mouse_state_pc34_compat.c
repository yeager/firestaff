#include "f0706_get_mouse_state_pc34_compat.h"

void ReDMCSB_F0706_GetMouseStatePc34Compat(
    const ReDMCSB_F0706_IODriverPc34 *ioDriver,
    int16_t *outX,
    int16_t *outY,
    int16_t *outButtons)
{
    /* ReDMCSB IO.C F0706: direct IODRV_13 call, no local policy. */
    if (ioDriver == 0 || ioDriver->getMouseState == 0) {
        return;
    }

    ioDriver->getMouseState(outX, outY, outButtons);
}
