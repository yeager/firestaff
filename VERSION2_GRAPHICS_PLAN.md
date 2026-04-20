# ReDMCSB Version 2, grafikplan

## Mål

Bygg en **fristående, lagligt distribuerbar version** med:
- ny högupplöst grafik
- förbättrat ljud
- inget beroende av originalspelets datafiler
- samma övergripande spelstruktur och känsla, men inte asset-kopior

## Grundprincip

Version 2 ska **inte** vara:
- AI-upskalad originalgrafik
- paint-over direkt ovanpå originalassets
- juridiskt grå “förbättrad originaldump”

Version 2 ska vara:
- **nyritad presentation**
- egen asset-pipeline
- tydlig separation mellan spel-logik och presentation

## Rekommenderad visuell riktning

### Stil
- mörk fantasy
- tydliga silhuetter
- renare UI än originalet
- handmålad 2D-känsla, inte fotorealism
- hög kontrast mellan interaktiva element och bakgrund

### Upplösning
Arbeta i en **hög master-upplösning** från början.

Rekommendation:
- master canvas för menyer och scener: **1920x1080**
- spelbara UI-paneler byggs modulärt
- exportera sedan till mindre targets vid behov

### Renderingstil
Två säkra spår, välj ett tidigt:

1. **Painterly 2D**
   - bakgrunder målade
   - figurer/objekt som rena sprites
   - bäst för snabbast väg till snygg V2

2. **HD pixel-inspired**
   - inte ren pixelart, men pixelinfluerad formgivning
   - skarpa former, begränsad palett, moderna effekter
   - svårare att få riktigt elegant

**Rekommendation: painterly 2D.**

## Asset-kategorier

### 1. UI och menyer
- titelbild
- huvudmeny
- undermenyer
- knappar
- highlight/selection-states
- paneler, ramar, ornament
- typsnitt/typografisystem

### 2. Ikoner och symboler
- inventory/verb/action-symboler
- statusikoner
- små UI-markörer
- cursor/highlight-indikatorer

### 3. Scenbakgrunder
- rum
- korridorer
- vyer
- övergångsbilder
- loading/splash/intro-bakgrunder

### 4. Interaktiva objekt
- dörrar
- kistor
- nyckelobjekt
- spell/ritual-objekt
- andra klickbara eller fokusbara element

### 5. Karaktärsrelaterat
- porträtt
- figurposer
- eventuella ansiktsuttryck
- dialogkopplade presentationselement

### 6. Effekter
- highlight-glow
- övergångar/fade
- aktiveringsflash
- partiklar, dimma, magiska effekter

## Teknisk struktur

## Separera logik och presentation

Version 1 lär oss struktur.
Version 2 ska återanvända så mycket logik som möjligt, men peka på en ny assetkälla.

### Rekommenderad modell
- **game logic layer**
  - menystruktur
  - state transitions
  - interaktionsregler
- **presentation layer**
  - vilka bilder som visas
  - layout
  - effekter
  - ljudkopplingar
- **asset manifest layer**
  - mappar logiska IDs till V2-assets

### Exempel
Istället för:
- "load graphic index 13"

använd:
- `menu.main.title`
- `menu.main.option.new_game.highlighted`
- `screen.submenu.inventory.background`

Det gör V2 ren, utbytbar och mycket enklare att underhålla.

## Filformat

Rekommendation:
- **PNG** för 2D-assets med alpha
- **WebP lossless** kan övervägas senare för distribution
- **SVG** för vissa UI-element där det passar
- **JSON** eller **YAML** för asset manifests

## Mappstruktur

Förslag:

```text
assets-v2/
  ui/
    title/
    menu/
    panels/
    fonts/
  icons/
  backgrounds/
    rooms/
    transitions/
  objects/
  portraits/
  effects/
  audio/
  manifests/
```

## Produktionspipeline

### Fas 1, art direction
- skapa moodboard
- definiera färgpaletter
- välj typografi
- gör 2 till 3 stilframes
- besluta exakt V2-look innan massproduktion

### Fas 2, designsystem
- definiera UI-grid
- definiera knappstater
- definiera highlight/activate-beteenden visuellt
- definiera standardstorlekar för paneler, ikoner, marginaler

### Fas 3, vertical slice
Bygg först en liten spelbar slice:
- titelbild
- huvudmeny
- en submenu
- en enkel scen
- ett activate-resultat

Om den ser bra ut, då först skalar vi.

### Fas 4, assetproduktion
- bakgrunder
- UI-paket
- interaktiva objekt
- effekter
- ljud

### Fas 5, integration
- koppla assets till manifest
- koppla manifest till renderlager
- testa state transitions, hover/highlight/activate

## AI-användning, ja men smart

AI är bra för:
- thumbnails
- moodboards
- variationsidéer
- kompositionsförslag
- färgutforskning

AI är dåligt som slutpipeline för V2 om målet är kvalitet och tydlig juridik.

**Rekommendation:**
- använd AI i preproduction
- människa-kurera och bygg slutassets rent
- spara prompts och ursprung för spårbarhet

## Juridisk hygien

För att hålla V2 ren:
- använd inte dumpade originalassets som slutmaterial
- undvik direkt paint-over av originalbilder
- undvik “för nära” rekonstruktion där originalbilden i praktiken bara är omritad
- håll designinspiriation på systemnivå, inte pixel-för-pixel

## Praktisk nästa ordning

1. Skriv ett **asset manifest-schema** för V2
2. Lista alla första V2-asset-ID:n för:
   - titel
   - huvudmeny
   - highlight states
   - activate states
   - första submenu
3. Gör en **art bible** på 1 sida
4. Producera en **vertical slice**
5. Koppla den till nuvarande meny-state/render/activate-seams

## Min rekommendation

Den bästa vägen är:
- använd Version 1 för att få fram **ren spelstruktur**
- bygg Version 2 som ett **nyillustrerat presentationslager** ovanpå den strukturen
- börja med **titel + huvudmeny + en submenu**
- vänta med 3D tills 2D-versionen känns genomarbetad och snygg

Det minskar både risk, juridiskt kladd och fulhack.
