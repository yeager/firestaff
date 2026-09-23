# Therons ursprungliga rotationskommandon

## Källor och gemensam startpunkt

Fångsterna kommer från den autentiska USA-skivan med Track 02-MD5
`ceb02343868f80cec899e9b239aff2da`, System Card 3.0-MD5
`ff1a674273fe3540ccef576376407d1d` och samma Mednafen-savestate med MD5
`f17f377df210b4a3ae904a13fb85a7f0`. Ingen genererad karta, RAM-bild,
palett eller knapphändelse ingår.

Button I observeras i originalets indatabuffert som `$28B8=$01`, skriven från
logisk HuC6280-PC `$44E5` och fysisk PC `$0D04E5`. Kommandospåret börjar vid
denna kant och innehåller exakt 65 536 ordnade skrivningar till huvud-RAM.

## Vänsterrotation

Den vänstra rörelsepanelsknappen köar kommandotyp `$01`. Det verifierade
klicket har intern koordinat `$7B/$8F`; X-värdet motsvarar den nio bitar breda
skärmkoordinaten genom originalets dubblering i `$D56A..$D578`.

Originalrutinen `$D900..$D92E` ändrar global riktning `$203F` från `1` till
`0`. Gruppfälten `$2944` och `$2948` ändras samtidigt från `1` till `0`.
Före- och efterbilderna av 8 KiB huvud-RAM har MD5
`2449d5b14c41565a9d6c71c7c61f481d` respektive
`4072c735edbe4d60182870876a8ceb79`.

Ett längre klick-/kontrollpar från samma startpunkt skiljer 27 226 presenterade
bildpunkter inom `(0,0)..(271,175)`. Klickets VRAM-MD5 är
`01ec4386a553b0382c737c593f8dc04d`; kontrollens är
`a44656d752b2910f48944831eaf23d61`.

## Högerrotation

Den högra rörelsepanelsknappen köar kommandotyp `$02` vid intern koordinat
`$98/$8F`. Samma originalrutin ändrar `$203F`, `$2944` och `$2948` från `1`
till `2`. Den tidigare synliga klick-/kontrollfångsten skiljer 27 430
bildpunkter.

## Produktionsgräns

Firestaff mappar endast originalkommandona `$01` och `$02` till vänster
respektive höger kvartsrotation. Övriga kommandotyper avvisas. Fångster vid
2 097 152 VDC-poster används som presentationsbevis men tas inte upp i den
atomiska VRAM-produktionslistan, eftersom CPU-portspåret korsar HuC6270:s
interna DMA. Den äldre rena 65 536-postersgränsen och kommando-RAM-beviset
förblir separata grindar.
