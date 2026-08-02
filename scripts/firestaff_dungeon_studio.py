#!/usr/bin/env python3
"""Firestaff Dungeon Studio.

Cross-platform Tk tool for creating and editing dungeon maps for all
Firestaff games. Reads and writes the .fsdung binary format which the
Firestaff runtime can load via include/firestaff_fsdung_loader.h.

Run:
  python3 scripts/firestaff_dungeon_studio.py --game dm1
"""

from __future__ import annotations

import argparse
import gettext
import locale
import os
import struct
import sys
from dataclasses import dataclass, field
from pathlib import Path
from typing import Any

# ── i18n setup ──

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

if getattr(sys, 'frozen', False) and hasattr(sys, '_MEIPASS'):
    _LOCALE_DIR = Path(sys._MEIPASS) / "po" / "locale"
else:
    _LOCALE_DIR = Path(__file__).resolve().parent.parent / "po" / "locale"
_current_lang = "en"


def _detect_system_lang() -> str:
    for env_var in ("LANG", "LC_ALL", "LC_MESSAGES", "LANGUAGE"):
        val = os.environ.get(env_var, "")
        if val:
            code = val.split(".")[0].split("_")[0].lower()
            if any(lc == code for lc, _, _ in LANG_META):
                return code
    if sys.platform == "darwin":
        try:
            import subprocess
            result = subprocess.run(["defaults", "read", "-g", "AppleLanguages"],
                                    capture_output=True, text=True, timeout=2)
            for line in result.stdout.splitlines():
                code = line.strip().strip('",').split("-")[0].lower()
                if any(lc == code for lc, _, _ in LANG_META):
                    return code
        except Exception:
            pass
    try:
        loc = locale.getlocale()[0] or locale.getdefaultlocale()[0] or ""
        code = loc.split("_")[0].lower()
        if any(lc == code for lc, _, _ in LANG_META):
            return code
    except Exception:
        pass
    return "en"


def _load_translations(lang: str) -> gettext.GNUTranslations | gettext.NullTranslations:
    try:
        return gettext.translation("firestaff_studio", localedir=str(_LOCALE_DIR),
                                   languages=[lang], fallback=True)
    except Exception:
        return gettext.NullTranslations()


_current_lang = _detect_system_lang()
_trans = _load_translations(_current_lang)
_ = _trans.gettext

try:
    import tkinter as tk
    from tkinter import colorchooser, filedialog, messagebox, simpledialog, ttk
except Exception as exc:
    raise SystemExit(f"Tkinter is required for Dungeon Studio: {exc}")

try:
    from PIL import Image, ImageTk
except Exception:
    Image = None  # type: ignore[assignment,misc]
    ImageTk = None  # type: ignore[assignment]

GAMES = ("dm1", "csb", "dm2", "theron", "nexus")
FSDUNG_MAGIC = b"FSDG"
FSDUNG_VERSION = 1
REPO_ROOT = Path(__file__).resolve().parents[1]
FIRESTAFF_LOGO = REPO_ROOT / "assets" / "branding" / "firestaff-logo.png"

TILE_TYPES = [
    (0, "Wall",       "#2a2a5a"),
    (1, "Corridor",   "#4a4a4a"),
    (2, "Pit",        "#3a1a1a"),
    (3, "Stairs",     "#5a4a2a"),
    (4, "Door",       "#6a4a3a"),
    (5, "Teleporter", "#3a2a6a"),
    (6, "Fake Wall",  "#4a3a5a"),
]

THING_TYPES = [
    (0,  "Door"),
    (1,  "Teleporter"),
    (2,  "Text"),
    (3,  "Sensor"),
    (4,  "Group"),
    (5,  "Weapon"),
    (6,  "Armour"),
    (7,  "Scroll"),
    (8,  "Potion"),
    (9,  "Container"),
    (10, "Junk"),
    (11, "Creature"),
]

THING_SYMBOLS = {
    0: "D", 1: "T", 2: "\"", 3: "!", 4: "G",
    5: "/", 6: "#", 7: "~", 8: "p", 9: "C", 10: "j", 11: "M",
}

CREATURE_TYPES_DM1 = [
    (0, "Giant Scorpion"), (1, "Swamp Slime"), (2, "Screamer"),
    (3, "Rockpile"), (4, "Ghost"), (5, "Stone Golem"),
    (6, "Mummy"), (7, "Black Flame"), (8, "Skeleton"),
    (9, "Couatl"), (10, "Vexirk"), (11, "Magenta Worm"),
    (12, "Trolin"), (13, "Giant Wasp"), (14, "Animated Armour"),
    (15, "Materializer"), (16, "Water Elemental"), (17, "Oitu"),
    (18, "Demon"), (19, "Lord Chaos"), (20, "Red Dragon"),
    (21, "Lord Order"),
]

SENSOR_FLOOR_TYPES = [
    (0,  "Disabled"),
    (1,  "Party+Creature+Object"),
    (3,  "Pressure Plate"),
    (4,  "Object Detector"),
    (6,  "Group Generator"),
    (7,  "Creature Detector"),
]

SENSOR_WALL_TYPES = [
    (1,   "Ornament Click"),
    (5,   "AND/OR Gate"),
    (6,   "Countdown"),
    (7,   "Proj Launcher"),
    (18,  "End Game"),
    (127, "Champion Portrait"),
]


@dataclass
class Thing:
    type: int = 0
    cell: int = 0
    subtype: int = 0


@dataclass
class Tile:
    type: int = 0
    things: list[Thing] = field(default_factory=list)


@dataclass
class DungeonMap:
    name: str = "Level 1"
    width: int = 32
    height: int = 32
    wall_set: int = 0
    floor_set: int = 0
    door_set0: int = 0
    door_set1: int = 0
    difficulty: int = 0
    tiles: list[list[Tile]] = field(default_factory=list)

    def init_tiles(self) -> None:
        self.tiles = []
        for x in range(self.width):
            col = []
            for y in range(self.height):
                is_border = x == 0 or y == 0 or x == self.width - 1 or y == self.height - 1
                col.append(Tile(type=0 if is_border else 1))
            self.tiles.append(col)


GFX_SLOTS_V1 = [
    ("entrance",   "Entrance Screen",    320, 200),
    ("door_anim",  "Door Animation",     32,  32),
    ("portraits",  "Champion Portraits", 32,  29),
    ("wall_orn",   "Wall Ornaments",     32,  32),
    ("floor_orn",  "Floor Ornaments",    32,  32),
]

GFX_SLOTS_V2 = [
    ("entrance_v2",   "Entrance Screen (V2)",    1280, 800),
    ("door_anim_v2",  "Door Animation (V2)",     128,  128),
    ("portraits_v2",  "Champion Portraits (V2)", 128,  116),
    ("wall_orn_v2",   "Wall Ornaments (V2)",     128,  128),
    ("floor_orn_v2",  "Floor Ornaments (V2)",    128,  128),
]

