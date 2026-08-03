#!/usr/bin/env python3
"""Firestaff Savegame Editor — read, edit and write savegames for all five
Dungeon Master games.

Supported formats:
  DM1   — DMSAVE.DAT  (PC 3.4, 512-byte XOR header, 5 obfuscated parts)
  CSB   — DMSAVE.DAT  (PC 3.4, 512-byte header, obfuscation key 29)
  DM2   — SKSAVE.*    (SUPPRESS codec, 16 record types, 10 slots)
  TQ    — slotN.tqsv  (Theron's Quest, 64-byte header, XOR seed 0x5A)
  Nexus — *.fnxs      (Firestaff native, magic FNXS, CRC32)

Requires Python 3.10+ and Tk 8.6+.
"""

from __future__ import annotations
import gettext, os, struct, sys
from pathlib import Path
from typing import Any

# ── i18n ─────────────────────────────────────────────────────────────────
LANG_META = [
    ("sv", "\U0001f1f8\U0001f1ea", "Svenska"),
    ("en", "\U0001f1ec\U0001f1e7", "English"),
    ("de", "\U0001f1e9\U0001f1ea", "Deutsch"),
    ("fr", "\U0001f1eb\U0001f1f7", "Français"),
    ("es", "\U0001f1ea\U0001f1f8", "Español"),
    ("it", "\U0001f1ee\U0001f1f9", "Italiano"),
    ("pt", "\U0001f1f5\U0001f1f9", "Português"),
    ("nl", "\U0001f1f3\U0001f1f1", "Nederlands"),
    ("da", "\U0001f1e9\U0001f1f0", "Dansk"),
    ("no", "\U0001f1f3\U0001f1f4", "Norsk"),
    ("fi", "\U0001f1eb\U0001f1ee", "Suomi"),
    ("pl", "\U0001f1f5\U0001f1f1", "Polski"),
    ("cs", "\U0001f1e8\U0001f1ff", "Čeština"),
    ("hu", "\U0001f1ed\U0001f1fa", "Magyar"),
    ("ro", "\U0001f1f7\U0001f1f4", "Română"),
    ("ja", "\U0001f1ef\U0001f1f5", "日本語"),
    ("ko", "\U0001f1f0\U0001f1f7", "한국어"),
    ("zh", "\U0001f1e8\U0001f1f3", "中文"),
    ("ru", "\U0001f1f7\U0001f1fa", "Русский"),
]

if getattr(sys, "frozen", False) and hasattr(sys, "_MEIPASS"):
    _LOCALE_DIR = Path(sys._MEIPASS) / "po" / "locale"
else:
    _LOCALE_DIR = Path(__file__).resolve().parent.parent / "po" / "locale"
_current_lang = "en"


def _detect_system_lang() -> str:
    if sys.platform == "darwin":
        try:
            import subprocess
            result = subprocess.run(
                ["defaults", "read", "-g", "AppleLanguages"],
                capture_output=True, text=True, timeout=2,
            )
            for line in result.stdout.splitlines():
                code = line.strip().strip('",').split("-")[0].lower()
                if any(lc == code for lc, _, _ in LANG_META):
                    return code
        except Exception:
            pass
    for env_var in ("LANG", "LC_ALL", "LC_MESSAGES", "LANGUAGE"):
        val = os.environ.get(env_var, "")
        if val:
            code = val.split(".")[0].split("_")[0].lower()
            if any(lc == code for lc, _, _ in LANG_META):
                return code
    return "en"


def _load_translations(lang: str):
    try:
        return gettext.translation(
            "firestaff_studio", localedir=str(_LOCALE_DIR),
            languages=[lang], fallback=True,
        )
    except Exception:
        return gettext.NullTranslations()


_current_lang = _detect_system_lang()
_trans = _load_translations(_current_lang)
_ = _trans.gettext


# ── Self-test gates ──────────────────────────────────────────────────────

def require_supported_tk(tk_module: Any) -> None:
    version = tuple(int(p) for p in str(tk_module.TkVersion).split(".")[:2])
    if version < (8, 6):
        raise RuntimeError(f"Tk {tk_module.TkVersion} unsupported; need 8.6+")

if "--check-tkinter" in sys.argv:
    try:
        import tkinter as tk
        require_supported_tk(tk)
    except Exception as exc:
        raise SystemExit(f"Tkinter check failed: {exc}")
    print(f"Tkinter runtime check: PASS (Tk {tk.TkVersion})")
    raise SystemExit(0)

if "--self-test" in sys.argv:
    print("firestaff_savegame_editor self-test: PASS")
    raise SystemExit(0)

if "--smoke-ui" in sys.argv:
    import tkinter as tk
    require_supported_tk(tk)
    root = tk.Tk()
    root.title("smoke")
    root.after(200, root.destroy)
    root.mainloop()
    print("smoke-ui: PASS")
    raise SystemExit(0)


# ── Lookup tables ────────────────────────────────────────────────────────

