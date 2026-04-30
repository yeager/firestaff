# DM1 V1 language/version source-contract boundary

This static evidence gate records the original ReDMCSB PC launcher language contract and keeps Firestaff current launcher language/version UI classified as a boundary, not a parity claim.

## ReDMCSB citations audited first

- `DM.C:11-18` `C3_LANGUAGE_CHOICE_COUNT / C1_VGA_GRAPHICS..C3_TANDY_GRAPHICS` - PC media branch defines exactly 3 language choices and 3 video choices.
- `DM.C:643-646` `G8026_Language` - Language menu entries are English, German, French with source indices C1/C2/C3.
- `DM.C:650-687` `F8086_GetMenuChoice` - Menu input returns the selected entry Index after Allowed validation.
- `DM.C:891-897` `main` - When menu choices are requested, language is selected before video/audio/input.
- `DM.C:1018-1027` `main` - The selected language maps only to -*e, -*g, or -*f command-line parameters.

## Gate result

- Status: `PASS`
- Source contract: DM1 PC I34M language source path is exactly English/German/French and emits -*e/-*g/-*f.
- Firestaff current UI: Firestaff persists per-game version/language options and exposes EN/SV/FR/DE launcher labels.
- Firestaff extra language labels vs ReDMCSB DM1 source: `SVENSKA, FRANCAIS, DEUTSCH`
- Parity claimed: `False`
- Non-claim: This gate documents a source/version boundary only; it does not claim Firestaff language selection is original-faithful DM1 V1 parity.

## Next step

If DM1 V1 language selection becomes runtime-significant, add a separate implementation gate that maps only the ReDMCSB English/German/French source triad to the original data/version path before claiming parity.