GFX_SLOTS = GFX_SLOTS_V1 + GFX_SLOTS_V2


@dataclass
class Dungeon:
    maps: list[DungeonMap] = field(default_factory=list)
    party_x: int = 1
    party_y: int = 1
    party_dir: int = 0
    graphics: dict[str, bytes] = field(default_factory=dict)

    @staticmethod
    def new_default() -> "Dungeon":
        d = Dungeon()
        m = DungeonMap()
        m.init_tiles()
        d.maps.append(m)
        return d


# ── .fsdung binary I/O ──

def save_fsdung(path: Path, dungeon: Dungeon) -> None:
    parts: list[bytes] = []
    hdr = bytearray(16)
    hdr[0:4] = FSDUNG_MAGIC
    struct.pack_into("<H", hdr, 4, FSDUNG_VERSION)
    hdr[6] = len(dungeon.maps)
    struct.pack_into("<H", hdr, 8, dungeon.party_x)
    struct.pack_into("<H", hdr, 10, dungeon.party_y)
    struct.pack_into("<H", hdr, 12, dungeon.party_dir)
    parts.append(bytes(hdr))

    for m in dungeon.maps:
        name_bytes = m.name.encode("utf-8")[:128]
        desc = bytearray(32)
        struct.pack_into("<H", desc, 0, len(name_bytes))
        struct.pack_into("<H", desc, 2, m.width)
        struct.pack_into("<H", desc, 4, m.height)
        desc[6] = m.wall_set
        desc[7] = m.floor_set
        desc[8] = m.door_set0
        desc[9] = m.door_set1
        desc[10] = m.difficulty
        parts.append(bytes(desc))
        parts.append(name_bytes)

    for m in dungeon.maps:
        tile_data = bytearray(m.width * m.height)
        things: list[tuple[int, int, Thing]] = []
        for x in range(m.width):
            for y in range(m.height):
                t = m.tiles[x][y]
                has_things = 0x10 if t.things else 0
                tile_data[x * m.height + y] = (t.type << 5) | has_things
                for th in t.things:
                    things.append((x, y, th))
        parts.append(bytes(tile_data))

        thing_buf = bytearray(2 + len(things) * 8)
        struct.pack_into("<H", thing_buf, 0, len(things))
        for i, (tx, ty, th) in enumerate(things):
            off = 2 + i * 8
            struct.pack_into("<H", thing_buf, off, tx)
            struct.pack_into("<H", thing_buf, off + 2, ty)
            thing_buf[off + 4] = th.type
            thing_buf[off + 5] = th.cell
            struct.pack_into("<H", thing_buf, off + 6, th.subtype)
        parts.append(bytes(thing_buf))

    # Graphics section: count(u16), then for each: name_len(u16), name, data_len(u32), data
    gfx_count = len(dungeon.graphics)
    gfx_hdr = struct.pack("<H", gfx_count)
    parts.append(gfx_hdr)
    for key, png_data in dungeon.graphics.items():
        key_bytes = key.encode("utf-8")
        parts.append(struct.pack("<H", len(key_bytes)))
        parts.append(key_bytes)
        parts.append(struct.pack("<I", len(png_data)))
        parts.append(png_data)

    path.write_bytes(b"".join(parts))


def load_fsdung(path: Path) -> Dungeon:
    data = path.read_bytes()
    if len(data) < 16 or data[0:4] != FSDUNG_MAGIC:
        raise ValueError("Not a .fsdung file")
    ver = struct.unpack_from("<H", data, 4)[0]
    if ver != FSDUNG_VERSION:
        raise ValueError(f"Unsupported version {ver}")

    d = Dungeon()
    d.party_x = struct.unpack_from("<H", data, 8)[0]
    d.party_y = struct.unpack_from("<H", data, 10)[0]
    d.party_dir = struct.unpack_from("<H", data, 12)[0]
    map_count = data[6]
    off = 16

    descs: list[tuple[str, int, int, int, int, int, int, int]] = []
    for _ in range(map_count):
        name_len = struct.unpack_from("<H", data, off)[0]
        w = struct.unpack_from("<H", data, off + 2)[0]
        h = struct.unpack_from("<H", data, off + 4)[0]
        ws, fs, ds0, ds1, diff = data[off+6], data[off+7], data[off+8], data[off+9], data[off+10]
        off += 32
        name = data[off:off+name_len].decode("utf-8", errors="replace")
        off += name_len
        descs.append((name, w, h, ws, fs, ds0, ds1, diff))

    for name, w, h, ws, fs, ds0, ds1, diff in descs:
        m = DungeonMap(name=name, width=w, height=h, wall_set=ws, floor_set=fs,
                       door_set0=ds0, door_set1=ds1, difficulty=diff)
        m.tiles = []
        for x in range(w):
            col = []
            for y in range(h):
                b = data[off]
                off += 1
                col.append(Tile(type=b >> 5))
            m.tiles.append(col)

        thing_count = struct.unpack_from("<H", data, off)[0]
        off += 2
        for _ in range(thing_count):
            tx = struct.unpack_from("<H", data, off)[0]
            ty = struct.unpack_from("<H", data, off + 2)[0]
            ttype = data[off + 4]
            tcell = data[off + 5]
            tsub = struct.unpack_from("<H", data, off + 6)[0]
            off += 8
            if tx < w and ty < h:
                m.tiles[tx][ty].things.append(Thing(type=ttype, cell=tcell, subtype=tsub))
        d.maps.append(m)

    # Graphics section (optional, may be absent in v1 files)
    if off < len(data) and off + 2 <= len(data):
        gfx_count = struct.unpack_from("<H", data, off)[0]
        off += 2
        for _ in range(gfx_count):
            key_len = struct.unpack_from("<H", data, off)[0]
            off += 2
            key = data[off:off + key_len].decode("utf-8", errors="replace")
            off += key_len
            data_len = struct.unpack_from("<I", data, off)[0]
            off += 4
            d.graphics[key] = bytes(data[off:off + data_len])
            off += data_len

    return d


# ── Graphics import support ──

def load_entrance_image(path: Path) -> Image.Image | None:
    if Image is None:
        return None
    try:
        return Image.open(path).convert("RGBA")
    except Exception:
        return None


# ── Main GUI ──

