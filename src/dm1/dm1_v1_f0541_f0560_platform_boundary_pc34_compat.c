#include "dm1_v1_f0541_f0560_platform_boundary_pc34_compat.h"

static const DM1_V1_F0541F0560PlatformBoundaryPc34 kBoundaries[] = {
    {"F0541_INPUT_WaitForMouseOrKeyboardActivity", "INPUT.C:279-286; ENDGAME.C:691-699", "Amiga Delay input wait", 0, "I34E selects F0712_AnyKeyboardOrMouseInput; no F0541 PC34 call exists."},
    {"F0543_INPUT_DeviceInterruptHandler", "INPUT.C:298-752; INPUT.C:771-807", "Amiga input.device interrupt chain", 0, "InputEvent/IND_ADDHANDLER is not selected by I34E/I34M."},
    {"F0545_MOUSE_AllocateMemory", "IO.C:621-651; STARTUP1.C:248-270", "Amiga chip-memory sprite allocation", 0, "AllocMem SPRITEIMAGE setup has no PC34 source callsite."},
    {"F0546_MOUSE_Deinitialize", "IO.C:653-676; STARTUP1.C:417-433", "Amiga sprite resource teardown", 0, "FreeMem sprite teardown has no PC34 source callsite."},
    {"F0547_MOUSE_SetPointerSprites", "IO.C:2221-2249", "Amiga hardware cursor sprites", 0, "UWORD sprite programming has no PC34 source route."},
    {"F0548_MOUSE_HidePointer", "IO.C:3639-3670", "Amiga/X68000 cursor hardware", 0, "The MEDIA421 gate excludes I34E/I34M."},
    {"F0549_MOUSE_ShowPointer", "IO.C:3672-3703", "Amiga/X68000 cursor hardware", 0, "The MEDIA421 gate excludes I34E/I34M."},
    {"F0550_VIDEO_FillScreenBox", "BLITFILL.C:179-194", "IIGS/Amiga screen fill", 0, "MEDIA353 selects only G14/G20 or Amiga; an existing generic helper is not PC34 provenance."},
    {"F0553_TEXT_MESSAGEAREA_MoveCursorToTopLeft", "TEXT.C:1407-1415", "Amiga message-area bitmap", 0, "MEDIA413 only; PC34 text uses its separately selected message route."},
    {"F0554_TEXT_MESSAGEAREA_DeleteLastCharacter_Unreferenced", "TEXT.C:1918-1934", "Amiga message-area bitmap", 0, "MEDIA413-only unreferenced helper."},
    {"F0555_TEXT_AllocateMemory", "TEXT.C:2032-2052", "Amiga bitplane allocation", 0, "AllocMem/InitBitMap route has no PC34 source callsite."},
    {"F0556_TEXT_Deinitialize", "TEXT.C:2054-2065", "Amiga bitplane teardown", 0, "FreeBitMap memory route has no PC34 source callsite."},
    {"F0560_SCROLLER_SetCommand", "SCRLMGMT.C:72-83; TEXT.C:1443", "Amiga task message port", 0, "PutMsg/WaitPort/GetMsg task protocol is absent from PC34."},
};

const DM1_V1_F0541F0560PlatformBoundaryPc34 *
dm1_v1_f0541_f0560_platform_boundary_pc34_at(unsigned int index) {
    return index < sizeof(kBoundaries) / sizeof(kBoundaries[0]) ? &kBoundaries[index] : 0;
}

unsigned int dm1_v1_f0541_f0560_platform_boundary_pc34_count(void) {
    return (unsigned int)(sizeof(kBoundaries) / sizeof(kBoundaries[0]));
}

int dm1_v1_f0541_f0560_platform_boundary_has_pc34_route(
    const DM1_V1_F0541F0560PlatformBoundaryPc34 *boundary) {
    return boundary && boundary->has_portable_pc34_route != 0;
}

const char *dm1_v1_f0541_f0560_platform_boundary_source_evidence_pc34(void) {
    return "ReDMCSB WIP20210206 INPUT.C:279-752; IO.C:621-676,2221-2249,3639-3703; "
           "TEXT.C:1407-1415,1918-1934,2032-2065; SCRLMGMT.C:72-83; "
           "2026-07-23_REDMCSB_F0541_F0560_PC34_SOURCE_AUDIT.tsv; "
           "F0544/F0551/F0552/F0557-F0559 retain their pre-existing separate owners; "
           "F0550's generic helper is not a PC34 source route.";
}
