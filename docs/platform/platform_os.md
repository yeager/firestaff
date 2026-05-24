# DM1 V1 Platform-Specific Code — Source Lock

## Platform Detection

The codebase uses standard preprocessor macros for OS detection:

| Macro | Platform |
|-------|----------|
| _WIN32 | Windows (MSVC or MinGW) |
| __APPLE__ | macOS |
| (implicit) | Linux / generic POSIX |

__APPLE__ is detected via #elif defined(__APPLE__) (no __MACH__ needed — __APPLE__ is the canonical macro).

## Platform-Specific Files

### fs_portable_compat.c (shared/fs_portable_compat.c)

Cross-platform filesystem abstraction used by all M11/M12 modules.

#### Header Includes
#if defined(_WIN32)
    #include <windows.h>
    #include <direct.h>
    #define FSP_SEP '\'
    #define FSP_ALT_SEP '/'
#else
    #include <unistd.h>
    #define FSP_SEP '/'
    #define FSP_ALT_SEP '\'
#endif

#### Path Separator
- Windows: \ (slash / accepted as alternate via FSP_ALT_SEP)
- POSIX: / (backslash \ accepted as alternate)

#### Directory Creation
#if defined(_WIN32)
    return _mkdir(path) == 0 || errno == EEXIST;
#else
    return mkdir(path, 0777) == 0 || errno == EEXIST;
#endif

#### User Data Directory Resolution
#if defined(_WIN32)
    // %APPDATA%\Firestaff
#elif defined(__APPLE__)
    // ~/Library/Application Support/Firestaff
#else
    // /firestaff (default ~/.local/share/firestaff)
#endif

#### User Config Directory Resolution
Same pattern as user data dir, but XDG_CONFIG_HOME on Linux, %APPDATA% on Windows.

#### Originals Directory
#if defined(_WIN32)
    // <module-path>\originals (via GetModuleFileNameA)
#else
    // ~/.firestaff/data (no GetModuleFileNameA)
#endif

### firestaff_startup.c (engine/firestaff_startup.c)

Platform-specific data directory and startup validation:
#ifdef _WIN32
    // %APPDATA%\Firestaff\data (fallback C:\Firestaff\data)
#else
    // ~/.firestaff/data (fallback /tmp/firestaff/data)
#endif

Directory creation uses mkdir -p on POSIX, mkdir with 2>nul on Windows.

### firestaff_data_validator.c (engine/firestaff_data_validator.c)

#ifdef _WIN32 / #ifndef _WIN32 for platform-specific logic.

### firestaff_accessibility.c (engine/firestaff_accessibility.c)

Windows-specific accessibility support (high-DPI awareness, accessibility hooks).

### config_m12.c (engine/config_m12.c)

#if !defined(_WIN32) for Linux/macOS config file path resolution.

### firestaff_mod.c (engine/firestaff_mod.c)

#ifdef _WIN32 for LoadLibrary / GetProcAddress for .dll mod plugins.

### render_sdl_m11.c

Explicitly has NO platform ifdefs — platform-neutral by design.

## Data Resolution Priority (FSP_ResolveDataDir)

1. Explicit requestedDir argument (if non-empty)
2. FIRESTAFF_DATA environment variable
3. Legacy ~/.firestaff/data on POSIX (only if directory already exists)
4. <user-data-dir>/data (platform-appropriate)
5. Current directory .

## CMake Platform Handling

Uses find_package(PkgConfig) which works on all platforms. SDL discovery uses
pkg-config on all Unix-like systems. PLATFORM_SPECIFIC_CMAKE not used —
platform differences handled in C code via #ifdef.
