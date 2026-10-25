#include "main_loop_m11.h"
#include "firestaff_version.h"
#include "render_sdl_m11.h"

#include <SDL3/SDL.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#elif defined(__APPLE__) || defined(__linux__)
#include <sys/resource.h>
#include <sys/utsname.h>
#if defined(__APPLE__)
#include <sys/sysctl.h>
#elif defined(__linux__)
#include <unistd.h>
#endif
#endif

#if defined(__APPLE__)
static int m11_debug_sysctl_string(const char* key,
                                   char* out,
                                   size_t outSize) {
    size_t size;
    if (!key || !out || outSize == 0U) return 0;
    out[0] = '\0';
    size = outSize;
    if (sysctlbyname(key, out, &size, NULL, 0) != 0 || size == 0U) return 0;
    out[outSize - 1U] = '\0';
    return out[0] != '\0';
}

static int m11_debug_sysctl_u64(const char* key, uint64_t* out) {
    size_t size;
    if (!key || !out) return 0;
    size = sizeof(*out);
    return sysctlbyname(key, out, &size, NULL, 0) == 0 &&
           size == sizeof(*out);
}

static int m11_debug_sysctl_int(const char* key, int* out) {
    size_t size;
    if (!key || !out) return 0;
    size = sizeof(*out);
    return sysctlbyname(key, out, &size, NULL, 0) == 0 &&
           size == sizeof(*out);
}
#endif

static int m11_debug_logical_cpu_count(void) {
#if SDL_VERSION_ATLEAST(3, 2, 0)
    return SDL_GetNumLogicalCPUCores();
#elif defined(_WIN32)
    {
        SYSTEM_INFO info;
        GetSystemInfo(&info);
        return (int)info.dwNumberOfProcessors;
    }
#elif defined(__APPLE__)
    {
        int count = 0;
        return m11_debug_sysctl_int("hw.logicalcpu", &count) ? count : -1;
    }
#elif defined(__linux__)
    {
        long count = sysconf(_SC_NPROCESSORS_ONLN);
        return count > 0 && count <= INT_MAX ? (int)count : -1;
    }
#else
    return -1;
#endif
}

static const char* m11_debug_cpu_architecture(void) {
#if defined(__aarch64__) || defined(__arm64__)
    return "arm64";
#elif defined(__x86_64__) || defined(_M_X64)
    return "x86_64";
#elif defined(__i386__) || defined(_M_IX86)
    return "x86";
#elif defined(__powerpc64__)
    return "ppc64";
#elif defined(__powerpc__)
    return "ppc";
#else
    return "unknown";
#endif
}

/* One compact host inventory per --debug launch. Runtime CPU and memory
 * samples remain in startup-frame; this records the hardware and active
 * presentation mode needed to interpret those samples. */
