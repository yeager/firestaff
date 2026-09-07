#!/usr/bin/env python3
"""Apply reviewed DE/ES/FR terminology after ``msgmerge``.

The game templates are regenerated frequently.  Keeping the small, reviewed
terminology layer here means a merge cannot silently replace it with a fuzzy
guess.  Entries not present in this table deliberately stay untranslated and
fall back to the selected game's original text; do not fill them with a
different game's wording.
"""
import argparse
from pathlib import Path

ROOT = Path(__file__).resolve().parent

# DM2's authenticated GDAT vocabulary shared by the HUD, inventory and spell
# UI.  Control byte \x01 and format tokens are intentionally retained.
DM2 = {
    "de": {
        "ADEPT \x011": "ADEPT \x011", "ANTI-FIRE": "FEUERSCHUTZ",
        "ANTI-MAGIC": "MAGIESCHUTZ", "APPLE": "APFEL",
        "APPRENTICE \x011": "LEHRLING \x011", "ARCHER-GUARD": "BOGENWACHE",
        "ARCHMASTER \x011": "ERZMEISTER \x011", "ARROW": "PFEIL",
        "AXE": "AXT", "BAG": "BEUTEL", "BARREL": "FASS",
        "BOW": "BOGEN", "CAST": "ZAUBERN", "CHEST": "TRUHE",
        "CLOSE": "SCHLIESSEN", "COIN": "MÜNZE", "DAGGER": "DOLCH",
        "DOOR": "TÜR", "DRINK": "TRINKEN", "DROP": "ABLEGEN",
        "EAT": "ESSEN", "FIREBALL": "FEUERBALL", "FLASK": "FLASCHE",
        "FOOD": "NAHRUNG", "GOLD": "GOLD", "HEALTH": "GESUNDHEIT",
        "KEY": "SCHLÜSSEL", "LOCK": "SCHLOSS", "MANA": "MANA",
        "OPEN": "ÖFFNEN", "POTION": "TRANK", "SAVE GAME": "SPIEL SPEICHERN",
        "LOAD GAME": "SPIEL LADEN", "SHIELD": "SCHILD", "SWORD": "SCHWERT",
        "THROW": "WERFEN", "TORCH": "FACKEL", "USE": "BENUTZEN",
        "WAND": "ZAUBERSTAB", "WATER": "WASSER",
    },
    "fr": {
        "ADEPT \x011": "ADEPTE \x011", "ANTI-FIRE": "ANTI-FEU",
        "ANTI-MAGIC": "ANTI-MAGIE", "APPLE": "POMME",
        "APPRENTICE \x011": "APPRENTI \x011", "ARCHER-GUARD": "GARDE ARCHER",
        "ARCHMASTER \x011": "ARCHIMAÎTRE \x011", "ARROW": "FLÈCHE",
        "AXE": "HACHE", "BAG": "SAC", "BARREL": "TONNEAU",
        "BOW": "ARC", "CAST": "LANCER", "CHEST": "COFFRE",
        "CLOSE": "FERMER", "COIN": "PIÈCE", "DAGGER": "DAGUE",
        "DOOR": "PORTE", "DRINK": "BOIRE", "DROP": "DÉPOSER",
        "EAT": "MANGER", "FIREBALL": "BOULE DE FEU", "FLASK": "FIOLE",
        "FOOD": "NOURRITURE", "GOLD": "OR", "HEALTH": "SANTÉ",
        "KEY": "CLÉ", "LOCK": "SERRURE", "MANA": "MANA",
        "OPEN": "OUVRIR", "POTION": "POTION", "SAVE GAME": "SAUVEGARDER",
        "LOAD GAME": "CHARGER", "SHIELD": "BOUCLIER", "SWORD": "ÉPÉE",
        "THROW": "LANCER", "TORCH": "TORCHE", "USE": "UTILISER",
        "WAND": "BAGUETTE", "WATER": "EAU",
    },
    "es": {
        "ADEPT \x011": "ADEPTO \x011", "ANTI-FIRE": "ANTIFUEGO",
        "ANTI-MAGIC": "ANTIMAGIA", "APPLE": "MANZANA",
        "APPRENTICE \x011": "APRENDIZ \x011", "ARCHER-GUARD": "GUARDIA ARQUERO",
        "ARCHMASTER \x011": "ARQUIMAESTRO \x011", "ARROW": "FLECHA",
        "AXE": "HACHA", "BAG": "BOLSA", "BARREL": "BARRIL",
        "BOW": "ARCO", "CAST": "LANZAR", "CHEST": "COFRE",
        "CLOSE": "CERRAR", "COIN": "MONEDA", "DAGGER": "DAGA",
        "DOOR": "PUERTA", "DRINK": "BEBER", "DROP": "SOLTAR",
        "EAT": "COMER", "FIREBALL": "BOLA DE FUEGO", "FLASK": "FRASCO",
        "FOOD": "COMIDA", "GOLD": "ORO", "HEALTH": "SALUD",
        "KEY": "LLAVE", "LOCK": "CERRADURA", "MANA": "MANÁ",
        "OPEN": "ABRIR", "POTION": "POCIÓN", "SAVE GAME": "GUARDAR PARTIDA",
        "LOAD GAME": "CARGAR PARTIDA", "SHIELD": "ESCUDO", "SWORD": "ESPADA",
        "THROW": "LANZAR", "TORCH": "ANTORCHA", "USE": "USAR",
        "WAND": "VARITA", "WATER": "AGUA",
    },
}

def refresh(path: Path, terms: dict[str, str]) -> int:
    lines = path.read_text(encoding="utf-8").splitlines()
    changed = 0
    for index, line in enumerate(lines[:-1]):
        if not line.startswith('msgid "') or not line.endswith('"'):
            continue
        # The reviewed DM2 keys are single-line ASCII GDAT labels, except for
        # the literal level control byte.  Do not use ``unicode_escape`` here:
        # it would corrupt future non-ASCII source labels.
        msgid = line[7:-1].replace('\\"', '"').replace('\\\\', '\\')
        if msgid not in terms or not lines[index + 1].startswith('msgstr "'):
            continue
        value = terms[msgid].replace("\\", "\\\\").replace('"', '\\"')
        lines[index + 1] = f'msgstr "{value}"'
        changed += 1
    path.write_text("\n".join(lines) + "\n", encoding="utf-8")
    return changed

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--po-dir", type=Path, default=ROOT)
    args = parser.parse_args()
    for language, terms in DM2.items():
        catalog = args.po_dir / f"dm2.{language}.po"
        print(f"{catalog.name}: {refresh(catalog, terms)} reviewed entries")
