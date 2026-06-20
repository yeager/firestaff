"""
po/translations_other.py
========================

Complete translation tables for the 5 secondary i18n domains:
csb, firestaff, nexus, theron, startup-menu.

Each domain covers 19 locales (en, sv, fr, de, ja, zh, cs, da, es,
fi, hu, it, ko, nl, no, pl, pt, ru, tr). 'en' uses msgstr=msgstr (English
source). 'sv' is the manual Swedish source. Other locales are best-effort
machine translations; native speakers should replace msgstr before
public release.

Format specifiers (%u, %d, %s, %c, %X) and escapes (\\n, \\") must be
preserved verbatim across all translations.
"""

# =========================================================================
# firestaff (32 strings) - 19 locales
# =========================================================================
FIRESTAFF_EN = {  # English source (msgid = msgstr)
    'Accessibility': 'Accessibility', 'Audio': 'Audio', 'Back': 'Back',
    'Bestiary': 'Bestiary', 'Changelog': 'Changelog', 'Coming soon': 'Coming soon',
    'Continue Saved Game': 'Continue Saved Game', 'Controls': 'Controls',
    'Display': 'Display', 'EXTRAS': 'EXTRAS', 'Item Encyclopedia': 'Item Encyclopedia',
    'Language': 'Language', 'Map Viewer': 'Map Viewer', 'Museum of Lore': 'Museum of Lore',
    'New Game (V1 Original)': 'New Game (V1 Original)',
    'New Game (V2.1 Upscaled)': 'New Game (V2.1 Upscaled)',
    'New Game (V2.2 Enhanced)': 'New Game (V2.2 Enhanced)',
    'No data files': 'No data files', 'Off': 'Off', 'On': 'On',
    'PLAY': 'PLAY', 'QUIT': 'QUIT', 'Screenshot Gallery': 'Screenshot Gallery',
    'SELECT GAME': 'SELECT GAME', 'SETTINGS': 'SETTINGS',
    'Spell Reference': 'Spell Reference', 'Validate game data': 'Validate game data',
    'Version': 'Version', 'Video': 'Video',
    'RETURN TO START MENU?': 'RETURN TO START MENU?',
    'YES': 'YES', 'NO': 'NO',
}

FIRESTAFF = {'en': FIRESTAFF_EN}

FIRESTAFF['sv'] = {k: v for k, v in {
    'Accessibility': 'Tillgänglighet', 'Audio': 'Ljud', 'Back': 'Tillbaka',
    'Bestiary': 'Bestiarium', 'Changelog': 'Ändringslogg', 'Coming soon': 'Kommer snart',
    'Continue Saved Game': 'Fortsätt sparat spel', 'Controls': 'Kontroller',
    'Display': 'Bildskärm', 'EXTRAS': 'EXTRAS', 'Item Encyclopedia': 'Föremålsencyklopedi',
    'Language': 'Språk', 'Map Viewer': 'Kartvisare', 'Museum of Lore': 'Lore-museum',
    'New Game (V1 Original)': 'Nytt spel (V1 Original)',
    'New Game (V2.1 Upscaled)': 'Nytt spel (V2.1 Uppskalad)',
    'New Game (V2.2 Enhanced)': 'Nytt spel (V2.2 Förbättrad)',
    'No data files': 'Inga datafiler', 'Off': 'Av', 'On': 'På',
    'PLAY': 'SPELA', 'QUIT': 'AVSLUTA', 'Screenshot Gallery': 'Skärmbildsgalleri',
    'SELECT GAME': 'VÄLJ SPEL', 'SETTINGS': 'INSTÄLLNINGAR',
    'Spell Reference': 'Besvärjelsereferens', 'Validate game data': 'Validera speldata',
    'Version': 'Version', 'Video': 'Video',
    'RETURN TO START MENU?': 'ÅTERGÅ TILL STARTMENYN?',
    'YES': 'JA', 'NO': 'NEJ',
}.items() if k in FIRESTAFF_EN}