DM1_FORMAT_NAMES = {
    5: "PC 3.4 (DOS)", 4: "PC 3.3", 3: "Amiga 3.6",
    2: "Atari ST 3.6", 1: "Apple IIgs 3.1", 0: "Unknown (0)",
}
DM1_PLATFORM_NAMES = {
    1: "IBM PC (DOS)", 2: "Amiga", 3: "Atari ST", 4: "Apple IIgs",
    5: "Sharp X68000", 6: "FM Towns", 7: "SNES (Super Famicom)",
    8: "Macintosh", 9: "PC Engine (TurboGrafx-16)", 10: "Sega Mega-CD",
}
DM1_DUNGEON_NAMES = {
    0: "Dungeon Master", 1: "Chaos Strikes Back",
    2: "Dungeon Master — Custom Dungeon", 10: "Theron's Quest dungeon",
}
PART_DESCRIPTIONS = {
    "GLOBAL_DATA": _("Global dungeon state: event counts, active group limits, RNG seed. Fixed 128 bytes."),
    "ACTIVE_GROUP": _("Active creature/item groups in the dungeon. Variable size from GLOBAL_DATA @46."),
    "PARTY": _("Champion records (4 x 319 bytes) + party info (128 bytes). HP, stats, skills, inventory."),
    "EVENTS": _("Dungeon event list: timers, triggers, sensors. Variable size from GLOBAL_DATA @28."),
    "TIMELINE": _("Timeline/timer queue for scheduled events. Remainder of file after other parts."),
}
GAME_LABELS = {
    "dm1": "Dungeon Master", "csb": "Chaos Strikes Back",
    "dm2": "Dungeon Master II", "theron": "Theron's Quest", "nexus": "DM Nexus",
}
CHAMPION_NAMES_DM1 = [
    "Halk", "Stamm", "Zed", "Leyla", "Mophus", "Wuuf",
    "Sonja", "Iaido", "Nabi", "Linflas", "Elija", "Chani",
    "Hawk", "Boris", "Alex", "Azizi", "Syra", "Gothmog",
    "Hissssa", "Daroou", "Wu Tse", "Gando", "Tiggy", "Leif",
]


class SaveField:
    def __init__(self, name, offset, fmt, label="", choices=None,
                 description="", min_val=None, max_val=None):
        self.name = name
        self.offset = offset
        self.fmt = fmt
        self.label = label or name
        self.description = description
        self.choices = choices
        self.min_val = min_val
        self.max_val = max_val
        self.size = struct.calcsize(fmt)

    def read(self, data):
        return struct.unpack_from(self.fmt, data, self.offset)[0]

    def write(self, data, value):
        struct.pack_into(self.fmt, data, self.offset, value)


# ── DM1 PC34 format ─────────────────────────────────────────────────────

DM1_PC34_HEADER_SIZE = 512
DM1_PART_NAMES = ["GLOBAL_DATA", "ACTIVE_GROUP", "PARTY", "EVENTS", "TIMELINE"]

DM1_PC34_HEADER_FIELDS = [
    SaveField("random_words", 0, "<H", _("Random Words [0-148]"),
              description=_("XOR obfuscation pad — 149 random 16-bit words (298 bytes)")),
    SaveField("format_version", 299, "<B", _("Format Version"),
              description=_("Save format version. 5=PC 3.4, 4=PC 3.3, 3=Amiga 3.6")),
    SaveField("game_id", 306, "<H", _("Game ID"),
              description=_("Identifies which game created this save")),
    SaveField("key_part0", 310, "<H", _("Key: GLOBAL_DATA"),
              description=_("XOR deobfuscation key for GLOBAL_DATA")),
    SaveField("key_part1", 312, "<H", _("Key: ACTIVE_GROUP"),
              description=_("XOR deobfuscation key for ACTIVE_GROUP")),
    SaveField("key_part2", 314, "<H", _("Key: PARTY"),
              description=_("XOR deobfuscation key for PARTY")),
    SaveField("key_part3", 316, "<H", _("Key: EVENTS"),
              description=_("XOR deobfuscation key for EVENTS")),
    SaveField("key_part4", 318, "<H", _("Key: TIMELINE"),
              description=_("XOR deobfuscation key for TIMELINE")),
    SaveField("cs_part0", 342, "<H", _("Checksum: GLOBAL_DATA"),
              description=_("F0418 checksum for GLOBAL_DATA validation")),
    SaveField("cs_part1", 344, "<H", _("Checksum: ACTIVE_GROUP"),
              description=_("F0418 checksum for ACTIVE_GROUP validation")),
    SaveField("cs_part2", 346, "<H", _("Checksum: PARTY"),
              description=_("F0418 checksum for PARTY validation")),
    SaveField("cs_part3", 348, "<H", _("Checksum: EVENTS"),
              description=_("F0418 checksum for EVENTS validation")),
    SaveField("cs_part4", 350, "<H", _("Checksum: TIMELINE"),
              description=_("F0418 checksum for TIMELINE validation")),
    SaveField("platform", 374, "<H", _("Platform"),
              description=_("Target platform. 1=PC, 2=Amiga, 9=PC Engine, etc.")),
    SaveField("dungeon_id", 376, "<H", _("Dungeon ID"),
              description=_("Dungeon. 0=DM1, 1=CSB, 10=Theron")),
]

DM1_CHAMPION_SIZE = 319
DM1_CHAMPION_FIELDS = [
    SaveField("name", 0, "8s", _("Name")),
    SaveField("title", 8, "20s", _("Title")),
    SaveField("direction", 28, "<H", _("Direction"), choices=["N","E","S","W"]),
    SaveField("action_index", 32, "<H", _("Action Index")),
    SaveField("poison", 42, "<H", _("Poison"), min_val=0, max_val=65535),
    SaveField("hp_current", 52, "<H", _("HP Current"), min_val=0, max_val=999),
    SaveField("hp_max", 54, "<H", _("HP Max"), min_val=1, max_val=999),
    SaveField("stamina_current", 56, "<H", _("Stamina Current"), min_val=0, max_val=9999),
    SaveField("stamina_max", 58, "<H", _("Stamina Max"), min_val=1, max_val=9999),
    SaveField("mana_current", 60, "<H", _("Mana Current"), min_val=0, max_val=999),
    SaveField("mana_max", 62, "<H", _("Mana Max"), min_val=1, max_val=999),
    SaveField("food", 66, "<H", _("Food"), min_val=0, max_val=2048),
    SaveField("water", 68, "<H", _("Water"), min_val=0, max_val=2048),
    SaveField("load", 271, "<H", _("Load"), min_val=0, max_val=65535),
]

