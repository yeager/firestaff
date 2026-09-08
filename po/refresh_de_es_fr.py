#!/usr/bin/env python3
"""Apply reviewed German, Spanish and French translations for current UI text.

This deliberately changes only entries explicitly reviewed below.  Missing
strings stay empty and use Firestaff's documented English fallback; the tool
must never convert an untranslated entry into a fake completed translation.
"""
from __future__ import annotations

import ast
from pathlib import Path

ROOT = Path(__file__).resolve().parent

DM1 = {
    "de": {
        "{champion} MUMBLES A MEANINGLESS SPELL.": "{champion} MURMELT EINEN SINNLOSEN ZAUBER.",
        "PRESS ENTER TO RETURN TO MENU": "ENTER DRÜCKEN, UM ZUM MENÜ ZURÜCKZUKEHREN",
        "ESC TO DISMISS": "ESC ZUM SCHLIESSEN", "RETURN TO MENU?": "ZUM MENÜ ZURÜCKKEHREN?",
        "RETURN TO START MENU?": "ZUM STARTMENÜ ZURÜCKKEHREN?", "YES": "JA", "NO": "NEIN", "OK": "OK",
        "LEADER HAND FULL": "HAND DES ANFÜHRERS VOLL", "HAND FULL": "HAND VOLL",
        "HAND ROUTE FAILED": "HANDÜBERGABE FEHLGESCHLAGEN", "ITEM IN HAND": "GEGENSTAND IN DER HAND",
        "SPELL CAST": "ZAUBER GEWIRKT", "FOUNTAIN": "BRUNNEN", "CONTAINER FILLED": "BEHÄLTER GEFÜLLT",
        "ENTER MENU": "MENÜ ÖFFNEN",
        "` MASTER": "` MEISTER", "a MASTER": "a MEISTER", "b MASTER": "b MEISTER",
        "c MASTER": "c MEISTER", "d MASTER": "d MEISTER", "e MASTER": "e MEISTER", "ARCHMASTER": "ERZMEISTER",
    },
    "es": {
        "{champion} MUMBLES A MEANINGLESS SPELL.": "{champion} MURMURA UN HECHIZO SIN SENTIDO.",
        "PRESS ENTER TO RETURN TO MENU": "PULSA INTRO PARA VOLVER AL MENÚ",
        "ESC TO DISMISS": "ESC PARA CERRAR", "RETURN TO MENU?": "¿VOLVER AL MENÚ?",
        "RETURN TO START MENU?": "¿VOLVER AL MENÚ DE INICIO?", "YES": "SÍ", "NO": "NO", "OK": "ACEPTAR",
        "LEADER HAND FULL": "MANO DEL LÍDER LLENA", "HAND FULL": "MANO LLENA",
        "HAND ROUTE FAILED": "FALLO AL PASAR EL OBJETO", "ITEM IN HAND": "OBJETO EN LA MANO",
        "SPELL CAST": "HECHIZO LANZADO", "FOUNTAIN": "FUENTE", "CONTAINER FILLED": "RECIPIENTE LLENO",
        "ENTER MENU": "ABRIR MENÚ",
        "` MASTER": "` MAESTRO", "a MASTER": "a MAESTRO", "b MASTER": "b MAESTRO",
        "c MASTER": "c MAESTRO", "d MASTER": "d MAESTRO", "e MASTER": "e MAESTRO", "ARCHMASTER": "ARQUIMAESTRO",
    },
    "fr": {
        "{champion} MUMBLES A MEANINGLESS SPELL.": "{champion} MARMONNE UN SORT SANS EFFET.",
        "PRESS ENTER TO RETURN TO MENU": "APPUYEZ SUR ENTRÉE POUR REVENIR AU MENU",
        "ESC TO DISMISS": "ÉCHAP POUR FERMER", "RETURN TO MENU?": "REVENIR AU MENU ?",
        "RETURN TO START MENU?": "REVENIR AU MENU DE DÉMARRAGE ?", "YES": "OUI", "NO": "NON", "OK": "OK",
        "LEADER HAND FULL": "MAIN DU CHEF PLEINE", "HAND FULL": "MAIN PLEINE",
        "HAND ROUTE FAILED": "TRANSFERT À LA MAIN ÉCHOUÉ", "ITEM IN HAND": "OBJET EN MAIN",
        "SPELL CAST": "SORT LANCÉ", "FOUNTAIN": "FONTAINE", "CONTAINER FILLED": "RÉCIPIENT REMPLI",
        "ENTER MENU": "OUVRIR LE MENU",
        "` MASTER": "` MAÎTRE", "a MASTER": "a MAÎTRE", "b MASTER": "b MAÎTRE",
        "c MASTER": "c MAÎTRE", "d MASTER": "d MAÎTRE", "e MASTER": "e MAÎTRE", "ARCHMASTER": "ARCHIMAÎTRE",
    },
}