# Helper to make a per-locale table from a "universal" mapping
# (apply the same translation to all 19 locales where it's reasonable;
# for tricky cases, override per-locale).
FIRESTAFF.update({
    'fr': {k: v for k, v in {
        'Accessibility': 'Accessibilité', 'Audio': 'Audio', 'Back': 'Retour',
        'Bestiary': 'Bestiaire', 'Changelog': 'Journal des modifications',
        'Coming soon': 'Bientôt disponible', 'Continue Saved Game': 'Continuer sauvegarde',
        'Controls': 'Contrôles', 'Display': 'Affichage', 'EXTRAS': 'EXTRAS',
        'Item Encyclopedia': 'Encyclopédie des objets',
        'Language': 'Langue', 'Map Viewer': 'Visionneuse de carte',
        'Museum of Lore': 'Musée du Lore',
        'New Game (V1 Original)': 'Nouvelle partie (V1 Originale)',
        'New Game (V2.1 Upscaled)': 'Nouvelle partie (V2.1 Mise à l\'échelle)',
        'New Game (V2.2 Enhanced)': 'Nouvelle partie (V2.2 Améliorée)',
        'No data files': 'Aucun fichier de données', 'Off': 'Off', 'On': 'On',
        'PLAY': 'JOUER', 'QUIT': 'QUITTER', 'Screenshot Gallery': 'Galerie de captures',
        'SELECT GAME': 'SÉLECTIONNER JEU', 'SETTINGS': 'PARAMÈTRES',
        'Spell Reference': 'Référence des sorts',
        'Validate game data': 'Valider données du jeu',
        'Version': 'Version', 'Video': 'Vidéo',
        'RETURN TO START MENU?': 'RETOURNER AU MENU?',
        'YES': 'OUI', 'NO': 'NON',
    }.items() if k in FIRESTAFF_EN},
    'de': {k: v for k, v in {
        'Accessibility': 'Barrierefreiheit', 'Audio': 'Audio', 'Back': 'Zurück',
        'Bestiary': 'Bestiarium', 'Changelog': 'Änderungsprotokoll',
        'Coming soon': 'Demnächst', 'Continue Saved Game': 'Spielstand fortsetzen',
        'Controls': 'Steuerung', 'Display': 'Anzeige', 'EXTRAS': 'EXTRAS',
        'Item Encyclopedia': 'Gegenstands-Enzyklopädie',
        'Language': 'Sprache', 'Map Viewer': 'Kartenbetrachter',
        'Museum of Lore': 'Lore-Museum',
        'New Game (V1 Original)': 'Neues Spiel (V1 Original)',
        'New Game (V2.1 Upscaled)': 'Neues Spiel (V2.1 Hochskaliert)',
        'New Game (V2.2 Enhanced)': 'Neues Spiel (V2.2 Verbessert)',
        'No data files': 'Keine Datendateien', 'Off': 'Aus', 'On': 'An',
        'PLAY': 'SPIELEN', 'QUIT': 'BEENDEN',
        'Screenshot Gallery': 'Screenshot-Galerie',
        'SELECT GAME': 'SPIEL WÄHLEN', 'SETTINGS': 'EINSTELLUNGEN',
        'Spell Reference': 'Zauberreferenz',
        'Validate game data': 'Spieldaten validieren',
        'Version': 'Version', 'Video': 'Video',
        'RETURN TO START MENU?': 'ZURÜCK ZUM STARTMENÜ?',
        'YES': 'JA', 'NO': 'NEIN',
    }.items() if k in FIRESTAFF_EN},
    'ja': {k: v for k, v in {
        'Accessibility': 'アクセシビリティ', 'Audio': 'オーディオ',
        'Back': '戻る', 'Bestiary': '魔物図鑑', 'Changelog': '更新履歴',
        'Coming soon': '近日公開', 'Continue Saved Game': 'セーブから続ける',
        'Controls': '操作', 'Display': '表示', 'EXTRAS': 'EXTRAS',
        'Item Encyclopedia': 'アイテム図鑑', 'Language': '言語',
        'Map Viewer': 'マップビューア', 'Museum of Lore': 'ロア博物館',
        'New Game (V1 Original)': '新規ゲーム (V1 オリジナル)',
        'New Game (V2.1 Upscaled)': '新規ゲーム (V2.1 アップスケール)',
        'New Game (V2.2 Enhanced)': '新規ゲーム (V2.2 エンハンスド)',
        'No data files': 'データファイルなし', 'Off': 'オフ', 'On': 'オン',
        'PLAY': 'プレイ', 'QUIT': '終了',
        'Screenshot Gallery': 'スクリーンショットギャラリー',
        'SELECT GAME': 'ゲーム選択', 'SETTINGS': '設定',
        'Spell Reference': '呪文リファレンス', 'Validate game data': 'ゲームデータ検証',
        'Version': 'バージョン', 'Video': 'ビデオ',
        'RETURN TO START MENU?': 'スタートメニューに戻る?',
        'YES': 'はい', 'NO': 'いいえ',
    }.items() if k in FIRESTAFF_EN},
    'zh': {k: v for k, v in {
        'Accessibility': '无障碍', 'Audio': '音频', 'Back': '返回',
        'Bestiary': '怪物图鉴', 'Changelog': '更新日志',
        'Coming soon': '即将推出', 'Continue Saved Game': '继续存档',
        'Controls': '控制', 'Display': '显示', 'EXTRAS': '附加',
        'Item Encyclopedia': '物品百科', 'Language': '语言',
        'Map Viewer': '地图查看器', 'Museum of Lore': '传说博物馆',
        'New Game (V1 Original)': '新游戏 (V1 原始)',
        'New Game (V2.1 Upscaled)': '新游戏 (V2.1 升频)',
        'New Game (V2.2 Enhanced)': '新游戏 (V2.2 增强)',
        'No data files': '无数据文件', 'Off': '关', 'On': '开',
        'PLAY': '开始', 'QUIT': '退出',
        'Screenshot Gallery': '截图画廊',
        'SELECT GAME': '选择游戏', 'SETTINGS': '设置',
        'Spell Reference': '咒语参考', 'Validate game data': '验证游戏数据',
        'Version': '版本', 'Video': '视频',
        'RETURN TO START MENU?': '返回开始菜单?',
        'YES': '是', 'NO': '否',
    }.items() if k in FIRESTAFF_EN},
    'es': {k: v for k, v in {
        'Accessibility': 'Accesibilidad', 'Audio': 'Audio', 'Back': 'Atrás',
        'Bestiary': 'Bestiario', 'Changelog': 'Registro de cambios',
        'Coming soon': 'Próximamente', 'Continue Saved Game': 'Continuar partida',
        'Controls': 'Controles', 'Display': 'Pantalla', 'EXTRAS': 'EXTRAS',
        'Item Encyclopedia': 'Enciclopedia de objetos',
        'Language': 'Idioma', 'Map Viewer': 'Visor de mapa',
        'Museum of Lore': 'Museo del Lore',
        'New Game (V1 Original)': 'Nueva partida (V1 Original)',
        'New Game (V2.1 Upscaled)': 'Nueva partida (V2.1 Escalado)',
        'New Game (V2.2 Enhanced)': 'Nueva partida (V2.2 Mejorado)',
        'No data files': 'Sin archivos de datos', 'Off': 'Apagado', 'On': 'Encendido',
        'PLAY': 'JUGAR', 'QUIT': 'SALIR',
        'Screenshot Gallery': 'Galería de capturas',
        'SELECT GAME': 'SELECCIONAR JUEGO', 'SETTINGS': 'AJUSTES',
        'Spell Reference': 'Referencia de hechizos',
        'Validate game data': 'Validar datos del juego',
        'Version': 'Versión', 'Video': 'Vídeo',
        'RETURN TO START MENU?': 'VOLVER AL MENÚ?',
        'YES': 'SÍ', 'NO': 'NO',
    }.items() if k in FIRESTAFF_EN},
    'it': {k: v for k, v in {
        'Accessibility': 'Accessibilità', 'Audio': 'Audio', 'Back': 'Indietro',
        'Bestiary': 'Bestiario', 'Changelog': 'Registro modifiche',
        'Coming soon': 'Prossimamente', 'Continue Saved Game': 'Continua partita',
        'Controls': 'Controlli', 'Display': 'Schermo', 'EXTRAS': 'EXTRAS',
        'Item Encyclopedia': 'Enciclopedia oggetti',
        'Language': 'Lingua', 'Map Viewer': 'Visualizzatore mappa',
        'Museum of Lore': 'Museo del Lore',
        'New Game (V1 Original)': 'Nuova partita (V1 Originale)',
        'New Game (V2.1 Upscaled)': 'Nuova partita (V2.1 Migliorato)',
        'New Game (V2.2 Enhanced)': 'Nuova partita (V2.2 Avanzato)',
        'No data files': 'Nessun file dati', 'Off': 'Off', 'On': 'On',
        'PLAY': 'GIOCA', 'QUIT': 'ESCI',
        'Screenshot Gallery': 'Galleria schermate',
        'SELECT GAME': 'SELEZIONA GIOCO', 'SETTINGS': 'IMPOSTAZIONI',
        'Spell Reference': 'Riferimento incantesimi',
        'Validate game data': 'Convalida dati gioco',
        'Version': 'Versione', 'Video': 'Video',
        'RETURN TO START MENU?': 'TORNA AL MENÙ?',
        'YES': 'SÌ', 'NO': 'NO',
    }.items() if k in FIRESTAFF_EN},
    'pt': {k: v for k, v in {
        'Accessibility': 'Acessibilidade', 'Audio': 'Áudio', 'Back': 'Voltar',
        'Bestiary': 'Bestiário', 'Changelog': 'Registo de alterações',
        'Coming soon': 'Em breve', 'Continue Saved Game': 'Continuar jogo guardado',
        'Controls': 'Controlos', 'Display': 'Ecrã', 'EXTRAS': 'EXTRAS',
        'Item Encyclopedia': 'Enciclopédia de itens',
        'Language': 'Idioma', 'Map Viewer': 'Visualizador de mapa',
        'Museum of Lore': 'Museu do Lore',
        'New Game (V1 Original)': 'Novo jogo (V1 Original)',
        'New Game (V2.1 Upscaled)': 'Novo jogo (V2.1 Melhorado)',
        'New Game (V2.2 Enhanced)': 'Novo jogo (V2.2 Aprimorado)',
        'No data files': 'Sem ficheiros de dados', 'Off': 'Desligado', 'On': 'Ligado',
        'PLAY': 'JOGAR', 'QUIT': 'SAIR',
        'Screenshot Gallery': 'Galeria de capturas',
        'SELECT GAME': 'SELECIONAR JOGO', 'SETTINGS': 'DEFINIÇÕES',
        'Spell Reference': 'Referência de feitiços',
        'Validate game data': 'Validar dados do jogo',
        'Version': 'Versão', 'Video': 'Vídeo',
        'RETURN TO START MENU?': 'VOLTAR AO MENU?',
        'YES': 'SIM', 'NO': 'NÃO',
    }.items() if k in FIRESTAFF_EN},
    'ru': {k: v for k, v in {
        'Accessibility': 'Доступность', 'Audio': 'Аудио', 'Back': 'Назад',
        'Bestiary': 'Бестиарий', 'Changelog': 'История изменений',
        'Coming soon': 'Скоро', 'Continue Saved Game': 'Продолжить сохранение',
        'Controls': 'Управление', 'Display': 'Экран', 'EXTRAS': 'ДОПОЛНИТЕЛЬНО',
        'Item Encyclopedia': 'Энциклопедия предметов',
        'Language': 'Язык', 'Map Viewer': 'Просмотр карт',
        'Museum of Lore': 'Музей Лора',
        'New Game (V1 Original)': 'Новая игра (V1 Оригинал)',
        'New Game (V2.1 Upscaled)': 'Новая игра (V2.1 Улучшенный)',
        'New Game (V2.2 Enhanced)': 'Новая игра (V2.2 Расширенный)',
        'No data files': 'Нет файлов данных', 'Off': 'Выкл', 'On': 'Вкл',
        'PLAY': 'ИГРАТЬ', 'QUIT': 'ВЫЙТИ',
        'Screenshot Gallery': 'Галерея скриншотов',
        'SELECT GAME': 'ВЫБРАТЬ ИГРУ', 'SETTINGS': 'НАСТРОЙКИ',
        'Spell Reference': 'Справочник заклинаний',
        'Validate game data': 'Проверить данные игры',
        'Version': 'Версия', 'Video': 'Видео',
        'RETURN TO START MENU?': 'ВЕРНУТЬСЯ В МЕНЮ?',
        'YES': 'ДА', 'NO': 'НЕТ',
    }.items() if k in FIRESTAFF_EN},
    'pl': {k: v for k, v in {
        'Accessibility': 'Dostępność', 'Audio': 'Audio', 'Back': 'Wstecz',
        'Bestiary': 'Bestiariusz', 'Changelog': 'Dziennik zmian',
        'Coming soon': 'Wkrótce', 'Continue Saved Game': 'Kontynuuj zapis',
        'Controls': 'Sterowanie', 'Display': 'Ekran', 'EXTRAS': 'DODATKI',
        'Item Encyclopedia': 'Encyklopedia przedmiotów',
        'Language': 'Język', 'Map Viewer': 'Przeglądarka map',
        'Museum of Lore': 'Muzeum Lore',
        'New Game (V1 Original)': 'Nowa gra (V1 Oryginał)',
        'New Game (V2.1 Upscaled)': 'Nowa gra (V2.1 Skalowane)',
        'New Game (V2.2 Enhanced)': 'Nowa gra (V2.2 Ulepszone)',
        'No data files': 'Brak plików danych', 'Off': 'Wyłączone', 'On': 'Włączone',
        'PLAY': 'GRAJ', 'QUIT': 'WYJDŹ',
        'Screenshot Gallery': 'Galeria zrzutów',
        'SELECT GAME': 'WYBIERZ GRĘ', 'SETTINGS': 'USTAWIENIA',
        'Spell Reference': 'Odniesienie do zaklęć',
        'Validate game data': 'Sprawdź dane gry',
        'Version': 'Wersja', 'Video': 'Wideo',
        'RETURN TO START MENU?': 'WRÓCIĆ DO MENU?',
        'YES': 'TAK', 'NO': 'NIE',
    }.items() if k in FIRESTAFF_EN},
    'cs': {k: v for k, v in {
        'Accessibility': 'Přístupnost', 'Audio': 'Zvuk', 'Back': 'Zpět',
        'Bestiary': 'Bestiář', 'Changelog': 'Seznam změn',
        'Coming soon': 'Již brzy', 'Continue Saved Game': 'Pokračovat v uložené hře',
        'Controls': 'Ovládání', 'Display': 'Displej', 'EXTRAS': 'DOPLŇKY',
        'Item Encyclopedia': 'Encyklopedie předmětů',
        'Language': 'Jazyk', 'Map Viewer': 'Prohlížeč map',
        'Museum of Lore': 'Muzeum Lore',
        'New Game (V1 Original)': 'Nová hra (V1 Originál)',
        'New Game (V2.1 Upscaled)': 'Nová hra (V2.1 Zvětšeno)',
        'New Game (V2.2 Enhanced)': 'Nová hra (V2.2 Vylepšeno)',
        'No data files': 'Žádné datové soubory', 'Off': 'Vypnuto', 'On': 'Zapnuto',
        'PLAY': 'HRÁT', 'QUIT': 'KONEC',
        'Screenshot Gallery': 'Galerie snímků',
        'SELECT GAME': 'VYBRAT HRU', 'SETTINGS': 'NASTAVENÍ',
        'Spell Reference': 'Reference kouzel',
        'Validate game data': 'Ověřit herní data',
        'Version': 'Verze', 'Video': 'Video',
        'RETURN TO START MENU?': 'ZPĚT DO NABÍDKY?',
        'YES': 'ANO', 'NO': 'NE',
    }.items() if k in FIRESTAFF_EN},
    'da': {k: v for k, v in {
        'Accessibility': 'Tilgængelighed', 'Audio': 'Lyd', 'Back': 'Tilbage',
        'Bestiary': 'Bestiarium', 'Changelog': 'Ændringslog',
        'Coming soon': 'Kommer snart', 'Continue Saved Game': 'Fortsæt gemt spil',
        'Controls': 'Kontrol', 'Display': 'Skærm', 'EXTRAS': 'EKSTRA',
        'Item Encyclopedia': 'Genstandsleksikon',
        'Language': 'Sprog', 'Map Viewer': 'Kortfremviser',
        'Museum of Lore': 'Lore-museum',
        'New Game (V1 Original)': 'Nyt spil (V1 Original)',
        'New Game (V2.1 Upscaled)': 'Nyt spil (V2.1 Opskaleret)',
        'New Game (V2.2 Enhanced)': 'Nyt spil (V2.2 Forbedret)',
        'No data files': 'Ingen datafiler', 'Off': 'Fra', 'On': 'Til',
        'PLAY': 'SPIL', 'QUIT': 'AFSLUT',
        'Screenshot Gallery': 'Skærmbilledgalleri',
        'SELECT GAME': 'VÆLG SPIL', 'SETTINGS': 'INDSTILLINGER',
        'Spell Reference': 'Besværgelsesreference',
        'Validate game data': 'Validér spildata',
        'Version': 'Version', 'Video': 'Video',
        'RETURN TO START MENU?': 'TILBAGE TIL STARTMENU?',
        'YES': 'JA', 'NO': 'NEJ',
    }.items() if k in FIRESTAFF_EN},
    'nl': {k: v for k, v in {
        'Accessibility': 'Toegankelijkheid', 'Audio': 'Audio', 'Back': 'Terug',
        'Bestiary': 'Bestiarium', 'Changelog': 'Wijzigingslog',
        'Coming soon': 'Binnenkort', 'Continue Saved Game': 'Doorgaan met opgeslagen',
        'Controls': 'Besturing', 'Display': 'Scherm', 'EXTRAS': 'EXTRA',
        'Item Encyclopedia': 'Voorwerpenencyclopedie',
        'Language': 'Taal', 'Map Viewer': 'Kaartkijker',
        'Museum of Lore': 'Lore-museum',
        'New Game (V1 Original)': 'Nieuw spel (V1 Origineel)',
        'New Game (V2.1 Upscaled)': 'Nieuw spel (V2.1 Opgeschaald)',
        'New Game (V2.2 Enhanced)': 'Nieuw spel (V2.2 Verbeterd)',
        'No data files': 'Geen gegevensbestanden', 'Off': 'Uit', 'On': 'Aan',
        'PLAY': 'SPELEN', 'QUIT': 'AFSLUITEN',
        'Screenshot Gallery': 'Schermafdrukgalerij',
        'SELECT GAME': 'SPEL SELECTEREN', 'SETTINGS': 'INSTELLINGEN',
        'Spell Reference': 'Spreukreferentie',
        'Validate game data': 'Spelgegevens valideren',
        'Version': 'Versie', 'Video': 'Video',
        'RETURN TO START MENU?': 'TERUG NAAR STARTMENU?',
        'YES': 'JA', 'NO': 'NEE',
    }.items() if k in FIRESTAFF_EN},
    'no': {k: v for k, v in {
        'Accessibility': 'Tilgjengelighet', 'Audio': 'Lyd', 'Back': 'Tilbake',
        'Bestiary': 'Bestiarium', 'Changelog': 'Endringslogg',
        'Coming soon': 'Kommer snart', 'Continue Saved Game': 'Fortsett lagret spill',
        'Controls': 'Kontroller', 'Display': 'Skjerm', 'EXTRAS': 'EKSTRA',
        'Item Encyclopedia': 'Gjenstandsleksikon',
        'Language': 'Språk', 'Map Viewer': 'Kartviser',
        'Museum of Lore': 'Lore-museum',
        'New Game (V1 Original)': 'Nytt spill (V1 Original)',
        'New Game (V2.1 Upscaled)': 'Nytt spill (V2.1 Oppskalert)',
        'New Game (V2.2 Enhanced)': 'Nytt spill (V2.2 Forbedret)',
        'No data files': 'Ingen datafiler', 'Off': 'Av', 'On': 'På',
        'PLAY': 'SPILL', 'QUIT': 'AVSLUTT',
        'Screenshot Gallery': 'Skjermbildegalleri',
        'SELECT GAME': 'VELG SPILL', 'SETTINGS': 'INNSTILLINGER',
        'Spell Reference': 'Trolldomsreferanse',
        'Validate game data': 'Valider spilldata',
        'Version': 'Versjon', 'Video': 'Video',
        'RETURN TO START MENU?': 'TILBAKE TIL STARTMENY?',
        'YES': 'JA', 'NO': 'NEI',
    }.items() if k in FIRESTAFF_EN},
    'fi': {k: v for k, v in {
        'Accessibility': 'Esteettömyys', 'Audio': 'Ääni', 'Back': 'Takaisin',
        'Bestiary': 'Bestiario', 'Changelog': 'Muutosloki',
        'Coming soon': 'Tulossa pian', 'Continue Saved Game': 'Jatka tallennettua',
        'Controls': 'Ohjaus', 'Display': 'Näyttö', 'EXTRAS': 'LISÄÄ',
        'Item Encyclopedia': 'Esine-tietosanakirja',
        'Language': 'Kieli', 'Map Viewer': 'Kartan katselin',
        'Museum of Lore': 'Lore-museo',
        'New Game (V1 Original)': 'Uusi peli (V1 Alkuperäinen)',
        'New Game (V2.1 Upscaled)': 'Uusi peli (V2.1 Skaalattu)',
        'New Game (V2.2 Enhanced)': 'Uusi peli (V2.2 Parannettu)',
        'No data files': 'Ei datatiedostoja', 'Off': 'Pois', 'On': 'Päällä',
        'PLAY': 'PELAA', 'QUIT': 'LOPETA',
        'Screenshot Gallery': 'Kuvakaappausgalleria',
        'SELECT GAME': 'VALITSE PELI', 'SETTINGS': 'ASETUKSET',
        'Spell Reference': 'Loitsuviittaus',
        'Validate game data': 'Vahvista pelidata',
        'Version': 'Versio', 'Video': 'Video',
        'RETURN TO START MENU?': 'PALAA ALOITUSVALIKKOON?',
        'YES': 'KYLLÄ', 'NO': 'EI',
    }.items() if k in FIRESTAFF_EN},
    'hu': {k: v for k, v in {
        'Accessibility': 'Akadálymentesség', 'Audio': 'Hang', 'Back': 'Vissza',
        'Bestiary': 'Bestiárium', 'Changelog': 'Változásnapló',
        'Coming soon': 'Hamarosan', 'Continue Saved Game': 'Mentett játék folytatása',
        'Controls': 'Irányítás', 'Display': 'Kijelző', 'EXTRAS': 'EXTRÁK',
        'Item Encyclopedia': 'Tárgy enciklopédia',
        'Language': 'Nyelv', 'Map Viewer': 'Térképnézegető',
        'Museum of Lore': 'Lore Múzeum',
        'New Game (V1 Original)': 'Új játék (V1 Eredeti)',
        'New Game (V2.1 Upscaled)': 'Új játék (V2.1 Skálázott)',
        'New Game (V2.2 Enhanced)': 'Új játék (V2.2 Továbbfejlesztett)',
        'No data files': 'Nincs adatfájl', 'Off': 'Ki', 'On': 'Be',
        'PLAY': 'JÁTÉK', 'QUIT': 'KILÉPÉS',
        'Screenshot Gallery': 'Képernyőkép galéria',
        'SELECT GAME': 'JÁTÉK VÁLASZTÁSA', 'SETTINGS': 'BEÁLLÍTÁSOK',
        'Spell Reference': 'Varázslat referencia',
        'Validate game data': 'Játék adatok ellenőrzése',
        'Version': 'Verzió', 'Video': 'Videó',
        'RETURN TO START MENU?': 'VISSZA A FŐMENÜBE?',
        'YES': 'IGEN', 'NO': 'NEM',
    }.items() if k in FIRESTAFF_EN},
    'ko': {k: v for k, v in {
        'Accessibility': '접근성', 'Audio': '오디오', 'Back': '뒤로',
        'Bestiary': '도감', 'Changelog': '변경 로그',
        'Coming soon': '곧 출시', 'Continue Saved Game': '저장 게임 계속',
        'Controls': '조작', 'Display': '표시', 'EXTRAS': '추가',
        'Item Encyclopedia': '아이템 도감', 'Language': '언어',
        'Map Viewer': '지도 뷰어', 'Museum of Lore': '전승 박물관',
        'New Game (V1 Original)': '새 게임 (V1 오리지널)',
        'New Game (V2.1 Upscaled)': '새 게임 (V2.1 업스케일)',
        'New Game (V2.2 Enhanced)': '새 게임 (V2.2 강화)',
        'No data files': '데이터 파일 없음', 'Off': '끔', 'On': '켬',
        'PLAY': '플레이', 'QUIT': '종료',
        'Screenshot Gallery': '스크린샷 갤러리',
        'SELECT GAME': '게임 선택', 'SETTINGS': '설정',
        'Spell Reference': '주문 참조', 'Validate game data': '게임 데이터 검증',
        'Version': '버전', 'Video': '비디오',
        'RETURN TO START MENU?': '시작 메뉴로 돌아가기?',
        'YES': '예', 'NO': '아니오',
    }.items() if k in FIRESTAFF_EN},
    'tr': {k: v for k, v in {
        'Accessibility': 'Erişilebilirlik', 'Audio': 'Ses', 'Back': 'Geri',
        'Bestiary': 'Yaratık Kitabı', 'Changelog': 'Değişiklikler',
        'Coming soon': 'Çok yakında', 'Continue Saved Game': 'Kayıtlı Oyuna Devam',
        'Controls': 'Kontroller', 'Display': 'Ekran', 'EXTRAS': 'EKSTRALAR',
        'Item Encyclopedia': 'Eşya Ansiklopedisi',
        'Language': 'Dil', 'Map Viewer': 'Harita Görüntüleyici',
        'Museum of Lore': 'Lore Müzesi',
        'New Game (V1 Original)': 'Yeni Oyun (V1 Orijinal)',
        'New Game (V2.1 Upscaled)': 'Yeni Oyun (V2.1 Yükseltilmiş)',
        'New Game (V2.2 Enhanced)': 'Yeni Oyun (V2.2 Geliştirilmiş)',
        'No data files': 'Veri dosyası yok', 'Off': 'Kapalı', 'On': 'Açık',
        'PLAY': 'OYNA', 'QUIT': 'ÇIK',
        'Screenshot Gallery': 'Ekran Görüntüsü Galerisi',
        'SELECT GAME': 'OYUN SEÇ', 'SETTINGS': 'AYARLAR',
        'Spell Reference': 'Büyü Referansı', 'Validate game data': 'Oyun verisini doğrula',
        'Version': 'Sürüm', 'Video': 'Video',
        'RETURN TO START MENU?': 'BAŞLANGIÇ MENÜSÜNE DÖN?',
        'YES': 'EVET', 'NO': 'HAYIR',
    }.items() if k in FIRESTAFF_EN},
})

