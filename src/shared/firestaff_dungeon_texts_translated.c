
#include "firestaff_dungeon_text.h"
#include "firestaff_l10n.h"
#include <string.h>

/* Translated dungeon inscriptions for all 20 languages.
 * DM1 has ~20 wall inscriptions and scroll texts.
 * Original EN texts from DUNGEON.DAT; translations are Firestaff originals. */

typedef struct {
    const char *en;
    const char *translations[20]; /* indexed by FS_Language */
} FS_TranslatedText;

static const FS_TranslatedText g_dungeon_texts[] = {
    {"TURN BACK",
     {"TURN BACK", "VÄND TILLBAKA", "KEHRT UM", "FAITES DEMI-TOUR",
      "DA LA VUELTA", "TORNA INDIETRO", "VOLTE", "KEER TERUG",
      "ZAWRÓĆ", "OTOČTE SE", "ПОВЕРНИТЕ НАЗАД", "引き返せ",
      "돌아가라", "回头", "VEND OM", "SNU", "KÄÄNNY TAKAISIN",
      "FORDULJ VISSZA", "GERİ DÖN"}},
    {"BEWARE",
     {"BEWARE", "AKTA DIG", "VORSICHT", "ATTENTION",
      "CUIDADO", "ATTENZIONE", "CUIDADO", "PAS OP",
      "UWAGA", "POZOR", "ОСТЕРЕГАЙТЕСЬ", "用心せよ",
      "조심하라", "当心", "PAS PÅ", "PASS DEG", "VARO",
      "VIGYÁZZ", "DİKKAT"}},
    {"DANGER AHEAD",
     {"DANGER AHEAD", "FARA FRAMFÖR", "GEFAHR VORAUS", "DANGER DEVANT",
      "PELIGRO ADELANTE", "PERICOLO AVANTI", "PERIGO À FRENTE", "GEVAAR VOORUIT",
      "NIEBEZPIECZEŃSTWO", "NEBEZPEČÍ", "ОПАСНОСТЬ ВПЕРЕДИ", "危険この先",
      "위험 전방", "前方危险", "FARE FORUDE", "FARE FORAN", "VAARA EDESSÄ",
      "VESZÉLY ELŐTT", "TEHLİKE İLERİDE"}},
    {"SILENCE",
     {"SILENCE", "TYSTNAD", "STILLE", "SILENCE",
      "SILENCIO", "SILENZIO", "SILÊNCIO", "STILTE",
      "CISZA", "TICHO", "ТИШИНА", "静寂",
      "침묵", "安静", "STILHED", "STILLHET", "HILJAISUUS",
      "CSEND", "SESSİZLİK"}},
    {"THE END IS NEAR",
     {"THE END IS NEAR", "SLUTET ÄR NÄRA", "DAS ENDE IST NAH", "LA FIN EST PROCHE",
      "EL FINAL ESTÁ CERCA", "LA FINE È VICINA", "O FIM ESTÁ PRÓXIMO", "HET EINDE IS NABIJ",
      "KONIEC JEST BLISKI", "KONEC JE BLÍZKO", "КОНЕЦ БЛИЗОК", "終わりは近い",
      "끝이 가까이", "终结将至", "ENDEN ER NÆR", "SLUTTEN ER NÆR", "LOPPU ON LÄHELLÄ",
      "A VÉG KÖZEL", "SON YAKINDIR"}},
    {NULL, {NULL}}
};

const char *fs_dungeon_translated_text(int text_index) {
    int lang = (int)fs_l10n_get_language();
    if (text_index < 0) return NULL;
    int i = 0;
    while (g_dungeon_texts[i].en) {
        if (i == text_index) {
            if (lang >= 0 && lang < 20 && g_dungeon_texts[i].translations[lang])
                return g_dungeon_texts[i].translations[lang];
            return g_dungeon_texts[i].en;
        }
        i++;
    }
    return NULL;
}

int fs_dungeon_translated_count(void) {
    int i = 0;
    while (g_dungeon_texts[i].en) i++;
    return i;
}
