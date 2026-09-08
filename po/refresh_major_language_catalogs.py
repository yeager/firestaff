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

# Authenticated FM Towns CSB action labels.  Every Japanese key below is
# annotated with its corresponding original action mnemonic in ``csb.pot``.
# Keep this table separate from item names: these are reviewed presentation
# strings, whereas item-name promotion needs an index-locked media mapping.
CSB_ACTIONS = {
    "sv": {
        "さえぎる": "PARRERA", "叩き切る": "HUGGA", "角笛を吹く": "BLÅS HORN",
        "ｺｲﾝﾄｽ": "KASTA MYNT", "殴る": "SLÅ", "蹴る": "SPARKA",
        "ときの声": "STRIDSROP", "刺す": "STICK", "降りる": "KLÄTTRA NER",
        "時間凍結": "FRYS LIV", "打つ": "TRÄFFA", "振り回す": "SVÄNGA",
        "突き刺す": "STÖTA", "突く": "STICKA", "かわす": "PARERA",
        "暴れ回る": "RASERI", "火炎弾": "ELDBOLL", "対霊呪文": "FÖRDREVA",
        "催眠術": "FÖRVIRRA", "稲妻の術": "BLIXT", "対霊武器": "STÖRA",
        "斬り払う": "NÄRKAMP", "念じる": "ÅKALLA", "斬り下ろす": "HUGGA",
        "斬り裂く": "KLYVA", "打ち割る": "KROSSA", "気絶させる": "BEDÖVA",
        "射る": "SKJUTA", "呪文防御": "MAGISKÖLD", "火炎防御": "ELDSKÖLD",
        "呪縛する": "FLUXBUR", "治療する": "LÄKA", "手なずける": "LUGNA",
        "魔術灯火": "LJUS", "透視": "FÖNSTER", "火炎攻撃": "SPOTTA",
        "追い払う": "VIFTA", "投げる": "KASTA", "融合する": "FÖRENA",
    },
    "fr": {
        "さえぎる": "BLOQUER", "叩き切る": "TRANCHER", "角笛を吹く": "SONNER LE COR",
        "ｺｲﾝﾄｽ": "LANCER UNE PIÈCE", "殴る": "FRAPPER", "蹴る": "DONNER UN COUP DE PIED",
        "ときの声": "CRI DE GUERRE", "刺す": "POIGNARDER", "降りる": "DESCENDRE",
        "時間凍結": "GELER LA VIE", "打つ": "COUP", "振り回す": "BALAYER",
        "突き刺す": "ENFONCER", "突く": "PIQUER", "かわす": "PARRER",
        "暴れ回る": "RAGE", "火炎弾": "BOULE DE FEU", "対霊呪文": "DISSIPER",
        "催眠術": "CONFONDRE", "稲妻の術": "FOUDRE", "対霊武器": "DÉSINTÉGRER",
        "斬り払う": "MÊLÉE", "念じる": "INVOQUER", "斬り下ろす": "TAILLADER",
        "斬り裂く": "FENDRE", "打ち割る": "BRISER", "気絶させる": "ÉTOURDIR",
        "射る": "TIRER", "呪文防御": "BOUCLIER MAGIQUE", "火炎防御": "BOUCLIER DE FEU",
        "呪縛する": "CAGE DE FLUX", "治療する": "SOIGNER", "手なずける": "APPRIVOISER",
        "魔術灯火": "LUMIÈRE", "透視": "FENÊTRE", "火炎攻撃": "CRACHER",
        "追い払う": "BRANDIR", "投げる": "LANCER", "融合する": "FUSIONNER",
    },
    "de": {
        "さえぎる": "BLOCKEN", "叩き切る": "HACKEN", "角笛を吹く": "HORN BLASEN",
        "ｺｲﾝﾄｽ": "MÜNZE WERFEN", "殴る": "SCHLAGEN", "蹴る": "TRETEN",
        "ときの声": "KRIEGSSCHREI", "刺す": "STECHEN", "降りる": "HINABSTEIGEN",
        "時間凍結": "LEBEN EINFRIEREN", "打つ": "TREFFEN", "振り回す": "SCHWINGEN",
        "突き刺す": "STOSSEN", "突く": "PIKSEN", "かわす": "PARIEREN",
        "暴れ回る": "BERSERKERWUT", "火炎弾": "FEUERBALL", "対霊呪文": "BANNEN",
        "催眠術": "VERWIRREN", "稲妻の術": "BLITZ", "対霊武器": "ZERSTÖREN",
        "斬り払う": "NAHKAMPF", "念じる": "BESCHWÖREN", "斬り下ろす": "SCHLITZEN",
        "斬り裂く": "SPALTEN", "打ち割る": "ZERTRÜMMERN", "気絶させる": "BETÄUBEN",
        "射る": "SCHIESSEN", "呪文防御": "ZAUBERSCHILD", "火炎防御": "FEUERSCHILD",
        "呪縛する": "FLUXKÄFIG", "治療する": "HEILEN", "手なずける": "BERUHIGEN",
        "魔術灯火": "LICHT", "透視": "FENSTER", "火炎攻撃": "SPUCKEN",
        "追い払う": "SCHWINGEN", "投げる": "WERFEN", "融合する": "VERSCHMELZEN",
    },
    "es": {
        "さえぎる": "BLOQUEAR", "叩き切る": "TAJAR", "角笛を吹く": "TOCAR EL CUERNO",
        "ｺｲﾝﾄｽ": "LANZAR MONEDA", "殴る": "GOLPEAR", "蹴る": "PATEAR",
        "ときの声": "GRITO DE GUERRA", "刺す": "APUÑALAR", "降りる": "BAJAR",
        "時間凍結": "CONGELAR VIDA", "打つ": "GOLPE", "振り回す": "BLANDIR",
        "突き刺す": "EMPUJAR", "突く": "PINCHAR", "かわす": "PARAR",
        "暴れ回る": "FURIA", "火炎弾": "BOLA DE FUEGO", "対霊呪文": "DISIPAR",
        "催眠術": "CONFUNDIR", "稲妻の術": "RAYO", "対霊武器": "DISRUPTIR",
        "斬り払う": "MELÉ", "念じる": "INVOCAR", "斬り下ろす": "TAJAR",
        "斬り裂く": "HENDER", "打ち割る": "APLASTAR", "気絶させる": "ATURDIR",
        "射る": "DISPARAR", "呪文防御": "ESCUDO MÁGICO", "火炎防御": "ESCUDO DE FUEGO",
        "呪縛する": "JAULA DE FLUJO", "治療する": "CURAR", "手なずける": "CALMAR",
        "魔術灯火": "LUZ", "透視": "VENTANA", "火炎攻撃": "ESCUPIR",
        "追い払う": "BLANDIR", "投げる": "LANZAR", "融合する": "FUSIONAR",
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
    for language, terms in CSB_ACTIONS.items():
        catalog = args.po_dir / f"csb.{language}.po"
        print(f"{catalog.name}: {refresh(catalog, terms)} reviewed action entries")