class DungeonStudio(tk.Tk):
    def __init__(self, initial_game: str):
        super().__init__()
        self.title("Firestaff Dungeon Studio")
        self.geometry("1280x820")
        self.minsize(1000, 680)
        self.game = tk.StringVar(value=initial_game)
        self.current_tool = tk.StringVar(value="select")
        self.current_tile_type = tk.IntVar(value=0)
        self.current_thing_type = tk.IntVar(value=5)
        self.status = tk.StringVar(value="Ready")
        self.show_grid = tk.BooleanVar(value=True)
        self.show_things = tk.BooleanVar(value=True)
        self.zoom = 20
        self.dungeon = Dungeon.new_default()
        self.current_map_idx = 0
        self.selected_tile: tuple[int, int] | None = None
        self.selected_thing_idx: int | None = None
        self.is_painting = False
        self.undo_stack: list[bytes] = []
        self.redo_stack: list[bytes] = []
        self.file_path: Path | None = None
        self._build_ui()
        self.refresh_all()

    @property
    def current_map(self) -> DungeonMap:
        return self.dungeon.maps[self.current_map_idx]

    # ── UI construction ──

    def _build_ui(self) -> None:
        # Top toolbar
        top = ttk.Frame(self, padding=8)
        top.pack(fill="x")

        ttk.Label(top, text=_("Game")).pack(side="left")
        game_box = ttk.Combobox(top, textvariable=self.game, values=GAMES, width=8, state="readonly")
        game_box.pack(side="left", padx=4)
        game_box.bind("<<ComboboxSelected>>", lambda _e: self.on_game_changed())

        ttk.Separator(top, orient="vertical").pack(side="left", fill="y", padx=8)

        ttk.Button(top, text=_("New"), command=self.new_dungeon).pack(side="left", padx=2)
        ttk.Button(top, text=_("Open .fsdung"), command=self.open_file).pack(side="left", padx=2)
        ttk.Button(top, text=_("Save .fsdung"), command=self.save_file).pack(side="left", padx=2)
        ttk.Button(top, text=_("Save As..."), command=self.save_file_as).pack(side="left", padx=2)

        ttk.Separator(top, orient="vertical").pack(side="left", fill="y", padx=8)

        ttk.Button(top, text=_("Add Level"), command=self.add_map).pack(side="left", padx=2)
        ttk.Button(top, text=_("Map Settings"), command=self.show_map_settings).pack(side="left", padx=2)
        ttk.Button(top, text=_("Statistics"), command=self.show_stats).pack(side="left", padx=2)
        ttk.Button(top, text=_("Validate"), command=self.validate_dungeon).pack(side="left", padx=2)
        ttk.Separator(top, orient="vertical").pack(side="left", fill="y", padx=8)
        ttk.Button(top, text=_("Test View"), command=self.show_test_view).pack(side="left", padx=2)

        # Language selector
        lang_frame = ttk.Frame(top)
        lang_frame.pack(side="right")
        self._lang_var = tk.StringVar(value=_current_lang)
        lang_menu = ttk.Menubutton(lang_frame, textvariable=self._lang_var, width=4)
        lang_menu.pack(side="right")
        lm = tk.Menu(lang_menu, tearoff=0)
        for lc, flag, name in LANG_META:
            lm.add_command(label=f"{flag} {name}", command=lambda c=lc: self._switch_lang(c))
        lang_menu["menu"] = lm

        # Main area
        main = ttk.PanedWindow(self, orient="horizontal")
        main.pack(fill="both", expand=True, padx=8, pady=(0, 4))

        # Left panel
        left = ttk.Frame(main, width=280)
        main.add(left, weight=0)

        # Tool selector
        tool_frame = ttk.LabelFrame(left, text=_("Tools"), padding=6)
        tool_frame.pack(fill="x", padx=6, pady=(6, 4))
        tools = [("select", _("Select")), ("paint", _("Paint")), ("thing", _("Place Thing")),
                 ("erase", _("Erase")), ("fill", _("Fill")), ("party", _("Set Party Start"))]
        tool_grid = ttk.Frame(tool_frame)
        tool_grid.pack(fill="x")
        for i, (tool_id, tool_name) in enumerate(tools):
            ttk.Radiobutton(tool_grid, text=tool_name, variable=self.current_tool,
                            value=tool_id).grid(row=i // 2, column=i % 2, sticky="w", padx=2)

        # Tile palette
        tile_frame = ttk.LabelFrame(left, text=_("Tile Types"), padding=6)
        tile_frame.pack(fill="x", padx=6, pady=4)
        for tid, tname, tcolor in TILE_TYPES:
            row = ttk.Frame(tile_frame)
            row.pack(fill="x", pady=1)
            swatch = tk.Canvas(row, width=16, height=16, highlightthickness=0, background=tcolor)
            swatch.pack(side="left", padx=(0, 6))
            ttk.Radiobutton(row, text=tname, variable=self.current_tile_type,
                            value=tid).pack(side="left")

        # Thing palette
        thing_frame = ttk.LabelFrame(left, text=_("Thing Type"), padding=6)
        thing_frame.pack(fill="x", padx=6, pady=4)
        for tid, tname in THING_TYPES:
            ttk.Radiobutton(thing_frame, text=f"{THING_SYMBOLS.get(tid, '?')} {tname}",
                            variable=self.current_thing_type, value=tid).pack(anchor="w")

        # View options
        view_frame = ttk.LabelFrame(left, text=_("View"), padding=6)
        view_frame.pack(fill="x", padx=6, pady=4)
        ttk.Checkbutton(view_frame, text=_("Show Grid"), variable=self.show_grid,
                        command=self.render_canvas).pack(anchor="w")
        ttk.Checkbutton(view_frame, text=_("Show Things"), variable=self.show_things,
                        command=self.render_canvas).pack(anchor="w")
        zoom_row = ttk.Frame(view_frame)
        zoom_row.pack(fill="x", pady=2)
        ttk.Label(zoom_row, text=_("Zoom")).pack(side="left")
        ttk.Button(zoom_row, text="-", width=3, command=self.zoom_out).pack(side="left", padx=2)
        ttk.Button(zoom_row, text="+", width=3, command=self.zoom_in).pack(side="left", padx=2)
        ttk.Button(zoom_row, text=_("Fit"), width=4, command=self.zoom_fit).pack(side="left", padx=2)

        # Center: level tabs + canvas
        center = ttk.Frame(main)
        main.add(center, weight=1)

        self.level_tab_frame = ttk.Frame(center)
        self.level_tab_frame.pack(fill="x")

        self.canvas_frame = ttk.Frame(center)
        self.canvas_frame.pack(fill="both", expand=True)
        self.canvas = tk.Canvas(self.canvas_frame, background="#15191d",
                                highlightthickness=0, cursor="crosshair")
        self.canvas.pack(fill="both", expand=True)

        self.canvas.bind("<Button-1>", self.on_canvas_click)
        self.canvas.bind("<B1-Motion>", self.on_canvas_drag)
        self.canvas.bind("<ButtonRelease-1>", self.on_canvas_release)
        self.canvas.bind("<Button-3>", self.on_canvas_right_click)
        self.canvas.bind("<MouseWheel>", self.on_canvas_scroll)
        self.canvas.bind("<Motion>", self.on_canvas_motion)

        # Right panel: properties
        right = ttk.Frame(main, width=260)
        main.add(right, weight=0)

        map_props = ttk.LabelFrame(right, text=_("Map Properties"), padding=6)
        map_props.pack(fill="x", padx=6, pady=(6, 4))
        self.prop_map_name = tk.StringVar()
        self.prop_map_size = tk.StringVar()
        ttk.Label(map_props, text=_("Name")).grid(row=0, column=0, sticky="w")
        ttk.Entry(map_props, textvariable=self.prop_map_name, width=20).grid(row=0, column=1, sticky="ew")
        self.prop_map_name.trace_add("write", lambda *_: self._update_map_name())
        ttk.Label(map_props, text=_("Size")).grid(row=1, column=0, sticky="w")
        ttk.Label(map_props, textvariable=self.prop_map_size).grid(row=1, column=1, sticky="w")

        self.prop_wall_set = tk.IntVar()
        self.prop_floor_set = tk.IntVar()
        self.prop_difficulty = tk.IntVar()
        ttk.Label(map_props, text=_("Wall Set")).grid(row=2, column=0, sticky="w")
        ttk.Spinbox(map_props, from_=0, to=3, width=4, textvariable=self.prop_wall_set,
                     command=self._update_map_sets).grid(row=2, column=1, sticky="w")
        ttk.Label(map_props, text=_("Floor Set")).grid(row=3, column=0, sticky="w")
        ttk.Spinbox(map_props, from_=0, to=3, width=4, textvariable=self.prop_floor_set,
                     command=self._update_map_sets).grid(row=3, column=1, sticky="w")
        ttk.Label(map_props, text=_("Difficulty")).grid(row=4, column=0, sticky="w")
        ttk.Spinbox(map_props, from_=0, to=15, width=4, textvariable=self.prop_difficulty,
                     command=self._update_map_sets).grid(row=4, column=1, sticky="w")
        map_props.columnconfigure(1, weight=1)

        # Tile properties
        tile_props = ttk.LabelFrame(right, text=_("Tile"), padding=6)
        tile_props.pack(fill="x", padx=6, pady=4)
        self.prop_tile_pos = tk.StringVar(value="—")
        self.prop_tile_type = tk.StringVar(value="—")
        ttk.Label(tile_props, text=_("Position")).grid(row=0, column=0, sticky="w")
        ttk.Label(tile_props, textvariable=self.prop_tile_pos).grid(row=0, column=1, sticky="w")
        ttk.Label(tile_props, text=_("Type")).grid(row=1, column=0, sticky="w")
        ttk.Label(tile_props, textvariable=self.prop_tile_type).grid(row=1, column=1, sticky="w")

        # Things at tile
        things_frame = ttk.LabelFrame(right, text=_("Things at Tile"), padding=6)
        things_frame.pack(fill="x", padx=6, pady=4)
        self.things_listbox = tk.Listbox(
            things_frame, height=6, font=("Menlo", 11),
            background="#15191d", foreground="#d9e4e6",
            selectbackground="#225b62", borderwidth=0,
            highlightthickness=1, highlightbackground="#2f3a40",
        )
        self.things_listbox.pack(fill="both", expand=True)
        self.things_listbox.bind("<<ListboxSelect>>", self.on_thing_selected)
        thing_btns = ttk.Frame(things_frame)
        thing_btns.pack(fill="x", pady=(4, 0))
        ttk.Button(thing_btns, text=_("Delete Thing"), command=self.delete_selected_thing).pack(side="left")

        # Thing properties
        thing_props = ttk.LabelFrame(right, text=_("Thing Properties"), padding=6)
        thing_props.pack(fill="x", padx=6, pady=4)
        self.prop_thing_type = tk.StringVar(value="—")
        self.prop_thing_cell = tk.IntVar(value=0)
        ttk.Label(thing_props, text=_("Type")).grid(row=0, column=0, sticky="w")
        ttk.Label(thing_props, textvariable=self.prop_thing_type).grid(row=0, column=1, sticky="w")
        ttk.Label(thing_props, text=_("Cell")).grid(row=1, column=0, sticky="w")
        cell_box = ttk.Combobox(thing_props, textvariable=self.prop_thing_cell,
                                values=["0 (NW)", "1 (NE)", "2 (SW)", "3 (SE)"], width=10, state="readonly")
        cell_box.grid(row=1, column=1, sticky="w")
        cell_box.bind("<<ComboboxSelected>>", lambda _: self._update_thing_cell())

        # Graphics import
        gfx_frame = ttk.LabelFrame(right, text=_("Graphics Override"), padding=6)
        gfx_frame.pack(fill="x", padx=6, pady=4)
        self._gfx_status_labels: dict[str, tk.StringVar] = {}
        for slot_id, slot_name, sw, sh in GFX_SLOTS:
            row = ttk.Frame(gfx_frame)
            row.pack(fill="x", pady=1)
            ttk.Button(row, text=_("Import {}...").format(slot_name),
                       command=lambda s=slot_id, w=sw, h=sh: self._import_gfx(s, w, h)).pack(side="left", fill="x", expand=True)
            sv = tk.StringVar(value="—")
            self._gfx_status_labels[slot_id] = sv
            ttk.Label(row, textvariable=sv, width=6).pack(side="right")

        self._gfx_preview_canvas = tk.Canvas(gfx_frame, width=160, height=100,
                                              background="#15191d", highlightthickness=0)
        self._gfx_preview_canvas.pack(fill="x", pady=(4, 0))
        self._gfx_preview_photo = None

        # Status bar
        status_bar = ttk.Frame(self, padding=(8, 4))
        status_bar.pack(fill="x")
        ttk.Label(status_bar, textvariable=self.status, anchor="w").pack(side="left", fill="x", expand=True)

    # ── Refresh ──

    def refresh_all(self) -> None:
        self.render_level_tabs()
        self.render_canvas()
        self.update_properties()
        self._update_gfx_status()
        self.update_status()

    def _update_gfx_status(self) -> None:
        for slot_id, _, _, _ in GFX_SLOTS:
            label = self._gfx_status_labels.get(slot_id)
            if label:
                label.set("OK" if slot_id in self.dungeon.graphics else "—")

    def update_status(self) -> None:
        m = self.current_map
        tile_count = sum(1 for x in range(m.width) for y in range(m.height) if m.tiles[x][y].type != 0)
        thing_count = sum(len(m.tiles[x][y].things) for x in range(m.width) for y in range(m.height))
        game = self.game.get().upper()
        supported = game == "DM1"
        tag = "" if supported else "  [" + _("Coming soon") + "]"
        pos = f"Pos: {self.selected_tile[0]},{self.selected_tile[1]}  " if self.selected_tile else ""
        self.status.set(
            f"{game}{tag}  |  {pos}Zoom: {self.zoom}  |  "
            f"Map: {self.current_map_idx + 1}/{len(self.dungeon.maps)}  |  "
            f"Tiles: {tile_count}  |  Things: {thing_count}  |  "
            f"Tool: {self.current_tool.get()}"
        )

    # ── Level tabs ──

    def render_level_tabs(self) -> None:
        for w in self.level_tab_frame.winfo_children():
            w.destroy()
        for i, m in enumerate(self.dungeon.maps):
            style = "TButton" if i != self.current_map_idx else "Accent.TButton"
            b = ttk.Button(self.level_tab_frame, text=m.name,
                           command=lambda idx=i: self.switch_map(idx))
            b.pack(side="left", padx=1)
            if i == self.current_map_idx:
                b.state(["pressed"])

    def switch_map(self, idx: int) -> None:
        self.current_map_idx = idx
        self.selected_tile = None
        self.selected_thing_idx = None
        self.refresh_all()

    def add_map(self) -> None:
        name = simpledialog.askstring(_("New Level"), _("Level name:"), initialvalue=f"Level {len(self.dungeon.maps) + 1}")
        if not name:
            return
        m = DungeonMap(name=name)
        m.init_tiles()
        self.dungeon.maps.append(m)
        self.current_map_idx = len(self.dungeon.maps) - 1
        self.refresh_all()

    # ── Canvas rendering (Pillow Image for performance) ──

    _TILE_RGB = {
        0: (42, 42, 90),
        1: (74, 74, 74),
        2: (58, 26, 26),
        3: (90, 74, 42),
        4: (106, 74, 58),
        5: (58, 42, 106),
        6: (74, 58, 90),
    }
    _GRID_RGB = (30, 30, 78)
    _SELECT_RGB = (0, 232, 143)
    _PARTY_RGB = (0, 180, 100)

    def render_canvas(self) -> None:
        self.canvas.delete("all")
        m = self.current_map
        z = self.zoom
        w, h = m.width * z, m.height * z

        # Use canvas rectangles — limited to tile count, not pixel count
        for tx in range(m.width):
            for ty in range(m.height):
                tile = m.tiles[tx][ty]
                r, g, b = self._TILE_RGB.get(tile.type, (50, 50, 50))
                color = f"#{r:02x}{g:02x}{b:02x}"
                outline = "#1e1e4e" if self.show_grid.get() and z >= 8 else ""
                self.canvas.create_rectangle(
                    tx * z, ty * z, (tx + 1) * z, (ty + 1) * z,
                    fill=color, outline=outline, width=1)

        # Selection highlight
        if self.selected_tile:
            sx, sy = self.selected_tile
            self.canvas.create_rectangle(
                sx * z + 1, sy * z + 1, (sx + 1) * z - 1, (sy + 1) * z - 1,
                outline="#00e88f", width=2)

        # Party marker
        px, py = self.dungeon.party_x, self.dungeon.party_y
        if 0 <= px < m.width and 0 <= py < m.height:
            cx, cy = px * z + z // 2, py * z + z // 2
            r = max(2, z // 4)
            self.canvas.create_oval(cx - r, cy - r, cx + r, cy + r, fill="#00b464", outline="")

        # Thing symbols
        if self.show_things.get() and z >= 14:
            for tx in range(m.width):
                for ty in range(m.height):
                    tile = m.tiles[tx][ty]
                    if tile.things:
                        sym = THING_SYMBOLS.get(tile.things[0].type, "?")
                        self.canvas.create_text(
                            tx * z + z // 2, ty * z + z // 2,
                            text=sym, fill="#ffcc00",
                            font=("Menlo", max(8, z // 3)))

        self.canvas.configure(scrollregion=(0, 0, w, h))

    # ── Canvas interaction ──

    def _tile_at(self, event: tk.Event) -> tuple[int, int] | None:
        cx = self.canvas.canvasx(event.x)
        cy = self.canvas.canvasy(event.y)
        x = int(cx // self.zoom)
        y = int(cy // self.zoom)
        m = self.current_map
        if 0 <= x < m.width and 0 <= y < m.height:
            return (x, y)
        return None

    def _push_undo(self) -> None:
        self.undo_stack.append(self._snapshot())
        if len(self.undo_stack) > 80:
            self.undo_stack.pop(0)
        self.redo_stack.clear()

    def _snapshot(self) -> bytes:
        m = self.current_map
        rows = []
        for x in range(m.width):
            for y in range(m.height):
                t = m.tiles[x][y]
                rows.append(struct.pack("B", t.type))
                rows.append(struct.pack("<H", len(t.things)))
                for th in t.things:
                    rows.append(struct.pack("BBH", th.type, th.cell, th.subtype))
        return b"".join(rows)

    def _restore_snapshot(self, snap: bytes) -> None:
        m = self.current_map
        off = 0
        for x in range(m.width):
            for y in range(m.height):
                t = m.tiles[x][y]
                t.type = struct.unpack_from("B", snap, off)[0]
                off += 1
                tc = struct.unpack_from("<H", snap, off)[0]
                off += 2
                t.things = []
                for _ in range(tc):
                    ttype, tcell, tsub = struct.unpack_from("BBH", snap, off)
                    off += 4
                    t.things.append(Thing(type=ttype, cell=tcell, subtype=tsub))

    def undo(self) -> None:
        if not self.undo_stack:
            return
        self.redo_stack.append(self._snapshot())
        self._restore_snapshot(self.undo_stack.pop())
        self.render_canvas()
        self.update_status()

    def redo(self) -> None:
        if not self.redo_stack:
            return
        self.undo_stack.append(self._snapshot())
        self._restore_snapshot(self.redo_stack.pop())
        self.render_canvas()
        self.update_status()

    def _apply_tool(self, pos: tuple[int, int]) -> None:
        x, y = pos
        m = self.current_map
        tile = m.tiles[x][y]
        tool = self.current_tool.get()

        if tool == "paint":
            tile.type = self.current_tile_type.get()
        elif tool == "erase":
            tile.type = 0
            tile.things.clear()
        elif tool == "thing":
            tile.things.append(Thing(type=self.current_thing_type.get()))
        elif tool == "fill":
            self._flood_fill(x, y, tile.type, self.current_tile_type.get())
        elif tool == "party":
            self.dungeon.party_x = x
            self.dungeon.party_y = y

        self.render_canvas()
        self.update_status()

    def _flood_fill(self, sx: int, sy: int, old_type: int, new_type: int) -> None:
        if old_type == new_type:
            return
        m = self.current_map
        stack = [(sx, sy)]
        visited: set[tuple[int, int]] = set()
        while stack:
            cx, cy = stack.pop()
            if (cx, cy) in visited:
                continue
            if cx < 0 or cy < 0 or cx >= m.width or cy >= m.height:
                continue
            if m.tiles[cx][cy].type != old_type:
                continue
            visited.add((cx, cy))
            m.tiles[cx][cy].type = new_type
            stack.extend([(cx-1, cy), (cx+1, cy), (cx, cy-1), (cx, cy+1)])

    def on_canvas_click(self, event: tk.Event) -> None:
        pos = self._tile_at(event)
        if not pos:
            return
        tool = self.current_tool.get()
        if tool == "select":
            self.selected_tile = pos
            self.selected_thing_idx = None
            self.update_properties()
            self.render_canvas()
        else:
            self._push_undo()
            self.is_painting = True
            self._apply_tool(pos)
            if tool in ("thing", "party"):
                self.selected_tile = pos
                self.update_properties()

    def on_canvas_drag(self, event: tk.Event) -> None:
        pos = self._tile_at(event)
        if not pos:
            return
        if self.is_painting and self.current_tool.get() in ("paint", "erase"):
            self._apply_tool(pos)

    def on_canvas_release(self, event: tk.Event) -> None:
        self.is_painting = False

    def on_canvas_right_click(self, event: tk.Event) -> None:
        pos = self._tile_at(event)
        if pos:
            self.selected_tile = pos
            self.dungeon.party_x, self.dungeon.party_y = pos
            self.update_properties()
            self.render_canvas()
            self.update_status()

    def on_canvas_scroll(self, event: tk.Event) -> None:
        if event.delta > 0:
            self.zoom_in()
        else:
            self.zoom_out()

    def on_canvas_motion(self, event: tk.Event) -> None:
        pos = self._tile_at(event)
        if pos:
            self.update_status()

    # ── Properties ──

    def update_properties(self) -> None:
        m = self.current_map
        self.prop_map_name.set(m.name)
        self.prop_map_size.set(f"{m.width} × {m.height}")
        self.prop_wall_set.set(m.wall_set)
        self.prop_floor_set.set(m.floor_set)
        self.prop_difficulty.set(m.difficulty)

        if self.selected_tile:
            x, y = self.selected_tile
            tile = m.tiles[x][y]
            self.prop_tile_pos.set(f"{x}, {y}")
            tname = TILE_TYPES[tile.type][1] if tile.type < len(TILE_TYPES) else "Unknown"
            self.prop_tile_type.set(tname)
            self._refresh_things_list()
        else:
            self.prop_tile_pos.set("—")
            self.prop_tile_type.set("—")
            self.things_listbox.delete(0, "end")

    def _refresh_things_list(self) -> None:
        self.things_listbox.delete(0, "end")
        if not self.selected_tile:
            return
        x, y = self.selected_tile
        tile = self.current_map.tiles[x][y]
        for i, th in enumerate(tile.things):
            name = next((n for tid, n in THING_TYPES if tid == th.type), "Unknown")
            sym = THING_SYMBOLS.get(th.type, "?")
            self.things_listbox.insert("end", f"{sym} {name} (cell {th.cell})")

    def on_thing_selected(self, _event: tk.Event) -> None:
        sel = self.things_listbox.curselection()
        if not sel or not self.selected_tile:
            return
        idx = sel[0]
        self.selected_thing_idx = idx
        x, y = self.selected_tile
        th = self.current_map.tiles[x][y].things[idx]
        name = next((n for tid, n in THING_TYPES if tid == th.type), "Unknown")
        self.prop_thing_type.set(f"{THING_SYMBOLS.get(th.type, '?')} {name}")
        self.prop_thing_cell.set(th.cell)

    def delete_selected_thing(self) -> None:
        if not self.selected_tile or self.selected_thing_idx is None:
            return
        x, y = self.selected_tile
        things = self.current_map.tiles[x][y].things
        if self.selected_thing_idx < len(things):
            self._push_undo()
            things.pop(self.selected_thing_idx)
            self.selected_thing_idx = None
            self._refresh_things_list()
            self.render_canvas()
            self.update_status()

    def _update_map_name(self) -> None:
        self.current_map.name = self.prop_map_name.get()
        self.render_level_tabs()

    def _update_map_sets(self) -> None:
        m = self.current_map
        m.wall_set = self.prop_wall_set.get()
        m.floor_set = self.prop_floor_set.get()
        m.difficulty = self.prop_difficulty.get()

    def _update_thing_cell(self) -> None:
        if not self.selected_tile or self.selected_thing_idx is None:
            return
        x, y = self.selected_tile
        things = self.current_map.tiles[x][y].things
        if self.selected_thing_idx < len(things):
            things[self.selected_thing_idx].cell = self.prop_thing_cell.get()
            self._refresh_things_list()

    # ── Zoom ──

    def zoom_in(self) -> None:
        self.zoom = min(64, self.zoom + 4)
        self.render_canvas()
        self.update_status()

    def zoom_out(self) -> None:
        self.zoom = max(4, self.zoom - 4)
        self.render_canvas()
        self.update_status()

    def zoom_fit(self) -> None:
        m = self.current_map
        cw = self.canvas.winfo_width() or 800
        ch = self.canvas.winfo_height() or 600
        self.zoom = max(4, min(64, min((cw - 20) // m.width, (ch - 20) // m.height)))
        self.render_canvas()
        self.update_status()

    def _switch_lang(self, lang_code: str) -> None:
        global _, _trans, _current_lang
        _current_lang = lang_code
        _trans = _load_translations(lang_code)
        _ = _trans.gettext
        self._lang_var.set(lang_code)
        messagebox.showinfo(_("Language"),
                            _("Language set to {}.\nRestart app for full effect.").format(lang_code))

    # ── File operations ──

    def new_dungeon(self) -> None:
        if self.game.get() != "dm1":
            messagebox.showinfo(_("Coming soon"), _("{} support coming in a future update").format(self.game.get().upper()))
            return
        self.dungeon = Dungeon.new_default()
        self.current_map_idx = 0
        self.selected_tile = None
        self.file_path = None
        self.undo_stack.clear()
        self.redo_stack.clear()
        self.refresh_all()
        self.status.set(_("New dungeon created"))

    def open_file(self) -> None:
        path = filedialog.askopenfilename(
            title=_("Open .fsdung"),
            filetypes=[("Firestaff Dungeon", "*.fsdung"), (_("All files"), "*.*")],
        )
        if not path:
            return
        try:
            self.dungeon = load_fsdung(Path(path))
            self.file_path = Path(path)
            self.current_map_idx = 0
            self.selected_tile = None
            self.undo_stack.clear()
            self.redo_stack.clear()
            self.refresh_all()
            self.status.set(_("Opened: {}").format(path))
        except Exception as exc:
            messagebox.showerror(_("Error"), str(exc))

    def save_file(self) -> None:
        if not self.file_path:
            self.save_file_as()
            return
        try:
            save_fsdung(self.file_path, self.dungeon)
            self.status.set(_("Saved: {}").format(self.file_path))
        except Exception as exc:
            messagebox.showerror(_("Error"), str(exc))

    def save_file_as(self) -> None:
        path = filedialog.asksaveasfilename(
            title=_("Save .fsdung"),
            defaultextension=".fsdung",
            filetypes=[("Firestaff Dungeon", "*.fsdung"), (_("All files"), "*.*")],
        )
        if not path:
            return
        self.file_path = Path(path)
        self.save_file()

    # ── Game selector ──

    def on_game_changed(self) -> None:
        game = self.game.get()
        if game != "dm1":
            messagebox.showinfo(_("Coming soon"), _("{} support coming in a future update").format(game.upper()))
            self.game.set("dm1")
            return
        self.new_dungeon()

    # ── Map settings ──

    def show_map_settings(self) -> None:
        dlg = tk.Toplevel(self)
        dlg.title(_("Map Settings"))
        dlg.geometry("320x220")
        dlg.transient(self)
        dlg.grab_set()

        m = self.current_map
        f = ttk.Frame(dlg, padding=16)
        f.pack(fill="both", expand=True)

        w_var = tk.IntVar(value=m.width)
        h_var = tk.IntVar(value=m.height)

        ttk.Label(f, text=_("Width (1-64)")).grid(row=0, column=0, sticky="w", pady=4)
        ttk.Spinbox(f, from_=1, to=64, width=6, textvariable=w_var).grid(row=0, column=1, sticky="w")
        ttk.Label(f, text=_("Height (1-64)")).grid(row=1, column=0, sticky="w", pady=4)
        ttk.Spinbox(f, from_=1, to=64, width=6, textvariable=h_var).grid(row=1, column=1, sticky="w")

        def apply() -> None:
            new_w = max(1, min(64, w_var.get()))
            new_h = max(1, min(64, h_var.get()))
            self._push_undo()
            old_tiles = m.tiles
            m.width = new_w
            m.height = new_h
            m.tiles = []
            for x in range(new_w):
                col = []
                for y in range(new_h):
                    if x < len(old_tiles) and y < len(old_tiles[x]):
                        col.append(old_tiles[x][y])
                    else:
                        col.append(Tile(type=0))
                m.tiles.append(col)
            dlg.destroy()
            self.refresh_all()

        ttk.Button(f, text=_("Apply"), command=apply).grid(row=3, column=0, columnspan=2, pady=16)

    # ── Statistics ──

    def show_stats(self) -> None:
        m = self.current_map
        counts: dict[int, int] = {}
        thing_counts: dict[int, int] = {}
        total_things = 0
        for x in range(m.width):
            for y in range(m.height):
                t = m.tiles[x][y]
                counts[t.type] = counts.get(t.type, 0) + 1
                for th in t.things:
                    thing_counts[th.type] = thing_counts.get(th.type, 0) + 1
                    total_things += 1

        lines = [
            _("Maps: {}").format(len(self.dungeon.maps)),
            _("Current: {} ({}×{})").format(m.name, m.width, m.height),
            "",
            _("Tiles:"),
        ]
        for tid, tname, _ in TILE_TYPES:
            lines.append(f"  {tname}: {counts.get(tid, 0)}")
        lines.append("\n" + _("Things: {}").format(total_things))
        for tid, tname in THING_TYPES:
            if thing_counts.get(tid, 0):
                lines.append(f"  {THING_SYMBOLS.get(tid, '?')} {tname}: {thing_counts[tid]}")

        messagebox.showinfo(_("Dungeon Statistics"), "\n".join(lines))

    # ── Validate ──

    def validate_dungeon(self) -> None:
        issues: list[str] = []
        for i, m in enumerate(self.dungeon.maps):
            has_non_wall = any(m.tiles[x][y].type != 0 for x in range(m.width) for y in range(m.height))
            if not has_non_wall:
                issues.append(_("Map {} \"{}\": all walls").format(i+1, m.name))

        m0 = self.dungeon.maps[0]
        px, py = self.dungeon.party_x, self.dungeon.party_y
        if 0 <= px < m0.width and 0 <= py < m0.height:
            if m0.tiles[px][py].type == 0:
                issues.append(_("Party start is inside a wall"))
        else:
            issues.append(_("Party start is outside map bounds"))

        if issues:
            messagebox.showwarning(_("Validation"), _("Issues found:") + "\n\n" + "\n".join(issues))
        else:
            messagebox.showinfo(_("Validation"), _("Dungeon is valid!"))

    # ── Graphics import ──

    def _import_gfx(self, slot_id: str, expected_w: int, expected_h: int) -> None:
        slot_name = next(n for s, n, *_ in GFX_SLOTS if s == slot_id)
        path = filedialog.askopenfilename(
            title=_("Import {}").format(slot_name),
            filetypes=[("PNG", "*.png"), ("BMP", "*.bmp"), (_("All"), "*.*")],
        )
        if not path:
            return
        if Image is None:
            messagebox.showerror(_("Error"), _("Pillow is required for graphics import"))
            return
        try:
            img = Image.open(path).convert("RGBA")
        except Exception as e:
            messagebox.showerror(_("Error"), _("Could not load image: {}").format(e))
            return
        if img.size != (expected_w, expected_h):
            if messagebox.askyesno(_("Resize?"),
                    _("Image is {}×{}, expected {}×{}.\nResize to fit?").format(
                        img.width, img.height, expected_w, expected_h)):
                img = img.resize((expected_w, expected_h), Image.LANCZOS)
            else:
                return
        import io
        buf = io.BytesIO()
        img.save(buf, format="PNG")
        self.dungeon.graphics[slot_id] = buf.getvalue()
        self._gfx_status_labels[slot_id].set("OK")
        self._show_gfx_preview(img)
        self._push_undo()
        self.status.set(_("{} imported from {}").format(slot_name, Path(path).name))

    def _show_gfx_preview(self, img: "Image.Image") -> None:
        if Image is None or ImageTk is None:
            return
        cw = self._gfx_preview_canvas.winfo_width() or 160
        ch = self._gfx_preview_canvas.winfo_height() or 100
        scale = min(cw / img.width, ch / img.height, 1.0)
        pw, ph = max(1, int(img.width * scale)), max(1, int(img.height * scale))
        preview = img.resize((pw, ph), Image.LANCZOS)
        self._gfx_preview_photo = ImageTk.PhotoImage(preview)
        self._gfx_preview_canvas.delete("all")
        self._gfx_preview_canvas.create_image(cw // 2, ch // 2, anchor="center",
                                               image=self._gfx_preview_photo)

    def import_entrance_image(self) -> None:
        self._import_gfx("entrance", 320, 200)

    # ── Test View ──

    def show_test_view(self) -> None:
        win = tk.Toplevel(self)
        win.title(_("Test View") + f" — {self.current_map.name}")
        win.geometry("640x520")

        m = self.current_map
        view_pos = [self.dungeon.party_x, self.dungeon.party_y]
        view_dir = [self.dungeon.party_dir]
        cell_sz = 28

        top_frame = ttk.Frame(win, padding=4)
        top_frame.pack(fill="x")
        pos_var = tk.StringVar()
        ttk.Label(top_frame, textvariable=pos_var, font=("Menlo", 11)).pack(side="left")

        dir_names = [_("North"), _("East"), _("South"), _("West")]
        dx_map = [0, 1, 0, -1]
        dy_map = [-1, 0, 1, 0]

        nav = ttk.Frame(top_frame)
        nav.pack(side="right")
        ttk.Button(nav, text=_("Turn L"), width=6,
                   command=lambda: turn(-1)).pack(side="left", padx=2)
        ttk.Button(nav, text=_("Fwd"), width=6,
                   command=lambda: move(1)).pack(side="left", padx=2)
        ttk.Button(nav, text=_("Back"), width=6,
                   command=lambda: move(-1)).pack(side="left", padx=2)
        ttk.Button(nav, text=_("Turn R"), width=6,
                   command=lambda: turn(1)).pack(side="left", padx=2)

        map_sel = ttk.Frame(win, padding=4)
        map_sel.pack(fill="x")
        for i, dm in enumerate(self.dungeon.maps):
            ttk.Button(map_sel, text=dm.name,
                       command=lambda idx=i: switch_level(idx)).pack(side="left", padx=2)

        canvas = tk.Canvas(win, background="#15191d", highlightthickness=0)
        canvas.pack(fill="both", expand=True, padx=4, pady=4)

        tile_colors = {
            0: "#2a2a5a", 1: "#5a5a5a", 2: "#5a2a2a",
            3: "#7a6a3a", 4: "#8a5a3a", 5: "#5a3a7a", 6: "#4a3a5a",
        }

        current_map_ref = [m]

        def update():
            canvas.delete("all")
            cm = current_map_ref[0]
            cw = canvas.winfo_width() or 500
            ch = canvas.winfo_height() or 400
            ox = max(0, (cw - cm.width * cell_sz) // 2)
            oy = max(0, (ch - cm.height * cell_sz) // 2)

            for tx in range(cm.width):
                for ty in range(cm.height):
                    tile = cm.tiles[tx][ty]
                    x0, y0 = ox + tx * cell_sz, oy + ty * cell_sz
                    color = tile_colors.get(tile.type, "#333")
                    canvas.create_rectangle(x0, y0, x0 + cell_sz, y0 + cell_sz,
                                            fill=color, outline="#1e1e4e", width=1)
                    if tile.things:
                        for th in tile.things:
                            sym = THING_SYMBOLS.get(th.type, "?")
                            tc = "#ff4444" if th.type == 11 else "#ffcc00"
                            canvas.create_text(x0 + cell_sz // 2, y0 + cell_sz // 2,
                                               text=sym, fill=tc, font=("Menlo", max(8, cell_sz // 3)))

            # Party arrow
            px, py = view_pos
            if 0 <= px < cm.width and 0 <= py < cm.height:
                cx = ox + px * cell_sz + cell_sz // 2
                cy = oy + py * cell_sz + cell_sz // 2
                r = cell_sz // 3
                d = view_dir[0]
                arrows = [(0, -r), (r, 0), (0, r), (-r, 0)]
                adx, ady = arrows[d % 4]
                canvas.create_oval(cx - r, cy - r, cx + r, cy + r,
                                   fill="#00c878", outline="#00ff9a", width=2)
                canvas.create_line(cx, cy, cx + adx, cy + ady,
                                   fill="#00ff9a", width=3, arrow="last")

            pos_var.set(f"({px}, {py}) facing {dir_names[view_dir[0] % 4]}  |  "
                        f"{cm.name}  |  Tile: {TILE_TYPES[cm.tiles[px][py].type][1] if 0 <= px < cm.width and 0 <= py < cm.height else '?'}")

        def move(step):
            d = view_dir[0] % 4
            nx = view_pos[0] + dx_map[d] * step
            ny = view_pos[1] + dy_map[d] * step
            cm = current_map_ref[0]
            if 0 <= nx < cm.width and 0 <= ny < cm.height:
                tile = cm.tiles[nx][ny]
                if tile.type != 0:  # Can't walk into walls
                    view_pos[0], view_pos[1] = nx, ny
            update()

        def turn(direction):
            view_dir[0] = (view_dir[0] + direction) % 4
            update()

        def switch_level(idx):
            current_map_ref[0] = self.dungeon.maps[idx]
            view_pos[0] = min(view_pos[0], current_map_ref[0].width - 1)
            view_pos[1] = min(view_pos[1], current_map_ref[0].height - 1)
            win.title(f"Test View — {current_map_ref[0].name}")
            update()

        def on_click(event):
            cm = current_map_ref[0]
            cw = canvas.winfo_width()
            ch = canvas.winfo_height()
            ox = max(0, (cw - cm.width * cell_sz) // 2)
            oy = max(0, (ch - cm.height * cell_sz) // 2)
            tx = (event.x - ox) // cell_sz
            ty = (event.y - oy) // cell_sz
            if 0 <= tx < cm.width and 0 <= ty < cm.height:
                if cm.tiles[tx][ty].type != 0:
                    view_pos[0], view_pos[1] = tx, ty
                    update()

        canvas.bind("<Button-1>", on_click)
        win.bind("<Up>", lambda _: move(1))
        win.bind("<Down>", lambda _: move(-1))
        win.bind("<Left>", lambda _: turn(-1))
        win.bind("<Right>", lambda _: turn(1))

        win.after(100, update)

    # ── Keyboard shortcuts ──

    def _bind_keys(self) -> None:
        self.bind_all("<Control-z>", lambda _: self.undo())
        self.bind_all("<Control-y>", lambda _: self.redo())
        self.bind_all("<Control-s>", lambda _: self.save_file())
        self.bind_all("<Control-n>", lambda _: self.new_dungeon())
        self.bind_all("<Control-o>", lambda _: self.open_file())


def main() -> None:
    parser = argparse.ArgumentParser(description="Firestaff Dungeon Studio")
    parser.add_argument("--game", choices=GAMES, default="dm1")
    parser.add_argument("--self-test", action="store_true")
    parser.add_argument("--screenshot", type=str, default=None)
    args = parser.parse_args()

    if args.self_test:
        d = Dungeon.new_default()
        import tempfile
        with tempfile.NamedTemporaryFile(suffix=".fsdung", delete=False) as f:
            p = Path(f.name)
        save_fsdung(p, d)
        d2 = load_fsdung(p)
        assert len(d2.maps) == 1
        assert d2.maps[0].width == 32
        assert d2.maps[0].height == 32
        assert d2.party_x == 1
        assert d2.party_y == 1
        p.unlink()
        print("Dungeon Studio self-test: PASS")
        return

    app = DungeonStudio(args.game)
    app._bind_keys()

    if args.screenshot and ImageTk is not None:
        def take_screenshot() -> None:
            app.update_idletasks()
            app.update()
            try:
                import subprocess
                subprocess.run(["screencapture", "-l",
                                str(app.winfo_id()), args.screenshot], check=False)
            except Exception:
                pass
            app.after(500, app.destroy)
        app.after(1000, take_screenshot)

    app.mainloop()


if __name__ == "__main__":
    main()
