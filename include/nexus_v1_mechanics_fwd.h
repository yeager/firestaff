#ifndef NEXUS_V1_MECHANICS_FWD_H
#define NEXUS_V1_MECHANICS_FWD_H

/* Forward declarations for the mechanics subsystem.
 *
 * NEXUS_ENGINE_FWD_FROM_HEADERS is #defined by nexus_v1_engine.h BEFORE
 * any other headers (as a macro in the .h itself, not set by .c files).
 * If it is already set when this file is included, engine.h has already
 * provided the Nexus_V1_Engine forward declaration and we skip it.
 *
 * Even with header guards on engine.h, the guard here prevents
 * accidental re-declaration when engine.h is included multiple times
 * in the same TU via different include paths. */

#ifndef NEXUS_ENGINE_FWD_FROM_HEADERS
struct Nexus_V1_Engine;
typedef struct Nexus_V1_Engine Nexus_V1_Engine;
#endif

/* Nexus_MechanicsState is fully defined in nexus_v1_mechanics.h.
 * This just provides the typedef alias. */
typedef struct Nexus_MechanicsState Nexus_MechanicsState;

#endif /* NEXUS_V1_MECHANICS_FWD_H */
