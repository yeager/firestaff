#include "firestaff_l10n.h"

static FS_Language g_lang = FS_LANG_EN;

static const char *g_strings[FS_LANG_COUNT][FS_STR_COUNT] = {
    [FS_LANG_EN] = {
        "PLAY", "SETTINGS", "EXTRAS", "QUIT",
        "SELECT GAME", "New Game (Original)", "New Game (10x Upscale)",
        "New Game (Modern Graphics)", "Continue Saved Game", "Coming soon",
        "Back", "Display", "Video", "Audio", "Controls", "Accessibility",
        "Museum of Lore", "Manual / Docs", "Bestiary", "Spell Reference", "Map Viewer",
        "Item Encyclopedia", "Changelog", "Screenshot Gallery",
        "Language", "On", "Off"
    },
    [FS_LANG_SV] = {
        "SPELA", "INSTÄLLNINGAR", "EXTRA", "AVSLUTA",
        "VÄLJ SPEL", "Nytt spel (original)", "Nytt spel (10x uppskalning)",
        "Nytt spel (modern grafik)", "Fortsätt sparat spel", "Kommer snart",
        "Tillbaka", "Bildskärm", "Video", "Ljud", "Kontroller", "Tillgänglighet",
        "Kunskapsmuseet", "Handbok / dokument", "Bestiarium", "Trollformler", "Kartvisare",
        "Föremålslexikon", "Ändringslogg", "Skärmbildsgalleri",
        "Språk", "På", "Av"
    },
    [FS_LANG_DE] = {
        "SPIELEN", "EINSTELLUNGEN", "EXTRAS", "BEENDEN",
        "SPIEL WÄHLEN", "Neues Spiel (Original)", "Neues Spiel (10x skaliert)",
        "Neues Spiel (moderne Grafik)", "Gespeichertes Spiel fortsetzen", "Demnächst",
        "Zurück", "Anzeige", "Video", "Audio", "Steuerung", "Barrierefreiheit",
        "Wissensmuseum", "Handbuch / Dokumentation", "Bestiarium", "Zaubersprüche", "Kartenansicht",
        "Gegenstandslexikon", "Änderungsprotokoll", "Screenshot-Galerie",
        "Sprache", "An", "Aus"
    },
    [FS_LANG_FR] = {
        "JOUER", "PARAMÈTRES", "EXTRAS", "QUITTER",
        "CHOISIR UN JEU", "Nouvelle partie (original)", "Nouvelle partie (agrandie 10x)",
        "Nouvelle partie (graphismes modernes)", "Continuer la partie", "Bientôt disponible",
        "Retour", "Affichage", "Vidéo", "Audio", "Contrôles", "Accessibilité",
        "Musée du Savoir", "Manuel / Documentation", "Bestiaire", "Référence des Sorts", "Visionneuse de Carte",
        "Encyclopédie des Objets", "Journal des Modifications", "Galerie de Captures",
        "Langue", "Activé", "Désactivé"
    },
    [FS_LANG_ES] = {
        "JUGAR", "AJUSTES", "EXTRAS", "SALIR",
        "ELEGIR JUEGO", "Nueva partida (original)", "Nueva partida (10x ampliada)",
        "Nueva partida (gráficos modernos)", "Continuar partida", "Próximamente",
        "Atrás", "Pantalla", "Vídeo", "Audio", "Controles", "Accesibilidad",
        "Museo de saber", "Manual / documentos", "Bestiario", "Referencia de hechizos", "Visor de mapas",
        "Enciclopedia de objetos", "Registro de cambios", "Galería de capturas",
        "Idioma", "Sí", "No"
    },
    [FS_LANG_IT] = {
        "GIOCA", "IMPOSTAZIONI", "EXTRA", "ESCI",
        "SCEGLI GIOCO", "Nuova partita (originale)", "Nuova partita (ingrandita 10x)",
        "Nuova partita (grafica moderna)", "Continua partita", "Prossimamente",
        "Indietro", "Schermo", "Video", "Audio", "Comandi", "Accessibilità",
        "Museo della lore", "Manuale / documenti", "Bestiario", "Riferimento incantesimi", "Visualizzatore mappe",
        "Enciclopedia oggetti", "Registro modifiche", "Galleria screenshot",
        "Lingua", "Sì", "No"
    },
    [FS_LANG_PT] = {
        "JOGAR", "DEFINIÇÕES", "EXTRAS", "SAIR",
        "ESCOLHER JOGO", "Novo jogo (original)", "Novo jogo (ampliação 10x)",
        "Novo jogo (gráficos modernos)", "Continuar jogo", "Em breve",
        "Voltar", "Ecrã", "Vídeo", "Áudio", "Controlos", "Acessibilidade",
        "Museu de lore", "Manual / documentos", "Bestiário", "Referência de feitiços", "Visualizador de mapas",
        "Enciclopédia de itens", "Registo de alterações", "Galeria de capturas",
        "Idioma", "Ligado", "Desligado"
    },
    [FS_LANG_NL] = {
        "SPELEN", "INSTELLINGEN", "EXTRA'S", "AFSLUITEN",
        "SPEL KIEZEN", "Nieuw spel (origineel)", "Nieuw spel (10x opgeschaald)",
        "Nieuw spel (moderne graphics)", "Opgeslagen spel hervatten", "Binnenkort",
        "Terug", "Weergave", "Video", "Audio", "Besturing", "Toegankelijkheid",
        "Lore-museum", "Handleiding / docs", "Bestiarium", "Spreukenreferentie", "Kaartviewer",
        "Voorwerpenencyclopedie", "Wijzigingslog", "Screenshotgalerij",
        "Taal", "Aan", "Uit"
    },
    [FS_LANG_PL] = {
        "GRAJ", "USTAWIENIA", "DODATKI", "WYJDŹ",
        "WYBIERZ GRĘ", "Nowa gra (oryginał)", "Nowa gra (powiększenie 10x)",
        "Nowa gra (nowoczesna grafika)", "Kontynuuj zapis", "Wkrótce",
        "Wstecz", "Ekran", "Wideo", "Audio", "Sterowanie", "Dostępność",
        "Muzeum wiedzy", "Instrukcja / dokumenty", "Bestiariusz", "Spis zaklęć", "Przeglądarka map",
        "Encyklopedia przedmiotów", "Dziennik zmian", "Galeria zrzutów",
        "Język", "Wł.", "Wył."
    },
    [FS_LANG_CS] = {
        "HRÁT", "NASTAVENÍ", "EXTRA", "UKONČIT",
        "VYBRAT HRU", "Nová hra (originál)", "Nová hra (10x zvětšení)",
        "Nová hra (moderní grafika)", "Pokračovat v uložené hře", "Již brzy",
        "Zpět", "Zobrazení", "Video", "Audio", "Ovládání", "Přístupnost",
        "Muzeum lore", "Manuál / dokumenty", "Bestiář", "Přehled kouzel", "Prohlížeč map",
        "Encyklopedie předmětů", "Seznam změn", "Galerie snímků",
        "Jazyk", "Zap.", "Vyp."
    },
    [FS_LANG_RU] = {
        "ИГРАТЬ", "НАСТРОЙКИ", "ДОПОЛНИТЕЛЬНО", "ВЫХОД",
        "ВЫБОР ИГРЫ", "Новая игра (оригинал)", "Новая игра (масштаб 10x)",
        "Новая игра (современная графика)", "Продолжить сохранение", "Скоро",
        "Назад", "Экран", "Видео", "Аудио", "Управление", "Доступность",
        "Музей мира", "Руководство / документы", "Бестиарий", "Справочник заклинаний", "Просмотр карт",
        "Энциклопедия предметов", "Журнал изменений", "Галерея снимков",
        "Язык", "Вкл.", "Выкл."
    },
    [FS_LANG_JA] = {
        "プレイ", "設定", "追加", "終了",
        "ゲーム選択", "新規ゲーム（オリジナル）", "新規ゲーム（10x拡大）",
        "新規ゲーム（現代グラフィック）", "保存から続行", "近日公開",
        "戻る", "表示", "映像", "音声", "操作", "アクセシビリティ",
        "資料館", "マニュアル", "図鑑", "呪文", "マップ",
        "アイテム", "更新履歴", "スクリーンショット",
        "言語", "オン", "オフ"
    },
    [FS_LANG_KO] = {
        "플레이", "설정", "추가", "종료",
        "게임 선택", "새 게임 (원본)", "새 게임 (10x 확대)",
        "새 게임 (현대 그래픽)", "저장한 게임 계속", "곧 공개",
        "뒤로", "디스플레이", "비디오", "오디오", "조작", "접근성",
        "지식 박물관", "매뉴얼 / 문서", "괴물 도감", "주문 참조", "지도 보기",
        "아이템 백과", "변경 기록", "스크린샷 갤러리",
        "언어", "켬", "끔"
    },
    [FS_LANG_ZH] = {
        "开始", "设置", "附加", "退出",
        "选择游戏", "新游戏（原版）", "新游戏（10x放大）",
        "新游戏（现代图形）", "继续存档", "即将推出",
        "返回", "显示", "视频", "音频", "控制", "辅助功能",
        "资料馆", "手册/文档", "怪物图鉴", "法术参考", "地图查看器",
        "物品百科", "更新日志", "截图图库",
        "语言", "开", "关"
    },
    [FS_LANG_DA] = {
        "SPIL", "INDSTILLINGER", "EKSTRA", "AFSLUT",
        "VÆLG SPIL", "Nyt spil (original)", "Nyt spil (10x opskaleret)",
        "Nyt spil (moderne grafik)", "Fortsæt gemt spil", "Kommer snart",
        "Tilbage", "Skærm", "Video", "Lyd", "Kontroller", "Tilgængelighed",
        "Lore-museum", "Manual / dokumenter", "Bestiarium", "Trylleformularer", "Kortviser",
        "Genstandsleksikon", "Ændringslog", "Skærmbilledgalleri",
        "Sprog", "Til", "Fra"
    },
    [FS_LANG_NO] = {
        "SPILL", "INNSTILLINGER", "EKSTRA", "AVSLUTT",
        "VELG SPILL", "Nytt spill (original)", "Nytt spill (10x oppskalert)",
        "Nytt spill (moderne grafikk)", "Fortsett lagret spill", "Kommer snart",
        "Tilbake", "Skjerm", "Video", "Lyd", "Kontroller", "Tilgjengelighet",
        "Lore-museum", "Håndbok / dokumenter", "Bestiarium", "Trylleformler", "Kartviser",
        "Gjenstandsleksikon", "Endringslogg", "Skjermbildegalleri",
        "Språk", "På", "Av"
    },
    [FS_LANG_FI] = {
        "PELAA", "ASETUKSET", "LISÄT", "LOPETA",
        "VALITSE PELI", "Uusi peli (alkuperäinen)", "Uusi peli (10x suurennos)",
        "Uusi peli (moderni grafiikka)", "Jatka tallennusta", "Tulossa",
        "Takaisin", "Näyttö", "Video", "Ääni", "Ohjaus", "Esteettömyys",
        "Tietomuseo", "Käsikirja / dokumentit", "Bestiaari", "Loitsuluettelo", "Karttakatselin",
        "Esine-ensyklopedia", "Muutosloki", "Kuvakaappausgalleria",
        "Kieli", "Päällä", "Pois"
    },
    [FS_LANG_HU] = {
        "JÁTÉK", "BEÁLLÍTÁSOK", "EXTRÁK", "KILÉPÉS",
        "JÁTÉK VÁLASZTÁSA", "Új játék (eredeti)", "Új játék (10x nagyítás)",
        "Új játék (modern grafika)", "Mentés folytatása", "Hamarosan",
        "Vissza", "Megjelenítés", "Videó", "Hang", "Vezérlés", "Akadálymentesség",
        "Tudástár", "Kézikönyv / dokumentumok", "Bestiárium", "Varázslatreferencia", "Térképnéző",
        "Tárgyenciklopédia", "Változásnapló", "Képgaléria",
        "Nyelv", "Be", "Ki"
    },
    [FS_LANG_TR] = {
        "OYNA", "AYARLAR", "EKSTRALAR", "ÇIK",
        "OYUN SEÇ", "Yeni oyun (orijinal)", "Yeni oyun (10x büyütme)",
        "Yeni oyun (modern grafikler)", "Kayıtlı oyuna devam", "Yakında",
        "Geri", "Ekran", "Video", "Ses", "Kontroller", "Erişilebilirlik",
        "Bilgi müzesi", "Kılavuz / belgeler", "Canavar rehberi", "Büyü referansı", "Harita görüntüleyici",
        "Eşya ansiklopedisi", "Değişiklik günlüğü", "Ekran görüntüsü galerisi",
        "Dil", "Açık", "Kapalı"
    },
    [FS_LANG_ID] = {
        "MAIN", "PENGATURAN", "EKSTRA", "KELUAR",
        "PILIH GAME", "Game baru (orisinal)", "Game baru (skala 10x)",
        "Game baru (grafik modern)", "Lanjutkan simpanan", "Segera hadir",
        "Kembali", "Tampilan", "Video", "Audio", "Kontrol", "Aksesibilitas",
        "Museum lore", "Manual / dokumen", "Bestiari", "Referensi mantra", "Penampil peta",
        "Ensiklopedia item", "Log perubahan", "Galeri tangkapan layar",
        "Bahasa", "Nyala", "Mati"
    },
};