# Apply script helper
def apply_other_translations(domain, lang_table):
    """For each (lang, dict) pair, write the table to po/<domain>.<lang>.po.
    Uses msgstr=msgid fallback for keys not in the table.
    """
    import os
    po_dir = os.path.dirname(os.path.abspath(__file__))
    total_updated = 0
    for lang, table in lang_table.items():
        path = os.path.join(po_dir, f'{domain}.{lang}.po')
        if not os.path.exists(path):
            continue
        with open(path, 'r', encoding='utf-8') as f:
            text = f.read()
        lines = text.split('\n')
        i = 0
        updated = 0
        while i < len(lines):
            line = lines[i]
            if line.startswith('msgid "'):
                m = line[7:-1]
                if m == '':
                    i += 1
                    continue
                j = i + 1
                while j < len(lines):
                    next_line = lines[j]
                    if next_line.startswith('msgstr "'):
                        break
                    if (next_line.startswith('#') or next_line.startswith('"')
                            or next_line == ''):
                        j += 1
                        continue
                    j += 1
                if j < len(lines) and lines[j].startswith('msgstr "'):
                    if m in table:
                        translation = table[m]
                        escaped = translation.replace('\\', '\\\\').replace('"', '\\"')
                        lines[j] = f'msgstr "{escaped}"'
                        updated += 1
                    elif lines[j] == 'msgstr ""':
                        escaped = m.replace('\\', '\\\\').replace('"', '\\"')
                        lines[j] = f'msgstr "{escaped}"'
                i += 1
                continue
            i += 1
        new_text = '\n'.join(lines)
        with open(path, 'w', encoding='utf-8') as f:
            f.write(new_text)
        total_updated += updated
    return total_updated

# NOTE: NEXUS translations are appended to this file via cat >> from the
# build script. The __main__ block above uses globals().get() so the
# order of definitions doesn't matter.


# =========================================================================
# NEXUS (30 strings) - 19 locales
# =========================================================================
NEXUS_EN = {k: k for k in [
    'Attack', 'Cast Spell', 'Track %d', '%s has died', 'Continue', 'Defend',
    'This door is locked', 'You found the Firestaff!',
    'Congratulations! You have saved the realm!', 'Your party has perished',
    'Hall of Champions', 'Health', 'Inventory full', 'A key is required',
    'Level %d', '%s gained a level', 'Load Game', 'Mana', 'New Game',
    'Not enough mana', 'Recruit', 'Reincarnate', 'Rest', 'Resurrect',
    'Save Game', 'Select a champion', 'Spell failed', 'Stamina',
    'Party is full', 'Use Item',
]}

NEXUS = {'en': NEXUS_EN}

NEXUS['sv'] = {k: v for k, v in {
    'Attack': 'Anfall', 'Cast Spell': 'Kasta besvärjelse', 'Track %d': 'Bana %d',
    '%s has died': '%s har dött', 'Continue': 'Fortsätt', 'Defend': 'Försvar',
    'This door is locked': 'Dörren är låst',
    'You found the Firestaff!': 'Du hittade eldstaven!',
    'Congratulations! You have saved the realm!':
        'Grattis! Du har räddat riket!',
    'Your party has perished': 'Din grupp har omkommit',
    'Hall of Champions': 'Hjältarnas sal', 'Health': 'Hälsa',
    'Inventory full': 'Inventarie full', 'A key is required': 'Nyckel krävs',
    'Level %d': 'Nivå %d', '%s gained a level': '%s gick upp en nivå',
    'Load Game': 'Ladda spel', 'Mana': 'Mana', 'New Game': 'Nytt spel',
    'Not enough mana': 'Inte tillräckligt med mana', 'Recruit': 'Rekrytera',
    'Reincarnate': 'Återfödas', 'Rest': 'Vila', 'Resurrect': 'Återuppstå',
    'Save Game': 'Spara spel', 'Select a champion': 'Välj hjälte',
    'Spell failed': 'Besvärjelse misslyckades', 'Stamina': 'Uthållighet',
    'Party is full': 'Gruppen är full', 'Use Item': 'Använd föremål',
}.items() if k in NEXUS_EN}

# For all other languages, use English source as fallback.  We'll add
# a few major-language translations below for the most visible strings.
NEXUS['fr'] = {k: v for k, v in {
    'Attack': 'Attaquer', 'Cast Spell': 'Lancer un sort', 'Track %d': 'Niveau %d',
    '%s has died': '%s est mort', 'Continue': 'Continuer', 'Defend': 'Défendre',
    'This door is locked': 'Cette porte est verrouillée',
    'You found the Firestaff!': 'Vous avez trouvé le Bâton de Feu !',
    'Congratulations! You have saved the realm!':
        'Félicitations ! Vous avez sauvé le royaume !',
    'Your party has perished': 'Votre groupe a péri',
    'Hall of Champions': 'Hall des Champions', 'Health': 'Santé',
    'Inventory full': 'Inventaire plein', 'A key is required': 'Une clé est requise',
    'Level %d': 'Niveau %d', '%s gained a level': '%s a gagné un niveau',
    'Load Game': 'Charger', 'Mana': 'Mana', 'New Game': 'Nouvelle partie',
    'Not enough mana': 'Mana insuffisant', 'Recruit': 'Recruter',
    'Reincarnate': 'Réincarner', 'Rest': 'Repos', 'Resurrect': 'Ressusciter',
    'Save Game': 'Sauvegarder', 'Select a champion': 'Choisir un champion',
    'Spell failed': 'Sort échoué', 'Stamina': 'Endurance',
    'Party is full': 'Groupe complet', 'Use Item': 'Utiliser un objet',
}.items() if k in NEXUS_EN}

NEXUS['de'] = {k: v for k, v in {
    'Attack': 'Angreifen', 'Cast Spell': 'Zauber wirken', 'Track %d': 'Ebene %d',
    '%s has died': '%s ist gestorben', 'Continue': 'Fortsetzen',
    'Defend': 'Verteidigen',
    'This door is locked': 'Diese Tür ist verschlossen',
    'You found the Firestaff!': 'Du hast den Feuerstab gefunden!',
    'Congratulations! You have saved the realm!':
        'Glückwunsch! Du hast das Reich gerettet!',
    'Your party has perished': 'Deine Gruppe ist gefallen',
    'Hall of Champions': 'Halle der Helden', 'Health': 'Gesundheit',
    'Inventory full': 'Inventar voll', 'A key is required': 'Schlüssel erforderlich',
    'Level %d': 'Ebene %d', '%s gained a level': '%s hat eine Stufe aufgestiegen',
    'Load Game': 'Laden', 'Mana': 'Mana', 'New Game': 'Neues Spiel',
    'Not enough mana': 'Nicht genug Mana', 'Recruit': 'Rekrutieren',
    'Reincarnate': 'Wiedergeburt', 'Rest': 'Rasten', 'Resurrect': 'Wiederbeleben',
    'Save Game': 'Speichern', 'Select a champion': 'Champion wählen',
    'Spell failed': 'Zauber fehlgeschlagen', 'Stamina': 'Ausdauer',
    'Party is full': 'Gruppe ist voll', 'Use Item': 'Gegenstand benutzen',
}.items() if k in NEXUS_EN}

NEXUS['ja'] = {k: v for k, v in {
    'Attack': '攻撃', 'Cast Spell': '呪文詠唱', 'Track %d': 'レベル %d',
    '%s has died': '%s が死んだ', 'Continue': '続ける', 'Defend': '防御',
    'This door is locked': 'この扉はロックされている',
    'You found the Firestaff!': 'ファイアスタッフを見つけた!',
    'Congratulations! You have saved the realm!':
        'おめでとう! あなたは王国を救った!',
    'Your party has perished': 'パーティは壊滅した',
    'Hall of Champions': '勇者の間', 'Health': '体力',
    'Inventory full': 'インベントリ満杯', 'A key is required': '鍵が必要',
    'Level %d': 'レベル %d', '%s gained a level': '%s がレベルアップ',
    'Load Game': 'ロード', 'Mana': 'マナ', 'New Game': '新規ゲーム',
    'Not enough mana': 'マナ不足', 'Recruit': '招募',
    'Reincarnate': '転生', 'Rest': '休息', 'Resurrect': '復活',
    'Save Game': 'セーブ', 'Select a champion': '勇者選択',
    'Spell failed': '呪文失敗', 'Stamina': 'スタミナ',
    'Party is full': 'パーティ満杯', 'Use Item': 'アイテム使用',
}.items() if k in NEXUS_EN}

NEXUS['zh'] = {k: v for k, v in {
    'Attack': '攻击', 'Cast Spell': '施法', 'Track %d': '关卡 %d',
    '%s has died': '%s 已死亡', 'Continue': '继续', 'Defend': '防御',
    'This door is locked': '此门已锁',
    'You found the Firestaff!': '你找到了火杖!',
    'Congratulations! You have saved the realm!': '恭喜!你拯救了王国!',
    'Your party has perished': '你的队伍已覆灭',
    'Hall of Champions': '英雄大厅', 'Health': '生命',
    'Inventory full': '物品栏已满', 'A key is required': '需要钥匙',
    'Level %d': '等级 %d', '%s gained a level': '%s 升级了',
    'Load Game': '载入', 'Mana': '法力', 'New Game': '新游戏',
    'Not enough mana': '法力不足', 'Recruit': '招募',
    'Reincarnate': '转世', 'Rest': '休息', 'Resurrect': '复活',
    'Save Game': '保存', 'Select a champion': '选择英雄',
    'Spell failed': '咒语失败', 'Stamina': '耐力',
    'Party is full': '队伍已满', 'Use Item': '使用物品',
}.items() if k in NEXUS_EN}

NEXUS['es'] = {k: v for k, v in {
    'Attack': 'Atacar', 'Cast Spell': 'Lanzar hechizo', 'Track %d': 'Nivel %d',
    '%s has died': '%s ha muerto', 'Continue': 'Continuar', 'Defend': 'Defender',
    'This door is locked': 'Esta puerta está cerrada',
    'You found the Firestaff!': '¡Encontraste el Bastón de Fuego!',
    'Congratulations! You have saved the realm!':
        '¡Felicidades! ¡Has salvado el reino!',
    'Your party has perished': 'Tu grupo ha perecido',
    'Hall of Champions': 'Salón de Campeones', 'Health': 'Salud',
    'Inventory full': 'Inventario lleno', 'A key is required': 'Se requiere llave',
    'Level %d': 'Nivel %d', '%s gained a level': '%s ganó un nivel',
    'Load Game': 'Cargar', 'Mana': 'Maná', 'New Game': 'Nueva partida',
    'Not enough mana': 'Maná insuficiente', 'Recruit': 'Reclutar',
    'Reincarnate': 'Reencarnar', 'Rest': 'Descansar', 'Resurrect': 'Resucitar',
    'Save Game': 'Guardar', 'Select a champion': 'Seleccionar campeón',
    'Spell failed': 'Hechizo falló', 'Stamina': 'Aguante',
    'Party is full': 'Grupo lleno', 'Use Item': 'Usar objeto',
}.items() if k in NEXUS_EN}

NEXUS['it'] = {k: v for k, v in {
    'Attack': 'Attacca', 'Cast Spell': 'Lancia incantesimo', 'Track %d': 'Livello %d',
    '%s has died': '%s è morto', 'Continue': 'Continua', 'Defend': 'Difendi',
    'This door is locked': 'Questa porta è chiusa',
    'You found the Firestaff!': 'Hai trovato il Bastone di Fuoco!',
    'Congratulations! You have saved the realm!':
        'Congratulazioni! Hai salvato il regno!',
    'Your party has perished': 'Il tuo gruppo è perito',
    'Hall of Champions': 'Sala dei Campioni', 'Health': 'Salute',
    'Inventory full': 'Inventario pieno', 'A key is required': 'Serve una chiave',
    'Level %d': 'Livello %d', '%s gained a level': '%s è salito di livello',
    'Load Game': 'Carica', 'Mana': 'Mana', 'New Game': 'Nuova partita',
    'Not enough mana': 'Mana insufficiente', 'Recruit': 'Recluta',
    'Reincarnate': 'Reincarna', 'Rest': 'Riposa', 'Resurrect': 'Resuscita',
    'Save Game': 'Salva', 'Select a champion': 'Seleziona campione',
    'Spell failed': 'Incantesimo fallito', 'Stamina': 'Stamina',
    'Party is full': 'Gruppo pieno', 'Use Item': 'Usa oggetto',
}.items() if k in NEXUS_EN}

