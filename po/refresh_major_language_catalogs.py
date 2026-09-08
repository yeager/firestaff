#!/usr/bin/env python3
"""Apply reviewed DE/ES/FR terminology after ``msgmerge``.

The game templates are regenerated frequently.  Keeping the small, reviewed
terminology layer here means a merge cannot silently replace it with a fuzzy
guess.  Entries not present in this table deliberately stay untranslated and
fall back to the selected game's original text; do not fill them with a
different game's wording.
"""
import argparse
import ast
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
    # Reviewed Simplified Chinese terminology for GDAT labels and dungeon
    # inscriptions.  Names invented by the game (for example BAINBLOOM and
    # DRAGOTH) intentionally remain source-identical; generic UI and world
    # vocabulary must never use an English fallback merely because the
    # original PC release did not ship a Chinese resource.
    "zh": {
        "ANTI-FIRE": "抗火", "ANTI-MAGIC": "抗魔法",
        "ARCHER-GUARD": "弓箭守卫", "ARCHMASTER \x011": "大法师 \x011",
        "ARTISAN \x011": "工匠 \x011", "ATTACK MINION": "攻击仆从",
        "AXEMAN": "斧兵", "AXEMAN THIEF": "斧兵盗贼", "BAD MERCHANT": "奸商",
        "BAG": "袋子", "BANDANA": "头巾", "BARREL": "木桶",
        "BASCINET": "护鼻盔", "BEFORE USING\nBOILER TURN ON\nWATER VALVES\nTWO LEVELS UP": "使用锅炉前\n请打开上方两层\n的水阀",
        "BLACK BOOTS": "黑靴", "BLACK SKIRT": "黑裙", "BLACK TOP": "黑上衣",
        "BLOOD KEY": "血钥匙", "BODICE": "紧身胸衣", "BOILER": "锅炉",
        "BONE": "骨头", "BRANCH": "树枝", "BREASTPLATE": "胸甲",
        "BRIGANDINE": "板条甲", "BUSH": "灌木", "CANCEL": "取消",
        "CARRY MINION": "搬运仆从", "CAVE IN": "塌方", "CAVERN BAT": "洞穴蝙蝠",
        "CAVERN TABLE": "洞穴桌", "CLAN CHIEF GEM": "氏族首领宝石",
        "CLAN KEY PIECE": "氏族钥匙碎片", "COMBAT STAFF": "战斗法杖",
        "COPPER COIN": "铜币", "COVER PLATE": "盖板", "CRAFTSMAN \x011": "工匠 \x011",
        "CRYSTAL SHIELD": "水晶盾", "DEAD BAT": "死蝙蝠", "DEBUG MAP": "调试地图",
        "DEXTERITY": "敏捷", "DIGGER WORM": "掘地虫", "DOOR GHOST": "门幽灵",
        "DOUBLET": "紧身上衣", "DRAGOTH ATTACK MINION": "Dragoth 攻击仆从",
        "EMERALD ORB": "翡翠宝珠", "EVIL ATTACK MINION": "邪恶攻击仆从",
        "EVIL FOUNTAIN": "邪恶喷泉", "EVIL GUARD MINION": "邪恶守卫仆从",
        "EVIL SCOUT MINION": "邪恶侦察仆从", "EXPERT \x011": "专家 \x011",
        "FACE PILLAR": "面孔石柱", "FAIRY CUSHION": "仙女坐垫",
        "FETCH MINION": "取物仆从", "FINE ROBE TOP": "精致长袍上衣",
        "FIRE GREAVE": "火焰护胫", "FIRE HELM": "火焰头盔",
        "FIRE PLATE": "火焰甲片", "FIRE POLEYN": "火焰膝甲",
        "FLAME ORB": "火焰宝珠", "FLASK": "烧瓶",
        "FURNACE ON\nLEVEL BELOW\nMUST BE STOKED\nAND BURNING": "下层的熔炉\n必须添燃料\n并保持燃烧",
        "GAME PAUSED": "游戏已暂停", "GREAT HELM": "大头盔", "GREAVES": "护胫",
        "GUARD MINION": "守卫仆从", "HEALTH": "生命", "HORNED HELM": "角盔",
        "INVOKE ZO TO\nCLOSE PORTALS\nTHAT THE\nEVIL MINIONS\nHAVE OPENED": "施放 ZO\n关闭邪恶仆从\n开启的传送门",
        "INVOKE\nZO EW KU\nTO CREATE A\nMINION THAT\nWILL ATTACK\nYOUR ENEMIES": "施放\nZO EW KU\n创造会攻击\n敌人的仆从",
        "INVOKE\nZO EW NETA\nTO CREATE A\nGUARDIAN\nMINION": "施放\nZO EW NETA\n创造守卫仆从",
        "JOURNEYMAN \x011": "熟练者 \x011", "KATANA": "武士刀",
        "LARGE GEAR": "大齿轮", "LEG PLATE": "腿甲", "LIGHTNING KEY": "闪电钥匙",
        "LIGHTNING ROD": "避雷针", "MACHETE": "砍刀", "MAGIC MAP": "魔法地图",
        "MAGIC MERCHANT": "魔法商人", "MAIL HELMET": "锁子头盔", "MANA": "法力",
        "MANA BLOSSOM": "法力花", "MERCHANT": "商人", "MERCHANT GUARD": "商人守卫",
        "MERCHANT TOMB": "商人墓", "METEOR METAL": "陨铁", "MINION MAP": "仆从地图",
        "MONEY BOX": "钱箱", "MOON KEY": "月亮钥匙", "MORNINGSTAR": "流星锤",
        "NEOPHYTE \x011": "新手 \x011", "NO CREATURE": "没有生物",
        "NO WATER\nCHECK PUMP\nOPERATION ON\nLEVEL BELOW": "没有水\n请检查下层\n水泵运作",
        "NOVICE \x011": "初学者 \x011", "OBELISK": "方尖碑", "PEDISTAL": "底座",
        "PILLAR": "石柱", "PIPE SHAFT": "管道井", "PIT GHOST": "坑洞幽灵",
        "PLANK": "木板", "POWER CRYSTAL": "能量水晶", "QUIVER": "箭袋",
        "RAINBOW WAND": "彩虹魔杖", "RED GEM": "红宝石", "REFLECTOR\nPRACTICE": "反射器\n练习",
        "RENEW THE LIFE\nOF A FALLEN\nCHAMPION": "复苏倒下的\n勇士的生命",
        "RESTART GAME": "重新开始游戏", "ROGUE STAVE": "游侠法杖",
        "RUNE CHARM": "符文护符", "SCALE HAUBERK": "鳞甲锁子衫",
        "SCALE MAIL": "鳞甲", "SCARAB": "圣甲虫", "SCOUT MAP": "侦察地图",
        "SCOUT MINION": "侦察仆从", "SCYTHE": "镰刀", "SERPENT STAFF": "蛇杖",
        "SHANK": "短刀", "SHIELD OF FIRE": "火焰之盾", "SHURIKEN": "手里剑",
        "SILVER COIN": "银币", "SKULL BRAZIER": "骷髅火盆", "SKULL KEY": "骷髅钥匙",
        "SLAYER ARROW": "杀手之箭", "SMALL PLANK": "小木板", "SPECTRE": "幽灵",
        "SPIKED CAPSTAN": "尖刺绞盘", "SPIKED WALL": "尖刺墙", "SPIRAL STAFF": "螺旋法杖",
        "SPIRIT CAP": "灵魂帽", "STALAGMITE": "石笋", "STAMINA": "耐力",
        "STEAK": "牛排", "STEAM ENGINE": "蒸汽机", "STRENGTH": "力量",
        "SUN CREST": "太阳徽记", "TABARD": "罩袍", "TANKARD": "大酒杯",
        "TAPESTRY": "挂毯", "THIGH PLATES": "大腿甲", "THORN DEMON": "荆棘恶魔",
        "TIGER STRIPED WORM": "虎纹蠕虫",
        "TO OPEN THE\nCASTLE DOOR\nYOU MUST GET\nA KEY PIECE\nFROM EACH OF\nTHE FOUR CLANS\nOF SKULLKEEP": "要打开\n城堡大门\n你必须从\n骷髅堡的\n四个氏族\n各取得一块\n钥匙碎片",
        "TOMBSTONE": "墓碑", "TORSO PLATE": "躯干甲", "TOWER BAT": "塔蝙蝠",
        "TRADING TABLE (BUYS)": "交易桌（收购）", "TRADING TABLE (SELLS)": "交易桌（出售）",
        "TREE": "树", "TREE GORGON": "树妖", "TUNIC": "束腰外衣",
        "U-HAUL MINION": "搬运仆从", "VACUUM FUSE": "真空保险丝",
        "VITALITY": "活力", "VOID DOOR": "虚空之门", "VORTEX": "涡流",
        "WAKE UP": "醒来", "WAR CLUB": "战棍", "WISDOM": "智慧", "WOLF": "狼",
        "WOOD SHIELD": "木盾", "WOOD TABLE": "木桌",
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


def _po_quoted_value(line: str) -> str:
    """Decode one gettext quoted value without accepting arbitrary Python."""
    return ast.literal_eval(line[line.index('"'):])


def _simple_catalog_values(path: Path) -> dict[str, str]:
    """Read one-line gettext pairs needed by the M564 index bridge.

    M564 object labels are deliberately short one-line entries.  Refuse a
    continuation instead of accidentally applying this bridge to prose or a
    formatted message.
    """
    values: dict[str, str] = {}
    msgid = None
    for line in path.read_text(encoding="utf-8").splitlines():
        if line.startswith("msgid "):
            msgid = _po_quoted_value(line)
        elif line.startswith("msgstr ") and msgid is not None:
            values[msgid] = _po_quoted_value(line)
            msgid = None
        elif line.startswith('"') and msgid is not None:
            # This is a multiline catalog entry outside the short M564 label
            # vocabulary.  Ignore it rather than accidentally pairing a
            # continuation with a later msgstr.
            msgid = None
    return values


def _csb_fmtowns_jp_m564_index_map(po_dir: Path) -> dict[str, str]:
    """Return the reviewed F31J-M564-to-English index correspondence.

    ``generate_csb_fmtowns_jp_l10n_map.py`` establishes the source contract:
    the first 177 English M564 rows and the selected F31J CP932 M564 rows are
    index-identical; the final English-only row has no F31J counterpart.
    The action rows preceding F31J M564 have their own explicit mapping.
    """
    english = []
    japanese = []
    comments: list[str] = []
    current = None
    for line in (po_dir / "csb.pot").read_text(encoding="utf-8").splitlines():
        if line.startswith("#."):
            comments.append(line)
        elif line.startswith("msgid "):
            current = _po_quoted_value(line)
        elif line.startswith("msgstr "):
            if current is not None:
                if any("Source-owned M564 object name" in c for c in comments):
                    english.append(current)
                if any("Authentic FM Towns JP:" in c for c in comments):
                    japanese.append(current)
            comments = []
            current = None
    action_count = 39
    if len(english) != 178 or len(japanese) != action_count + 177:
        raise ValueError(
            "CSB FM Towns M564 source order changed: "
            f"english={len(english)} japanese={len(japanese)}")
    return dict(zip(japanese[action_count:], english[:177], strict=True))


def refresh_csb_fmtowns_jp_object_names(po_dir: Path, language: str) -> int:
    """Promote index-locked F31J names through that locale's M564 entries."""
    catalog = po_dir / f"csb.{language}.po"
    current = _simple_catalog_values(catalog)
    mapping = _csb_fmtowns_jp_m564_index_map(po_dir)
    terms = {jp: current[english] for jp, english in mapping.items()
             if english in current and current[english]}
    if len(terms) != 177:
        raise ValueError(
            f"{catalog}: expected 177 localizable F31J M564 entries, "
            f"got {len(terms)}")
    return refresh(catalog, terms)

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
    for language in (
            "sv", "fr", "de", "ja", "zh", "cs", "da", "es", "fi",
            "hu", "it", "ko", "nl", "no", "pl", "pt", "ru", "tr", "id"):
        catalog = args.po_dir / f"csb.{language}.po"
        print(f"{catalog.name}: {refresh_csb_fmtowns_jp_object_names(args.po_dir, language)} "
              "index-locked F31J M564 entries")
