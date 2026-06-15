# pass765 DM1 V1 mirror-candidate C040 chrome inventory owner swap regression

Status: contract-only DM1 V1 runtime regression.

Scope: `test_dm1_v1_mirror_candidate_c040_chrome_inventory_owner_swap_pc34_compat` pins the C040 mirror-candidate panel chrome after an inventory-owner swap while the C040 panel remains live. A C027 non-leader portrait owner swap retargets C037/C038/C039 status boxes, the C30 hand-slot indicator, and portrait overlay to the new leader while preserving G0299. The close path then runs against the new leader, clears G0299, and redraws M516 background chrome.

Non-overlap: this is not the inventory portrait click ignore path, pass694 resurrect + champion-switch + reopen path, C045 close-while-candidate path, or C038 scroll-pickup non-leader panel-live path.

Anchors: CHAMDRAW.C F0291/F0296; REVIVE.C F0280:124-132 and F0282:744-806; COMMAND.C F0359:1985-1990; CHEST.C F0333:30-67 and F0334:113-132; CHAMPION.C F0284:93-131, F0297:243-298, F0298:270-298, F0300:511-515, F0301:606-614, F0302:662-714; PANEL.C F0344:1159-1205, F0345:1230-1275, F0352:2260-2330; DEFS.H:2088 C30/C040/G0299/G0425/G0426/M070/M516.

No original assets are required.
