# Utkast: mail till Christophe Fontanel

**Status:** Draft. Skickas manuellt av Danne via DM Forum PM, Discord eller email (när adress hittas).
**Mål:** Få explicit tillstånd att konsultera ReDMCSB-källkoden för vår port + eventuellt uppmuntra till öppen licens på ReDMCSB själv.
**Ton:** Respektfull, saklig, kort. Fontanel är en precision-orienterad reverse-engineer — värdesätter brevity och tydliga frågor.

---

## Subject

Permission request — deriving an open-source Dungeon Master port from ReDMCSB

## Body (EN)

Hi Christophe,

Your ReDMCSB project is extraordinary. The care you've put into reverse engineering 36 versions of Dungeon Master and Chaos Strikes Back, and the depth of your `BUGX_YY` / `CHANGEX_YY` documentation, represents decades of work that would otherwise be lost. Thank you.

I'm working on an open-source port of DM/CSB called **Firestaff** (named after the game's central artefact). My aim is preservation: a deterministic, modular engine that reads original player-owned DUNGEON.DAT files and makes the games playable on modern systems (Linux, macOS, Windows). No commercial distribution, no assets redistributed — just the engine.

The architecture is purely original code: I study ReDMCSB as reference documentation, write my own implementation in a different style (pure data-layer modules with invariant-based probes), and never copy code verbatim. Current status: 18 of 20 milestones passing with full bit-identical round-trip serialisation, driven by your insights into Fontanel's MEDIA240/PC 3.4 structures.

Two requests, if you have the time:

1. **Permission confirmation.** I'd like to confirm that using ReDMCSB as a *reference for reverse engineering* — without redistributing your source — is acceptable to you. Your work is credited prominently in Firestaff's documentation ("Based on reverse engineering research by Christophe Fontanel"), and the final product will link back to dmweb.free.fr.

2. **Explicit licence.** ReDMCSB currently ships without an explicit licence. If you'd consider adding one — even a permissive one like CC-BY or MIT — it would dramatically clarify what downstream projects can do with it. Clones like Return to Chaos, DMNet, Dungeon Master Java, and my own Firestaff would all benefit.

A separate technical question: I've scraped your `BugsAndChanges.htm` into a structured JSON database (201 entries) for use in a per-bug toggle system in Firestaff's startup menu — letting players choose which historical version of DM/CSB they want to play, down to individual bugs. Would you be open to that JSON being published as a community resource (CC-BY, fully credited to you) so other clones can consume it too?

I'm happy to answer any questions about Firestaff's architecture or scope. The code is at (will share link when repo goes public), and I can share early access if you're curious.

Thanks for everything you've already given this community.

Best regards,
Daniel Nylander
Stockholm, Sweden
daniel@danielnylander.se

---

## Body (FR — om Fontanel föredrar franska; han är fransktalande enligt 1.3a FR-historiken)

Bonjour Christophe,

Votre projet ReDMCSB est extraordinaire. Le soin que vous avez apporté à la rétro-ingénierie de 36 versions de Dungeon Master et Chaos Strikes Back, et la profondeur de votre documentation `BUGX_YY` / `CHANGEX_YY`, représentent des décennies de travail qui seraient autrement perdues. Merci.

Je travaille sur un port open-source de DM/CSB appelé **Firestaff** (nommé d'après l'artefact central du jeu). Mon objectif est la préservation : un moteur déterministe et modulaire qui lit les fichiers DUNGEON.DAT originaux du joueur et rend les jeux jouables sur les systèmes modernes (Linux, macOS, Windows). Pas de distribution commerciale, pas d'assets redistribués — juste le moteur.

L'architecture est du code purement original : j'étudie ReDMCSB comme documentation de référence, j'écris ma propre implémentation dans un style différent (modules de couche de données purs avec sondes basées sur des invariants), et je ne copie jamais de code verbatim. État actuel : 18 des 20 jalons passent avec une sérialisation round-trip bit-identique complète, guidée par vos insights sur les structures MEDIA240/PC 3.4 de Fontanel.

Deux demandes, si vous avez le temps :

1. **Confirmation de permission.** Je souhaiterais confirmer que l'utilisation de ReDMCSB comme *référence pour la rétro-ingénierie* — sans redistribuer votre source — est acceptable pour vous. Votre travail est crédité de manière proéminente dans la documentation de Firestaff ("Basé sur les recherches de rétro-ingénierie de Christophe Fontanel"), et le produit final pointera vers dmweb.free.fr.

2. **Licence explicite.** ReDMCSB est actuellement distribué sans licence explicite. Si vous envisageriez d'en ajouter une — même permissive comme CC-BY ou MIT — cela clarifierait considérablement ce que les projets en aval peuvent en faire. Les clones comme Return to Chaos, DMNet, Dungeon Master Java, et mon propre Firestaff en bénéficieraient tous.

Une question technique séparée : j'ai scrapé votre `BugsAndChanges.htm` dans une base de données JSON structurée (201 entrées) pour l'utiliser dans un système de toggle par bug dans le menu de démarrage de Firestaff — permettant aux joueurs de choisir quelle version historique de DM/CSB ils veulent jouer, jusqu'aux bugs individuels. Seriez-vous ouvert à ce que ce JSON soit publié comme ressource communautaire (CC-BY, entièrement crédité à vous) afin que d'autres clones puissent également le consommer ?

Je serais heureux de répondre à toute question sur l'architecture ou la portée de Firestaff. Le code est sur (je partagerai le lien quand le repo deviendra public), et je peux partager un accès anticipé si vous êtes curieux.

Merci pour tout ce que vous avez déjà donné à cette communauté.

Cordialement,
Daniel Nylander
Stockholm, Suède
daniel@danielnylander.se

---

## Skicka-via-alternativ

### Alt 1: DM Forum PM (rekommenderas)
1. Gå till https://www.dungeon-master.com/forum/viewtopic.php?t=29805 (ReDMCSB-tråden)
2. Hitta Fontanels forum-användarnamn (sannolikt `Christophe Fontanel` eller `cfontanel`)
3. Klicka på användarnamnet → PM
4. Klistra in meddelandet

### Alt 2: DM Discord
1. Gå till https://discord.gg/5XZkPvx
2. Introducera dig i general/welcome
3. Fråga moderatorer om Fontanel är aktiv där
4. Om ja: DM direkt

### Alt 3: Email via forum-profil
1. Registrera forum-konto (gratis)
2. Hitta Fontanels profil
3. Klicka "email user" om det är aktiverat

### Alt 4: Kommentar på dmweb.free.fr (ingen sån feature verkar finnas)

## Efter svar

Oavsett svar:
- **Positivt + licens**: Uppdatera Firestaff-repo med licens-referens. Publicera bug-databasen som community-resurs.
- **Positivt + ingen licens**: Behåll "Based on research by Christophe Fontanel" i credits, publicera INTE bug-databasen i en form som kan tolkas som re-distribution av hans IP.
- **Inget svar inom 4 veckor**: Fortsätt utveckling baserat på fair-use för reverse engineering. Credit bevaras oavsett.
- **Negativt**: Respektera. Vi bygger från Doug Bell's Quora-intervju + andra offentliga källor istället. Förlorar några procents djup men projektet står.