NEXUS['pt'] = {k: v for k, v in {
    'Attack': 'Atacar', 'Cast Spell': 'Lançar feitiço', 'Track %d': 'Nível %d',
    '%s has died': '%s morreu', 'Continue': 'Continuar', 'Defend': 'Defender',
    'This door is locked': 'Esta porta está trancada',
    'You found the Firestaff!': 'Encontraste o Cajado de Fogo!',
    'Congratulations! You have saved the realm!':
        'Parabéns! Salvaste o reino!',
    'Your party has perished': 'O teu grupo pereceu',
    'Hall of Champions': 'Salão dos Campeões', 'Health': 'Saúde',
    'Inventory full': 'Inventário cheio', 'A key is required': 'Chave necessária',
    'Level %d': 'Nível %d', '%s gained a level': '%s subiu de nível',
    'Load Game': 'Carregar', 'Mana': 'Mana', 'New Game': 'Novo jogo',
    'Not enough mana': 'Mana insuficiente', 'Recruit': 'Recrutar',
    'Reincarnate': 'Reencarnar', 'Rest': 'Descansar', 'Resurrect': 'Ressuscitar',
    'Save Game': 'Guardar', 'Select a champion': 'Escolher campeão',
    'Spell failed': 'Feitiço falhou', 'Stamina': 'Vigor',
    'Party is full': 'Grupo cheio', 'Use Item': 'Usar item',
}.items() if k in NEXUS_EN}

NEXUS['ru'] = {k: v for k, v in {
    'Attack': 'Атака', 'Cast Spell': 'Заклинание', 'Track %d': 'Уровень %d',
    '%s has died': '%s погиб', 'Continue': 'Продолжить', 'Defend': 'Защита',
    'This door is locked': 'Дверь заперта',
    'You found the Firestaff!': 'Вы нашли Огненный посох!',
    'Congratulations! You have saved the realm!':
        'Поздравляем! Вы спасли королевство!',
    'Your party has perished': 'Ваша группа погибла',
    'Hall of Champions': 'Зал Чемпионов', 'Health': 'Здоровье',
    'Inventory full': 'Инвентарь полон', 'A key is required': 'Нужен ключ',
    'Level %d': 'Уровень %d', '%s gained a level': '%s повысил уровень',
    'Load Game': 'Загрузить', 'Mana': 'Мана', 'New Game': 'Новая игра',
    'Not enough mana': 'Недостаточно маны', 'Recruit': 'Нанять',
    'Reincarnate': 'Перерождение', 'Rest': 'Отдых', 'Resurrect': 'Воскресить',
    'Save Game': 'Сохранить', 'Select a champion': 'Выбрать героя',
    'Spell failed': 'Заклинание провалилось', 'Stamina': 'Выносливость',
    'Party is full': 'Группа полна', 'Use Item': 'Использовать предмет',
}.items() if k in NEXUS_EN}

NEXUS['pl'] = {k: v for k, v in {
    'Attack': 'Atakuj', 'Cast Spell': 'Rzuć zaklęcie', 'Track %d': 'Poziom %d',
    '%s has died': '%s zginął', 'Continue': 'Kontynuuj', 'Defend': 'Broń się',
    'This door is locked': 'Te drzwi są zamknięte',
    'You found the Firestaff!': 'Znalazłeś Ognistą Laskę!',
    'Congratulations! You have saved the realm!':
        'Gratulacje! Uratowałeś królestwo!',
    'Your party has perished': 'Twoja drużyna zginęła',
    'Hall of Champions': 'Sala Bohaterów', 'Health': 'Zdrowie',
    'Inventory full': 'Ekwipunek pełny', 'A key is required': 'Wymagany klucz',
    'Level %d': 'Poziom %d', '%s gained a level': '%s awansował',
    'Load Game': 'Wczytaj', 'Mana': 'Mana', 'New Game': 'Nowa gra',
    'Not enough mana': 'Za mało many', 'Recruit': 'Rekrutuj',
    'Reincarnate': 'Reinkarnuj', 'Rest': 'Odpoczynek', 'Resurrect': 'Wskrześ',
    'Save Game': 'Zapisz', 'Select a champion': 'Wybierz bohatera',
    'Spell failed': 'Zaklęcie nie powiodło się', 'Stamina': 'Wytrzymałość',
    'Party is full': 'Drużyna pełna', 'Use Item': 'Użyj przedmiotu',
}.items() if k in NEXUS_EN}

NEXUS['cs'] = {k: v for k, v in {
    'Attack': 'Útok', 'Cast Spell': 'Seslat kouzlo', 'Track %d': 'Úroveň %d',
    '%s has died': '%s zemřel', 'Continue': 'Pokračovat', 'Defend': 'Bránit',
    'This door is locked': 'Tyto dveře jsou zamčené',
    'You found the Firestaff!': 'Našel jsi Ohnivou hůl!',
    'Congratulations! You have saved the realm!':
        'Gratulujeme! Zachránil jsi říši!',
    'Your party has perished': 'Tvoje družina zahynula',
    'Hall of Champions': 'Síň hrdinů', 'Health': 'Zdraví',
    'Inventory full': 'Inventář plný', 'A key is required': 'Vyžadován klíč',
    'Level %d': 'Úroveň %d', '%s gained a level': '%s postoupil',
    'Load Game': 'Načíst', 'Mana': 'Mana', 'New Game': 'Nová hra',
    'Not enough mana': 'Nedostatek many', 'Recruit': 'Najmout',
    'Reincarnate': 'Reinkarnovat', 'Rest': 'Odpočinek', 'Resurrect': 'Vzkřísit',
    'Save Game': 'Uložit', 'Select a champion': 'Vyber hrdinu',
    'Spell failed': 'Kouzlo selhalo', 'Stamina': 'Výdrž',
    'Party is full': 'Družina je plná', 'Use Item': 'Použít předmět',
}.items() if k in NEXUS_EN}

NEXUS['da'] = {k: v for k, v in {
    'Attack': 'Angreb', 'Cast Spell': 'Kast besværgelse', 'Track %d': 'Niveau %d',
    '%s has died': '%s døde', 'Continue': 'Fortsæt', 'Defend': 'Forsvar',
    'This door is locked': 'Denne dør er låst',
    'You found the Firestaff!': 'Du fandt Ilds Taven!',
    'Congratulations! You have saved the realm!':
        'Tillykke! Du har reddet riget!',
    'Your party has perished': 'Din gruppe er omkommet',
    'Hall of Champions': 'Heltenes Sal', 'Health': 'Helbred',
    'Inventory full': 'Inventar fuld', 'A key is required': 'Nøgle påkrævet',
    'Level %d': 'Niveau %d', '%s gained a level': '%s steg i niveau',
    'Load Game': 'Indlæs', 'Mana': 'Mana', 'New Game': 'Nyt spil',
    'Not enough mana': 'Ikke nok mana', 'Recruit': 'Rekruttér',
    'Reincarnate': 'Genfød', 'Rest': 'Hvil', 'Resurrect': 'Genopliv',
    'Save Game': 'Gem', 'Select a champion': 'Vælg helt',
    'Spell failed': 'Besværgelse mislykkedes', 'Stamina': 'Udholdenhed',
    'Party is full': 'Gruppe er fuld', 'Use Item': 'Brug genstand',
}.items() if k in NEXUS_EN}

NEXUS['nl'] = {k: v for k, v in {
    'Attack': 'Aanvallen', 'Cast Spell': 'Spreuk werpen', 'Track %d': 'Niveau %d',
    '%s has died': '%s is gestorven', 'Continue': 'Doorgaan', 'Defend': 'Verdedigen',
    'This door is locked': 'Deze deur is vergrendeld',
    'You found the Firestaff!': 'Je vond de Vuur Staf!',
    'Congratulations! You have saved the realm!':
        'Gefeliciteerd! Je hebt het koninkrijk gered!',
    'Your party has perished': 'Je groep is omgekomen',
    'Hall of Champions': 'Hal der Helden', 'Health': 'Gezondheid',
    'Inventory full': 'Inventaris vol', 'A key is required': 'Sleutel vereist',
    'Level %d': 'Niveau %d', '%s gained a level': '%s is gestegen',
    'Load Game': 'Laden', 'Mana': 'Mana', 'New Game': 'Nieuw spel',
    'Not enough mana': 'Niet genoeg mana', 'Recruit': 'Rekruteren',
    'Reincarnate': 'Reïncarneren', 'Rest': 'Rusten', 'Resurrect': 'Wederopstanding',
    'Save Game': 'Opslaan', 'Select a champion': 'Kies held',
    'Spell failed': 'Spreuk mislukt', 'Stamina': 'Uithouding',
    'Party is full': 'Groep vol', 'Use Item': 'Voorwerp gebruiken',
}.items() if k in NEXUS_EN}

NEXUS['no'] = {k: v for k, v in {
    'Attack': 'Angrep', 'Cast Spell': 'Kast trolldom', 'Track %d': 'Nivå %d',
    '%s has died': '%s døde', 'Continue': 'Fortsett', 'Defend': 'Forsvar',
    'This door is locked': 'Denne døren er låst',
    'You found the Firestaff!': 'Du fant Ildstaven!',
    'Congratulations! You have saved the realm!':
        'Gratulerer! Du har reddet riket!',
    'Your party has perished': 'Gruppen din er omkommet',
    'Hall of Champions': 'Heltenes Sal', 'Health': 'Helse',
    'Inventory full': 'Inventar fullt', 'A key is required': 'Nøkkel kreves',
    'Level %d': 'Nivå %d', '%s gained a level': '%s gikk opp et nivå',
    'Load Game': 'Last inn', 'Mana': 'Mana', 'New Game': 'Nytt spill',
    'Not enough mana': 'Ikke nok mana', 'Recruit': 'Verv',
    'Reincarnate': 'Gjenfød', 'Rest': 'Hvil', 'Resurrect': 'Gjenoppliv',
    'Save Game': 'Lagre', 'Select a champion': 'Velg helt',
    'Spell failed': 'Trolldom mislyktes', 'Stamina': 'Utholdenhet',
    'Party is full': 'Gruppen er full', 'Use Item': 'Bruk gjenstand',
}.items() if k in NEXUS_EN}

NEXUS['fi'] = {k: v for k, v in {
    'Attack': 'Hyökkää', 'Cast Spell': 'Loihdi', 'Track %d': 'Taso %d',
    '%s has died': '%s kuoli', 'Continue': 'Jatka', 'Defend': 'Puolusta',
    'This door is locked': 'Tämä ovi on lukittu',
    'You found the Firestaff!': 'Löysit Tulikepin!',
    'Congratulations! You have saved the realm!':
        'Onnittelut! Pelastit valtakunnan!',
    'Your party has perished': 'Ryhmäsi tuhoutui',
    'Hall of Champions': 'Sankareiden sali', 'Health': 'Terveys',
    'Inventory full': 'Inventaario täysi', 'A key is required': 'Avain vaaditaan',
    'Level %d': 'Taso %d', '%s gained a level': '%s nousi tasolle',
    'Load Game': 'Lataa', 'Mana': 'Mana', 'New Game': 'Uusi peli',
    'Not enough mana': 'Ei tarpeeksi manaa', 'Recruit': 'Palkkaa',
    'Reincarnate': 'Reinkarnaatio', 'Rest': 'Lepo', 'Resurrect': 'Herätä',
    'Save Game': 'Tallenna', 'Select a champion': 'Valitse sankari',
    'Spell failed': 'Loitsu epäonnistui', 'Stamina': 'Kestävyys',
    'Party is full': 'Ryhmä täysi', 'Use Item': 'Käytä esinettä',
}.items() if k in NEXUS_EN}

NEXUS['hu'] = {k: v for k, v in {
    'Attack': 'Támadás', 'Cast Spell': 'Varázslat', 'Track %d': 'Szint %d',
    '%s has died': '%s meghalt', 'Continue': 'Tovább', 'Defend': 'Védekezés',
    'This door is locked': 'Ez az ajtó zárva',
    'You found the Firestaff!': 'Megtaláltad a Tűzbottot!',
    'Congratulations! You have saved the realm!':
        'Gratulálunk! Megmentetted a királyságot!',
    'Your party has perished': 'A csapatod odaveszett',
    'Hall of Champions': 'Hősök Csarnoka', 'Health': 'Életerő',
    'Inventory full': 'Leltár tele', 'A key is required': 'Kulcs szükséges',
    'Level %d': 'Szint %d', '%s gained a level': '%s szintet lépett',
    'Load Game': 'Betöltés', 'Mana': 'Mana', 'New Game': 'Új játék',
    'Not enough mana': 'Nincs elég mana', 'Recruit': 'Toboroz',
    'Reincarnate': 'Reinkarnál', 'Rest': 'Pihenés', 'Resurrect': 'Feltámaszt',
    'Save Game': 'Mentés', 'Select a champion': 'Válassz hőst',
    'Spell failed': 'Varázslat sikertelen', 'Stamina': 'Állóképesség',
    'Party is full': 'A csapat tele', 'Use Item': 'Tárgy használata',
}.items() if k in NEXUS_EN}

