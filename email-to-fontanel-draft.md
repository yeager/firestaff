# Draft, message to Christophe Fontanel

**Status:** Draft. To be sent manually by Daniel via DM Forum PM, Discord, or email if an address is found.
**Goal:** Get explicit permission to consult ReDMCSB as a reference source for Firestaff, and optionally encourage an explicit licence on ReDMCSB itself.
**Tone:** Respectful, factual, brief. Fontanel is a precision-oriented reverse engineer, so brevity and clean questions matter.

---

## Subject

Permission request, deriving an open-source Dungeon Master port from ReDMCSB

## Body (EN)

Hi Christophe,

Your ReDMCSB project is extraordinary. The care you have put into reverse engineering so many versions of Dungeon Master and Chaos Strikes Back, and the depth of your `BUGX_YY` / `CHANGEX_YY` documentation, represents decades of work that might otherwise have been lost. Thank you.

I am working on an open-source port of DM/CSB called **Firestaff**. The goal is preservation: a deterministic, modular engine that reads original player-owned DUNGEON.DAT files and makes the games playable on modern systems (Linux, macOS, Windows). No commercial distribution, no asset redistribution, just the engine.

The implementation is original code. I use ReDMCSB as reference documentation, write my own implementation in a different style, and do not copy code verbatim. Firestaff is structured around pure data-layer modules and invariant-based probes.

If you have the time, I would like to ask two things:

1. **Permission confirmation.** Is it acceptable to you that Firestaff uses ReDMCSB as a reverse-engineering reference, without redistributing your source code? Your work will be credited prominently in Firestaff's documentation, and the final project will link back to dmweb.free.fr.

2. **Explicit licence.** ReDMCSB currently ships without an explicit licence. If you would consider adding one, even a simple permissive licence, it would make the legal situation much clearer for downstream preservation projects.

A separate technical question: I have structured the contents of `BugsAndChanges.htm` into a JSON database for use in Firestaff's per-bug toggle system. Would you be comfortable with that JSON being published as a community resource, with full credit to you and a link back to your documentation?

I am happy to answer any questions about Firestaff's architecture or scope.

Thank you again for everything you have already given this community.

Best regards,
Daniel Nylander
Stockholm, Sweden
daniel@danielnylander.se

---

## Body (FR, if French is preferred)

Bonjour Christophe,

Votre projet ReDMCSB est extraordinaire. Le soin que vous avez apporté à la rétro-ingénierie de nombreuses versions de Dungeon Master et Chaos Strikes Back, ainsi que la richesse de votre documentation `BUGX_YY` / `CHANGEX_YY`, représentent des décennies de travail qui auraient autrement pu être perdues. Merci.

Je travaille sur un port open-source de DM/CSB appelé **Firestaff**. L'objectif est la préservation : un moteur déterministe et modulaire qui lit les fichiers DUNGEON.DAT originaux appartenant au joueur et rend les jeux jouables sur des systèmes modernes (Linux, macOS, Windows). Aucune distribution commerciale, aucune redistribution d'assets, uniquement le moteur.

L'implémentation est du code original. J'utilise ReDMCSB comme documentation de référence, j'écris ma propre implémentation dans un style différent, et je ne copie pas le code verbatim. Firestaff est structuré autour de modules de couche de données purs et de sondes basées sur des invariants.

Si vous avez un moment, j'aimerais vous demander deux choses :

1. **Confirmation de permission.** Est-il acceptable pour vous que Firestaff utilise ReDMCSB comme référence de rétro-ingénierie, sans redistribuer votre code source ? Votre travail sera crédité de manière visible dans la documentation de Firestaff, et le projet final renverra vers dmweb.free.fr.

2. **Licence explicite.** ReDMCSB est actuellement distribué sans licence explicite. Si vous envisagiez d'en ajouter une, même simple et permissive, cela clarifierait beaucoup la situation juridique pour les projets de préservation qui s'appuient sur votre travail.

Question technique distincte : j'ai structuré le contenu de `BugsAndChanges.htm` dans une base JSON destinée au système de toggles par bug de Firestaff. Seriez-vous d'accord pour que ce JSON soit publié comme ressource communautaire, avec attribution complète et lien vers votre documentation ?

Je serais heureux de répondre à toute question sur l'architecture ou la portée de Firestaff.

Merci encore pour tout ce que vous avez déjà apporté à cette communauté.

Cordialement,
Daniel Nylander
Stockholm, Suède
daniel@danielnylander.se

---

## Sending options

### Option 1, DM Forum PM (recommended)
1. Go to https://www.dungeon-master.com/forum/viewtopic.php?t=29805
2. Find Fontanel's forum username
3. Open a private message
4. Paste the message

### Option 2, DM Discord
1. Go to https://discord.gg/5XZkPvx
2. Introduce yourself in a general or welcome channel
3. Ask whether Fontanel is active there
4. If yes, contact him directly

### Option 3, email via forum profile
1. Register a forum account
2. Find Fontanel's profile
3. Use the "email user" action if it is enabled

## After a reply

- **Positive + explicit licence**: update Firestaff docs accordingly and consider publishing the bug database as a community resource
- **Positive + no licence**: keep prominent credit, but be careful not to present the derived bug database as a redistribution of his IP
- **No reply after 4 weeks**: continue development based on reverse-engineering fair-use assumptions, while keeping credit intact
- **Negative reply**: respect it, and rely on other public sources instead