static const char *g_lang_names[FS_LANG_COUNT] = {
    "English", "Svenska", "Deutsch", "Français",
    "Español", "Italiano", "Português", "Nederlands",
    "Polski", "Čeština", "Русский", "日本語",
    "한국어", "简体中文", "Dansk", "Norsk",
    "Suomi", "Magyar", "Türkçe", "Bahasa Indonesia"
};

void fs_l10n_set_language(FS_Language lang) {
    if (lang >= 0 && lang < FS_LANG_COUNT) g_lang = lang;
}

FS_Language fs_l10n_get_language(void) { return g_lang; }

const char *fs_l10n_get(FS_StringId id) {
    if (id < 0 || id >= FS_STR_COUNT) return "???";
    const char *s = g_strings[g_lang][id];
    return s ? s : g_strings[FS_LANG_EN][id];
}

const char *fs_l10n_language_name(FS_Language lang) {
    if (lang < 0 || lang >= FS_LANG_COUNT) return "???";
    return g_lang_names[lang];
}

/* ══════════════════════════════════════════════════════════════════════
 * System language detection + asset language binding
 * ══════════════════════════════════════════════════════════════════════ */

#include <stdlib.h>

FS_Language fs_l10n_language_from_locale(const char *locale) {
    if (!locale || !locale[0]) return FS_LANG_EN;
    if ((locale[0] == 's' || locale[0] == 'S') &&
        (locale[1] == 'v' || locale[1] == 'V')) return FS_LANG_SV;
    if ((locale[0] == 'd' || locale[0] == 'D') &&
        (locale[1] == 'e' || locale[1] == 'E')) return FS_LANG_DE;
    if ((locale[0] == 'f' || locale[0] == 'F') &&
        (locale[1] == 'r' || locale[1] == 'R')) return FS_LANG_FR;
    if ((locale[0] == 'e' || locale[0] == 'E') &&
        (locale[1] == 's' || locale[1] == 'S')) return FS_LANG_ES;
    if ((locale[0] == 'i' || locale[0] == 'I') &&
        (locale[1] == 't' || locale[1] == 'T')) return FS_LANG_IT;
    if ((locale[0] == 'p' || locale[0] == 'P') &&
        (locale[1] == 't' || locale[1] == 'T')) return FS_LANG_PT;
    if ((locale[0] == 'n' || locale[0] == 'N') &&
        (locale[1] == 'l' || locale[1] == 'L')) return FS_LANG_NL;
    if ((locale[0] == 'p' || locale[0] == 'P') &&
        (locale[1] == 'l' || locale[1] == 'L')) return FS_LANG_PL;
    if ((locale[0] == 'c' || locale[0] == 'C') &&
        (locale[1] == 's' || locale[1] == 'S')) return FS_LANG_CS;
    if ((locale[0] == 'r' || locale[0] == 'R') &&
        (locale[1] == 'u' || locale[1] == 'U')) return FS_LANG_RU;
    if ((locale[0] == 'j' || locale[0] == 'J') &&
        (locale[1] == 'a' || locale[1] == 'A')) return FS_LANG_JA;
    if ((locale[0] == 'k' || locale[0] == 'K') &&
        (locale[1] == 'o' || locale[1] == 'O')) return FS_LANG_KO;
    if ((locale[0] == 'z' || locale[0] == 'Z') &&
        (locale[1] == 'h' || locale[1] == 'H')) return FS_LANG_ZH;
    if ((locale[0] == 'd' || locale[0] == 'D') &&
        (locale[1] == 'a' || locale[1] == 'A')) return FS_LANG_DA;
    if (((locale[0] == 'n' || locale[0] == 'N') &&
         (locale[1] == 'b' || locale[1] == 'B')) ||
        ((locale[0] == 'n' || locale[0] == 'N') &&
         (locale[1] == 'o' || locale[1] == 'O'))) return FS_LANG_NO;
    if ((locale[0] == 'f' || locale[0] == 'F') &&
        (locale[1] == 'i' || locale[1] == 'I')) return FS_LANG_FI;
    if ((locale[0] == 'h' || locale[0] == 'H') &&
        (locale[1] == 'u' || locale[1] == 'U')) return FS_LANG_HU;
    if ((locale[0] == 't' || locale[0] == 'T') &&
        (locale[1] == 'r' || locale[1] == 'R')) return FS_LANG_TR;
    if ((locale[0] == 'i' || locale[0] == 'I') &&
        (locale[1] == 'd' || locale[1] == 'D')) return FS_LANG_ID;
    return FS_LANG_EN;
}

FS_Language fs_l10n_detect_system_language(void) {
    /* GNU/Linux LANGUAGE is a colon-separated user-preference list.  Steam
     * Game Mode may expose the UI locale there while leaving LANG at its
     * English runtime default, so honour it before LANG after the two
     * locale-category overrides. */
    const char *env_vars[] = {"LC_ALL", "LC_MESSAGES", "LANGUAGE", "LANG", NULL};
    int i;
    for (i = 0; env_vars[i]; i++) {
        const char *val = getenv(env_vars[i]);
        if (!val || !val[0]) continue;
        return fs_l10n_language_from_locale(val);
    }
    return FS_LANG_EN; /* default */
}

/* Map UI language → asset dungeon.dat language.
 * Swedish has no DM1 dungeon.dat, falls back to English. */
int fs_l10n_to_asset_language(FS_Language ui_lang) {
    switch (ui_lang) {
    case FS_LANG_DE: return 1;
    case FS_LANG_FR: return 2;
    case FS_LANG_EN:
    default:
        return 0;
    }
}

void fs_l10n_init_from_system(void) {
    fs_l10n_set_language(fs_l10n_detect_system_language());
}