STARTUP = {
    "de": {"FRANÇAIS": "FRANZÖSISCH", "FIT SMOOTH": "SANFT EINPASSEN", "COPY ORIGINAL GAME FILES INTO THE DATA DIRECTORY": "ORIGINALE SPIELDATEIEN IN DAS DATENVERZEICHNIS KOPIEREN", "DATA DIRECTORY NOT FOUND": "DATENVERZEICHNIS NICHT GEFUNDEN", "NO SAVES FOUND": "KEINE SPIELSTÄNDE GEFUNDEN", "CHOOSE GAME DATA FOLDER": "SPIELDATENORDNER WÄHLEN", "SELECT A .FSART FILE": "EINE .FSART-DATEI AUSWÄHLEN", "SET": "FESTLEGEN", "EDITING": "BEARBEITEN", "INPUT MODE": "EINGABEMODUS", "TOUCH CONTROLS": "TOUCH-STEUERUNG", "DATA DIRECTORY": "DATENVERZEICHNIS", "ORIGINAL DATA": "ORIGINALDATEN", "MASTER VOLUME": "GESAMTLAUTSTÄRKE", "MUTE AUDIO": "TON STUMMSCHALTEN", "FONT SCALE": "SCHRIFTSKALIERUNG", "MINIMAP": "MINIKARTE", "AUTOMAP": "AUTOMATISCHE KARTE", "AMBIENT SOUND": "UMGEBUNGSKLANG", "UI SCALE": "OBERFLÄCHENSKALIERUNG", "CUSTOM MUSIC": "EIGENE MUSIK", "CUSTOM DUNGEONS": "EIGENE DUNGEONS", "OPEN...": "ÖFFNEN...", "DATA": "DATEN", "DATA FILES NOT FOUND": "DATENDATEIEN NICHT GEFUNDEN", "ENTER FOR GAME MENU": "EINGABE FÜR SPIELMENÜ", "NO DM1 SAVES FOUND": "KEINE DM1-SPIELSTÄNDE GEFUNDEN", "ESC SETTINGS": "ESC: EINSTELLUNGEN", "ENTER LOAD   ESC SETTINGS": "EINGABE: LADEN   ESC: EINSTELLUNGEN", "SELECT TO VERIFY ORIGINAL DATA": "AUSWÄHLEN, UM ORIGINALDATEN ZU PRÜFEN", "NO DATA SOURCE YET": "NOCH KEINE DATENQUELLE", "ARCHITECTURE": "ARCHITEKTUR", "LOCKED BY V2.0 FILTERED MODE": "DURCH GEFILTERTEN V2.0-MODUS GESPERRT", "> DATA FILES NOT FOUND": "> DATENDATEIEN NICHT GEFUNDEN", "> RENDERER UNAVAILABLE": "> RENDERER NICHT VERFÜGBAR", "AND THE PRESERVATION PROJECTS": "UND DIE BEWAHRUNGSPROJEKTE", "CREDITS AND ARCHIVE": "DANKSAGUNGEN UND ARCHIV", "ARCHIVE SECTIONS": "ARCHIVBEREICHE", "CHOOSE PLATFORM": "PLATTFORM WÄHLEN", "CHOOSE PRESENTATION": "DARSTELLUNG WÄHLEN", "LAUNCH >": "STARTEN >"},
    "es": {"FRANÇAIS": "FRANCÉS", "FIT SMOOTH": "AJUSTAR SUAVE", "COPY ORIGINAL GAME FILES INTO THE DATA DIRECTORY": "COPIA LOS ARCHIVOS ORIGINALES DEL JUEGO EN EL DIRECTORIO DE DATOS", "DATA DIRECTORY NOT FOUND": "DIRECTORIO DE DATOS NO ENCONTRADO", "NO SAVES FOUND": "NO SE ENCONTRARON PARTIDAS", "CHOOSE GAME DATA FOLDER": "ELIGE LA CARPETA DE DATOS DEL JUEGO", "SELECT A .FSART FILE": "SELECCIONA UN ARCHIVO .FSART", "SET": "ESTABLECER", "EDITING": "EDICIÓN", "INPUT MODE": "MODO DE ENTRADA", "TOUCH CONTROLS": "CONTROLES TÁCTILES", "DATA DIRECTORY": "DIRECTORIO DE DATOS", "ORIGINAL DATA": "DATOS ORIGINALES", "MASTER VOLUME": "VOLUMEN PRINCIPAL", "MUTE AUDIO": "SILENCIAR AUDIO", "FONT SCALE": "ESCALA DE FUENTE", "MINIMAP": "MINIMAPA", "AUTOMAP": "MAPA AUTOMÁTICO", "AMBIENT SOUND": "SONIDO AMBIENTAL", "UI SCALE": "ESCALA DE INTERFAZ", "CUSTOM MUSIC": "MÚSICA PERSONALIZADA", "CUSTOM DUNGEONS": "MAZMORRAS PERSONALIZADAS", "OPEN...": "ABRIR...", "DATA": "DATOS", "DATA FILES NOT FOUND": "ARCHIVOS DE DATOS NO ENCONTRADOS", "ENTER FOR GAME MENU": "INTRO PARA EL MENÚ DEL JUEGO", "NO DM1 SAVES FOUND": "NO SE ENCONTRARON PARTIDAS DE DM1", "ESC SETTINGS": "ESC: AJUSTES", "ENTER LOAD   ESC SETTINGS": "INTRO: CARGAR   ESC: AJUSTES", "SELECT TO VERIFY ORIGINAL DATA": "SELECCIONA PARA VERIFICAR LOS DATOS ORIGINALES", "NO DATA SOURCE YET": "AÚN NO HAY FUENTE DE DATOS", "ARCHITECTURE": "ARQUITECTURA", "LOCKED BY V2.0 FILTERED MODE": "BLOQUEADO POR EL MODO FILTRADO V2.0", "> DATA FILES NOT FOUND": "> ARCHIVOS DE DATOS NO ENCONTRADOS", "> RENDERER UNAVAILABLE": "> RENDERIZADOR NO DISPONIBLE", "AND THE PRESERVATION PROJECTS": "Y LOS PROYECTOS DE PRESERVACIÓN", "CREDITS AND ARCHIVE": "CRÉDITOS Y ARCHIVO", "ARCHIVE SECTIONS": "SECCIONES DEL ARCHIVO", "CHOOSE PLATFORM": "ELIGE PLATAFORMA", "CHOOSE PRESENTATION": "ELIGE PRESENTACIÓN", "LAUNCH >": "INICIAR >"},
    "fr": {"FRANÇAIS": "FRANÇAIS", "FIT SMOOTH": "AJUSTER EN DOUCEUR", "COPY ORIGINAL GAME FILES INTO THE DATA DIRECTORY": "COPIEZ LES FICHIERS ORIGINAUX DU JEU DANS LE DOSSIER DE DONNÉES", "DATA DIRECTORY NOT FOUND": "DOSSIER DE DONNÉES INTROUVABLE", "NO SAVES FOUND": "AUCUNE SAUVEGARDE TROUVÉE", "CHOOSE GAME DATA FOLDER": "CHOISIR LE DOSSIER DE DONNÉES DU JEU", "SELECT A .FSART FILE": "SÉLECTIONNER UN FICHIER .FSART", "SET": "DÉFINIR", "EDITING": "ÉDITION", "INPUT MODE": "MODE DE SAISIE", "TOUCH CONTROLS": "COMMANDES TACTILES", "DATA DIRECTORY": "DOSSIER DE DONNÉES", "ORIGINAL DATA": "DONNÉES ORIGINALES", "MASTER VOLUME": "VOLUME PRINCIPAL", "MUTE AUDIO": "COUPER LE SON", "FONT SCALE": "TAILLE DU TEXTE", "MINIMAP": "MINICARTE", "AUTOMAP": "CARTE AUTOMATIQUE", "AMBIENT SOUND": "SON AMBIANT", "UI SCALE": "ÉCHELLE DE L’INTERFACE", "CUSTOM MUSIC": "MUSIQUE PERSONNALISÉE", "CUSTOM DUNGEONS": "DONJONS PERSONNALISÉS", "OPEN...": "OUVRIR...", "DATA": "DONNÉES", "DATA FILES NOT FOUND": "FICHIERS DE DONNÉES INTROUVABLES", "ENTER FOR GAME MENU": "ENTRÉE POUR LE MENU DU JEU", "NO DM1 SAVES FOUND": "AUCUNE SAUVEGARDE DM1 TROUVÉE", "ESC SETTINGS": "ÉCHAP : PARAMÈTRES", "ENTER LOAD   ESC SETTINGS": "ENTRÉE : CHARGER   ÉCHAP : PARAMÈTRES", "SELECT TO VERIFY ORIGINAL DATA": "SÉLECTIONNER POUR VÉRIFIER LES DONNÉES ORIGINALES", "NO DATA SOURCE YET": "PAS ENCORE DE SOURCE DE DONNÉES", "ARCHITECTURE": "ARCHITECTURE", "LOCKED BY V2.0 FILTERED MODE": "VERROUILLÉ PAR LE MODE FILTRÉ V2.0", "> DATA FILES NOT FOUND": "> FICHIERS DE DONNÉES INTROUVABLES", "> RENDERER UNAVAILABLE": "> MOTEUR DE RENDU INDISPONIBLE", "AND THE PRESERVATION PROJECTS": "ET LES PROJETS DE PRÉSERVATION", "CREDITS AND ARCHIVE": "CRÉDITS ET ARCHIVES", "ARCHIVE SECTIONS": "SECTIONS DES ARCHIVES", "CHOOSE PLATFORM": "CHOISIR LA PLATEFORME", "CHOOSE PRESENTATION": "CHOISIR LA PRÉSENTATION", "LAUNCH >": "LANCER >"},
}