void M11_Debug_LogHardware(void) {
    SDL_Window* window = M11_Render_GetWindow();
    SDL_Renderer* renderer = M11_Render_GetRenderer();
    SDL_DisplayID displayId = 0;
    const SDL_DisplayMode* mode = NULL;
    const char* displayName = NULL;
    const char* rendererName = NULL;
    int gpuDeviceAvailable = 0;
    int windowWidth = 0, windowHeight = 0;
    int drawableWidth = 0, drawableHeight = 0;
    int logicalCpus;
    int ramMiB;
    int cacheLineBytes = -1;
    int effectiveVsync = -1;
    int effectiveVsyncKnown = 0;
    const char* pixelFormat = NULL;
    int rendererMaxTextureSize = -1;
    int rendererHdrKnown = 0;
    int rendererHdrEnabled = 0;
    int rendererOutputColorspace = -1;
    double systemLoad[3] = { -1.0, -1.0, -1.0 };
    int systemLoadCount = 0;
#if SDL_VERSION_ATLEAST(3, 2, 0)
    displayId = window ? SDL_GetDisplayForWindow(window) : 0;
    mode = displayId ? SDL_GetCurrentDisplayMode(displayId) : NULL;
    displayName = displayId ? SDL_GetDisplayName(displayId) : NULL;
#endif
    if (renderer) {
#if SDL_VERSION_ATLEAST(3, 2, 0)
        SDL_PropertiesID properties = SDL_GetRendererProperties(renderer);
        rendererName = SDL_GetRendererName(renderer);
        effectiveVsyncKnown = SDL_GetRenderVSync(renderer, &effectiveVsync);
        if (properties) {
            void* gpu = SDL_GetPointerProperty(
                properties, SDL_PROP_RENDERER_GPU_DEVICE_POINTER, NULL);
            rendererMaxTextureSize = (int)SDL_GetNumberProperty(
                properties, SDL_PROP_RENDERER_MAX_TEXTURE_SIZE_NUMBER, -1);
            rendererHdrKnown = SDL_HasProperty(
                properties, SDL_PROP_RENDERER_HDR_ENABLED_BOOLEAN);
            rendererHdrEnabled = SDL_GetBooleanProperty(
                properties, SDL_PROP_RENDERER_HDR_ENABLED_BOOLEAN, 0);
            rendererOutputColorspace = (int)SDL_GetNumberProperty(
                properties, SDL_PROP_RENDERER_OUTPUT_COLORSPACE_NUMBER, -1);
            gpuDeviceAvailable = gpu != NULL;
        }
#endif
    }
    logicalCpus = m11_debug_logical_cpu_count();
    ramMiB = SDL_GetSystemRAM();
#if defined(__ANDROID__)
    {
        FILE* loadAverage = fopen("/proc/loadavg", "rb");
        if (loadAverage) {
            systemLoadCount = fscanf(loadAverage, "%lf %lf %lf",
                                     &systemLoad[0], &systemLoad[1],
                                     &systemLoad[2]);
            fclose(loadAverage);
        }
    }
#elif defined(__APPLE__) || defined(__linux__)
    systemLoadCount = getloadavg(systemLoad, 3);
#endif
#if SDL_VERSION_ATLEAST(3, 2, 0)
    cacheLineBytes = SDL_GetCPUCacheLineSize();
#endif
    (void)M11_Render_GetWindowAndDrawableSize(
        &windowWidth, &windowHeight, &drawableWidth, &drawableHeight);
    fprintf(stderr,
            "firestaff: hardware version=%s build=%s os=%s arch=%s logical-cpus=%d ram-mib=%d cache-line-bytes=%d pointer-bits=%u system-load-1m=",
            FIRESTAFF_VERSION_STRING,
#ifdef NDEBUG
            "release",
#else
            "debug",
#endif
            SDL_GetPlatform() ? SDL_GetPlatform() : "unknown",
            m11_debug_cpu_architecture(), logicalCpus, ramMiB,
            cacheLineBytes, (unsigned int)(sizeof(void*) * CHAR_BIT));
    if (systemLoadCount > 0) fprintf(stderr, "%.2f", systemLoad[0]);
    else fprintf(stderr, "unavailable");
    fprintf(stderr, " system-load-5m=");
    if (systemLoadCount > 1) fprintf(stderr, "%.2f", systemLoad[1]);
    else fprintf(stderr, "unavailable");
    fprintf(stderr, " system-load-15m=");
    if (systemLoadCount > 2) fprintf(stderr, "%.2f\n", systemLoad[2]);
    else fprintf(stderr, "unavailable\n");
#if SDL_VERSION_ATLEAST(3, 2, 0)
    fprintf(stderr,
            "firestaff: hardware-cpu-features sse2=%d sse3=%d sse4.1=%d avx=%d avx2=%d avx512f=%d neon=%d\n",
            SDL_HasSSE2(), SDL_HasSSE3(), SDL_HasSSE41(), SDL_HasAVX(),
            SDL_HasAVX2(), SDL_HasAVX512F(), SDL_HasNEON());
#endif
#if defined(__APPLE__)
    {
        char model[128], cpu[128], osVersion[64];
        uint64_t memoryBytes = 0;
        int physicalCpus = -1;
        int performanceCpus = -1, efficiencyCpus = -1, translated = 0;
        int hasModel = m11_debug_sysctl_string("hw.model", model, sizeof(model));
        int hasCpu = m11_debug_sysctl_string(
            "machdep.cpu.brand_string", cpu, sizeof(cpu));
        int hasOsVersion = m11_debug_sysctl_string(
            "kern.osproductversion", osVersion, sizeof(osVersion));
        int hasPhysicalCpus = m11_debug_sysctl_int("hw.physicalcpu", &physicalCpus);
        int hasMemory = m11_debug_sysctl_u64("hw.memsize", &memoryBytes);
        (void)m11_debug_sysctl_int("hw.perflevel0.physicalcpu",
                                   &performanceCpus);
        (void)m11_debug_sysctl_int("hw.perflevel1.physicalcpu",
                                   &efficiencyCpus);
        fprintf(stderr,
                "firestaff: hardware-macos model=%s cpu=%s physical-cpus=",
                hasModel ? model : "unavailable",
                hasCpu ? cpu : "unavailable");
        if (hasPhysicalCpus) fprintf(stderr, "%d", physicalCpus);
        else fprintf(stderr, "unavailable");
        fprintf(stderr, " performance-cores=%d efficiency-cores=%d memory-bytes=",
                performanceCpus, efficiencyCpus);
        if (hasMemory) fprintf(stderr, "%llu", (unsigned long long)memoryBytes);
        else fprintf(stderr, "unavailable");
        fprintf(stderr, " os-version=%s process-translated=",
                hasOsVersion ? osVersion : "unavailable");
        if (m11_debug_sysctl_int("sysctl.proc_translated", &translated))
            fprintf(stderr, "%d\n", translated);
        else
            fprintf(stderr, "unavailable\n");
    }
#elif defined(__linux__)
    {
        struct utsname info;
        char cpuModel[256] = "unavailable";
        FILE* cpuInfo = fopen("/proc/cpuinfo", "rb");
        if (cpuInfo) {
            char line[512];
            while (fgets(line, sizeof(line), cpuInfo)) {
                char* value = strchr(line, ':');
                if (value && (strncmp(line, "model name", 10) == 0 ||
                              strncmp(line, "Hardware", 8) == 0 ||
                              strncmp(line, "Processor", 9) == 0)) {
                    size_t length;
                    ++value;
                    while (*value == ' ' || *value == '\t') ++value;
                    length = strcspn(value, "\r\n");
                    if (length >= sizeof(cpuModel)) length = sizeof(cpuModel) - 1U;
                    memcpy(cpuModel, value, length);
                    cpuModel[length] = '\0';
                    break;
                }
            }
            fclose(cpuInfo);
        }
        if (uname(&info) == 0)
            fprintf(stderr,
                    "firestaff: hardware-linux kernel=%s release=%s machine=%s cpu=%s\n",
                    info.sysname, info.release, info.machine, cpuModel);
    }
#endif
#if SDL_VERSION_ATLEAST(3, 2, 0)
    if (mode) pixelFormat = SDL_GetPixelFormatName(mode->format);
#endif
    fprintf(stderr, "firestaff: display id=%u name=%s current-mode=",
            (unsigned int)displayId,
            displayName && displayName[0] ? displayName : "unavailable");
    if (mode) fprintf(stderr, "%dx%d", mode->w, mode->h);
    else fprintf(stderr, "unavailable");
    fprintf(stderr, " refresh-hz=");
    if (mode) fprintf(stderr, "%.3f", (double)mode->refresh_rate);
    else fprintf(stderr, "unavailable");
    fprintf(stderr,
            " pixel-density=%.3f pixel-format=%s window=%dx%d drawable=%dx%d drawable-scale=%.3fx%.3f renderer=%s gpu-device-pointer=%d gpu-device-name=unavailable-through-sdl gpu-utilization=unavailable renderer-max-texture-size=%d hdr-known=%d hdr-enabled=%d output-colorspace=0x%x video-driver=%s requested-vsync=%d effective-vsync-known=%d effective-vsync=%d\n",
            mode ? (double)mode->pixel_density : 0.0,
            pixelFormat ? pixelFormat : "unavailable",
            windowWidth, windowHeight, drawableWidth, drawableHeight,
            windowWidth > 0 ? (double)drawableWidth / windowWidth : 0.0,
            windowHeight > 0 ? (double)drawableHeight / windowHeight : 0.0,
            rendererName ? rendererName : "unavailable",
            gpuDeviceAvailable,
            rendererMaxTextureSize, rendererHdrKnown, rendererHdrEnabled,
            rendererOutputColorspace,
            SDL_GetCurrentVideoDriver() ? SDL_GetCurrentVideoDriver() : "unknown",
            M11_Render_GetVSync(), effectiveVsyncKnown, effectiveVsync);
}
