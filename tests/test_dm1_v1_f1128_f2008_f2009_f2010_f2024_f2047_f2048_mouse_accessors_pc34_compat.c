#include "dm1_v1_input_command_queue_pc34_compat.h"

#include <assert.h>
#include <string.h>

int main(void)
{
    struct Dm1V1UsioMouseStatusPc34Compat status;
    const char* evidence;
    (void)evidence;

    status.mouseButtons = DM1_V1_BUTTON_LEFT;
    status.mouseX = 123;
    status.mouseY = 45;

    assert(F1128_IsLeftMouseButtonDown(&status) == 1);
    assert(F2008_IsLeftMouseButtonDown(&status) == 1);
    assert(F2024_IsLeftMouseButtonDown(&status) == 1);
    assert(F2009_GetMouseX(&status) == 123);
    assert(F2010_GetMouseY(&status) == 45);
    assert(F2047_GetMouseX(&status) == 123);
    assert(F2048_GetMouseY(&status) == 45);

    status.mouseButtons = DM1_V1_BUTTON_RIGHT;
    status.mouseX = -7;
    status.mouseY = 201;
    assert(F1128_IsLeftMouseButtonDown(&status) == 0);
    assert(F2008_IsLeftMouseButtonDown(&status) == 0);
    assert(F2024_IsLeftMouseButtonDown(&status) == 0);
    assert(F2009_GetMouseX(&status) == -7);
    assert(F2010_GetMouseY(&status) == 201);
    assert(F2047_GetMouseX(&status) == -7);
    assert(F2048_GetMouseY(&status) == 201);

    assert(F1128_IsLeftMouseButtonDown(0) == 0);
    assert(F2008_IsLeftMouseButtonDown(0) == 0);
    assert(F2024_IsLeftMouseButtonDown(0) == 0);
    assert(F2009_GetMouseX(0) == 0);
    assert(F2010_GetMouseY(0) == 0);
    assert(F2047_GetMouseX(0) == 0);
    assert(F2048_GetMouseY(0) == 0);

    evidence = F1128_IsLeftMouseButtonDown_SourceEvidence();
    assert(evidence != 0);
    assert(strstr(evidence, "FILLBOX.C:6") != 0);
    assert(strstr(evidence, "does not poll a host cursor") != 0);

    evidence = F2008_F2024_IsLeftMouseButtonDown_SourceEvidence();
    assert(evidence != 0);
    assert(strstr(evidence, "CEDT006.C:1323") != 0);
    assert(strstr(evidence, "HINT001.C:219") != 0);

    evidence = F2009_F2010_F2047_F2048_MouseCoordinate_SourceEvidence();
    assert(evidence != 0);
    assert(strstr(evidence, "CEDT006.C:1324") != 0);
    assert(strstr(evidence, "FILLBOX.C:837/843") != 0);

    return 0;
}