STUDIO = {
    "de": {"Firestaff Artpack Studio 0.2": "Firestaff Artpack Studio 0.2", "Settings...": "Einstellungen...", "Settings": "Einstellungen", "AI Command Template": "KI-Befehlsvorlage", "Settings saved": "Einstellungen gespeichert", "Save": "Speichern", "Show grid": "Raster anzeigen", "Show things": "Objekte anzeigen", "Game ID": "Spiel-ID", "Direction": "Richtung", "HP Current": "HP aktuell", "Stamina Current": "Ausdauer aktuell", "Mana Current": "Mana aktuell", "File": "Datei", "Save As": "Speichern unter", "All Files": "Alle Dateien", "Loaded: {}": "Geladen: {}", "Warning": "Warnung", "Saved as: {}": "Gespeichert als: {}", "Save Parts": "Spielstandteile", "Status": "Status", "Valid": "Gültig", "Description": "Beschreibung", "No champion data": "Keine Championdaten", "No global data": "Keine globalen Daten", "Global Data": "Globale Daten"},
    "es": {"Firestaff Artpack Studio 0.2": "Firestaff Artpack Studio 0.2", "Settings...": "Ajustes...", "Settings": "Ajustes", "AI Command Template": "Plantilla de comando de IA", "Settings saved": "Ajustes guardados", "Save": "Guardar", "Show grid": "Mostrar cuadrícula", "Show things": "Mostrar objetos", "Game ID": "ID del juego", "Direction": "Dirección", "HP Current": "PV actual", "Stamina Current": "Resistencia actual", "Mana Current": "Maná actual", "File": "Archivo", "Save As": "Guardar como", "All Files": "Todos los archivos", "Loaded: {}": "Cargado: {}", "Warning": "Advertencia", "Saved as: {}": "Guardado como: {}", "Save Parts": "Partes de guardado", "Status": "Estado", "Valid": "Válido", "Description": "Descripción", "No champion data": "No hay datos de campeones", "No global data": "No hay datos globales", "Global Data": "Datos globales"},
    "fr": {"Firestaff Artpack Studio 0.2": "Firestaff Artpack Studio 0.2", "Settings...": "Paramètres...", "Settings": "Paramètres", "AI Command Template": "Modèle de commande IA", "Settings saved": "Paramètres enregistrés", "Save": "Enregistrer", "Show grid": "Afficher la grille", "Show things": "Afficher les objets", "Game ID": "ID du jeu", "Direction": "Direction", "HP Current": "PV actuels", "Stamina Current": "Endurance actuelle", "Mana Current": "Mana actuel", "File": "Fichier", "Save As": "Enregistrer sous", "All Files": "Tous les fichiers", "Loaded: {}": "Chargé : {}", "Warning": "Avertissement", "Saved as: {}": "Enregistré sous : {}", "Save Parts": "Parties de sauvegarde", "Status": "État", "Valid": "Valide", "Description": "Description", "No champion data": "Aucune donnée de champion", "No global data": "Aucune donnée globale", "Global Data": "Données globales"},
}