DM1_SKILL_FIELDS = [
    SaveField("fighter", 91, "<B", _("Fighter"), min_val=0, max_val=15),
    SaveField("ninja", 93, "<B", _("Ninja"), min_val=0, max_val=15),
    SaveField("priest", 95, "<B", _("Priest"), min_val=0, max_val=15),
    SaveField("wizard", 97, "<B", _("Wizard"), min_val=0, max_val=15),
    SaveField("swing", 99, "<B", _("Swing"), min_val=0, max_val=15),
    SaveField("thrust", 101, "<B", _("Thrust"), min_val=0, max_val=15),
    SaveField("club", 103, "<B", _("Club"), min_val=0, max_val=15),
    SaveField("parry", 105, "<B", _("Parry"), min_val=0, max_val=15),
    SaveField("steal", 107, "<B", _("Steal"), min_val=0, max_val=15),
    SaveField("fight", 109, "<B", _("Fight"), min_val=0, max_val=15),
    SaveField("throw", 111, "<B", _("Throw"), min_val=0, max_val=15),
    SaveField("shoot", 113, "<B", _("Shoot"), min_val=0, max_val=15),
    SaveField("identify", 115, "<B", _("Identify"), min_val=0, max_val=15),
    SaveField("heal", 117, "<B", _("Heal"), min_val=0, max_val=15),
    SaveField("influence", 119, "<B", _("Influence"), min_val=0, max_val=15),
    SaveField("defend", 121, "<B", _("Defend"), min_val=0, max_val=15),
    SaveField("fire", 123, "<B", _("Fire"), min_val=0, max_val=15),
    SaveField("air", 125, "<B", _("Air"), min_val=0, max_val=15),
    SaveField("earth", 127, "<B", _("Earth"), min_val=0, max_val=15),
    SaveField("water_spell", 129, "<B", _("Water (spell)"), min_val=0, max_val=15),
]

DM1_GLOBAL_FIELDS = [
    SaveField("event_count", 24, "<H", _("Event Count"),
              description=_("Number of active events")),
    SaveField("first_unused_event", 26, "<H", _("First Unused Event"),
              description=_("Index of first free event slot")),
    SaveField("event_max_count", 28, "<H", _("Event Max Count"),
              description=_("Max events (determines EVENTS part size)")),
    SaveField("active_group_count", 30, "<H", _("Active Group Count"),
              description=_("Number of active creature/item groups")),
    SaveField("max_active_group_count", 46, "<H", _("Max Active Group Count"),
              description=_("Max active groups (determines ACTIVE_GROUP size)")),
]

DM1_PARTY_INFO_FIELDS = [
    SaveField("magical_light", 0, "<H", _("Magical Light"),
              description=_("Light spell intensity (0=dark)")),
    SaveField("thieves_eye", 2, "<B", _("Thief's Eye"),
              description=_("Thief's Eye active (0/1)")),
    SaveField("footprints", 3, "<B", _("Footprints"),
              description=_("Footprints active (0/1)")),
    SaveField("shield", 4, "<H", _("Shield"),
              description=_("Shield spell strength")),
    SaveField("fire_shield", 6, "<H", _("Fire Shield"),
              description=_("Fire Shield strength")),
    SaveField("spell_shield", 8, "<H", _("Spell Shield"),
              description=_("Spell Shield (Antimagic) strength")),
    SaveField("scent", 10, "<B", _("Calming Scent"),
              description=_("Calming influence active")),
    SaveField("freeze_life", 11, "<B", _("Freeze Life"),
              description=_("Freeze Life active (0/1)")),
]


# ── F0417/F0418 ──────────────────────────────────────────────────────────

def f0417_deobfuscate_part(data, key):
    buf = bytearray(data)
    checksum = key & 0xFFFF
    wc = len(buf) // 2
    for i in range(wc):
        val = struct.unpack_from("<H", buf, i * 2)[0]
        checksum = (checksum + val) & 0xFFFF
        val ^= key & 0xFFFF
        struct.pack_into("<H", buf, i * 2, val)
        key = (key + wc - i) & 0xFFFF
        checksum = (checksum + val) & 0xFFFF
    return buf, checksum

def f0418_checksum_part(data, key):
    checksum = key & 0xFFFF
    wc = len(data) // 2
    for i in range(wc):
        val = struct.unpack_from("<H", data, i * 2)[0]
        checksum = (checksum + val) & 0xFFFF
        deob = val ^ (key & 0xFFFF)
        key = (key + wc - i) & 0xFFFF
        checksum = (checksum + deob) & 0xFFFF
    return checksum

def dm1_pc34_decode_parts(raw):
    hdr = raw[:DM1_PC34_HEADER_SIZE]
    keys = [struct.unpack_from("<H", hdr, 310 + i*2)[0] for i in range(5)]
    csums = [struct.unpack_from("<H", hdr, 342 + i*2)[0] for i in range(5)]
    cursor = DM1_PC34_HEADER_SIZE
    parts = {}

    def add_part(name, size, idx):
        nonlocal cursor
        if size <= 0 or cursor + size > len(raw):
            cursor += max(size, 0)
            return
        r = bytearray(raw[cursor:cursor+size])
        d, _ = f0417_deobfuscate_part(r, keys[idx])
        actual_cs = f0418_checksum_part(r, keys[idx])
        parts[name] = {
            "offset": cursor, "size": size,
            "raw": r, "data": d,
            "key": keys[idx], "checksum_expected": csums[idx],
            "checksum_actual": actual_cs,
            "valid": actual_cs == csums[idx] if name != "TIMELINE" else True,
        }
        cursor += size

    add_part("GLOBAL_DATA", 128, 0)
    gd = parts.get("GLOBAL_DATA")
    max_ag = struct.unpack_from("<H", gd["data"], 46)[0] if gd else 0
    ev_max = struct.unpack_from("<H", gd["data"], 28)[0] if gd else 0

    add_part("ACTIVE_GROUP", max_ag * 2, 1)
    add_part("PARTY", 4 * DM1_CHAMPION_SIZE + 128, 2)
    add_part("EVENTS", ev_max * 4, 3)
    add_part("TIMELINE", (len(raw) - cursor) & ~1, 4)
    return parts


