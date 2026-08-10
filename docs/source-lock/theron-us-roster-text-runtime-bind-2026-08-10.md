# Theron US-rostertext: runtimebindning (2026-08-10)

Den verifierade startup-runtimevägen använder nu
`theron_v1_track02_catalog_startup_roster_names()` direkt på den autentiska
Track 02-filen innan forcefield-party-init. För kända US- och JP-BIN ersätter
den source-bundna katalogen hostens valfria namnlista. Alla namn behåller sin
rå offset/proveniens i katalogen och skickas sedan till den befintliga
party/HUD-textvägen.

Detta täpper till en konkret avvikelse där ett värdlevererat namn kunde vinna
över en riktig codonpost. Om katalogen inte kan verifieras avvisas handoffen
utan partiell party-state.

Gränsen är avsiktlig: detta bevisar Firestaffs textbindning, men inte att
originalets HuC6280-textkonsument har samma byte-/font-/VDC-rutt. US-titlar,
fontgrafik, JP-porträtt och originalets fulla presentation kräver fortsatt
source-bound runtimecapture.