# The shared action vocabulary is deliberately maintained separately from the
# data-derived catalogs.  A key is applied only when it exists in a game's
# canonical template, so it cannot accidentally invent text for another game.
GAME_ACTIONS = {
    "de": {"BLOCK": "BLOCKEN", "CHOP": "HACKEN", "BLOW HORN": "HORN BLASEN", "FLIP": "UMDREHEN", "PUNCH": "SCHLAGEN", "KICK": "TRETEN", "WAR CRY": "KRIEGSSCHREI", "STAB": "ZUSTECHEN", "CLIMB DOWN": "HINABSTEIGEN", "FREEZE LIFE": "LEBEN EINFRIEREN", "HIT": "TREFFEN", "SWING": "SCHWINGEN", "THRUST": "STOSS", "JAB": "STICH", "PARRY": "PARRIEREN", "HACK": "HIEB", "BERZERK": "BERSERKER", "FIREBALL": "FEUERBALL", "CONFUSE": "VERWIRREN", "LIGHTNING": "BLITZ", "DISRUPT": "STÖREN", "MELEE": "NAHKAMPF", "INVOKE": "ANRUFEN", "SLASH": "SCHNITT", "CLEAVE": "SPALTEN", "BASH": "ZERSCHMETTERN", "STUN": "BETÄUBEN", "SHOOT": "SCHIESSEN", "HEAL": "HEILEN", "CALM": "BERUHIGEN", "LIGHT": "LICHT", "THROW": "WERFEN", "WATER": "WASSER", "CANCEL": "ABBRECHEN", "CAPE": "UMHANG", "CHEESE": "KÄSE", "CLUB": "KEULE", "COMPASS": "KOMPASS", "CROSSBOW": "ARMBRUST", "FIGHTER": "KÄMPFER", "GAME PAUSED": "SPIEL PAUSIERT", "GHOST": "GEIST", "GOLD KEY": "GOLDSCHLÜSSEL", "HEALTH": "GESUNDHEIT", "STAFF OF IRRA": "STAB DES IRRA"},
    "es": {"BLOCK": "BLOQUEAR", "CHOP": "TAJAR", "BLOW HORN": "TOCAR CUERNO", "FLIP": "GIRAR", "PUNCH": "GOLPEAR", "KICK": "PATEAR", "WAR CRY": "GRITO DE GUERRA", "STAB": "APUÑALAR", "CLIMB DOWN": "BAJAR", "FREEZE LIFE": "CONGELAR VIDA", "HIT": "GOLPEAR", "SWING": "BLANDIR", "THRUST": "ESTOCADA", "JAB": "PINCHAR", "PARRY": "PARAR", "HACK": "TAJAR", "BERZERK": "BERSERK", "FIREBALL": "BOLA DE FUEGO", "CONFUSE": "CONFUNDIR", "LIGHTNING": "RAYO", "DISRUPT": "PERTURBAR", "MELEE": "CUERPO A CUERPO", "INVOKE": "INVOCAR", "SLASH": "TAJAR", "CLEAVE": "HENDIR", "BASH": "GOLPEAR", "STUN": "ATURDIR", "SHOOT": "DISPARAR", "HEAL": "CURAR", "CALM": "CALMAR", "LIGHT": "LUZ", "THROW": "LANZAR", "WATER": "AGUA", "CANCEL": "CANCELAR", "CAPE": "CAPA", "CHEESE": "QUESO", "CLUB": "GARROTE", "COMPASS": "BRÚJULA", "CROSSBOW": "BALLESTA", "FIGHTER": "GUERRERO", "GAME PAUSED": "JUEGO EN PAUSA", "GHOST": "FANTASMA", "GOLD KEY": "LLAVE DE ORO", "HEALTH": "SALUD", "STAFF OF IRRA": "BASTÓN DE IRRA"},
    "fr": {"BLOCK": "BLOQUER", "CHOP": "TAILLER", "BLOW HORN": "SONNER DU COR", "FLIP": "RETOURNER", "PUNCH": "FRAPPER", "KICK": "DONNER UN COUP DE PIED", "WAR CRY": "CRI DE GUERRE", "STAB": "POIGNARDER", "CLIMB DOWN": "DESCENDRE", "FREEZE LIFE": "GELER LA VIE", "HIT": "FRAPPER", "SWING": "BALAYER", "THRUST": "ESTOCADE", "JAB": "PIQUER", "PARRY": "PARER", "HACK": "TAILLER", "BERZERK": "BERSERK", "FIREBALL": "BOULE DE FEU", "CONFUSE": "CONFONDRE", "LIGHTNING": "FOUDRE", "DISRUPT": "PERTURBER", "MELEE": "CORPS À CORPS", "INVOKE": "INVOQUER", "SLASH": "TAILLADER", "CLEAVE": "FENDRE", "BASH": "ASSOMMER", "STUN": "ÉTOURDIR", "SHOOT": "TIRER", "HEAL": "SOIGNER", "CALM": "APAISER", "LIGHT": "LUMIÈRE", "THROW": "LANCER", "WATER": "EAU", "CANCEL": "ANNULER", "CAPE": "CAPE", "CHEESE": "FROMAGE", "CLUB": "GOURDIN", "COMPASS": "BOUSSOLE", "CROSSBOW": "ARBALETE", "FIGHTER": "COMBATTANT", "GAME PAUSED": "JEU EN PAUSE", "GHOST": "FANTÔME", "GOLD KEY": "CLÉ D’OR", "HEALTH": "SANTÉ", "STAFF OF IRRA": "BÂTON D’IRRA"},
}