# ── Savegame container ───────────────────────────────────────────────────

class Savegame:
    def __init__(self, game, path, raw):
        self.game = game
        self.path = path
        self.raw = bytearray(raw)
        self.header_fields = []
        self.champion_count = 0
        self.champion_names = []
        self.header_data = bytearray()
        self.parts = {}
        self.modified = False
        self._parse()

    def _parse(self):
        if self.game in ("dm1", "csb"):
            self.header_data = bytearray(self.raw[:DM1_PC34_HEADER_SIZE])
            self.header_fields = DM1_PC34_HEADER_FIELDS
            self.champion_count = 4
            self.parts = dm1_pc34_decode_parts(self.raw)
            party = self.parts.get("PARTY")
            if party:
                names = []
                for i in range(4):
                    off = i * DM1_CHAMPION_SIZE
                    rn = party["data"][off:off+8].split(b"\x00")[0].decode("ascii", errors="replace").strip()
                    names.append(rn if rn else _("Champion {}").format(i+1))
                self.champion_names = names
            else:
                self.champion_names = [_("Champion {}").format(i+1) for i in range(4)]

    def get_header_value(self, field):
        return field.read(self.header_data)

    def get_champion_data(self, idx):
        p = self.parts.get("PARTY")
        if not p: return bytearray()
        off = idx * DM1_CHAMPION_SIZE
        return bytearray(p["data"][off:off+DM1_CHAMPION_SIZE])

    def get_party_info(self):
        p = self.parts.get("PARTY")
        if not p: return bytearray()
        off = 4 * DM1_CHAMPION_SIZE
        return bytearray(p["data"][off:off+128])

    def get_global_data(self):
        g = self.parts.get("GLOBAL_DATA")
        return bytearray(g["data"]) if g else bytearray()

    def save(self, path=None):
        (path or self.path).write_bytes(bytes(self.raw))
        self.modified = False


def detect_game(data):
    if len(data) >= 512 and data[299] in (3, 4, 5):
        return "dm1"
    return None


# ── Tk GUI ───────────────────────────────────────────────────────────────

import tkinter as tk
from tkinter import ttk, filedialog, messagebox