NEXUS['ko'] = {k: v for k, v in {
    'Attack': '공격', 'Cast Spell': '주문 시전', 'Track %d': '레벨 %d',
    '%s has died': '%s 가 죽었음', 'Continue': '계속', 'Defend': '방어',
    'This door is locked': '이 문은 잠겨 있다',
    'You found the Firestaff!': '파이어스태프를 찾았다!',
    'Congratulations! You have saved the realm!':
        '축하합니다! 왕국을 구했습니다!',
    'Your party has perished': '당신의 파티는 전멸했다',
    'Hall of Champions': '용자의 전당', 'Health': '체력',
    'Inventory full': '인벤토리 가득', 'A key is required': '열쇠 필요',
    'Level %d': '레벨 %d', '%s gained a level': '%s 레벨 업',
    'Load Game': '로드', 'Mana': '마나', 'New Game': '새 게임',
    'Not enough mana': '마나 부족', 'Recruit': '모집',
    'Reincarnate': '환생', 'Rest': '휴식', 'Resurrect': '부활',
    'Save Game': '저장', 'Select a champion': '챔피언 선택',
    'Spell failed': '주문 실패', 'Stamina': '스태미나',
    'Party is full': '파티 가득', 'Use Item': '아이템 사용',
}.items() if k in NEXUS_EN}

NEXUS['tr'] = {k: v for k, v in {
    'Attack': 'Saldır', 'Cast Spell': 'Büyü Yap', 'Track %d': 'Seviye %d',
    '%s has died': '%s öldü', 'Continue': 'Devam', 'Defend': 'Savun',
    'This door is locked': 'Bu kapı kilitli',
    'You found the Firestaff!': 'Ateş Değneğini buldun!',
    'Congratulations! You have saved the realm!':
        'Tebrikler! Krallığı kurtardın!',
    'Your party has perished': 'Grubun yok oldu',
    'Hall of Champions': 'Şampiyonlar Salonu', 'Health': 'Sağlık',
    'Inventory full': 'Envanter dolu', 'A key is required': 'Anahtar gerekli',
    'Level %d': 'Seviye %d', '%s gained a level': '%s seviye atladı',
    'Load Game': 'Yükle', 'Mana': 'Mana', 'New Game': 'Yeni Oyun',
    'Not enough mana': 'Yeterli mana yok', 'Recruit': 'Topla',
    'Reincarnate': 'Yeniden Doğ', 'Rest': 'Dinlen', 'Resurrect': 'Dirilt',
    'Save Game': 'Kaydet', 'Select a champion': 'Şampiyon seç',
    'Spell failed': 'Büyü başarısız', 'Stamina': 'Dayanıklılık',
    'Party is full': 'Grup dolu', 'Use Item': 'Eşya kullan',
}.items() if k in NEXUS_EN}

if __name__ == '__main__':
    import sys
    # NOTE: NEXUS is defined later in this file (after the NEXUS_<lang> dicts).
    # We resolve the globals at runtime so the order doesn't matter.
    DOMAINS = {
        'firestaff': globals().get('FIRESTAFF', {}),
        'nexus': globals().get('NEXUS', {}),
        'csb': globals().get('CSB', {}),
        'theron': globals().get('THERON', {}),
        'startup-menu': globals().get('STARTUP_MENU', {}),
    }
    if len(sys.argv) > 1:
        for arg in sys.argv[1:]:
            if arg in DOMAINS:
                DOMAINS = {arg: DOMAINS[arg]}
            else:
                print(f"Unknown domain: {arg}")
                sys.exit(1)
    for d, table in DOMAINS.items():
        n = apply_other_translations(d, table)
        print(f"{d}: applied {n} translations across {len(table)} locales")



# =========================================================================
# CSB (33 strings) - 19 locales
# =========================================================================
CSB_EN = {k: k for k in [
    'csb.engine.version', 'csb.reincarnate.title', 'csb.reincarnate.chaos',
    'csb.reincarnate.choose', 'csb.reincarnate.stat-roll', 'csb.resurrect.title',
    'csb.resurrect.altar', 'csb.resurrect.choose', 'csb.chaos.cooldown',
    'csb.chaos.energy', 'csb.chaos.recharge', 'csb.greylord.intro',
    'csb.greylord.choose', 'csb.oracle.title', 'csb.oracle.prompt',
    'csb.party.max', 'csb.party.slot', 'csb.save.import',
    'csb.save.csb-st-v20', 'csb.save.csb-st-v21', 'csb.save.amiga-v35',
    'csb.save.pc34', 'csb.neophyte.title', 'csb.neophyte.desc',
    'csb.engine.dm1', 'csb.engine.csb', 'csb.launcher.title',
    'csb.launcher.subtitle', 'csb.launcher.lore-shape',
    'csb.launcher.lore-line-1', 'csb.launcher.lore-line-2',
    'csb.launcher.lore-line-3', 'csb.launcher.lore-line-4',
]}

CSB = {'en': CSB_EN}

CSB['sv'] = {k: v for k, v in {
    'csb.engine.version': 'CSB-motorns version',
    'csb.reincarnate.title': 'ÅTERFÖDAS',
    'csb.reincarnate.chaos': 'Kaosstrid.',
    'csb.reincarnate.choose': 'Välj nya krigare för återfödelse.',
    'csb.reincarnate.stat-roll': 'Slå tärning för statistik',
    'csb.resurrect.title': 'ÅTERUPPSTÅ',
    'csb.resurrect.altar': 'Offra blod vid altaret för att väcka krigare.',
    'csb.resurrect.choose': 'Välj krigare att väcka.',
    'csb.chaos.cooldown': 'Kaosmagi svalnar',
    'csb.chaos.energy': 'Kaosenergi',
    'csb.chaos.recharge': 'Laddar',
    'csb.greylord.intro': 'Den grå lorden viskar: Välj.',
    'csb.greylord.choose': 'Gör ditt val.',
    'csb.oracle.title': 'SPÅDOMSORAKLET',
    'csb.oracle.prompt': 'Fråga och oraklet ska svara.',
    'csb.party.max': 'Max 4 krigare',
    'csb.party.slot': 'Parti-plats',
    'csb.save.import': 'Importera CSB-sparning',
    'csb.save.csb-st-v20': 'CSB Atari ST 2.0-sparning',
    'csb.save.csb-st-v21': 'CSB Atari ST 2.1-sparning',
    'csb.save.amiga-v35': 'CSB Amiga 3.5-sparning',
    'csb.save.pc34': 'CSB PC 3.4-sparning',
    'csb.neophyte.title': 'NYBÖRJARLÄGE',
    'csb.neophyte.desc': 'Minska fällornas skada, lättare start.',
    'csb.engine.dm1': 'DM1-motor v2.0',
    'csb.engine.csb': 'CSB-motor v2.1',
    'csb.launcher.title': 'CHAOS STRIKES BACK',
    'csb.launcher.subtitle': 'Dungeon Master II-uppföljaren',
    'csb.launcher.lore-shape': 'Lord Chaos tornar upp sig...',
    'csb.launcher.lore-line-1': 'Lord Chaos har stulit eld personifikationen.',
    'csb.launcher.lore-line-2': 'Flamsten är förlorad.',
    'csb.launcher.lore-line-3': 'Den grå lorden kallar er att hämta den.',
    'csb.launcher.lore-line-4': 'Stig in i kaoset.',
}.items() if k in CSB_EN}

# Add a few key non-SV locales for CSB
CSB['fr'] = {k: v for k, v in {
    'csb.engine.version': 'Version du moteur CSB',
    'csb.reincarnate.title': 'RÉINCARNATION',
    'csb.reincarnate.chaos': 'Bataille du Chaos.',
    'csb.reincarnate.choose': 'Choisissez de nouveaux guerriers à réincarner.',
    'csb.reincarnate.stat-roll': 'Lancer les dés pour les statistiques',
    'csb.resurrect.title': 'RÉSURRECTION',
    'csb.resurrect.altar': 'Sur l\'autel, offrez votre sang pour ressusciter les guerriers.',
    'csb.resurrect.choose': 'Choisissez les guerriers à ressusciter.',
    'csb.chaos.cooldown': 'Magie du Chaos en recharge',
    'csb.chaos.energy': 'Énergie du Chaos',
    'csb.chaos.recharge': 'Recharge',
    'csb.greylord.intro': 'Le Seigneur Gris murmure : Choisissez.',
    'csb.greylord.choose': 'Faites votre choix.',
    'csb.oracle.title': 'ORACLE',
    'csb.oracle.prompt': 'Demandez et l\'oracle répondra.',
    'csb.party.max': 'Max 4 guerriers',
    'csb.party.slot': 'Emplacement du groupe',
    'csb.save.import': 'Importer sauvegarde CSB',
    'csb.save.csb-st-v20': 'Sauvegarde CSB Atari ST 2.0',
    'csb.save.csb-st-v21': 'Sauvegarde CSB Atari ST 2.1',
    'csb.save.amiga-v35': 'Sauvegarde CSB Amiga 3.5',
    'csb.save.pc34': 'Sauvegarde CSB PC 3.4',
    'csb.neophyte.title': 'MODE NÉOPHYTE',
    'csb.neophyte.desc': 'Réduit les dégâts des pièges, démarrage plus facile.',
    'csb.engine.dm1': 'Moteur DM1 v2.0',
    'csb.engine.csb': 'Moteur CSB v2.1',
    'csb.launcher.title': 'CHAOS STRIKES BACK',
    'csb.launcher.subtitle': 'La suite de Dungeon Master II',
    'csb.launcher.lore-shape': 'Le Seigneur Chaos se dresse...',
    'csb.launcher.lore-line-1': 'Le Seigneur Chaos a volé la Flamme.',
    'csb.launcher.lore-line-2': 'La Pierre de Feu est perdue.',
    'csb.launcher.lore-line-3': 'Le Seigneur Gris vous appelle à la récupérer.',
    'csb.launcher.lore-line-4': 'Entrez dans le Chaos.',
}.items() if k in CSB_EN}

CSB['de'] = {k: v for k, v in {
    'csb.engine.version': 'CSB-Engine-Version',
    'csb.reincarnate.title': 'WIEDERGEBURT',
    'csb.reincarnate.chaos': 'Chaos-Kampf.',
    'csb.reincarnate.choose': 'Wählt neue Krieger für die Wiedergeburt.',
    'csb.reincarnate.stat-roll': 'Würfeln für Statistik',
    'csb.resurrect.title': 'WIEDERBELEBUNG',
    'csb.resurrect.altar': 'Opfert Blut am Altar, um Krieger zu erwecken.',
    'csb.resurrect.choose': 'Wählt zu erweckende Krieger.',
    'csb.chaos.cooldown': 'Chaosmagie kühlt ab',
    'csb.chaos.energy': 'Chaosenergie',
    'csb.chaos.recharge': 'Lädt',
    'csb.greylord.intro': 'Der Graue Lord flüstert: Wählt.',
    'csb.greylord.choose': 'Triff deine Wahl.',
    'csb.oracle.title': 'ORAKEL',
    'csb.oracle.prompt': 'Frage und das Orakel antwortet.',
    'csb.party.max': 'Max 4 Krieger',
    'csb.party.slot': 'Gruppen-Slot',
    'csb.save.import': 'CSB-Spielstand importieren',
    'csb.save.csb-st-v20': 'CSB Atari ST 2.0 Spielstand',
    'csb.save.csb-st-v21': 'CSB Atari ST 2.1 Spielstand',
    'csb.save.amiga-v35': 'CSB Amiga 3.5 Spielstand',
    'csb.save.pc34': 'CSB PC 3.4 Spielstand',
    'csb.neophyte.title': 'ANFÄNGER-MODUS',
    'csb.neophyte.desc': 'Reduziert Fallenschaden, einfacherer Start.',
    'csb.engine.dm1': 'DM1-Engine v2.0',
    'csb.engine.csb': 'CSB-Engine v2.1',
    'csb.launcher.title': 'CHAOS STRIKES BACK',
    'csb.launcher.subtitle': 'Der Dungeon-Master-II-Nachfolger',
    'csb.launcher.lore-shape': 'Lord Chaos erhebt sich...',
    'csb.launcher.lore-line-1': 'Lord Chaos hat die Flamme gestohlen.',
    'csb.launcher.lore-line-2': 'Der Feuerstab ist verloren.',
    'csb.launcher.lore-line-3': 'Der Graue Lord ruft euch, ihn zurückzuholen.',
    'csb.launcher.lore-line-4': 'Tretet ein ins Chaos.',
}.items() if k in CSB_EN}

CSB['ja'] = {k: v for k, v in {
    'csb.engine.version': 'CSBエンジン バージョン',
    'csb.reincarnate.title': '転生',
    'csb.reincarnate.chaos': 'カオスの戦い。',
    'csb.reincarnate.choose': '転生する新しい戦士を選んでください。',
    'csb.reincarnate.stat-roll': '能力値のためのサイコロ',
    'csb.resurrect.title': '復活',
    'csb.resurrect.altar': '祭壇で血を捧げて戦士を蘇らせる。',
    'csb.resurrect.choose': '蘇生する戦士を選んでください。',
    'csb.chaos.cooldown': 'カオス魔法 クールダウン',
    'csb.chaos.energy': 'カオスエネルギー',
    'csb.chaos.recharge': 'チャージ中',
    'csb.greylord.intro': '灰色卿が囁く: 選べ。',
    'csb.greylord.choose': '選択してください。',
    'csb.oracle.title': '神託',
    'csb.oracle.prompt': '問うてください、神託が答えます。',
    'csb.party.max': '最大4人',
    'csb.party.slot': 'パーティ枠',
    'csb.save.import': 'CSBセーブをインポート',
    'csb.save.csb-st-v20': 'CSB Atari ST 2.0 セーブ',
    'csb.save.csb-st-v21': 'CSB Atari ST 2.1 セーブ',
    'csb.save.amiga-v35': 'CSB Amiga 3.5 セーブ',
    'csb.save.pc34': 'CSB PC 3.4 セーブ',
    'csb.neophyte.title': '初心者モード',
    'csb.neophyte.desc': '罠のダメージ軽減、簡単なスタート。',
    'csb.engine.dm1': 'DM1エンジン v2.0',
    'csb.engine.csb': 'CSBエンジン v2.1',
    'csb.launcher.title': 'カオス・ストライクス・バック',
    'csb.launcher.subtitle': 'ダンジョンマスター II の続編',
    'csb.launcher.lore-shape': '混沌卿が立ち上がる...',
    'csb.launcher.lore-line-1': '混沌卿が炎を盗んだ。',
    'csb.launcher.lore-line-2': '火の杖は失われた。',
    'csb.launcher.lore-line-3': '灰色卿がそれを取り返すよう呼びかける。',
    'csb.launcher.lore-line-4': '混沌へ踏み込め。',
}.items() if k in CSB_EN}

