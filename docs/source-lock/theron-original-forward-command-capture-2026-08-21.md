# Therons ursprungliga rörelsekommandon

## Autentisk källa

Fångsten `work/theron-forward-command-complete-v1` kommer från USA-utgåvans
äkta Track 02 (MD5 `ceb02343868f80cec899e9b239aff2da`), System Card 3.0
(MD5 `ff1a674273fe3540ccef576376407d1d`) och savestate
`f17f377df210b4a3ae904a13fb85a7f0`. Indatan är
`right@1:140,down@2:33,i@145:5`; ingen genererad speldata används.

Button I vid intern klickkoordinat `$8A/$8F` köar kommandotyp `$03`. Spåret
innehåller exakt 65 536 ordnade huvud-RAM-skrivningar. Dess MD5 är
`cae8749574e4668c62b71b131fe62e74`; det autentiska 64 KiB-kodfönstrets MD5
är `77d2253b4aed117847d8d1d685506ca0`.

## Originalets rörelse- och positionskedja

Ködispatchen vid `$D3B0..$D3CB` skickar kommandotyper `$03..$06` till
`$CD87`. För den fångade `$03`-händelsen läser rutinen aktuell position ur
`$40/$41`, räknar fram destinationsrutan och för den godkända förflyttningen
vidare destinationen `$03/$03` genom `$45/$46`.

Den permanenta positionscommitten sker vid `$C1FA`:

```asm
tii $20B4,$2040,$0002
```

Skrivspåret registrerar båda destinationsbytena vid logisk PC `$C203` och
fysisk PC `$0DC203`. Startpositionen är `$40/$41 = $02/$03`; efter committen
är den `$03/$03`. RAM-bilden före kommandot har MD5
`86f7cec0a943402daae0ec0acdf7a372`, och bilden efter att kön nollställs genom
`$2905=$00` har MD5 `eae73a9991630a6fe2bcc48c2c99861c`.

## Bakåt- och sidokommandon

Tre ytterligare fångster från samma autentiska startläge sluter panelens
fyra rörelsekommandon:

- `$98/$A5` köar `$04`, sidsteg höger.
- `$8A/$A5` köar `$05`, bakåt.
- `$7B/$A5` köar `$06`, sidsteg vänster.

Originalrutinen beräknar rörelseriktningen som
`(kommandotyp - 3 + $3F) & 3`, där `$3F` är blickriktningen. `$05` committar
position `$02/$03 → $01/$03` genom samma `$C1FA`-väg; kommandospårets MD5 är
`0488166dec2eaec19b8874b01f7f1fba`.

Från detta startläge är båda sidorutorna blockerade. `$04`- och `$06`-spåren
saknar därför skrivningar till `$2040/$2041`; kommandokön nollställs i stället
vid `$CC41`. Spårens MD5 är `009168a72f01bf4e8bb6095195c880cf`
respektive `ff097d6bdd2345e21ca40baa6e3fa489`.

## Produktionsgräns

Firestaff mappar `$03..$06` till samma relativa riktningar och använder den
redan laddade Theron-världens riktiga Track 02-rutor och objekt för passering.
Bakåt- och sidosteg bevarar blickriktningen. Fångsterna bevisar en lyckad
framåt- och bakåtförflyttning samt originalets blockerade sidoväg; vidare
specialrutor förblir under sina egna befintliga realdatagrindar.
