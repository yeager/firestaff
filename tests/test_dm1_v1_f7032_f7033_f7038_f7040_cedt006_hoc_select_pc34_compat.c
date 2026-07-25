#include "dm1_v1_cedt006_champion_editor_pc34_compat.h"

#include <assert.h>
#include <string.h>

static void fill_champions(DM1_V1_CEDT006_ChampionSummaryPc34 champions[4])
{
    memset(champions, 0, sizeof(DM1_V1_CEDT006_ChampionSummaryPc34) * 4u);
    champions[0].present = 1;
    strcpy(champions[0].name, "TIGGY");
    strcpy(champions[0].title, "ARCHMASTER");
    champions[0].portraitIndex = 2;
    champions[0].portraitPixelsProven = 1;

    champions[1].present = 1;
    strcpy(champions[1].name, "WUUF");
    strcpy(champions[1].title, "BIKA");
    champions[1].portraitIndex = 7;
    champions[1].portraitPixelsProven = 0;
}

static void test_top_name_and_portrait_receipts_are_source_bounded(void)
{
    DM1_V1_CEDT006_ChampionSummaryPc34 champions[4];
    DM1_V1_CEDT006_TopNameReceiptPc34 nameReceipt;
    (void)nameReceipt;
    DM1_V1_CEDT006_PortraitsAndNamesReceiptPc34 listReceipt;
    (void)listReceipt;

    fill_champions(champions);

    assert(F7032_DrawChampionNameOnTopOfScreen(0, &champions[0],
                                               &nameReceipt) == 1);
    assert(nameReceipt.valid == 1);
    assert(nameReceipt.championIndex == 0);
    assert(strcmp(nameReceipt.name, "TIGGY") == 0);
    assert(nameReceipt.sourceLineStart == 351);

    assert(F7033_DrawPortraitsAndNamesOnTopOfScreen(champions, 4,
                                                    &listReceipt) == 1);
    assert(listReceipt.valid == 1);
    assert(listReceipt.championCount == 4);
    assert(listReceipt.nameReceiptCount == 2);
    assert(listReceipt.portraitReceiptCount == 1);
    assert(listReceipt.rejectedChampionCount == 2);
    assert(listReceipt.sourceLineStart == 372);

    assert(F7032_DrawChampionNameOnTopOfScreen(3, &champions[3],
                                               &nameReceipt) == 0);
    assert(F7033_DrawPortraitsAndNamesOnTopOfScreen(champions, 5,
                                                    &listReceipt) == 0);
}

static void test_name_title_edition_and_selection_use_caller_champion(void)
{
    DM1_V1_CEDT006_ChampionSummaryPc34 champions[4];
    DM1_V1_CEDT006_NameOrTitleEditionReceiptPc34 editReceipt;
    (void)editReceipt;
    DM1_V1_CEDT006_SelectChampionReceiptPc34 selectReceipt;
    (void)selectReceipt;

    fill_champions(champions);

    assert(F7038_PrintChampionNameOrTitleForEdition(1, &champions[1], 0,
                                                    &editReceipt) == 1);
    assert(editReceipt.valid == 1);
    assert(editReceipt.editTitle == 0);
    assert(strcmp(editReceipt.text, "WUUF") == 0);
    assert(editReceipt.sourceLineStart == 472);

    assert(F7038_PrintChampionNameOrTitleForEdition(1, &champions[1], 1,
                                                    &editReceipt) == 1);
    assert(strcmp(editReceipt.text, "BIKA") == 0);

    assert(F7040_SelectChampion(champions, 4, 0, 1, &selectReceipt) == 1);
    assert(selectReceipt.valid == 1);
    assert(selectReceipt.selectedChampionIndex == 0);
    assert(selectReceipt.previousChampionIndex == 1);
    assert(strcmp(selectReceipt.name, "TIGGY") == 0);
    assert(strcmp(selectReceipt.title, "ARCHMASTER") == 0);
    assert(selectReceipt.portraitIndex == 2);
    assert(selectReceipt.drawNameRequested == 1);
    assert(selectReceipt.drawTitleRequested == 1);
    assert(selectReceipt.drawPortraitRequested == 1);
    assert(selectReceipt.sourceLineStart == 512);

    assert(F7040_SelectChampion(champions, 4, 3, 0, &selectReceipt) == 0);
    assert(F7040_SelectChampion(champions, 4, -1, 0, &selectReceipt) == 0);
}

static void test_source_evidence_names_no_synthetic_boundaries(void)
{
    const char *evidence = F7032_F7033_F7038_F7040_CEDT006_SourceEvidencePc34();
    (void)evidence;

    assert(strstr(evidence, "CEDT006.C:351") != 0);
    assert(strstr(evidence, "CEDT006.C:372") != 0);
    assert(strstr(evidence, "CEDT006.C:472") != 0);
    assert(strstr(evidence, "CEDT006.C:512") != 0);
    assert(strstr(evidence, "caller-owned champion") != 0);
    assert(strstr(evidence, "does not synthesize") != 0);
    assert(strstr(evidence, "portrait pixels") != 0);
}

int main(void)
{
    test_top_name_and_portrait_receipts_are_source_bounded();
    test_name_title_edition_and_selection_use_caller_champion();
    test_source_evidence_names_no_synthetic_boundaries();
    return 0;
}
