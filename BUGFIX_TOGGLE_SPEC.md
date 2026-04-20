# Firestaff — Bug/Change Toggle Spec

**Status:** Spec (post-M10 implementation).
**Skapad:** 2026-04-20
**Källkanon:** http://dmweb.free.fr/Stuff/ReDMCSB/Documentation/BugsAndChanges.htm
**Primär databas:** Fontanels `BUGX_YY` och `CHANGEX_YY_KATEGORI` taggar i ReDMCSB-källkoden

---

## 1. Översikt

Firestaff låter användaren välja vilken historisk version av Dungeon Master / Chaos Strikes Back de vill spela — ner till individuella buggar och ändringar. Fontanels egen dokumentation ger oss 93 buggar + 108 ändringar spridda över 9 game-versioner. Vi bygger en UI som respekterar hans taxonomi och låter spelare mixa fritt.

---

## 2. Datakälla och struktur

### 2.1 Fontanels taxonomi

**Versions-koder (0-8):**

| Kod | Version                  |
|-----|--------------------------|
| 0   | DM Atari ST 1.0a EN      |
| 1   | DM Atari ST 1.0b EN      |
| 2   | DM Atari ST 1.1 EN       |
| 3   | DM Atari ST 1.2 EN       |
| 4   | DM Atari ST 1.2 GE       |
| 5   | DM Atari ST 1.3a FR      |
| 6   | DM Atari ST 1.3b FR      |
| 7   | CSB Atari ST 2.0 EN      |
| 8   | CSB Atari ST 2.1 EN      |

**Notera:** PC 3.4 (vår baseline) är inte numrerad — Fontanels WIP-kod dokumenterar Atari ST-historien. Vi får själva fastställa vilka Atari-fixes som är inbakade i PC 3.4 och vilka som inte är det.

**Bug-ID:** `BUGX_YY` — X = version där buggen introducerades, YY = löpnummer
**Change-ID:** `CHANGEX_YY_KATEGORI` — X = version, YY = löpnummer, KATEGORI ∈ {FIX, OPTIMIZATION, LOCALIZATION, IMPROVEMENT}

### 2.2 Totala antal (per Fontanel)

- **Bugs: 93**
  - Introducerade i 1.0a EN: 89
  - Introducerade i 1.1 EN: 1
  - Introducerade i 1.3a FR: 1
  - Introducerade i CSB 2.0 EN: 2
- **Changes: 108**
  - 1.0b EN: 3
  - 1.1 EN: 21
  - 1.2 EN: 21
  - 1.2 GE: 1
  - 1.3a FR: 7
  - 1.3b FR: 1
  - CSB 2.0 EN: 40
  - CSB 2.1 EN: 14

Totalt **201 individuella togglar**.

### 2.3 Datamodell (JSON/YAML)

```json
{
  "id": "BUG0_02",
  "type": "bug",                           // "bug" | "change"
  "symptom": "After 850-1000 hours the game hangs",
  "cause": "24-bit event-time overflow in timeline",
  "affected_versions": [0, 1, 2, 3, 4, 5, 6, 7, 8],
  "affected_files": ["GAMELOOP.C"],
  "resolution": "none",                    // "none" | "fixed_in:<change_id>" | "partial"
  "fixed_by_change": null,                 // CHANGEX_YY_KATEGORI if applicable
  "category": null,                        // FIX/OPTIMIZATION/LOCALIZATION/IMPROVEMENT for changes
  "default_state": "original",             // "original" | "fixed" (per-profile override)
  "requires_fontanel_review": false,
  "our_implementation_status": "implemented | deferred | na",
  "firestaff_flag_id": 2,                  // bit-index in bug_flags_compat mask
  "applicable_games": ["DM1", "CSB"]       // gate — hides toggle in irrelevant games
}
```

Databasens fullständiga omfång är ~201 poster × 11 fält. Initial scrape från dmweb.free.fr, sedan kurerat manuellt och versionshanterat i projektrepo.

---

## 3. UI-val

### 3.1 Version-presets (en-klick)

Snabbval för vanliga spelsätt:

| Preset                        | Beskrivning                                                                 | Default för nybörjare? |
|-------------------------------|-----------------------------------------------------------------------------|------------------------|
| **DM 1.0a EN (purist)**       | Original 1987. Alla 89 introduktionsbuggar PÅ. Inga changes.                 | ❌                     |
| **DM 1.0b EN**                | Efter 3 tidiga fixar.                                                       | ❌                     |
| **DM 1.1 EN**                 | 1988 — 21 changes + BUG2_00 introducerad.                                    | ❌                     |
| **DM 1.2 EN (senaste EN)**    | Engelska slutversionen för Atari.                                           | ❌                     |
| **DM 1.3b FR (senaste DM)**   | Alla Atari-fixes tom 1.3b. Franska lokaliserad.                              | ❌                     |
| **CSB 2.1 EN (senaste CSB)**  | Alla Atari-fixes tom CSB 2.1.                                               | ❌                     |
| **PC 3.4 baseline**           | Vad Fontanels WIP-kod faktiskt implementerar. **Vårt rekommenderade default.** | ✅ DM1                  |
| **Modern**                    | Alla FIX + utvalda IMPROVEMENT changes. QoL-fokus.                          | ❌ (opt-in)             |
| **Custom**                    | Användarens egen mix.                                                       | —                      |

**Implementation:** varje preset är en JSON-fil med en lista av `{id, state}`-par. Vid val → UI uppdaterar alla togglar. Användaren kan sedan modifiera och spara som Custom.

### 3.2 Custom mix (advanced)

**Full kontroll:** individuella togglar för varje bug/change (~201 stycken).

**Gruppering i UI:**
- Bugs (93) — expanderbar sektion
  - Per affected-version (1.0a: 89, 1.1: 1, 1.3a: 1, CSB 2.0: 2)
- Changes (108) — expanderbar sektion
  - Per kategori:
    - FIX (~60)
    - OPTIMIZATION (~20)
    - LOCALIZATION (~15)
    - IMPROVEMENT (~30) ← troligen mest intressant för användaren
  - Per version

**Per-post UI (tooltip + expand):**
- ID + symptom (one-liner)
- Cause (full förklaring)
- Affected versions
- Resolution
- Affected files (för nördarna)
- Impact-bedömning: "Silent/Visual/Gameplay/Breaking"
- Recommended state: "original" eller "fixed"

**Sökfilter:**
- "Visa bara buggar som påverkar vald version"
- "Visa bara changes som INTE är i min version"
- "Sök fritextsymtom"
- "Sök filnamn" (för kodarkeologi)

### 3.3 Changes är också togglable (inte bara buggar)

Kritisk insikt: Fontanels changes är inte alla fixes. Exempel:

- **CHANGE3_14_IMPROVEMENT** — eventuell gameplay-tweak, kanske vill spelaren INTE ha den
- **CHANGE4_00_LOCALIZATION** — tyska textändringar från DM 1.2 GE. Vill en EN-spelare ha dem? Nej.
- **CHANGE7_01_FIX** — fixar BUG0_03 (graphical glitch). Självklart PÅ i default men togglable för research.
- **CHANGE7_14_FIX** — del av BUG0_00 (useless code cleanup). Ingen speleffekt men ändrar binary.

**UI-logik:**
- FIX-changes: default PÅ, men kan slås av för att återskapa originalbugg
- IMPROVEMENT-changes: default PÅ om de matchar vald preset, annars AV
- LOCALIZATION-changes: default PÅ endast för matchande språk
- OPTIMIZATION-changes: default PÅ (inga spelsemantikändringar)

### 3.4 Gate per spel

**Problem:** DM1-specifika buggar ska inte synas när spelaren valt CSB eller DM2. Motsatsen gäller också.

**Lösning:** varje post har `applicable_games: ["DM1", "CSB", "DM2"]`. UI filtrerar listan baserat på vilket spel som är valt i startmenyn.

**Mappning:**
- `BUG0_*` → DM1-specifika om `affected_versions ⊆ {0-6}`
- `BUG0_*` → både DM1 och CSB om `affected_versions` inkluderar {7, 8}
- `BUG7_*` → CSB-specifika (introducerade i CSB 2.0)
- `CHANGE7_*` → CSB-specifika changes
- DM2 får egen bug/change-lista när vi portar DM2 (troligen M12+)

**UX:** när spelaren byter spel i startmenyn → bug-listan uppdateras, custom-valet bevaras för varje spel separat.

---

## 4. Community-värde

### 4.1 Publicera databasen

Bug/change-databasen (`firestaff-bugs.json` eller YAML) publiceras som:
- Separat GitHub-repo: `firestaff/dm-bug-database`
- Licens: CC-BY 4.0 (eller motsvarande preservation-vänlig)
- Versioned: schemaversioning för framtida utbyggnad

### 4.2 Återanvändning

Andra DM-portar kan konsumera databasen:
- **ScummVM** (har delvis DM-stöd)
- **Dungeon Master Java** (dmjava)
- **RTC (Return to Chaos)** — Fontanels tidiga projekt
- **Dungeon Master .NET** om den finns
- Custom dungeon editors

### 4.3 Credit