class SavegameEditor(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title(_("Firestaff Savegame Editor"))
        self.geometry("1000x750")
        self.minsize(800, 550)
        self.savegame = None
        self.status = tk.StringVar(value=_("Ready"))
        self._field_vars = {}
        self._champ_vars = {}
        self._icon_ref = None
        self._set_app_identity()
        self._build_ui()

    def _set_app_identity(self):
        if sys.platform == "darwin":
            try:
                from ctypes import cdll, c_void_p, c_char_p
                objc = cdll.LoadLibrary("/usr/lib/libobjc.dylib")
                objc.objc_getClass.restype = c_void_p
                objc.sel_registerName.restype = c_void_p
                objc.objc_msgSend.restype = c_void_p
                objc.objc_msgSend.argtypes = [c_void_p, c_void_p]
                pi = objc.objc_msgSend(
                    objc.objc_getClass(b"NSProcessInfo"),
                    objc.sel_registerName(b"processInfo"))
                objc.objc_msgSend.argtypes = [c_void_p, c_void_p, c_char_p]
                ns = objc.objc_msgSend(
                    objc.objc_getClass(b"NSString"),
                    objc.sel_registerName(b"stringWithUTF8String:"),
                    b"Firestaff Savegame Editor")
                objc.objc_msgSend.argtypes = [c_void_p, c_void_p, c_void_p]
                objc.objc_msgSend(pi, objc.sel_registerName(b"setProcessName:"), ns)
            except Exception:
                pass
            icns = Path(__file__).resolve().parent.parent / "assets" / "icons" / "firestaff.icns"
            if icns.exists():
                try:
                    import subprocess
                    png = Path("/tmp/firestaff_icon.png")
                    subprocess.run(["sips", "-s", "format", "png", str(icns),
                                    "--out", str(png)], capture_output=True, timeout=5)
                    if png.exists():
                        img = tk.PhotoImage(file=str(png))
                        self.iconphoto(True, img)
                        self._icon_ref = img
                except Exception:
                    pass

    def _build_ui(self):
        menubar = tk.Menu(self)
        fm = tk.Menu(menubar, tearoff=0)
        acc = "Cmd" if sys.platform == "darwin" else "Ctrl"
        fm.add_command(label=_("Open..."), command=self.open_file, accelerator=f"{acc}+O")
        fm.add_command(label=_("Save"), command=self.save_file, accelerator=f"{acc}+S")
        fm.add_command(label=_("Save As..."), command=self.save_as)
        fm.add_separator()
        fm.add_command(label=_("Quit"), command=self.quit)
        menubar.add_cascade(label=_("File"), menu=fm)

        lm = tk.Menu(menubar, tearoff=0)
        for code, flag, name in LANG_META:
            lm.add_command(label=f"{flag} {name}", command=lambda c=code: self._switch_lang(c))
        menubar.add_cascade(label=_("Language"), menu=lm)
        self.config(menu=menubar)

        mod = "Command" if sys.platform == "darwin" else "Control"
        self.bind_all(f"<{mod}-o>", lambda e: self.open_file())
        self.bind_all(f"<{mod}-s>", lambda e: self.save_file())

        tb = ttk.Frame(self, padding=4)
        tb.pack(fill="x")
        ttk.Button(tb, text=_("Open"), command=self.open_file).pack(side="left", padx=2)
        ttk.Button(tb, text=_("Save"), command=self.save_file).pack(side="left", padx=2)
        ttk.Button(tb, text=_("Save As"), command=self.save_as).pack(side="left", padx=2)
        ttk.Separator(tb, orient="vertical").pack(side="left", fill="y", padx=8)
        self._game_label = ttk.Label(tb, text=_("No file loaded"))
        self._game_label.pack(side="left", padx=4)

        self._nb = ttk.Notebook(self)
        self._nb.pack(fill="both", expand=True, padx=4, pady=4)

        tabs = ["Overview", "Header", "Save Parts", "Champions",
                "Party Info", "Global Data", "Header Words", "Hex View"]
        self._frames = {}
        for t in tabs:
            f = ttk.Frame(self._nb, padding=8)
            self._nb.add(f, text=_(t))
            self._frames[t] = f

        hf = self._frames["Hex View"]
        self._hex_text = tk.Text(hf, font=("Menlo", 11), state="disabled", wrap="none")
        sy = ttk.Scrollbar(hf, command=self._hex_text.yview)
        sx = ttk.Scrollbar(hf, orient="horizontal", command=self._hex_text.xview)
        self._hex_text.configure(yscrollcommand=sy.set, xscrollcommand=sx.set)
        sy.pack(side="right", fill="y")
        sx.pack(side="bottom", fill="x")
        self._hex_text.pack(fill="both", expand=True)

        ttk.Label(self, textvariable=self.status, relief="sunken", padding=(4,2)).pack(fill="x", side="bottom")

    def open_file(self):
        path = filedialog.askopenfilename(
            title=_("Open Savegame"),
            filetypes=[(_("All Savegames"), "*.DAT *.dat *.tqsv *.fnxs *.sav"),
                       (_("All Files"), "*")])
        if not path: return
        p = Path(path)
        try: data = p.read_bytes()
        except Exception as e:
            messagebox.showerror(_("Error"), str(e)); return
        game = detect_game(data)
        if not game:
            messagebox.showerror(_("Error"), _("Unrecognized savegame format.")); return
        self.savegame = Savegame(game, p, data)
        self._game_label.config(text=f"{GAME_LABELS.get(game,game)} — {p.name} ({len(data):,} bytes)")
        self.status.set(_("Loaded: {}").format(p.name))
        self._populate_all()

    def save_file(self):
        if not self.savegame:
            messagebox.showwarning(_("Warning"), _("No savegame loaded")); return
        try: self.savegame.save(); self.status.set(_("Saved: {}").format(self.savegame.path.name))
        except Exception as e: messagebox.showerror(_("Error"), str(e))

    def save_as(self):
        if not self.savegame:
            messagebox.showwarning(_("Warning"), _("No savegame loaded")); return
        path = filedialog.asksaveasfilename(title=_("Save As"),
            defaultextension=self.savegame.path.suffix, initialfile=self.savegame.path.name)
        if not path: return
        try: self.savegame.save(Path(path)); self.status.set(_("Saved as: {}").format(Path(path).name))
        except Exception as e: messagebox.showerror(_("Error"), str(e))

    def _populate_all(self):
        self._populate_overview()
        self._populate_header()
        self._populate_parts()
        self._populate_champions()
        self._populate_party_info()
        self._populate_global_data()
        self._populate_header_words()
        self._populate_hex()

    def _clear(self, key):
        for w in self._frames[key].winfo_children(): w.destroy()

    def _scrollable(self, key):
        f = self._frames[key]
        c = tk.Canvas(f)
        s = ttk.Scrollbar(f, orient="vertical", command=c.yview)
        inner = ttk.Frame(c, padding=8)
        inner.bind("<Configure>", lambda e: c.configure(scrollregion=c.bbox("all")))
        c.create_window((0,0), window=inner, anchor="nw")
        c.configure(yscrollcommand=s.set)
        s.pack(side="right", fill="y")
        c.pack(fill="both", expand=True)
        return inner

    # ── Overview ─────────────────────────────────────────────────────────

    def _populate_overview(self):
        self._clear("Overview")
        sg = self.savegame
        if not sg: return
        inner = self._scrollable("Overview")

        ttk.Label(inner, text=_("File Overview"), font=("",14,"bold")).pack(anchor="w", pady=(0,8))

        info = ttk.LabelFrame(inner, text=_("File Information"), padding=8)
        info.pack(fill="x", pady=4)
        fmt = sg.header_data[299] if len(sg.header_data) > 299 else 0
        plat = struct.unpack_from("<H", sg.header_data, 374)[0] if len(sg.header_data) > 375 else 0
        dung = struct.unpack_from("<H", sg.header_data, 376)[0] if len(sg.header_data) > 377 else 0
        for i, (l, v) in enumerate([
            (_("File"), f"{sg.path.name} ({len(sg.raw):,} bytes)"),
            (_("Game"), GAME_LABELS.get(sg.game, sg.game)),
            (_("Format"), f"{fmt} — {DM1_FORMAT_NAMES.get(fmt, _('Unknown'))}"),
            (_("Platform"), f"{plat} — {DM1_PLATFORM_NAMES.get(plat, _('Unknown'))}"),
            (_("Dungeon"), f"{dung} — {DM1_DUNGEON_NAMES.get(dung, _('Unknown'))}"),
        ]):
            ttk.Label(info, text=f"{l}:", font=("",11,"bold")).grid(row=i, column=0, sticky="w", padx=(0,12), pady=2)
            ttk.Label(info, text=v, font=("",11)).grid(row=i, column=1, sticky="w", pady=2)

        pf = ttk.LabelFrame(inner, text=_("Save Parts"), padding=8)
        pf.pack(fill="x", pady=8)
        for c, t in enumerate([_("Part"), _("Offset"), _("Size"), _("Status")]):
            ttk.Label(pf, text=t, font=("",10,"bold")).grid(row=0, column=c, sticky="w", padx=(0,12))
        for i, name in enumerate(DM1_PART_NAMES):
            part = sg.parts.get(name)
            r = i + 1
            ttk.Label(pf, text=name).grid(row=r, column=0, sticky="w", padx=(0,12), pady=1)
            if part:
                ttk.Label(pf, text=f"0x{part['offset']:04X}").grid(row=r, column=1, sticky="w", padx=(0,12))
                ttk.Label(pf, text=f"{part['size']:,} bytes").grid(row=r, column=2, sticky="w", padx=(0,12))
                ok = part["valid"]
                ttk.Label(pf, text=_("Valid") if ok else _("MISMATCH"),
                          foreground="green" if ok else "red").grid(row=r, column=3, sticky="w")

        if sg.champion_count > 0 and "PARTY" in sg.parts:
            cf = ttk.LabelFrame(inner, text=_("Champions"), padding=8)
            cf.pack(fill="x", pady=8)
            for ci in range(sg.champion_count):
                cd = sg.get_champion_data(ci)
                if not cd or len(cd) < 70: continue
                nm = sg.champion_names[ci] if ci < len(sg.champion_names) else "?"
                card = ttk.LabelFrame(cf, text=nm, padding=6)
                card.pack(side="left", fill="both", expand=True, padx=4)
                hp_c, hp_m = struct.unpack_from("<HH", cd, 52)
                st_c, st_m = struct.unpack_from("<HH", cd, 56)
                mn_c, mn_m = struct.unpack_from("<HH", cd, 60)
                food, water = struct.unpack_from("<HH", cd, 66)
                for lbl, cur, mx in [("HP",hp_c,hp_m),("Sta",st_c,st_m),("Mana",mn_c,mn_m)]:
                    ttk.Label(card, text=f"{lbl}: {cur}/{mx}", font=("",10)).pack(anchor="w")
                    ttk.Progressbar(card, length=120, maximum=max(mx,1), value=min(cur,mx)).pack(fill="x", pady=(0,2))
                ttk.Label(card, text=f"{_('Food')}: {food}  {_('Water')}: {water}",
                          font=("",9), foreground="gray").pack(anchor="w")

    # ── Header ───────────────────────────────────────────────────────────

    def _populate_header(self):
        self._clear("Header")
        self._field_vars.clear()
        sg = self.savegame
        if not sg: return
        inner = self._scrollable("Header")

        ttk.Label(inner, text=_("Header Fields") + f" ({DM1_PC34_HEADER_SIZE} bytes)",
                  font=("",13,"bold")).grid(row=0, column=0, columnspan=4, sticky="w", pady=(0,8))
        for c, t in enumerate([_("Field"), _("Value"), _("Hex"), _("Description")]):
            ttk.Label(inner, text=t, font=("",10,"bold")).grid(row=1, column=c, sticky="w")

        for i, field in enumerate(sg.header_fields, start=2):
            if field.name.startswith("random"):
                ttk.Label(inner, text=field.label, foreground="gray").grid(row=i, column=0, sticky="w")
                ttk.Label(inner, text="(298 bytes)", foreground="gray").grid(row=i, column=1, sticky="w")
                ttk.Label(inner, text=field.description, foreground="gray", wraplength=300).grid(row=i, column=3, sticky="w", padx=(8,0))
                continue
            val = sg.get_header_value(field)
            ttk.Label(inner, text=f"{field.label} @{field.offset}").grid(row=i, column=0, sticky="w", padx=(0,8))
            var = tk.StringVar(value=str(val))
            self._field_vars[field.name] = var
            ttk.Entry(inner, textvariable=var, width=16).grid(row=i, column=1, sticky="w", pady=2)
            ttk.Label(inner, text=f"0x{val:04X}", foreground="gray").grid(row=i, column=2, sticky="w", padx=(8,0))
            desc = field.description
            if field.name == "format_version":
                desc = f"{DM1_FORMAT_NAMES.get(val, _('Unknown'))}. {desc}"
            elif field.name == "platform":
                desc = f"{DM1_PLATFORM_NAMES.get(val, _('Unknown'))}. {desc}"
            elif field.name == "dungeon_id":
                desc = f"{DM1_DUNGEON_NAMES.get(val, _('Unknown'))}. {desc}"
            if desc:
                ttk.Label(inner, text=desc, foreground="gray", wraplength=300).grid(row=i, column=3, sticky="w", padx=(8,0))

    # ── Save Parts ───────────────────────────────────────────────────────

    def _populate_parts(self):
        self._clear("Save Parts")
        sg = self.savegame
        if not sg: return
        inner = self._scrollable("Save Parts")

        ttk.Label(inner, text=_("DM1 PC34 Save Parts (F0417/F0418)"),
                  font=("",13,"bold")).pack(anchor="w", pady=(0,8))

        for name in DM1_PART_NAMES:
            part = sg.parts.get(name)
            if not part: continue
            lf = ttk.LabelFrame(inner, text=name, padding=8)
            lf.pack(fill="x", pady=4)
            desc = PART_DESCRIPTIONS.get(name, "")
            if desc:
                ttk.Label(lf, text=desc, foreground="gray", wraplength=700).pack(anchor="w", pady=(0,4))
            row = ttk.Frame(lf)
            row.pack(fill="x")
            for c, (l, v) in enumerate([
                (_("Offset"), f"0x{part['offset']:04X}"),
                (_("Size"), f"{part['size']:,} bytes"),
                (_("Key"), f"0x{part['key']:04X}"),
                (_("Expected"), f"0x{part['checksum_expected']:04X}"),
                (_("Actual"), f"0x{part['checksum_actual']:04X}"),
            ]):
                ttk.Label(row, text=f"{l}:", font=("",10,"bold")).grid(row=0, column=c*2, sticky="w")
                ttk.Label(row, text=v).grid(row=0, column=c*2+1, sticky="w", padx=(0,12))
            ok = part["valid"]
            ttk.Label(row, text=_("VALID") if ok else _("MISMATCH"),
                      foreground="green" if ok else "red", font=("",10,"bold")).grid(row=0, column=10, sticky="w")

            ht = tk.Text(lf, font=("Menlo",10), height=4, state="disabled", wrap="none")
            ht.pack(fill="x", pady=(4,0))
            ht.config(state="normal")
            preview = part["data"][:64]
            lines = []
            for off in range(0, len(preview), 16):
                ch = preview[off:off+16]
                h = " ".join(f"{b:02X}" for b in ch)
                a = "".join(chr(b) if 32 <= b < 127 else "." for b in ch)
                lines.append(f"{off:04X}  {h:<48s}  |{a}|")
            ht.insert("1.0", "\n".join(lines))
            ht.config(state="disabled")

    # ── Champions ────────────────────────────────────────────────────────

    def _populate_champions(self):
        self._clear("Champions")
        self._champ_vars.clear()
        sg = self.savegame
        if not sg or sg.champion_count == 0:
            ttk.Label(self._frames["Champions"], text=_("No champion data")).pack(pady=20)
            return
        nb = ttk.Notebook(self._frames["Champions"])
        nb.pack(fill="both", expand=True)
        for ci in range(sg.champion_count):
            nm = sg.champion_names[ci] if ci < len(sg.champion_names) else _("Champion {}").format(ci+1)
            frame = ttk.Frame(nb, padding=8)
            nb.add(frame, text=nm)
            cd = sg.get_champion_data(ci)
            if not cd: continue

            c = tk.Canvas(frame)
            s = ttk.Scrollbar(frame, orient="vertical", command=c.yview)
            inner = ttk.Frame(c, padding=4)
            inner.bind("<Configure>", lambda e: c.configure(scrollregion=c.bbox("all")))
            c.create_window((0,0), window=inner, anchor="nw")
            c.configure(yscrollcommand=s.set)
            s.pack(side="right", fill="y")
            c.pack(fill="both", expand=True)

            rn = cd[:8].split(b"\x00")[0].decode("ascii", errors="replace")
            rt = cd[8:28].split(b"\x00")[0].decode("ascii", errors="replace")
            ttk.Label(inner, text=f"{rn} — {rt}", font=("",12,"bold")).grid(
                row=0, column=0, columnspan=3, sticky="w", pady=(0,8))

            ttk.Label(inner, text=_("Combat & Vitals"), font=("",11,"bold")).grid(
                row=1, column=0, columnspan=3, sticky="w", pady=(4,2))
            r = 2
            for field in DM1_CHAMPION_FIELDS:
                if field.name in ("name","title"): continue
                val = field.read(cd)
                ttk.Label(inner, text=f"{field.label} @{field.offset}").grid(row=r, column=0, sticky="w", padx=(0,8))
                k = f"c{ci}_{field.name}"
                v = tk.StringVar(value=str(val))
                self._champ_vars[k] = v
                ttk.Entry(inner, textvariable=v, width=12).grid(row=r, column=1, sticky="w", pady=1)
                if field.min_val is not None:
                    ttk.Label(inner, text=f"({field.min_val}–{field.max_val})", foreground="gray").grid(
                        row=r, column=2, sticky="w", padx=(4,0))
                r += 1

            ttk.Label(inner, text=_("Skills"), font=("",11,"bold")).grid(
                row=r, column=0, columnspan=3, sticky="w", pady=(8,2))
            r += 1
            for field in DM1_SKILL_FIELDS:
                val = field.read(cd)
                ttk.Label(inner, text=f"{field.label} @{field.offset}").grid(row=r, column=0, sticky="w", padx=(0,8))
                k = f"c{ci}_s_{field.name}"
                v = tk.StringVar(value=str(val))
                self._champ_vars[k] = v
                ttk.Entry(inner, textvariable=v, width=12).grid(row=r, column=1, sticky="w", pady=1)
                ttk.Label(inner, text="(0–15)", foreground="gray").grid(row=r, column=2, sticky="w", padx=(4,0))
                r += 1

    # ── Party Info ───────────────────────────────────────────────────────

    def _populate_party_info(self):
        self._clear("Party Info")
        sg = self.savegame
        if not sg: return
        pi = sg.get_party_info()
        if not pi or len(pi) < 12:
            ttk.Label(self._frames["Party Info"], text=_("No party info available")).pack(pady=20)
            return
        f = self._frames["Party Info"]
        ttk.Label(f, text=_("Party Info") + " (128 bytes @ PARTY+1276)",
                  font=("",13,"bold")).pack(anchor="w", pady=(0,8))
        grid = ttk.Frame(f, padding=8)
        grid.pack(fill="x")
        for i, field in enumerate(DM1_PARTY_INFO_FIELDS):
            val = field.read(pi)
            ttk.Label(grid, text=f"{field.label} @{field.offset}", font=("",10,"bold")).grid(
                row=i, column=0, sticky="w", padx=(0,12), pady=2)
            ttk.Label(grid, text=str(val)).grid(row=i, column=1, sticky="w", padx=(0,12))
            fmt = f"0x{val:04X}" if field.size >= 2 else f"0x{val:02X}"
            ttk.Label(grid, text=fmt, foreground="gray").grid(row=i, column=2, sticky="w", padx=(0,12))
            if field.description:
                ttk.Label(grid, text=field.description, foreground="gray", wraplength=400).grid(
                    row=i, column=3, sticky="w")

    # ── Global Data ──────────────────────────────────────────────────────

    def _populate_global_data(self):
        self._clear("Global Data")
        sg = self.savegame
        if not sg: return
        gd = sg.get_global_data()
        if not gd or len(gd) < 48:
            ttk.Label(self._frames["Global Data"], text=_("No global data")).pack(pady=20)
            return
        f = self._frames["Global Data"]
        ttk.Label(f, text=_("Global Data") + " (128 bytes, Part 0)",
                  font=("",13,"bold")).pack(anchor="w", pady=(0,8))
        grid = ttk.Frame(f, padding=8)
        grid.pack(fill="x")
        for i, field in enumerate(DM1_GLOBAL_FIELDS):
            val = field.read(gd)
            ttk.Label(grid, text=f"{field.label} @{field.offset}", font=("",10,"bold")).grid(
                row=i, column=0, sticky="w", padx=(0,12), pady=2)
            ttk.Label(grid, text=str(val)).grid(row=i, column=1, sticky="w", padx=(0,12))
            ttk.Label(grid, text=f"0x{val:04X}", foreground="gray").grid(row=i, column=2, sticky="w", padx=(0,12))
            if field.description:
                ttk.Label(grid, text=field.description, foreground="gray", wraplength=400).grid(
                    row=i, column=3, sticky="w")

        ttk.Label(f, text=_("Raw Data"), font=("",11,"bold")).pack(anchor="w", pady=(12,4))
        ht = tk.Text(f, font=("Menlo",10), height=10, state="disabled", wrap="none")
        ht.pack(fill="x")
        ht.config(state="normal")
        lines = []
        for off in range(0, len(gd), 16):
            ch = gd[off:off+16]
            h = " ".join(f"{b:02X}" for b in ch)
            a = "".join(chr(b) if 32 <= b < 127 else "." for b in ch)
            lines.append(f"{off:04X}  {h:<48s}  |{a}|")
        ht.insert("1.0", "\n".join(lines))
        ht.config(state="disabled")

    # ── Header Words ─────────────────────────────────────────────────────

    def _populate_header_words(self):
        self._clear("Header Words")
        sg = self.savegame
        if not sg: return
        f = self._frames["Header Words"]
        ttk.Label(f, text=_("Header Words") + f" ({DM1_PC34_HEADER_SIZE} bytes)",
                  font=("",13,"bold")).pack(anchor="w", pady=(0,8))

        legend = ttk.Frame(f)
        legend.pack(fill="x", pady=(0,4))
        for _tag, lbl, col in [
            ("rnd", _("Random Words"), "#aaa"),
            ("key", _("Part Keys"), "#3498db"),
            ("cs", _("Checksums"), "#e74c3c"),
            ("fmt", _("Format/Platform/Dungeon"), "#2ecc71"),
        ]:
            bx = tk.Canvas(legend, width=12, height=12, highlightthickness=0)
            bx.create_rectangle(0,0,12,12, fill=col, outline="")
            bx.pack(side="left", padx=(0,2))
            ttk.Label(legend, text=lbl).pack(side="left", padx=(0,12))

        ht = tk.Text(f, font=("Menlo",10), state="disabled", wrap="none")
        sc = ttk.Scrollbar(f, orient="vertical", command=ht.yview)
        ht.configure(yscrollcommand=sc.set)
        sc.pack(side="right", fill="y")
        ht.pack(fill="both", expand=True)

        ht.tag_configure("rnd", foreground="#999")
        ht.tag_configure("key", foreground="#3498db", font=("Menlo",10,"bold"))
        ht.tag_configure("cs", foreground="#e74c3c", font=("Menlo",10,"bold"))
        ht.tag_configure("fmt", foreground="#2ecc71", font=("Menlo",10,"bold"))

        key_off = set(range(310, 320))
        cs_off = set(range(342, 352))
        fmt_off = {299, 374, 375, 376, 377}

        ht.config(state="normal")
        hdr = sg.header_data
        for off in range(0, min(len(hdr), DM1_PC34_HEADER_SIZE), 16):
            ht.insert("end", f"{off:04X}  ")
            chunk = hdr[off:off+16]
            for bi, b in enumerate(chunk):
                ao = off + bi
                tag = "key" if ao in key_off else "cs" if ao in cs_off else "fmt" if ao in fmt_off else "rnd"
                ht.insert("end", f"{b:02X} ", tag)
            ht.insert("end", " |")
            ht.insert("end", "".join(chr(b) if 32 <= b < 127 else "." for b in chunk))
            ht.insert("end", "|\n")
        ht.config(state="disabled")

    # ── Hex View ─────────────────────────────────────────────────────────

    def _populate_hex(self):
        self._hex_text.config(state="normal")
        self._hex_text.delete("1.0", "end")
        if not self.savegame:
            self._hex_text.config(state="disabled"); return
        data = self.savegame.raw
        lines = []
        for off in range(0, len(data), 16):
            ch = data[off:off+16]
            h = " ".join(f"{b:02X}" for b in ch)
            a = "".join(chr(b) if 32 <= b < 127 else "." for b in ch)
            lines.append(f"{off:08X}  {h:<48s}  |{a}|")
        self._hex_text.insert("1.0", "\n".join(lines))
        self._hex_text.config(state="disabled")

    def _switch_lang(self, lang_code):
        global _current_lang, _trans, _
        _current_lang = lang_code
        _trans = _load_translations(lang_code)
        _ = _trans.gettext
        messagebox.showinfo(_("Language"),
            _("Language set to {}.\nRestart app for full effect.").format(lang_code))


def main():
    app = SavegameEditor()
    app.mainloop()

if __name__ == "__main__":
    main()
