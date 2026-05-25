# DM2 V1 Configuration

## Configuration System

DM2 uses SkCodeParam class for compile-time and runtime configuration flags.
These are static bool values that control engine behavior.

## SkCodeParam Options

### Game Mode Flags

| Flag | Default | Purpose |
|------|---------|---------|
| bDM1Mode | false | Dungeon loaded is native DM1 format |
| bDM1TQMode | false | Running as Dungeon Master I: The Chaos Plugin |
| bBWMode | false | Experimental Bloodwych compatibility mode |

### Engine Options

| Flag | Default | Purpose |
|------|---------|---------|
| bUseDM2ExtendedMode | - | Extended DM2 feature set |
| bUsePowerDebug | - | Power debugging mode |
| bUseFixedMode | - | Fixed/stable mode (safety checks) |
| bUseIngameDebug | - | In-game debug overlay |
| bUseSuperMode | - | Super/cheat mode |
| bUseSuperInfoEye | - | Info eye display |

### Audio Options

| Flag | Default | Purpose |
|------|---------|---------|
| bMIDIMusicEnabled | - | Enable MIDI music playback |
| bUseExtendedSound | - | Extended sound effects |
| bUseMultilanguageExtended | - | Multi-language text support |
| bUseVaryingPlaybackFrequency | - | Variable playback rate |
| bUsePlayerWalkSound | - | Player footsteps sound |

### Visual Options

| Flag | Default | Purpose |
|------|---------|---------|
| bFullLight | false | Debug: always full lighting (no dark areas) |

### Debug/Testing Options

| Flag | Default | Purpose |
|------|---------|---------|
| bDisableFogEffect | - | Disable fog/darkness rendering |
| bWeakDoors | - | Doors take less damage |
| bWalkOverPits | - | Can walk over pit squares |
| bUseCustomSpells | - | Read spells from GDAT |
| bUnlimitedCharges | - | Items never run out of charges |
| bUseScrollIDType | - | Use scroll ID type system |
| bForceOrnateSound | - | Force ornate tile sound |
| bDebugInfoMapInit | - | Debug map initialization |
| bDebugNoImageDecodingAssert | - | Skip image decode asserts |
| bDebugBypassNullPointers | - | Bypass null pointer checks |

### Save Game

| Flag | Default | Purpose |
|------|---------|---------|
| bForceSaveGameReadOK | - | Force successful save game read |

## Command-Line Configuration

ProcessArgs() [SKULLWIN/main.cpp:244] handles these options:

### Language Flags
- -en: English (0x10)
- -jp: Japanese (0x20)
- -de: German (0x30)
- -fr: French (0x40)
- -es: Spanish (0x50)
- -it: Italian (0x60)

### GDAT Version Flags
- -vx: Extended version
- -v1 through -v5: Specific GDAT versions
- -cartoon: Cartoon mode (v5 + cartoon)

### Dungeon Flags
- -dm1: DM1 format dungeon
- -csb: CSB format dungeon
- -beta: Beta dungeon format
- -dm2: DM2 format dungeon

### Music Flags
- -hmp: HMP to MIDI conversion
- -mod: MOD to OGG conversion
- -ogg: OGG audio
- -segacd: Sega CD audio track to OGG

## Runtime Configuration

Configuration is set during INIT() [SkWinCore.cpp:55593]:
1. GDAT loaded via three-phase interface loading
2. iCompatibilityFlag read from GDAT [SkWinCore.cpp:55620]
   - if == 3: bDM1Mode = true, bDM1TQMode = true
3. Command-line overrides applied via ProcessArgs()

## DM2 vs DM1 Configuration

| Aspect | DM1 | DM2 |
|--------|-----|-----|
| Config class | Less structured | SkCodeParam static flags |
| Command-line | Minimal | Full i18n + engine flags |
| Compatibility mode | None | DM1TQ detection |
| Debug options | Limited | Extensive (bUseFixedMode, etc.) |

## Source References

- Config class: skproject/SKWIN/SkCodeParam.h
- Command-line: skproject/SKULLWIN/main.cpp:244 (ProcessArgs)
- Compatibility: skproject/SKWIN/SkWinCore.cpp:55620
- Init: skproject/SKWIN/SkWinCore.cpp:55593
