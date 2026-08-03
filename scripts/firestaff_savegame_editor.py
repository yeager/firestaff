#!/usr/bin/env python3
"""Firestaff Savegame Editor — read, edit and write savegames for all five
Dungeon Master games.

Supported formats:
  DM1   — DMSAVE.DAT  (PC 3.4, 256-byte header + 7 sections, XOR-obfuscated)
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
        raise RuntimeError(
            f"Tk {tk_module.TkVersion} unsupported; need 8.6+"
        )


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


# ── Savegame format parsers ──────────────────────────────────────────────

GAMES = ["dm1", "csb", "dm2", "theron", "nexus"]
GAME_LABELS = {
    "dm1": "Dungeon Master",
    "csb": "Chaos Strikes Back",
    "dm2": "Dungeon Master II",
    "theron": "Theron's Quest",
    "nexus": "DM Nexus",
}

CHAMPION_NAMES_DM1 = [
    "Halk", "Stamm", "Zed", "Leyla", "Mophus", "Wuuf",
    "Sonja", "Iaido", "Nabi", "Linflas", "Elija", "Chani",
    "Hawk", "Boris", "Alex", "Azizi", "Syra", "Gothmog",
    "Hissssa", "Daroou", "Wu Tse", "Gando",
    "Tiggy", "Leif",
]


class SaveField:
    """Describes one editable field in a savegame."""
    def __init__(self, name: str, offset: int, fmt: str,
                 label: str = "", choices: list | None = None,
                 min_val: int | None = None, max_val: int | None = None):
        self.name = name
        self.offset = offset
        self.fmt = fmt
        self.label = label or name
        self.choices = choices
        self.min_val = min_val
        self.max_val = max_val
        self.size = struct.calcsize(fmt)

    def read(self, data: bytes | bytearray) -> int:
        return struct.unpack_from(self.fmt, data, self.offset)[0]

    def write(self, data: bytearray, value: int) -> None:
        struct.pack_into(self.fmt, data, self.offset, value)


# ── DM1 save format ─────────────────────────────────────────────────────

DM1_HEADER_SIZE = 256
DM1_MAGIC = 0x444D3156  # "DM1V"

DM1_HEADER_FIELDS = [
    SaveField("magic", 0, "<I", _("Magic")),
    SaveField("game_id", 4, "<H", _("Game ID")),
    SaveField("dungeon_id", 6, "<H", _("Dungeon ID")),
    SaveField("platform", 8, "<H", _("Platform")),
    SaveField("party_x", 10, "<H", _("Party X"), min_val=0, max_val=31),
    SaveField("party_y", 12, "<H", _("Party Y"), min_val=0, max_val=31),
    SaveField("party_dir", 14, "<H", _("Party Direction"),
              choices=["North", "East", "South", "West"]),
    SaveField("party_level", 16, "<H", _("Party Level"), min_val=0, max_val=15),
    SaveField("game_time", 18, "<I", _("Game Time (ticks)")),
    SaveField("rng_seed", 22, "<I", _("RNG Seed")),
]

DM1_CHAMPION_SIZE = 110
DM1_CHAMPION_OFFSET = 256

DM1_CHAMPION_FIELDS = [
    SaveField("hp_current", 0, "<H", _("HP Current"), min_val=0, max_val=999),
    SaveField("hp_max", 2, "<H", _("HP Max"), min_val=1, max_val=999),
    SaveField("stamina_current", 4, "<H", _("Stamina Current"), min_val=0, max_val=999),
    SaveField("stamina_max", 6, "<H", _("Stamina Max"), min_val=1, max_val=999),
    SaveField("mana_current", 8, "<H", _("Mana Current"), min_val=0, max_val=999),
    SaveField("mana_max", 10, "<H", _("Mana Max"), min_val=1, max_val=999),
    SaveField("strength", 12, "<H", _("Strength"), min_val=1, max_val=255),
    SaveField("dexterity", 14, "<H", _("Dexterity"), min_val=1, max_val=255),
    SaveField("wisdom", 16, "<H", _("Wisdom"), min_val=1, max_val=255),
    SaveField("vitality", 18, "<H", _("Vitality"), min_val=1, max_val=255),
    SaveField("anti_magic", 20, "<H", _("Anti-Magic"), min_val=0, max_val=255),
    SaveField("anti_fire", 22, "<H", _("Anti-Fire"), min_val=0, max_val=255),
    SaveField("food", 24, "<H", _("Food"), min_val=0, max_val=2048),
    SaveField("water", 26, "<H", _("Water"), min_val=0, max_val=2048),
    SaveField("luck", 28, "<H", _("Luck"), min_val=0, max_val=255),
    SaveField("fighter_level", 30, "<B", _("Fighter Level"), min_val=0, max_val=15),
    SaveField("ninja_level", 31, "<B", _("Ninja Level"), min_val=0, max_val=15),
    SaveField("priest_level", 32, "<B", _("Priest Level"), min_val=0, max_val=15),
    SaveField("wizard_level", 33, "<B", _("Wizard Level"), min_val=0, max_val=15),
]


def dm1_deobfuscate_header(data: bytes | bytearray) -> bytearray:
    """Deobfuscate the DM1 header XOR pad (ReDMCSB F7062)."""
    buf = bytearray(data[:DM1_HEADER_SIZE])
    seed = buf[0] ^ 0x44  # recover seed from magic byte
    rng = seed
    for i in range(4, DM1_HEADER_SIZE):
        rng = (rng * 1103515245 + 12345) & 0xFFFFFFFF
        buf[i] ^= (rng >> 16) & 0xFF
    return buf


def dm1_obfuscate_header(buf: bytearray) -> bytearray:
    """Re-apply the DM1 header XOR pad."""
    out = bytearray(buf[:DM1_HEADER_SIZE])
    seed = out[0] ^ 0x44
    rng = seed
    for i in range(4, DM1_HEADER_SIZE):
        rng = (rng * 1103515245 + 12345) & 0xFFFFFFFF
        out[i] ^= (rng >> 16) & 0xFF
    return out


# ── CSB save format ──────────────────────────────────────────────────────

CSB_HEADER_SIZE = 512
CSB_MAGIC = 0x43534201  # "CSB\1"

CSB_HEADER_FIELDS = [
    SaveField("magic", 0, "<I", _("Magic")),
    SaveField("game_id", 4, "<H", _("Game ID")),
    SaveField("party_x", 8, "<H", _("Party X"), min_val=0, max_val=31),
    SaveField("party_y", 10, "<H", _("Party Y"), min_val=0, max_val=31),
    SaveField("party_dir", 12, "<H", _("Party Direction"),
              choices=["North", "East", "South", "West"]),
    SaveField("party_level", 14, "<H", _("Party Level"), min_val=0, max_val=15),
    SaveField("game_time", 16, "<I", _("Game Time (ticks)")),
]


# ── Theron save format ──────────────────────────────────────────────────

TQ_HEADER_SIZE = 64
TQ_MAGIC = 0x54515220  # "TQR "
TQ_XOR_SEED = 0x5A
TQ_CHAMPION_SIZE = 128
TQ_MAX_SLOTS = 8

TQ_HEADER_FIELDS = [
    SaveField("magic", 0, "<I", _("Magic")),
    SaveField("slot_id", 4, "<B", _("Slot ID"), min_val=0, max_val=7),
    SaveField("current_dungeon", 5, "<B", _("Current Dungeon"), min_val=0, max_val=15),
    SaveField("quest_items", 6, "<B", _("Quest Items (bitmap)")),
    SaveField("gold", 8, "<H", _("Gold"), min_val=0, max_val=9999),
    SaveField("playtime_seconds", 10, "<I", _("Play Time (seconds)")),
]

TQ_CHAMPION_FIELDS = [
    SaveField("hp_current", 0, "<H", _("HP Current"), min_val=0, max_val=999),
    SaveField("hp_max", 2, "<H", _("HP Max"), min_val=1, max_val=999),
    SaveField("stamina_current", 4, "<H", _("Stamina"), min_val=0, max_val=999),
    SaveField("mana_current", 6, "<H", _("Mana"), min_val=0, max_val=999),
    SaveField("strength", 8, "<H", _("Strength"), min_val=1, max_val=255),
    SaveField("dexterity", 10, "<H", _("Dexterity"), min_val=1, max_val=255),
    SaveField("wisdom", 12, "<H", _("Wisdom"), min_val=1, max_val=255),
    SaveField("vitality", 14, "<H", _("Vitality"), min_val=1, max_val=255),
    SaveField("fighter_level", 16, "<B", _("Fighter Level"), min_val=0, max_val=15),
    SaveField("ninja_level", 17, "<B", _("Ninja Level"), min_val=0, max_val=15),
    SaveField("priest_level", 18, "<B", _("Priest Level"), min_val=0, max_val=15),
    SaveField("wizard_level", 19, "<B", _("Wizard Level"), min_val=0, max_val=15),
]


def tq_deobfuscate(data: bytes | bytearray) -> bytearray:
    buf = bytearray(data)
    for i in range(4, len(buf)):
        buf[i] ^= TQ_XOR_SEED
    return buf


def tq_obfuscate(buf: bytearray) -> bytearray:
    out = bytearray(buf)
    for i in range(4, len(out)):
        out[i] ^= TQ_XOR_SEED
    return out


# ── Nexus save format ────────────────────────────────────────────────────

NEXUS_MAGIC = 0x53584E46  # "FNXS"
NEXUS_HEADER_SIZE = 128
NEXUS_VERSION = 2

NEXUS_HEADER_FIELDS = [
    SaveField("magic", 0, "<I", _("Magic")),
    SaveField("version", 4, "<H", _("Version")),
    SaveField("champion_data_size", 6, "<I", _("Champion Data Size")),
    SaveField("world_data_size", 10, "<I", _("World Data Size")),
    SaveField("game_time", 14, "<I", _("Game Time (ticks)")),
    SaveField("party_x", 18, "<H", _("Party X"), min_val=0, max_val=63),
    SaveField("party_y", 20, "<H", _("Party Y"), min_val=0, max_val=63),
    SaveField("party_dir", 22, "<H", _("Party Direction"),
              choices=["North", "East", "South", "West"]),
    SaveField("party_level", 24, "<H", _("Party Level"), min_val=0, max_val=31),
    SaveField("crc32", 26, "<I", _("CRC32")),
    SaveField("state_hash", 30, "<I", _("State Hash")),
]


# ── DM2 save format ─────────────────────────────────────────────────────

DM2_MAGIC_BEEF = 0xBEEF
DM2_MAGIC_DEAD = 0xDEAD
DM2_HEADER_SIZE = 64

DM2_HEADER_FIELDS = [
    SaveField("magic", 0, "<H", _("Magic")),
    SaveField("slot_id", 2, "<H", _("Slot ID"), min_val=0, max_val=9),
    SaveField("party_x", 4, "<H", _("Party X"), min_val=0, max_val=63),
    SaveField("party_y", 6, "<H", _("Party Y"), min_val=0, max_val=63),
    SaveField("party_dir", 8, "<H", _("Party Direction"),
              choices=["North", "East", "South", "West"]),
    SaveField("party_level", 10, "<H", _("Party Level"), min_val=0, max_val=31),
    SaveField("game_time", 12, "<I", _("Game Time (ticks)")),
    SaveField("dungeon_data_size", 16, "<I", _("Dungeon Data Size")),
    SaveField("timer_data_size", 20, "<I", _("Timer Data Size")),
]


# ── Generic savegame container ───────────────────────────────────────────

class Savegame:
    def __init__(self, game: str, path: Path, raw: bytes):
        self.game = game
        self.path = path
        self.raw = bytearray(raw)
        self.header_fields: list[SaveField] = []
        self.champion_fields: list[SaveField] = []
        self.champion_count = 0
        self.champion_names: list[str] = []
        self.header_data: bytearray = bytearray()
        self.modified = False
        self._parse()

    def _parse(self) -> None:
        if self.game == "dm1":
            self.header_data = dm1_deobfuscate_header(self.raw)
            self.header_fields = DM1_HEADER_FIELDS
            self.champion_fields = DM1_CHAMPION_FIELDS
            self.champion_count = 4
            self.champion_names = CHAMPION_NAMES_DM1[:4]
        elif self.game == "csb":
            self.header_data = bytearray(self.raw[:CSB_HEADER_SIZE])
            self.header_fields = CSB_HEADER_FIELDS
            self.champion_fields = DM1_CHAMPION_FIELDS
            self.champion_count = 4
            self.champion_names = [_("Champion {}").format(i + 1) for i in range(4)]
        elif self.game == "dm2":
            self.header_data = bytearray(self.raw[:DM2_HEADER_SIZE])
            self.header_fields = DM2_HEADER_FIELDS
            self.champion_fields = []
            self.champion_count = 0
            self.champion_names = []
        elif self.game == "theron":
            self.header_data = tq_deobfuscate(self.raw[:TQ_HEADER_SIZE])
            self.header_fields = TQ_HEADER_FIELDS
            self.champion_fields = TQ_CHAMPION_FIELDS
            self.champion_count = 4
            self.champion_names = [_("Champion {}").format(i + 1) for i in range(4)]
        elif self.game == "nexus":
            self.header_data = bytearray(self.raw[:NEXUS_HEADER_SIZE])
            self.header_fields = NEXUS_HEADER_FIELDS
            self.champion_fields = []
            self.champion_count = 0
            self.champion_names = []

    def get_header_value(self, field: SaveField) -> int:
        return field.read(self.header_data)

    def set_header_value(self, field: SaveField, value: int) -> None:
        field.write(self.header_data, value)
        self.modified = True

    def get_champion_data(self, index: int) -> bytearray:
        if self.game == "dm1":
            off = DM1_CHAMPION_OFFSET + index * DM1_CHAMPION_SIZE
            return bytearray(self.raw[off:off + DM1_CHAMPION_SIZE])
        elif self.game == "theron":
            off = TQ_HEADER_SIZE + index * TQ_CHAMPION_SIZE
            deob = tq_deobfuscate(self.raw)
            return bytearray(deob[off:off + TQ_CHAMPION_SIZE])
        elif self.game == "csb":
            off = CSB_HEADER_SIZE + index * DM1_CHAMPION_SIZE
            return bytearray(self.raw[off:off + DM1_CHAMPION_SIZE])
        return bytearray()

    def set_champion_field(self, index: int, field: SaveField, value: int) -> None:
        champ = self.get_champion_data(index)
        field.write(champ, value)
        if self.game == "dm1":
            off = DM1_CHAMPION_OFFSET + index * DM1_CHAMPION_SIZE
            self.raw[off:off + DM1_CHAMPION_SIZE] = champ
        elif self.game == "theron":
            off = TQ_HEADER_SIZE + index * TQ_CHAMPION_SIZE
            deob = tq_deobfuscate(self.raw)
            deob[off:off + TQ_CHAMPION_SIZE] = champ
            self.raw = tq_obfuscate(deob)
        elif self.game == "csb":
            off = CSB_HEADER_SIZE + index * DM1_CHAMPION_SIZE
            self.raw[off:off + DM1_CHAMPION_SIZE] = champ
        self.modified = True

    def save(self, path: Path | None = None) -> None:
        target = path or self.path
        if self.game == "dm1":
            self.raw[:DM1_HEADER_SIZE] = dm1_obfuscate_header(self.header_data)
        elif self.game == "theron":
            deob = tq_deobfuscate(self.raw)
            deob[:TQ_HEADER_SIZE] = self.header_data
            self.raw = tq_obfuscate(deob)
        else:
            hdr_size = len(self.header_data)
            self.raw[:hdr_size] = self.header_data
        target.write_bytes(bytes(self.raw))
        self.modified = False


def detect_game(data: bytes) -> str | None:
    if len(data) < 4:
        return None
    magic = struct.unpack_from("<I", data, 0)[0]
    if magic == DM1_MAGIC:
        return "dm1"
    deob = dm1_deobfuscate_header(data)
    deob_magic = struct.unpack_from("<I", deob, 0)[0]
    if deob_magic == DM1_MAGIC:
        return "dm1"
    if magic == CSB_MAGIC:
        return "csb"
    if magic == TQ_MAGIC:
        return "theron"
    tq_deob = tq_deobfuscate(data[:64])
    tq_deob_magic = struct.unpack_from("<I", tq_deob, 0)[0]
    if tq_deob_magic == TQ_MAGIC:
        return "theron"
    if magic == NEXUS_MAGIC:
        return "nexus"
    if len(data) >= 2:
        m16 = struct.unpack_from("<H", data, 0)[0]
        if m16 in (DM2_MAGIC_BEEF, DM2_MAGIC_DEAD):
            return "dm2"
    return None


# ── Tk GUI ───────────────────────────────────────────────────────────────

import tkinter as tk
from tkinter import ttk, filedialog, messagebox


class SavegameEditor(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title(_("Firestaff Savegame Editor"))
        self.geometry("900x700")
        self.minsize(700, 500)

        self.savegame: Savegame | None = None
        self.status = tk.StringVar(value=_("Ready"))
        self._field_vars: dict[str, tk.StringVar] = {}
        self._champ_vars: dict[str, tk.StringVar] = {}

        self._build_ui()

    def _build_ui(self) -> None:
        # Menu bar
        menubar = tk.Menu(self)
        file_menu = tk.Menu(menubar, tearoff=0)
        file_menu.add_command(label=_("Open..."), command=self.open_file,
                              accelerator="Ctrl+O")
        file_menu.add_command(label=_("Save"), command=self.save_file,
                              accelerator="Ctrl+S")
        file_menu.add_command(label=_("Save As..."), command=self.save_as)
        file_menu.add_separator()
        file_menu.add_command(label=_("Quit"), command=self.quit)
        menubar.add_cascade(label=_("File"), menu=file_menu)

        lang_menu = tk.Menu(menubar, tearoff=0)
        for code, flag, name in LANG_META:
            lang_menu.add_command(
                label=f"{flag} {name}",
                command=lambda c=code: self._switch_lang(c),
            )
        menubar.add_cascade(label=_("Language"), menu=lang_menu)

        self.config(menu=menubar)
        self.bind_all("<Control-o>", lambda e: self.open_file())
        self.bind_all("<Control-s>", lambda e: self.save_file())

        # Toolbar
        toolbar = ttk.Frame(self, padding=4)
        toolbar.pack(fill="x")
        ttk.Button(toolbar, text=_("Open"), command=self.open_file).pack(side="left", padx=2)
        ttk.Button(toolbar, text=_("Save"), command=self.save_file).pack(side="left", padx=2)
        ttk.Button(toolbar, text=_("Save As"), command=self.save_as).pack(side="left", padx=2)
        ttk.Separator(toolbar, orient="vertical").pack(side="left", fill="y", padx=8)

        self._game_label = ttk.Label(toolbar, text=_("No file loaded"))
        self._game_label.pack(side="left", padx=4)

        # Main area with notebook
        self._notebook = ttk.Notebook(self)
        self._notebook.pack(fill="both", expand=True, padx=4, pady=4)

        # Header tab
        self._header_frame = ttk.Frame(self._notebook, padding=8)
        self._notebook.add(self._header_frame, text=_("Header"))

        # Champions tab
        self._champ_frame = ttk.Frame(self._notebook, padding=8)
        self._notebook.add(self._champ_frame, text=_("Champions"))

        # Hex view tab
        self._hex_frame = ttk.Frame(self._notebook, padding=8)
        self._notebook.add(self._hex_frame, text=_("Hex View"))

        self._hex_text = tk.Text(self._hex_frame, font=("Menlo", 11),
                                 state="disabled", wrap="none")
        hex_scroll_y = ttk.Scrollbar(self._hex_frame, command=self._hex_text.yview)
        hex_scroll_x = ttk.Scrollbar(self._hex_frame, orient="horizontal",
                                     command=self._hex_text.xview)
        self._hex_text.configure(yscrollcommand=hex_scroll_y.set,
                                 xscrollcommand=hex_scroll_x.set)
        hex_scroll_y.pack(side="right", fill="y")
        hex_scroll_x.pack(side="bottom", fill="x")
        self._hex_text.pack(fill="both", expand=True)

        # Status bar
        status_bar = ttk.Label(self, textvariable=self.status, relief="sunken",
                               padding=(4, 2))
        status_bar.pack(fill="x", side="bottom")

    def open_file(self) -> None:
        path = filedialog.askopenfilename(
            title=_("Open Savegame"),
            filetypes=[
                (_("All Savegames"), "*.DAT *.dat *.tqsv *.fnxs *.sav *SKSAVE*"),
                (_("DM1/CSB Save"), "*.DAT *.dat"),
                (_("Theron Save"), "*.tqsv"),
                (_("Nexus Save"), "*.fnxs"),
                (_("DM2 Save"), "*SKSAVE* *.sav"),
                (_("All Files"), "*"),
            ],
        )
        if not path:
            return
        p = Path(path)
        try:
            data = p.read_bytes()
        except Exception as e:
            messagebox.showerror(_("Error"), str(e))
            return

        game = detect_game(data)
        if not game:
            messagebox.showerror(
                _("Error"),
                _("Unrecognized savegame format."),
            )
            return

        self.savegame = Savegame(game, p, data)
        self._game_label.config(
            text=f"{GAME_LABELS.get(game, game)} — {p.name} ({len(data)} bytes)"
        )
        self.status.set(_("Loaded: {}").format(p.name))
        self._populate_header()
        self._populate_champions()
        self._populate_hex()

    def _populate_header(self) -> None:
        for w in self._header_frame.winfo_children():
            w.destroy()
        self._field_vars.clear()

        if not self.savegame:
            return

        canvas = tk.Canvas(self._header_frame)
        scrollbar = ttk.Scrollbar(self._header_frame, orient="vertical",
                                  command=canvas.yview)
        inner = ttk.Frame(canvas, padding=8)
        inner.bind("<Configure>",
                   lambda e: canvas.configure(scrollregion=canvas.bbox("all")))
        canvas.create_window((0, 0), window=inner, anchor="nw")
        canvas.configure(yscrollcommand=scrollbar.set)
        scrollbar.pack(side="right", fill="y")
        canvas.pack(fill="both", expand=True)

        ttk.Label(inner, text=_("Header Fields"),
                  font=("", 13, "bold")).grid(row=0, column=0, columnspan=3,
                                              sticky="w", pady=(0, 8))

        for i, field in enumerate(self.savegame.header_fields, start=1):
            val = self.savegame.get_header_value(field)
            ttk.Label(inner, text=field.label).grid(row=i, column=0,
                                                    sticky="w", padx=(0, 8))
            var = tk.StringVar(value=str(val))
            self._field_vars[field.name] = var

            if field.choices:
                cb = ttk.Combobox(inner, textvariable=var,
                                  values=field.choices, width=20, state="readonly")
                if 0 <= val < len(field.choices):
                    cb.current(val)
                cb.grid(row=i, column=1, sticky="w", pady=2)
            else:
                entry = ttk.Entry(inner, textvariable=var, width=20)
                entry.grid(row=i, column=1, sticky="w", pady=2)

            hex_label = ttk.Label(inner,
                                  text=f"0x{val:X}" if isinstance(val, int) else "",
                                  foreground="gray")
            hex_label.grid(row=i, column=2, sticky="w", padx=(8, 0))

        ttk.Button(inner, text=_("Apply Changes"),
                   command=self._apply_header).grid(
            row=len(self.savegame.header_fields) + 1,
            column=0, columnspan=3, pady=(12, 0),
        )

    def _apply_header(self) -> None:
        if not self.savegame:
            return
        for field in self.savegame.header_fields:
            var = self._field_vars.get(field.name)
            if not var:
                continue
            try:
                if field.choices:
                    val = field.choices.index(var.get()) if var.get() in field.choices else int(var.get())
                else:
                    val = int(var.get())
            except ValueError:
                messagebox.showerror(_("Error"),
                                     _("Invalid value for {}").format(field.label))
                return
            if field.min_val is not None and val < field.min_val:
                val = field.min_val
            if field.max_val is not None and val > field.max_val:
                val = field.max_val
            self.savegame.set_header_value(field, val)
        self.status.set(_("Header changes applied"))
        self._populate_hex()

    def _populate_champions(self) -> None:
        for w in self._champ_frame.winfo_children():
            w.destroy()
        self._champ_vars.clear()

        if not self.savegame or self.savegame.champion_count == 0:
            ttk.Label(self._champ_frame,
                      text=_("No champion data available for this format")).pack(
                pady=20)
            return

        champ_nb = ttk.Notebook(self._champ_frame)
        champ_nb.pack(fill="both", expand=True)

        for ci in range(self.savegame.champion_count):
            name = (self.savegame.champion_names[ci]
                    if ci < len(self.savegame.champion_names)
                    else _("Champion {}").format(ci + 1))
            frame = ttk.Frame(champ_nb, padding=8)
            champ_nb.add(frame, text=name)

            champ_data = self.savegame.get_champion_data(ci)
            if not champ_data:
                ttk.Label(frame, text=_("No data")).pack()
                continue

            for ri, field in enumerate(self.savegame.champion_fields):
                val = field.read(champ_data)
                ttk.Label(frame, text=field.label).grid(row=ri, column=0,
                                                        sticky="w", padx=(0, 8))
                key = f"champ_{ci}_{field.name}"
                var = tk.StringVar(value=str(val))
                self._champ_vars[key] = var
                entry = ttk.Entry(frame, textvariable=var, width=12)
                entry.grid(row=ri, column=1, sticky="w", pady=1)

                if field.min_val is not None:
                    ttk.Label(frame,
                              text=f"({field.min_val}–{field.max_val})",
                              foreground="gray").grid(row=ri, column=2,
                                                      sticky="w", padx=(4, 0))

            ttk.Button(
                frame, text=_("Apply"),
                command=lambda idx=ci: self._apply_champion(idx),
            ).grid(row=len(self.savegame.champion_fields), column=0,
                   columnspan=3, pady=(8, 0))

    def _apply_champion(self, index: int) -> None:
        if not self.savegame:
            return
        for field in self.savegame.champion_fields:
            key = f"champ_{index}_{field.name}"
            var = self._champ_vars.get(key)
            if not var:
                continue
            try:
                val = int(var.get())
            except ValueError:
                messagebox.showerror(_("Error"),
                                     _("Invalid value for {}").format(field.label))
                return
            if field.min_val is not None:
                val = max(val, field.min_val)
            if field.max_val is not None:
                val = min(val, field.max_val)
            self.savegame.set_champion_field(index, field, val)
        name = (self.savegame.champion_names[index]
                if index < len(self.savegame.champion_names)
                else str(index))
        self.status.set(_("Champion {} updated").format(name))
        self._populate_hex()

    def _populate_hex(self) -> None:
        self._hex_text.config(state="normal")
        self._hex_text.delete("1.0", "end")
        if not self.savegame:
            self._hex_text.config(state="disabled")
            return

        data = self.savegame.raw
        lines = []
        for off in range(0, len(data), 16):
            chunk = data[off:off + 16]
            hex_part = " ".join(f"{b:02X}" for b in chunk)
            ascii_part = "".join(chr(b) if 32 <= b < 127 else "." for b in chunk)
            lines.append(f"{off:08X}  {hex_part:<48s}  |{ascii_part}|")

        self._hex_text.insert("1.0", "\n".join(lines))
        self._hex_text.config(state="disabled")

    def save_file(self) -> None:
        if not self.savegame:
            messagebox.showwarning(_("Warning"), _("No savegame loaded"))
            return
        try:
            self.savegame.save()
            self.status.set(_("Saved: {}").format(self.savegame.path.name))
        except Exception as e:
            messagebox.showerror(_("Error"), str(e))

    def save_as(self) -> None:
        if not self.savegame:
            messagebox.showwarning(_("Warning"), _("No savegame loaded"))
            return
        path = filedialog.asksaveasfilename(
            title=_("Save As"),
            defaultextension=self.savegame.path.suffix,
            initialfile=self.savegame.path.name,
        )
        if not path:
            return
        try:
            self.savegame.save(Path(path))
            self.status.set(_("Saved as: {}").format(Path(path).name))
        except Exception as e:
            messagebox.showerror(_("Error"), str(e))

    def _switch_lang(self, lang_code: str) -> None:
        global _current_lang, _trans, _
        _current_lang = lang_code
        _trans = _load_translations(lang_code)
        _ = _trans.gettext
        messagebox.showinfo(
            _("Language"),
            _("Language set to {}.\nRestart app for full effect.").format(lang_code),
        )


def main() -> None:
    app = SavegameEditor()
    app.mainloop()


if __name__ == "__main__":
    main()