def msgid(block: str) -> str | None:
    for line in block.splitlines():
        if line.startswith("msgid "):
            return ast.literal_eval(line[6:])
    return None

def replace(block: str, value: str) -> str:
    lines = block.splitlines()
    lines = [line for line in lines if line != "#, fuzzy"]
    rendered = repr(value).replace("'", '"')
    for index, line in enumerate(lines):
        if line.startswith("msgstr "):
            lines[index] = f"msgstr {rendered}"
            return "\n".join(lines)
    raise ValueError("entry has no msgstr")

def apply(path: Path, values: dict[str, str]) -> int:
    blocks = path.read_text(encoding="utf-8").split("\n\n")
    changed = 0
    for index, block in enumerate(blocks):
        key = msgid(block)
        if key in values:
            updated = replace(block, values[key])
            changed += updated != block
            blocks[index] = updated
    path.write_text("\n\n".join(blocks), encoding="utf-8")
    return changed

def main() -> None:
    for language in ("de", "es", "fr"):
        for domain, table, path in (
            ("dm1", DM1, ROOT / f"dm1.{language}.po"),
            ("csb", GAME_ACTIONS, ROOT / f"csb.{language}.po"),
            ("dm2", GAME_ACTIONS, ROOT / f"dm2.{language}.po"),
            ("startup-menu", STARTUP, ROOT / f"startup-menu.{language}.po"),
            ("studio", STUDIO, ROOT / "studio" / f"{language}.po"),
        ):
            print(f"{path.relative_to(ROOT)}: {apply(path, table[language])} reviewed {domain} entries")

if __name__ == "__main__":
    main()