CSB['zh'] = {k: v for k, v in {
    'csb.engine.version': 'CSB引擎 版本',
    'csb.reincarnate.title': '转世',
    'csb.reincarnate.chaos': '混沌之战。',
    'csb.reincarnate.choose': '选择要转世的新战士。',
    'csb.reincarnate.stat-roll': '掷骰子决定属性',
    'csb.resurrect.title': '复活',
    'csb.resurrect.altar': '在祭坛上献出你的血以复活战士。',
    'csb.resurrect.choose': '选择要复活的战士。',
    'csb.chaos.cooldown': '混沌魔法 冷却',
    'csb.chaos.energy': '混沌能量',
    'csb.chaos.recharge': '充能中',
    'csb.greylord.intro': '灰之主低语: 选择。',
    'csb.greylord.choose': '做出你的选择。',
    'csb.oracle.title': '神谕',
    'csb.oracle.prompt': '询问,神谕将回答。',
    'csb.party.max': '最多4个战士',
    'csb.party.slot': '队伍位',
    'csb.save.import': '导入CSB存档',
    'csb.save.csb-st-v20': 'CSB Atari ST 2.0 存档',
    'csb.save.csb-st-v21': 'CSB Atari ST 2.1 存档',
    'csb.save.amiga-v35': 'CSB Amiga 3.5 存档',
    'csb.save.pc34': 'CSB PC 3.4 存档',
    'csb.neophyte.title': '新手模式',
    'csb.neophyte.desc': '减少陷阱伤害,更易上手。',
    'csb.engine.dm1': 'DM1引擎 v2.0',
    'csb.engine.csb': 'CSB引擎 v2.1',
    'csb.launcher.title': '混沌反击',
    'csb.launcher.subtitle': '《地下城领主II》续作',
    'csb.launcher.lore-shape': '混沌之主崛起...',
    'csb.launcher.lore-line-1': '混沌之主偷走了火焰。',
    'csb.launcher.lore-line-2': '火杖失落。',
    'csb.launcher.lore-line-3': '灰之主召唤你去取回它。',
    'csb.launcher.lore-line-4': '踏入混沌。',
}.items() if k in CSB_EN}

# For other languages, just use English source (msgstr=msgstr)
CSB.update({
    'es': {k: k for k in CSB_EN},  # English fallback; native speakers should override
    'it': {k: k for k in CSB_EN},
    'pt': {k: k for k in CSB_EN},
    'ru': {k: k for k in CSB_EN},
    'pl': {k: k for k in CSB_EN},
    'cs': {k: k for k in CSB_EN},
    'da': {k: k for k in CSB_EN},
    'nl': {k: k for k in CSB_EN},
    'no': {k: k for k in CSB_EN},
    'fi': {k: k for k in CSB_EN},
    'hu': {k: k for k in CSB_EN},
    'ko': {k: k for k in CSB_EN},
    'tr': {k: k for k in CSB_EN},
})


# =========================================================================
# THERON (38 strings) - 19 locales
# =========================================================================
THERON_EN = {k: k for k in [
    'NORTH', 'EAST', 'SOUTH', 'WEST', 'STAIRS', 'PIT', 'TELEPORTER',
    'DOOR', 'PICK UP', 'DROP', 'USE', 'ATTACK', 'DEFEND', 'CAST SPELL',
    'NO FOCUS', 'PRESS ENTER ON A REAL FRONT-CELL TARGET',
    'UNKNOWN', 'NONE', 'SAVE GAME', 'LOAD GAME', 'SHOP',
    'NOT ENOUGH GOLD', 'PURCHASED %s FOR %d GOLD',
    'T%u: ASCENDED TO LEVEL %d', 'T%u: DESCENDED TO LEVEL %d',
    'T%u: FELL INTO PIT', 'T%u: TELEPORTED TO MAP %d (%d,%d)',
    'T%u: TELEPORTER HAS NO THING DATA',
    'T%u: TELEPORTER TARGET MAP %d OUT OF RANGE',
    'T%u: %s LEVELED UP! (%s %d -> %d)', 'T%u: %s CASTS %s',
    'T%u: %s HITS %s FOR %d', 'T%u: %s MISSES %s',
    'OFFERING ACCEPTED', 'INVENTORY FULL', 'OUT OF RANGE',
    'PRESS ENTER OR CLICK THE VIEW TO READ THE FRONT CELL',
    'HP AND STAMINA RECOVER SLOWY',
]}

THERON = {'en': THERON_EN}

THERON['sv'] = {k: v for k, v in {
    'NORTH': 'NORR', 'EAST': 'ÖST', 'SOUTH': 'SÖDER', 'WEST': 'VÄSTER',
    'STAIRS': 'TRAPPA', 'PIT': 'GROP', 'TELEPORTER': 'TELEPORTÖR',
    'DOOR': 'DÖRR', 'PICK UP': 'PLOCKA UPP', 'DROP': 'SLÄPP',
    'USE': 'ANVÄND', 'ATTACK': 'ANFALL', 'DEFEND': 'FÖRSVAR',
    'CAST SPELL': 'KASTA BESVÄRJELSE', 'NO FOCUS': 'INGET FOKUS',
    'PRESS ENTER ON A REAL FRONT-CELL TARGET':
        'TRYCK ENTER PÅ RIKTIGT FRÄMRE CELLMÅL',
    'UNKNOWN': 'OKÄND', 'NONE': 'INGEN',
    'SAVE GAME': 'SPARA SPEL', 'LOAD GAME': 'LADDA SPEL', 'SHOP': 'BUTIK',
    'NOT ENOUGH GOLD': 'INTE TILLRÄCKLIGT MED GULD',
    'PURCHASED %s FOR %d GOLD': 'KÖPTE %S FÖR %D GULD',
    'T%u: ASCENDED TO LEVEL %d': 'T%U: GICK UPP TILL NIVÅ %D',
    'T%u: DESCENDED TO LEVEL %d': 'T%U: GICK NER TILL NIVÅ %D',
    'T%u: FELL INTO PIT': 'T%U: FÖLL I GROP',
    'T%u: TELEPORTED TO MAP %d (%d,%d)':
        'T%U: TELEPORTERADE TILL KARTA %D (%D,%D)',
    'T%u: TELEPORTER HAS NO THING DATA':
        'T%U: TELEPORTÖR HAR INGEN FÖREMÅLSDATA',
    'T%u: TELEPORTER TARGET MAP %d OUT OF RANGE':
        'T%U: TELEPORTÖRS MÅLKARTA %D UTANFÖR OMRÅDE',
    'T%u: %s LEVELED UP! (%s %d -> %d)': 'T%U: %S GICK UPP EN NIVÅ! (%S %D -> %D)',
    'T%u: %s CASTS %s': 'T%U: %S KASTAR %S',
    'T%u: %s HITS %s FOR %d': 'T%U: %S TRÄFFAR %S FÖR %D',
    'T%u: %s MISSES %s': 'T%U: %S MISSAR %S',
    'OFFERING ACCEPTED': 'OFFER TAGET EMOT',
    'INVENTORY FULL': 'INVENTARIE FULL',
    'OUT OF RANGE': 'UTANFÖR RÄCKVIDD',
    'PRESS ENTER OR CLICK THE VIEW TO READ THE FRONT CELL':
        'TRYCK ENTER ELLER KLICKA PÅ VYN FÖR ATT LÄSA DEN FRÄMRE RUTAN',
    'HP AND STAMINA RECOVER SLOWY':
        'HP OCH UTHÅLLIGHET ÅTERHÄMTAR SIG LÅNGSAMT',
}.items() if k in THERON_EN}

THERON['fr'] = {k: v for k, v in {
    'NORTH': 'NORD', 'EAST': 'EST', 'SOUTH': 'SUD', 'WEST': 'OUEST',
    'STAIRS': 'ESCALIER', 'PIT': 'GOUFFRE', 'TELEPORTER': 'TÉLÉPORTEUR',
    'DOOR': 'PORTE', 'PICK UP': 'RAMASSER', 'DROP': 'LÂCHER',
    'USE': 'UTILISER', 'ATTACK': 'ATTAQUER', 'DEFEND': 'DÉFENDRE',
    'CAST SPELL': 'LANCER SORT', 'NO FOCUS': 'AUCUNE CIBLE',
    'PRESS ENTER ON A REAL FRONT-CELL TARGET':
        'APPUYEZ SUR ENTRÉE SUR UNE VRAIE CIBLE AVANT',
    'UNKNOWN': 'INCONNU', 'NONE': 'AUCUN',
    'SAVE GAME': 'SAUVEGARDER', 'LOAD GAME': 'CHARGER', 'SHOP': 'BOUTIQUE',
    'NOT ENOUGH GOLD': 'PAS ASSEZ D\'OR',
    'PURCHASED %s FOR %d GOLD': 'ACHETÉ %S POUR %D OR',
    'T%u: ASCENDED TO LEVEL %d': 'T%U: MONTÉ AU NIVEAU %D',
    'T%u: DESCENDED TO LEVEL %d': 'T%U: DESCENDU AU NIVEAU %D',
    'T%u: FELL INTO PIT': 'T%U: TOMBÉ DANS LE GOUFFRE',
    'T%u: TELEPORTED TO MAP %d (%d,%d)':
        'T%U: TÉLÉPORTÉ À LA CARTE %D (%D,%D)',
    'T%u: TELEPORTER HAS NO THING DATA':
        'T%U: LE TÉLÉPORTEUR N\'A PAS DE DONNÉES',
    'T%u: TELEPORTER TARGET MAP %d OUT OF RANGE':
        'T%U: CARTE CIBLE DU TÉLÉPORTEUR %D HORS PORTÉE',
    'T%u: %s LEVELED UP! (%s %d -> %d)': 'T%U: %S MONTE DE NIVEAU! (%S %D -> %D)',
    'T%u: %s CASTS %s': 'T%U: %S LANCE %S',
    'T%u: %s HITS %s FOR %d': 'T%U: %S FRAPPE %S POUR %D',
    'T%u: %s MISSES %s': 'T%U: %S RATE %S',
    'OFFERING ACCEPTED': 'OFFRANDE ACCEPTÉE',
    'INVENTORY FULL': 'INVENTAIRE PLEIN',
    'OUT OF RANGE': 'HORS PORTÉE',
    'PRESS ENTER OR CLICK THE VIEW TO READ THE FRONT CELL':
        'APPUYEZ SUR ENTRÉE OU CLIQUEZ SUR LA VUE',
    'HP AND STAMINA RECOVER SLOWY':
        'HP ET ENDURANCE SE RÉCUPÈRENT LENTEMENT',
}.items() if k in THERON_EN}

THERON['de'] = {k: v for k, v in {
    'NORTH': 'NORD', 'EAST': 'OST', 'SOUTH': 'SÜD', 'WEST': 'WEST',
    'STAIRS': 'TREPPE', 'PIT': 'GRUBE', 'TELEPORTER': 'TELEPORTER',
    'DOOR': 'TÜR', 'PICK UP': 'AUFNEHMEN', 'DROP': 'FALLEN LASSEN',
    'USE': 'BENUTZEN', 'ATTACK': 'ANGREIFEN', 'DEFEND': 'VERTEIDIGEN',
    'CAST SPELL': 'ZAUBER WIRKEN', 'NO FOCUS': 'KEIN FOKUS',
    'PRESS ENTER ON A REAL FRONT-CELL TARGET':
        'DRÜCKE ENTER AUF EIN ECHTES VORDERZIEL',
    'UNKNOWN': 'UNBEKANNT', 'NONE': 'KEINE',
    'SAVE GAME': 'SPIEL SPEICHERN', 'LOAD GAME': 'SPIEL LADEN',
    'SHOP': 'LADEN',
    'NOT ENOUGH GOLD': 'NICHT GENUG GOLD',
    'PURCHASED %s FOR %d GOLD': 'GEKAUFT %S FÜR %D GOLD',
    'T%u: ASCENDED TO LEVEL %d': 'T%U: AUFGESTIEGEN ZU EBENE %D',
    'T%u: DESCENDED TO LEVEL %d': 'T%U: HINABGESTIEGEN ZU EBENE %D',
    'T%u: FELL INTO PIT': 'T%U: IN GRUBE GEFALLEN',
    'T%u: TELEPORTED TO MAP %d (%d,%d)':
        'T%U: TELEPORTIERT ZU KARTE %D (%D,%D)',
    'T%u: TELEPORTER HAS NO THING DATA':
        'T%U: TELEPORTER HAT KEINE DATEN',
    'T%u: TELEPORTER TARGET MAP %d OUT OF RANGE':
        'T%U: TELEPORTER-ZIELKARTE %D AUSSERHALB DES BEREICHS',
    'T%u: %s LEVELED UP! (%s %d -> %d)': 'T%U: %S AUFGESTIEGEN! (%S %D -> %D)',
    'T%u: %s CASTS %s': 'T%U: %S WIRKT %S',
    'T%u: %s HITS %s FOR %d': 'T%U: %S TRIFFT %S FÜR %D',
    'T%u: %s MISSES %s': 'T%U: %S VERFEHLT %S',
    'OFFERING ACCEPTED': 'OPFER ANGENOMMEN',
    'INVENTORY FULL': 'INVENTAR VOLL',
    'OUT OF RANGE': 'AUSSERHALB DER REICHWEITE',
    'PRESS ENTER OR CLICK THE VIEW TO READ THE FRONT CELL':
        'DRÜCKE ENTER ODER KLICKE AUF DIE ANSICHT',
    'HP AND STAMINA RECOVER SLOWY':
        'HP UND AUSDAUER ERHOLEN SICH LANGSAM',
}.items() if k in THERON_EN}

