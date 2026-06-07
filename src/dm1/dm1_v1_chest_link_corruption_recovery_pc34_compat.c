#include "dm1_v1_chest_link_corruption_recovery_pc34_compat.h"

static const char s_f0334_anchor[] =
    "ReDMCSB CHEST.C F0334:117-132 sets Container->Slot to "
    "C0xFFFE_THING_ENDOFLIST, iterates eight G0425_aT_ChestSlots entries, "
    "and rewrites only entries not equal to C0xFFFF_THING_NONE.";

static const char s_f0333_anchor[] =
    "ReDMCSB CHEST.C F0333:53-67 starts at Container->Slot, walks the "
    "rewritten Next chain until C0xFFFE_THING_ENDOFLIST, and materializes "
    "visible links into G0425_aT_ChestSlots in order.";

static const char s_sentinel_anchor[] =
    "ReDMCSB CHEST.C F0333:58-67 and F0334:117-132 distinguish "
    "C0xFFFF_THING_NONE from C0xFFFE_THING_ENDOFLIST; NONE is an empty "
    "visible slot, END terminates the rewritten link array.";

static const char s_contract_note[] =
    "contract_only=1; source-locked chest link corruption recovery gate; "
    "mid-array C0xFFFF_THING_NONE is skipped by CHEST.C F0334:117-132; "
    "leading C0xFFFF_THING_NONE does not short-circuit close/reopen; "
    "CHEST.C F0333:53-67 rematerializes the rewritten links in order.";

static const char s_source_summary[] =
    "contract_only=1; ReDMCSB CHEST.C F0334:117-132 close-write loop is "
    "bounded to eight visible G0425 slots and skips C0xFFFF_THING_NONE "
    "while preserving non-empty siblings; ReDMCSB CHEST.C F0333:53-67 "
    "reopen loop walks the compacted link array until C0xFFFE_THING_ENDOFLIST; "
    "last valid Next remains END; leading NONE remains an empty slot, not an "
    "early return.";

static const DM1_V1_ChestLinkCorruptionRecoveryContractPc34 s_contract = {
    1,
    DM1_V1_CHEST_LINK_CORRUPTION_SLOT_COUNT,
    DM1_V1_CHEST_LINK_CORRUPTION_THING_NONE,
    DM1_V1_CHEST_LINK_CORRUPTION_THING_ENDOFLIST,
    DM1_V1_CHEST_LINK_CORRUPTION_SLOT_COUNT,
    1,
    1,
    1,
    1,
    {
        {
            0x210, 0x220, DM1_V1_CHEST_LINK_CORRUPTION_THING_NONE, 0x230,
            0x240, DM1_V1_CHEST_LINK_CORRUPTION_THING_NONE, 0x250, 0x260
        },
        6,
        { 0x210, 0x220, 0x230, 0x240, 0x250, 0x260, 0, 0 },
        6,
        {
            0x210, 0x220, 0x230, 0x240, 0x250, 0x260,
            DM1_V1_CHEST_LINK_CORRUPTION_THING_NONE,
            DM1_V1_CHEST_LINK_CORRUPTION_THING_NONE
        },
        2,
        DM1_V1_CHEST_LINK_CORRUPTION_THING_ENDOFLIST,
        0x210,
        0x260,
        DM1_V1_CHEST_LINK_CORRUPTION_THING_ENDOFLIST,
        DM1_V1_CHEST_LINK_CORRUPTION_THING_ENDOFLIST,
        0
    },
    {
        {
            DM1_V1_CHEST_LINK_CORRUPTION_THING_NONE, 0x310, 0x320,
            DM1_V1_CHEST_LINK_CORRUPTION_THING_NONE, 0x330,
            DM1_V1_CHEST_LINK_CORRUPTION_THING_NONE,
            DM1_V1_CHEST_LINK_CORRUPTION_THING_NONE,
            DM1_V1_CHEST_LINK_CORRUPTION_THING_NONE
        },
        3,
        { 0x310, 0x320, 0x330, 0, 0, 0, 0, 0 },
        3,
        {
            0x310, 0x320, 0x330,
            DM1_V1_CHEST_LINK_CORRUPTION_THING_NONE,
            DM1_V1_CHEST_LINK_CORRUPTION_THING_NONE,
            DM1_V1_CHEST_LINK_CORRUPTION_THING_NONE,
            DM1_V1_CHEST_LINK_CORRUPTION_THING_NONE,
            DM1_V1_CHEST_LINK_CORRUPTION_THING_NONE
        },
        0,
        DM1_V1_CHEST_LINK_CORRUPTION_THING_ENDOFLIST,
        0x310,
        0x330,
        DM1_V1_CHEST_LINK_CORRUPTION_THING_ENDOFLIST,
        DM1_V1_CHEST_LINK_CORRUPTION_THING_ENDOFLIST,
        0
    },
    s_f0334_anchor,
    s_f0333_anchor,
    s_sentinel_anchor,
    s_contract_note,
    s_source_summary
};

const DM1_V1_ChestLinkCorruptionRecoveryContractPc34 *
dm1_v1_chest_link_corruption_recovery_contract_pc34(void)
{
    return &s_contract;
}
