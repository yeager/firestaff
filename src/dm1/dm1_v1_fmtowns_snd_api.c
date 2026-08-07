#include "dm1_v1_fmtowns_snd_api.h"
#include <stddef.h>
#include <string.h>

/* Byte-verified vaddrs from EDM.EXP SYM1 (see
 * dm1_v1_fmtowns_edm_sym1). Sorted by category then vaddr for
 * human readability. */
const dm1_v1_fmtowns_snd_api_entry_t
dm1_v1_fmtowns_snd_api_entries[DM1_V1_FMTOWNS_SND_API_COUNT] = {
    /* Core init/shutdown/basic ops. */
    { "SND_INIT",             0x000212dbu, "core", "sound driver init"          },
    { "SND_END",              0x00021342u, "core", "sound driver shutdown"      },
    { "SND_KEY_ON",           0x00021367u, "core", "note-on"                    },
    { "SND_KEY_OFF",          0x00021384u, "core", "note-off"                   },
    { "SND_KEY_ABORT",        0x0002143fu, "core", "abort a playing note"       },
    { "SND_PAN_SET",          0x00021399u, "core", "set channel pan"            },
    { "SND_INST_CHANGE",      0x000213b3u, "core", "change instrument"          },
    { "SND_INST_WRITE",       0x000213cdu, "core", "write instrument bank"      },
    { "SND_INST_READ",        0x000213ecu, "core", "read instrument bank"       },
    { "SND_PITCH_CHANGE",     0x0002140bu, "core", "set pitch/frequency"        },
    { "SND_VOLUME_CHANGE",    0x00021425u, "core", "set channel volume"         },

    /* FM synth (Yamaha YM2612 / YM3438). */
    { "SND_FM_READ_STATUS",   0x00021454u, "fm",   "read FM status register"    },
    { "SND_FM_WRITE_DATA",    0x00021461u, "fm",   "write FM data register"     },
    { "SND_FM_WRITE_SAVE_DATA", 0x0002147eu, "fm", "checkpoint FM write"        },
    { "SND_FM_READ_SAVE_DATA",  0x0002149bu, "fm", "read saved FM data"         },
    { "SND_FM_TIMER_A_SET",   0x000214bdu, "fm",   "set FM timer A period"      },
    { "SND_FM_TIMER_B_SET",   0x000214d7u, "fm",   "set FM timer B period"      },
    { "SND_FM_TIMER_A_START", 0x000214f1u, "fm",   "start FM timer A"           },
    { "SND_FM_TIMER_B_START", 0x000214fcu, "fm",   "start FM timer B"           },
    { "SND_FM_LFO_SET",       0x00021507u, "fm",   "set FM LFO"                 },

    /* PCM (RF5C68-alike). */
    { "SND_PCM_WAVE_SET",     0x0002151cu, "pcm",  "load PCM waveform"          },
    { "SND_PCM_MODE_SET",     0x0002153bu, "pcm",  "set PCM mode"               },
    { "SND_PCM_SOUND_SET",    0x00021550u, "pcm",  "assign PCM voice"           },
    { "SND_PCM_SOUND_DELETE", 0x00021565u, "pcm",  "delete PCM voice"           },
    { "SND_PCM_REC",          0x0002157au, "pcm",  "record PCM"                 },
    { "SND_PCM_REC2",         0x000215b0u, "pcm",  "record PCM (variant)"       },
    { "SND_PCM_PLAY",         0x000215d4u, "pcm",  "play PCM"                   },
    { "SND_PCM_REC_STOP",     0x000215f6u, "pcm",  "stop PCM record"            },
    { "SND_PCM_PLAY_STOP",    0x00021601u, "pcm",  "stop PCM playback"          },
    { "SND_PCM_STATUS",       0x00021616u, "pcm",  "read PCM status"            },
    { "SND_PCM_ABORT",        0x0002162du, "pcm",  "abort PCM"                  },

    /* Electronic volume (RS232C-adjacent hardware). */
    { "SND_ELEVOL_SET",       0x00021940u, "vol",  "set elevol"                 },
    { "SND_ELEVOL_INIT",      0x0002195du, "vol",  "init elevol"                },
    { "SND_ELEVOL_READ",      0x00021968u, "vol",  "read elevol"                },
    { "SND_ELEVOL_MUTE",      0x00021991u, "vol",  "elevol mute"                },
    { "SND_ELEVOL_ALL_MUTE",  0x000219c8u, "vol",  "elevol mute all"            }
};

const dm1_v1_fmtowns_snd_api_entry_t *
dm1_v1_fmtowns_snd_api_lookup_pc34(const char *name) {
    unsigned int i;
    if (!name) return NULL;
    for (i = 0; i < DM1_V1_FMTOWNS_SND_API_COUNT; ++i) {
        if (strcmp(dm1_v1_fmtowns_snd_api_entries[i].name, name) == 0) {
            return &dm1_v1_fmtowns_snd_api_entries[i];
        }
    }
    return NULL;
}

uint32_t dm1_v1_fmtowns_snd_api_vaddr_pc34(const char *name) {
    const dm1_v1_fmtowns_snd_api_entry_t *e =
        dm1_v1_fmtowns_snd_api_lookup_pc34(name);
    return e ? e->vaddr : 0u;
}