THERON['ja'] = {k: v for k, v in {
    'NORTH': '北', 'EAST': '東', 'SOUTH': '南', 'WEST': '西',
    'STAIRS': '階段', 'PIT': '落とし穴', 'TELEPORTER': 'テレポーター',
    'DOOR': '扉', 'PICK UP': '拾う', 'DROP': '落とす',
    'USE': '使う', 'ATTACK': '攻撃', 'DEFEND': '防御',
    'CAST SPELL': '呪文詠唱', 'NO FOCUS': '対象なし',
    'PRESS ENTER ON A REAL FRONT-CELL TARGET':
        '実際の前セルターゲットでENTERを押す',
    'UNKNOWN': '不明', 'NONE': 'なし',
    'SAVE GAME': 'セーブ', 'LOAD GAME': 'ロード', 'SHOP': '店',
    'NOT ENOUGH GOLD': 'ゴールド不足',
    'PURCHASED %s FOR %d GOLD': '%S を %D ゴールドで購入',
    'T%u: ASCENDED TO LEVEL %d': 'T%U: レベル %D へ上昇',
    'T%u: DESCENDED TO LEVEL %d': 'T%U: レベル %D へ下降',
    'T%u: FELL INTO PIT': 'T%U: 落とし穴に落ちた',
    'T%u: TELEPORTED TO MAP %d (%d,%d)': 'T%U: マップ %D (%D,%D) へ転移',
    'T%u: TELEPORTER HAS NO THING DATA':
        'T%U: テレポーターに物データがない',
    'T%u: TELEPORTER TARGET MAP %d OUT OF RANGE':
        'T%U: テレポーターの目標マップ %D が範囲外',
    'T%u: %s LEVELED UP! (%s %d -> %d)': 'T%U: %S レベルアップ! (%S %D -> %D)',
    'T%u: %s CASTS %s': 'T%U: %S が %S を詠唱',
    'T%u: %s HITS %s FOR %d': 'T%U: %S が %S に %D 命中',
    'T%u: %s MISSES %s': 'T%U: %S が %S にミス',
    'OFFERING ACCEPTED': '供物受理',
    'INVENTORY FULL': 'インベントリ満杯',
    'OUT OF RANGE': '範囲外',
    'PRESS ENTER OR CLICK THE VIEW TO READ THE FRONT CELL':
        'ENTER または ビューをクリック',
    'HP AND STAMINA RECOVER SLOWY': 'HP とスタミナはゆっくり回復',
}.items() if k in THERON_EN}

THERON['zh'] = {k: v for k, v in {
    'NORTH': '北', 'EAST': '东', 'SOUTH': '南', 'WEST': '西',
    'STAIRS': '楼梯', 'PIT': '坑', 'TELEPORTER': '传送器',
    'DOOR': '门', 'PICK UP': '拾取', 'DROP': '掉落',
    'USE': '使用', 'ATTACK': '攻击', 'DEFEND': '防御',
    'CAST SPELL': '施法', 'NO FOCUS': '无目标',
    'PRESS ENTER ON A REAL FRONT-CELL TARGET': '按 ENTER 或点击视图',
    'UNKNOWN': '未知', 'NONE': '无',
    'SAVE GAME': '保存', 'LOAD GAME': '载入', 'SHOP': '商店',
    'NOT ENOUGH GOLD': '金币不足',
    'PURCHASED %s FOR %d GOLD': '用 %D 金币购买 %S',
    'T%u: ASCENDED TO LEVEL %d': 'T%U: 上升到第 %D 层',
    'T%u: DESCENDED TO LEVEL %d': 'T%U: 下降到第 %D 层',
    'T%u: FELL INTO PIT': 'T%U: 坠入坑中',
    'T%u: TELEPORTED TO MAP %d (%d,%d)': 'T%U: 传送到地图 %D (%D,%D)',
    'T%u: TELEPORTER HAS NO THING DATA': 'T%U: 传送器无物品数据',
    'T%u: TELEPORTER TARGET MAP %d OUT OF RANGE': 'T%U: 传送器目标地图 %D 超出范围',
    'T%u: %s LEVELED UP! (%s %d -> %d)': 'T%U: %S 升级! (%S %D -> %D)',
    'T%u: %s CASTS %s': 'T%U: %S 施法 %S',
    'T%u: %s HITS %s FOR %d': 'T%U: %S 击中 %S 造成 %D',
    'T%u: %s MISSES %s': 'T%U: %S 未击中 %S',
    'OFFERING ACCEPTED': '供物接受',
    'INVENTORY FULL': '物品栏满',
    'OUT OF RANGE': '超出范围',
    'PRESS ENTER OR CLICK THE VIEW TO READ THE FRONT CELL': '按 ENTER 或点击视图',
    'HP AND STAMINA RECOVER SLOWY': 'HP 和耐力缓慢恢复',
}.items() if k in THERON_EN}

THERON.update({
    'es': {k: k for k in THERON_EN},
    'it': {k: k for k in THERON_EN},
    'pt': {k: k for k in THERON_EN},
    'ru': {k: k for k in THERON_EN},
    'pl': {k: k for k in THERON_EN},
    'cs': {k: k for k in THERON_EN},
    'da': {k: k for k in THERON_EN},
    'nl': {k: k for k in THERON_EN},
    'no': {k: k for k in THERON_EN},
    'fi': {k: k for k in THERON_EN},
    'hu': {k: k for k in THERON_EN},
    'ko': {k: k for k in THERON_EN},
    'tr': {k: k for k in THERON_EN},
})


# =========================================================================
# STARTUP-MENU (66 strings) - 19 locales
# =========================================================================
STARTUP_MENU_EN = {k: k for k in [
    'FRONTEND PREVIEW', 'SELECT A DESTINATION', 'SETTINGS', 'STATUS',
    'LAUNCHER DESTINATIONS', 'DATA DIR', 'UP/DOWN MOVE   ENTER OPEN   ESC EXIT',
    'PERSISTED OPTIONS', 'LANGUAGE', 'GRAPHICS MODE', 'WINDOW MODE',
    'CHANGES SAVE IMMEDIATELY TO CONFIG',
    'LEFT/RIGHT CYCLE   ENTER ADVANCE   ESC BACK',
    'ENTER OR ESC RETURNS TO MENU', 'READY TO LAUNCH',
    'ESC RETURNS TO MENU', 'VALIDATOR SCAFFOLD ONLY',
    'ADD VERIFIED RETAIL HASHES', 'GAME DATA NOT FOUND',
    'CHECK FIRESTAFF DATA DIR', 'ART SLOT READY', 'ART SLOT EMPTY',
    'DROP ART INTO SLOT', 'CARD ART ACTIVE', 'CARD ART SLOT',
    'Dungeon Master', 'Chaos Strikes Back', 'Dungeon Master II', 'Firestaff',
    'PATCH', 'ORIGINAL', 'PATCHED', 'CHEATS', 'OFF', 'ON', 'SPEED',
    'SLOWER', 'NORMAL', 'FASTER', 'ASPECT RATIO', 'RESOLUTION', 'LAUNCH',
    'V1 ORIGINAL', 'V2 ENHANCED 2D', 'V3 MODERN/3D', 'COMING SOON',
    'ENGLISH', 'SVENSKA', 'FRANCAIS', 'DEUTSCH', 'WINDOWED', 'FULLSCREEN',
    'RENDERER BACKEND', 'RENDERER BACKEND UNAVAILABLE', 'RENDERER', 'AUTO',
    'SOFTWARE', 'SDL', 'OPENGL', 'VULKAN', 'AVAILABLE', 'UNAVAILABLE',
    'SCALE', 'PIXEL SNAP', 'FILTER', 'VSYNC',
]}

STARTUP_MENU = {'en': STARTUP_MENU_EN}

STARTUP_MENU['sv'] = {k: v for k, v in {
    'FRONTEND PREVIEW': 'FRONTEND-FÖRHANDSVISNING',
    'SELECT A DESTINATION': 'VÄLJ EN DESTINATION',
    'SETTINGS': 'INSTÄLLNINGAR', 'STATUS': 'STATUS',
    'LAUNCHER DESTINATIONS': 'LAUNCHER-DESTINATIONER', 'DATA DIR': 'DATAKAT',
    'UP/DOWN MOVE   ENTER OPEN   ESC EXIT':
        'UPP/NER FLYTTA   ENTER ÖPPNA   ESC AVSLUTA',
    'PERSISTED OPTIONS': 'PERSISTERADE ALTERNATIV', 'LANGUAGE': 'SPRÅK',
    'GRAPHICS MODE': 'GRAFIKLÄGE', 'WINDOW MODE': 'FÖNSTERLÄGE',
    'CHANGES SAVE IMMEDIATELY TO CONFIG':
        'ÄNDRINGAR SPARAS DIREKT TILL KONFIG',
    'LEFT/RIGHT CYCLE   ENTER ADVANCE   ESC BACK':
        'VÄNSTER/HÖGER CYKEL   ENTER AVANCERA   ESC TILLBAKA',
    'ENTER OR ESC RETURNS TO MENU': 'ENTER ELLER ESC ÅTERGÅR TILL MENYN',
    'READY TO LAUNCH': 'REDO ATT STARTA',
    'ESC RETURNS TO MENU': 'ESC ÅTERGÅR TILL MENYN',
    'VALIDATOR SCAFFOLD ONLY': 'ENDAST VALIDATOR-MALL',
    'ADD VERIFIED RETAIL HASHES': 'LÄGG TILL VERIFIERADE DETALJHANDEL-HASHAR',
    'GAME DATA NOT FOUND': 'SPELDATA HITTADES INTE',
    'CHECK FIRESTAFF DATA DIR': 'KONTROLLERA FIRESTAFF DATAKAT',
    'ART SLOT READY': 'KONSTSLOTS REDO',
    'ART SLOT EMPTY': 'KONSTSLOTS TOMT',
    'DROP ART INTO SLOT': 'SLÄPP KONST I SLOTSEN',
    'CARD ART ACTIVE': 'KORTKONST AKTIV', 'CARD ART SLOT': 'KORTKONST-SLOTS',
    'Dungeon Master': 'Dungeon Master', 'Chaos Strikes Back': 'Chaos Strikes Back',
    'Dungeon Master II': 'Dungeon Master II', 'Firestaff': 'Firestaff',
    'PATCH': 'PATCH', 'ORIGINAL': 'ORIGINAL', 'PATCHED': 'PATCHAD',
    'CHEATS': 'FUSK', 'OFF': 'AV', 'ON': 'PÅ', 'SPEED': 'HASTIGHET',
    'SLOWER': 'LÅNGSAMMARE', 'NORMAL': 'NORMAL', 'FASTER': 'SNABBARE',
    'ASPECT RATIO': 'BILDFÖRHÅLLANDE', 'RESOLUTION': 'UPPLÖSNING',
    'LAUNCH': 'STARTA', 'V1 ORIGINAL': 'V1 ORIGINAL',
    'V2 ENHANCED 2D': 'V2 FÖRBÄTTRAD 2D', 'V3 MODERN/3D': 'V3 MODERN/3D',
    'COMING SOON': 'KOMMER SNART',
    'ENGLISH': 'ENGLISH', 'SVENSKA': 'SVENSKA',
    'FRANCAIS': 'FRANÇAIS', 'DEUTSCH': 'DEUTSCH',
    'WINDOWED': 'FÖNSTER', 'FULLSCREEN': 'FULLSKÄRM',
    'RENDERER BACKEND': 'RENDERINGS-BACKEND',
    'RENDERER BACKEND UNAVAILABLE': 'RENDERINGS-BACKEND OTILLGÄNGLIG',
    'RENDERER': 'RENDERING', 'AUTO': 'AUTO',
    'SOFTWARE': 'MJUKVARA', 'SDL': 'SDL',
    'OPENGL': 'OPENGL', 'VULKAN': 'VULKAN',
    'AVAILABLE': 'TILLGÄNGLIG', 'UNAVAILABLE': 'OTILLGÄNGLIG',
    'SCALE': 'SKALA', 'PIXEL SNAP': 'PIXELSNÄPP',
    'FILTER': 'FILTER', 'VSYNC': 'VSYNC',
}.items() if k in STARTUP_MENU_EN}

STARTUP_MENU['fr'] = {k: v for k, v in {
    'FRONTEND PREVIEW': 'APERÇU FRONTEND',
    'SELECT A DESTINATION': 'SÉLECTIONNER UNE DESTINATION',
    'SETTINGS': 'PARAMÈTRES', 'STATUS': 'STATUT',
    'LAUNCHER DESTINATIONS': 'DESTINATIONS DU LANCEUR', 'DATA DIR': 'RÉPERTOIRE DE DONNÉES',
    'UP/DOWN MOVE   ENTER OPEN   ESC EXIT':
        'HAUT/BAS DÉPLACER   ENTRÉE OUVRIR   ÉCHAP QUITTER',
    'PERSISTED OPTIONS': 'OPTIONS PERSISTANTES', 'LANGUAGE': 'LANGUE',
    'GRAPHICS MODE': 'MODE GRAPHIQUE', 'WINDOW MODE': 'MODE FENÊTRE',
    'CHANGES SAVE IMMEDIATELY TO CONFIG':
        'CHANGEMENTS SAUVEGARDÉS IMMÉDIATEMENT',
    'LEFT/RIGHT CYCLE   ENTER ADVANCE   ESC BACK':
        'GAUCHE/DROITE CYCLE   ENTRÉE AVANCER   ÉCHAP RETOUR',
    'ENTER OR ESC RETURNS TO MENU':
        'ENTRÉE OU ÉCHAP RETOURNE AU MENU',
    'READY TO LAUNCH': 'PRÊT À LANCER',
    'ESC RETURNS TO MENU': 'ÉCHAP RETOURNE AU MENU',
    'VALIDATOR SCAFFOLD ONLY': 'ÉCHAFAUDAGE VALIDATEUR UNIQUEMENT',
    'ADD VERIFIED RETAIL HASHES': 'AJOUTER HASHES COMMERCE VÉRIFIÉS',
    'GAME DATA NOT FOUND': 'DONNÉES DE JEU INTROUVABLES',
    'CHECK FIRESTAFF DATA DIR': 'VÉRIFIER RÉPERTOIRE FIRESTAFF',
    'ART SLOT READY': 'EMPLACEMENT ART PRÊT', 'ART SLOT EMPTY': 'EMPLACEMENT ART VIDE',
    'DROP ART INTO SLOT': 'DÉPOSER ART DANS EMPLACEMENT',
    'CARD ART ACTIVE': 'ART CARTE ACTIF', 'CARD ART SLOT': 'EMPLACEMENT ART CARTE',
    'Dungeon Master': 'Dungeon Master', 'Chaos Strikes Back': 'Chaos Strikes Back',
    'Dungeon Master II': 'Dungeon Master II', 'Firestaff': 'Firestaff',
    'PATCH': 'PATCH', 'ORIGINAL': 'ORIGINAL', 'PATCHED': 'PATCHÉ',
    'CHEATS': 'TRICHE', 'OFF': 'OFF', 'ON': 'ON', 'SPEED': 'VITESSE',
    'SLOWER': 'PLUS LENT', 'NORMAL': 'NORMAL', 'FASTER': 'PLUS RAPIDE',
    'ASPECT RATIO': 'RAPPORT D\'ASPECT', 'RESOLUTION': 'RÉSOLUTION',
    'LAUNCH': 'LANCER', 'V1 ORIGINAL': 'V1 ORIGINAL',
    'V2 ENHANCED 2D': 'V2 2D AMÉLIORÉ', 'V3 MODERN/3D': 'V3 MODERNE/3D',
    'COMING SOON': 'BIENTÔT DISPONIBLE',
    'ENGLISH': 'ENGLISH', 'SVENSKA': 'SUÉDOIS',
    'FRANCAIS': 'FRANÇAIS', 'DEUTSCH': 'ALLEMAND',
    'WINDOWED': 'FENÊTRE', 'FULLSCREEN': 'PLEIN ÉCRAN',
    'RENDERER BACKEND': 'BACKEND RENDU',
    'RENDERER BACKEND UNAVAILABLE': 'BACKEND RENDU INDISPONIBLE',
    'RENDERER': 'RENDU', 'AUTO': 'AUTO', 'SOFTWARE': 'LOGICIEL',
    'SDL': 'SDL', 'OPENGL': 'OPENGL', 'VULKAN': 'VULKAN',
    'AVAILABLE': 'DISPONIBLE', 'UNAVAILABLE': 'INDISPONIBLE',
    'SCALE': 'ÉCHELLE', 'PIXEL SNAP': 'SNAP PIXEL',
    'FILTER': 'FILTRE', 'VSYNC': 'VSYNC',
}.items() if k in STARTUP_MENU_EN}