Varje post i databasen har:

```json
{
  "data_source": "Based on ReDMCSB documentation by Christophe Fontanel",
  "data_source_url": "http://dmweb.free.fr/Stuff/ReDMCSB/Documentation/BugsAndChanges.htm",
  "scraped_at": "2026-04-20",
  "contributors": []
}
```

Fontanel får full erkännande. Vår värdeskapning = strukturerad JSON + toggle-UI + mapping till implementation.

### 4.4 Framtida utbyggnad (pull requests välkomna)

- **Video/GIF-demos** per bug (hur reproducerar man BUG0_02? visa gameplay)
- **Replay-filer** som demonstrerar buggen
- **Community-buggar** som Fontanel inte dokumenterat
- **PC 3.4-specifika buggar** som inte finns i Atari-versionerna

---

## 5. Teknisk implementation (post-M10)

### 5.1 Firestaff-sidan

**Modul:** `bug_flags_compat`
- Bit-mask: minst 256 bits (för framtidsskydd; 201 nu + headroom)
- Per-spel separata masks (DM1, CSB, DM2 har egen)
- Serialiseras i `GameConfig_Compat` (Phase 20-struktur) och i save-filer
- Varje kod-site som beror på en flagga:

```c
if (BUG_ACTIVE(game, BUG0_02)) {
    // Original buggy behavior — 24-bit overflow
    scheduled_time = (game_time + delay) & 0xFFFFFF;
} else {
    // Fixed — use full 32-bit time
    scheduled_time = game_time + delay;
}
```

### 5.2 Save-fil-integration

Save-filer markerar vilken profil som användes:
- Phase 15 save-blob utökas med ett `ProfileHash_Compat`-fält
- Vid load: jämför load-tid-profil med nuvarande profil
- Om mismatch → warning popup: "Din save skapades med profil X, aktuell profil är Y. Spela ändå?"
- Optionally: låt spelaren "snap-to" save-profilen automatiskt

### 5.3 Replay-integration (TODO top-3 replay-system)

Replay-determinism kräver identiska flaggor som recording:
- `TickStreamRecord_Compat` innehåller profile-hash
- Replay-verifier avvisar replays med profil-mismatch
- Kör bit-identiskt samma path:

```c
// Samma flag-set → samma world-hash efter N ticks → replay verified
assert(crc32(world_after_N_ticks) == replay_expected_hash);
```

### 5.4 Startmenyns UI-flöde

1. Spelare öppnar startmenyn
2. Väljer spel (DM1/CSB/DM2)
3. Under "Game version": dropdown med presets
4. Default för nya spelare: "PC 3.4 baseline"
5. Advanced-knapp → öppnar Custom-mix dialog
6. Spara som egen preset → lägger till i dropdown
7. När spelare startar new game: profilen fryses i save-filen

---

## 6. Prioritering

### Must-have (V2)
- [ ] Scrape dmweb.free.fr → JSON-databas med ~201 poster
- [ ] `bug_flags_compat` modul
- [ ] 5 version-presets
- [ ] Gate per spel
- [ ] Save-fil profile-hash
- [ ] UI: dropdown med presets

### Should-have (V2.1)
- [ ] Custom mix UI
- [ ] Sökfilter
- [ ] Tooltips med full info
- [ ] Save-file profile-mismatch warning

### Nice-to-have (V3+)
- [ ] Publicera som community-repo
- [ ] Video-demos per bug
- [ ] Replay-filer som demonstrerar buggar
- [ ] Community contribution workflow

---

## 7. Open questions (NEEDS DECISION)

1. **Vad är default för en ny spelare?** Rekommenderar "PC 3.4 baseline" = spel som Fontanels WIP faktiskt implementerar, eftersom det är den mest kompletta och testade versionen. Purister kan välja 1.0a EN.

2. **Ska vi inkludera DM2 nu eller vänta?** DM2 är utanför ReDMCSB-dokumentationen. Vi behöver separat forskning + egen databas. Vänta till DM2-portningsfasen (M12+).

3. **Vilken licens för bug-databasen?** CC-BY 4.0 är standard för preservation-data. Danne beslutar.

4. **Ska changes vara togglable by default, eller endast för "advanced users"?** Förslag: i UI — dölj changes bakom "Advanced" expand-knapp. Visa bara buggar i default-vy eftersom de är mer intuitiva för spelaren att förstå.

5. **Hur gör vi med PC 3.4-specifika buggar/changes som inte finns i Atari-docs?** Eget numreringschema `BUGPC_YY`? Eller `BUG10_YY` (fortsätter sekvensen)? Fråga Fontanel om råd.