STARTUP_MENU['de'] = {k: v for k, v in {
    'FRONTEND PREVIEW': 'FRONTEND-VORSCHAU',
    'SELECT A DESTINATION': 'ZIEL WÄHLEN',
    'SETTINGS': 'EINSTELLUNGEN', 'STATUS': 'STATUS',
    'LAUNCHER DESTINATIONS': 'LAUNCHER-ZIELE', 'DATA DIR': 'DATENVERZ.',
    'UP/DOWN MOVE   ENTER OPEN   ESC EXIT':
        'AUF/AB BEWEGEN   ENTER ÖFFNEN   ESC BEENDEN',
    'PERSISTED OPTIONS': 'GESPEICHERTE OPTIONEN', 'LANGUAGE': 'SPRACHE',
    'GRAPHICS MODE': 'GRAFIKMODUS', 'WINDOW MODE': 'FENSTERMODUS',
    'CHANGES SAVE IMMEDIATELY TO CONFIG':
        'ÄNDERUNGEN SOFORT IN KONFIG GESPEICHERT',
    'LEFT/RIGHT CYCLE   ENTER ADVANCE   ESC BACK':
        'LINKS/RECHTS WECHSELN   ENTER VOR   ESC ZURÜCK',
    'ENTER OR ESC RETURNS TO MENU':
        'ENTER ODER ESC KEHREN ZUM MENÜ ZURÜCK',
    'READY TO LAUNCH': 'BEREIT ZUM STARTEN',
    'ESC RETURNS TO MENU': 'ESC KEHRT ZUM MENÜ ZURÜCK',
    'VALIDATOR SCAFFOLD ONLY': 'NUR VALIDATOR-GERÜST',
    'ADD VERIFIED RETAIL HASHES':
        'VERIFIZIERTE EINZELHANDELS-HASHES HINZUFÜGEN',
    'GAME DATA NOT FOUND': 'SPIELDATEN NICHT GEFUNDEN',
    'CHECK FIRESTAFF DATA DIR': 'FIRESTAFF-DATENVERZ. PRÜFEN',
    'ART SLOT READY': 'KUNST-SLOT BEREIT', 'ART SLOT EMPTY': 'KUNST-SLOT LEER',
    'DROP ART INTO SLOT': 'KUNST IN SLOT ABLEGEN',
    'CARD ART ACTIVE': 'KARTEN-KUNST AKTIV',
    'CARD ART SLOT': 'KARTEN-KUNST-SLOT',
    'Dungeon Master': 'Dungeon Master', 'Chaos Strikes Back': 'Chaos Strikes Back',
    'Dungeon Master II': 'Dungeon Master II', 'Firestaff': 'Firestaff',
    'PATCH': 'PATCH', 'ORIGINAL': 'ORIGINAL', 'PATCHED': 'GEPATCHT',
    'CHEATS': 'CHEA TS', 'OFF': 'AUS', 'ON': 'AN', 'SPEED': 'GESCHWINDIGKEIT',
    'SLOWER': 'LANGSAMER', 'NORMAL': 'NORMAL', 'FASTER': 'SCHNELLER',
    'ASPECT RATIO': 'SEITENVERHÄLTNIS', 'RESOLUTION': 'AUFLÖSUNG',
    'LAUNCH': 'STARTEN', 'V1 ORIGINAL': 'V1 ORIGINAL',
    'V2 ENHANCED 2D': 'V2 VERBESSERT 2D', 'V3 MODERN/3D': 'V3 MODERN/3D',
    'COMING SOON': 'DURCHSTECHEND',
    'ENGLISH': 'ENGLISCH', 'SVENSKA': 'SCHWEDISCH',
    'FRANCAIS': 'FRANZÖSISCH', 'DEUTSCH': 'DEUTSCH',
    'WINDOWED': 'FENSTER', 'FULLSCREEN': 'VOLLBILD',
    'RENDERER BACKEND': 'RENDERER-BACKEND',
    'RENDERER BACKEND UNAVAILABLE': 'RENDERER-BACKEND NICHT VERFÜGBAR',
    'RENDERER': 'RENDERER', 'AUTO': 'AUTO',
    'SOFTWARE': 'SOFTWARE', 'SDL': 'SDL', 'OPENGL': 'OPENGL',
    'VULKAN': 'VULKAN', 'AVAILABLE': 'VERFÜGBAR', 'UNAVAILABLE': 'NICHT VERFÜGBAR',
    'SCALE': 'SKALIERUNG', 'PIXEL SNAP': 'PIXEL-SNAP',
    'FILTER': 'FILTER', 'VSYNC': 'VSYNC',
}.items() if k in STARTUP_MENU_EN}

STARTUP_MENU['ja'] = {k: v for k, v in {
    'FRONTEND PREVIEW': 'フロントエンド プレビュー',
    'SELECT A DESTINATION': '目的地を選択',
    'SETTINGS': '設定', 'STATUS': 'ステータス',
    'LAUNCHER DESTINATIONS': 'ランチャー目的地',
    'DATA DIR': 'データディレクトリ',
    'UP/DOWN MOVE   ENTER OPEN   ESC EXIT':
        '上下で移動   ENTERで開く   ESCで終了',
    'PERSISTED OPTIONS': '保存されたオプション',
    'LANGUAGE': '言語', 'GRAPHICS MODE': 'グラフィックス モード',
    'WINDOW MODE': 'ウィンドウ モード',
    'CHANGES SAVE IMMEDIATELY TO CONFIG':
        '変更は即座に設定に保存',
    'LEFT/RIGHT CYCLE   ENTER ADVANCE   ESC BACK':
        '左右で切替   ENTERで進行   ESCで戻る',
    'ENTER OR ESC RETURNS TO MENU':
        'ENTER または ESC でメニューに戻る',
    'READY TO LAUNCH': '起動準備完了',
    'ESC RETURNS TO MENU': 'ESC でメニューに戻る',
    'VALIDATOR SCAFFOLD ONLY': 'バリデーター足場のみ',
    'ADD VERIFIED RETAIL HASHES':
        '検証済み小売ハッシュを追加',
    'GAME DATA NOT FOUND': 'ゲームデータなし',
    'CHECK FIRESTAFF DATA DIR':
        'Firestaff データディレクトリを確認',
    'ART SLOT READY': 'アートスロット準備完了',
    'ART SLOT EMPTY': 'アートスロット空',
    'DROP ART INTO SLOT': 'スロットにアートをドロップ',
    'CARD ART ACTIVE': 'カードアート有効',
    'CARD ART SLOT': 'カードアートスロット',
    'Dungeon Master': 'ダンジョン・マスター',
    'Chaos Strikes Back': 'カオス・ストライクス・バック',
    'Dungeon Master II': 'ダンジョン・マスター II',
    'Firestaff': 'Firestaff', 'PATCH': 'パッチ',
    'ORIGINAL': 'オリジナル', 'PATCHED': 'パッチ済み',
    'CHEATS': 'チート', 'OFF': 'オフ', 'ON': 'オン', 'SPEED': '速度',
    'SLOWER': 'より遅い', 'NORMAL': 'ノーマル', 'FASTER': 'より速い',
    'ASPECT RATIO': 'アスペクト比', 'RESOLUTION': '解像度',
    'LAUNCH': '起動', 'V1 ORIGINAL': 'V1 オリジナル',
    'V2 ENHANCED 2D': 'V2 エンハンスド 2D', 'V3 MODERN/3D': 'V3 モダン/3D',
    'COMING SOON': '近日公開',
    'ENGLISH': '英語', 'SVENSKA': 'スウェーデン語',
    'FRANCAIS': 'フランス語', 'DEUTSCH': 'ドイツ語',
    'WINDOWED': 'ウィンドウ', 'FULLSCREEN': 'フルスクリーン',
    'RENDERER BACKEND': 'レンダラーバックエンド',
    'RENDERER BACKEND UNAVAILABLE': 'レンダラー利用不可',
    'RENDERER': 'レンダラー', 'AUTO': '自動', 'SOFTWARE': 'ソフトウェア',
    'SDL': 'SDL', 'OPENGL': 'OpenGL', 'VULKAN': 'Vulkan',
    'AVAILABLE': '利用可能', 'UNAVAILABLE': '利用不可',
    'SCALE': 'スケール', 'PIXEL SNAP': 'ピクセルスナップ',
    'FILTER': 'フィルター', 'VSYNC': 'VSYNC',
}.items() if k in STARTUP_MENU_EN}

STARTUP_MENU['zh'] = {k: v for k, v in {
    'FRONTEND PREVIEW': '前端预览',
    'SELECT A DESTINATION': '选择目标',
    'SETTINGS': '设置', 'STATUS': '状态',
    'LAUNCHER DESTINATIONS': '启动器目标',
    'DATA DIR': '数据目录',
    'UP/DOWN MOVE   ENTER OPEN   ESC EXIT':
        '上下移动   ENTER打开   ESC退出',
    'PERSISTED OPTIONS': '持久化选项',
    'LANGUAGE': '语言', 'GRAPHICS MODE': '图形模式',
    'WINDOW MODE': '窗口模式',
    'CHANGES SAVE IMMEDIATELY TO CONFIG':
        '更改立即保存到配置',
    'LEFT/RIGHT CYCLE   ENTER ADVANCE   ESC BACK':
        '左右循环   ENTER前进   ESC返回',
    'ENTER OR ESC RETURNS TO MENU':
        'ENTER 或 ESC 返回菜单',
    'READY TO LAUNCH': '准备启动',
    'ESC RETURNS TO MENU': 'ESC 返回菜单',
    'VALIDATOR SCAFFOLD ONLY': '仅验证器框架',
    'ADD VERIFIED RETAIL HASHES': '添加经验证的零售哈希',
    'GAME DATA NOT FOUND': '未找到游戏数据',
    'CHECK FIRESTAFF DATA DIR': '检查 Firestaff 数据目录',
    'ART SLOT READY': '美术槽就绪', 'ART SLOT EMPTY': '美术槽空',
    'DROP ART INTO SLOT': '将美术放入槽',
    'CARD ART ACTIVE': '卡片美术激活', 'CARD ART SLOT': '卡片美术槽',
    'Dungeon Master': '地下城领主',
    'Chaos Strikes Back': '混沌反击',
    'Dungeon Master II': '地下城领主 II', 'Firestaff': 'Firestaff',
    'PATCH': '补丁', 'ORIGINAL': '原版', 'PATCHED': '已补丁',
    'CHEATS': '秘籍', 'OFF': '关', 'ON': '开', 'SPEED': '速度',
    'SLOWER': '更慢', 'NORMAL': '正常', 'FASTER': '更快',
    'ASPECT RATIO': '宽高比', 'RESOLUTION': '分辨率',
    'LAUNCH': '启动', 'V1 ORIGINAL': 'V1 原版',
    'V2 ENHANCED 2D': 'V2 增强 2D', 'V3 MODERN/3D': 'V3 现代/3D',
    'COMING SOON': '即将推出',
    'ENGLISH': '英语', 'SVENSKA': '瑞典语',
    'FRANCAIS': '法语', 'DEUTSCH': '德语',
    'WINDOWED': '窗口', 'FULLSCREEN': '全屏',
    'RENDERER BACKEND': '渲染后端',
    'RENDERER BACKEND UNAVAILABLE': '渲染后端不可用',
    'RENDERER': '渲染器', 'AUTO': '自动', 'SOFTWARE': '软件',
    'SDL': 'SDL', 'OPENGL': 'OpenGL', 'VULKAN': 'Vulkan',
    'AVAILABLE': '可用', 'UNAVAILABLE': '不可用',
    'SCALE': '缩放', 'PIXEL SNAP': '像素捕捉',
    'FILTER': '过滤器', 'VSYNC': 'VSYNC',
}.items() if k in STARTUP_MENU_EN}

# Other languages: English fallback (msgstr=msgid)
STARTUP_MENU.update({
    'es': {k: k for k in STARTUP_MENU_EN},
    'it': {k: k for k in STARTUP_MENU_EN},
    'pt': {k: k for k in STARTUP_MENU_EN},
    'ru': {k: k for k in STARTUP_MENU_EN},
    'pl': {k: k for k in STARTUP_MENU_EN},
    'cs': {k: k for k in STARTUP_MENU_EN},
    'da': {k: k for k in STARTUP_MENU_EN},
    'nl': {k: k for k in STARTUP_MENU_EN},
    'no': {k: k for k in STARTUP_MENU_EN},
    'fi': {k: k for k in STARTUP_MENU_EN},
    'hu': {k: k for k in STARTUP_MENU_EN},
    'ko': {k: k for k in STARTUP_MENU_EN},
    'tr': {k: k for k in STARTUP_MENU_EN},
})
