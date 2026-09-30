#include "dm2_v1_midi_backend.h"

#ifdef __APPLE__
#include <CoreMIDI/CoreMIDI.h>
#include <AudioToolbox/AudioToolbox.h>
#include <AudioUnit/AudioUnit.h>

static MIDIClientRef g_dm2_midi_client;
static MIDIPortRef g_dm2_midi_port;
static MIDIEndpointRef g_dm2_midi_destination;
static AUGraph g_dm2_audio_graph;
static MusicDeviceComponent g_dm2_audio_synth;
static enum {
    DM2_MIDI_ROUTE_NONE = 0,
    DM2_MIDI_ROUTE_COREMIDI,
    DM2_MIDI_ROUTE_AUDIO_UNIT
} g_dm2_midi_route;
#endif
static DM2_V1_MidiBackendState g_dm2_midi_state;
static unsigned g_dm2_music_volume_0_128 = 128u;

#ifdef __APPLE__
static int dm2_v1_midi_backend_open_audio_unit(void)
{
    AudioComponentDescription synth_desc = {
        kAudioUnitType_MusicDevice,
        kAudioUnitSubType_DLSSynth,
        kAudioUnitManufacturer_Apple,
        0u,
        0u
    };
    AudioComponentDescription output_desc = {
        kAudioUnitType_Output,
        kAudioUnitSubType_DefaultOutput,
        kAudioUnitManufacturer_Apple,
        0u,
        0u
    };
    AUNode synth_node = 0;
    AUNode output_node = 0;
    AudioUnit unit = NULL;

    /* DLSMusicDevice uses macOS's installed General MIDI sound bank. This
     * keeps the Mac edition's original MIDI events intact and requires no
     * bundled replacement bank. Absence of a component or output device is
     * an ordinary unavailable-backend result. */
    if (!AudioComponentFindNext(NULL, &synth_desc) ||
        !AudioComponentFindNext(NULL, &output_desc) ||
        NewAUGraph(&g_dm2_audio_graph) != noErr ||
        AUGraphAddNode(g_dm2_audio_graph, &synth_desc, &synth_node) != noErr ||
        AUGraphAddNode(g_dm2_audio_graph, &output_desc, &output_node) != noErr ||
        AUGraphConnectNodeInput(g_dm2_audio_graph, synth_node, 0,
                                output_node, 0) != noErr ||
        AUGraphOpen(g_dm2_audio_graph) != noErr ||
        AUGraphNodeInfo(g_dm2_audio_graph, synth_node, NULL, &unit) != noErr ||
        !unit ||
        AUGraphInitialize(g_dm2_audio_graph) != noErr ||
        AUGraphStart(g_dm2_audio_graph) != noErr) {
        if (g_dm2_audio_graph) {
            (void)AUGraphStop(g_dm2_audio_graph);
            (void)AUGraphUninitialize(g_dm2_audio_graph);
            (void)DisposeAUGraph(g_dm2_audio_graph);
            g_dm2_audio_graph = NULL;
        }
        g_dm2_audio_synth = NULL;
        return 0;
    }
    g_dm2_audio_synth = (MusicDeviceComponent)unit;
    g_dm2_midi_route = DM2_MIDI_ROUTE_AUDIO_UNIT;
    return 1;
}

static int dm2_v1_midi_endpoint_can_render(
    MIDIEndpointRef endpoint)
{
    SInt32 offline = 0;
    SInt32 is_sampler = 0;
    SInt32 supports_general_midi = 0;
    OSStatus offline_status;
    OSStatus sampler_status;
    OSStatus general_midi_status;

    if (endpoint == 0) return 0;
    offline_status = MIDIObjectGetIntegerProperty(
        endpoint, kMIDIPropertyOffline, &offline);
    if (offline_status != noErr || offline != 0) return 0;

    /* A CoreMIDI destination only transports MIDI.  Do not mark an arbitrary
     * virtual endpoint (for example IAC) as audible playback unless its
     * inherited device/entity properties identify a General MIDI or sampler
     * sound generator.  Otherwise a successful MIDISend can silently discard
     * the soundtrack when the system DLS Audio Unit is unavailable. */
    sampler_status = MIDIObjectGetIntegerProperty(
        endpoint, kMIDIPropertyIsSampler, &is_sampler);
    general_midi_status = MIDIObjectGetIntegerProperty(
        endpoint, kMIDIPropertySupportsGeneralMIDI,
        &supports_general_midi);
    return (sampler_status == noErr && is_sampler != 0) ||
           (general_midi_status == noErr && supports_general_midi != 0);
}
#endif

int dm2_v1_midi_backend_is_compiled(void)
{
#ifdef __APPLE__
    return 1;
#else
    return 0;
#endif
}

DM2_V1_MidiBackendState dm2_v1_midi_backend_open(void)
{
#ifdef __APPLE__
    OSStatus status;
    ItemCount destination_count;
    ItemCount destination_index;
    if (g_dm2_midi_state == DM2_V1_MIDI_BACKEND_READY) {
        return g_dm2_midi_state;
    }
    dm2_v1_midi_backend_close();
    /* Prefer the system DLS synth so MIDI remains audible on a stock Mac.
     * CoreMIDI's first destination can be a virtual/IAC port with no sound
     * generator attached; treating that port as successful playback silently
     * drops the game's soundtrack. Keep external MIDI as a fallback when the
     * system synth or audio output is unavailable. */
    if (dm2_v1_midi_backend_open_audio_unit()) {
        g_dm2_midi_state = DM2_V1_MIDI_BACKEND_READY;
        dm2_v1_midi_backend_set_music_volume(g_dm2_music_volume_0_128);
        return g_dm2_midi_state;
    }
    destination_count = MIDIGetNumberOfDestinations();
    if (destination_count > 0u) {
        status = MIDIClientCreate(CFSTR("Firestaff DM2 MIDI"), NULL, NULL,
                                  &g_dm2_midi_client);
        if (status == noErr) {
            status = MIDIOutputPortCreate(g_dm2_midi_client,
                                          CFSTR("DM2 HMP output"),
                                          &g_dm2_midi_port);
            if (status == noErr) {
                for (destination_index = 0u;
                     destination_index < destination_count;
                     ++destination_index) {
                    MIDIEndpointRef candidate =
                        MIDIGetDestination(destination_index);
                    if (!dm2_v1_midi_endpoint_can_render(candidate)) continue;
                    g_dm2_midi_destination = candidate;
                    g_dm2_midi_route = DM2_MIDI_ROUTE_COREMIDI;
                    g_dm2_midi_state = DM2_V1_MIDI_BACKEND_READY;
                    dm2_v1_midi_backend_set_music_volume(
                        g_dm2_music_volume_0_128);
                    return g_dm2_midi_state;
                }
            }
        }
        if (g_dm2_midi_port) {
            MIDIPortDispose(g_dm2_midi_port);
            g_dm2_midi_port = 0;
        }
        if (g_dm2_midi_client) {
            MIDIClientDispose(g_dm2_midi_client);
            g_dm2_midi_client = 0;
        }
        g_dm2_midi_destination = 0;
    }
#endif
    return g_dm2_midi_state;
}

DM2_V1_MidiBackendState dm2_v1_midi_backend_state(void)
{
    return g_dm2_midi_state;
}

void dm2_v1_midi_backend_set_music_volume(unsigned volume_0_128)
{
    DM2_V1_MusicScheduledEvent event;
    unsigned channel;
    if (volume_0_128 > 128u) volume_0_128 = 128u;
    g_dm2_music_volume_0_128 = volume_0_128;
    if (g_dm2_midi_state != DM2_V1_MIDI_BACKEND_READY) return;
    for (channel = 0u; channel < 16u; ++channel) {
        event.status = (uint8_t)(0xb0u | channel);
        event.data1 = 7u; /* MIDI channel volume controller. */
        event.data2 = 127u;
        event.data_size = 2u;
        if (!dm2_v1_midi_backend_send(&event)) break;
    }
}

int dm2_v1_midi_backend_send(const DM2_V1_MusicScheduledEvent *event)
{
#ifdef __APPLE__
    MIDIPacketList packets;
    MIDIPacket *packet;
    uint8_t bytes[3];
    uint8_t data2;
    OSStatus status;
    if (!event || g_dm2_midi_state != DM2_V1_MIDI_BACKEND_READY ||
        event->status < 0x80u || event->status >= 0xf0u ||
        event->data_size == 0 || event->data_size > 2) {
        return 0;
    }
    data2 = event->data2;
    if ((event->status & 0xf0u) == 0xb0u && event->data_size == 2u &&
        event->data1 == 7u) {
        data2 = (uint8_t)(((unsigned)data2 * g_dm2_music_volume_0_128 +
                           64u) / 128u);
    }
    if (g_dm2_midi_route == DM2_MIDI_ROUTE_AUDIO_UNIT) {
        status = MusicDeviceMIDIEvent(g_dm2_audio_synth, event->status,
                                      event->data1, data2, 0u);
        if (status == noErr) return 1;
        g_dm2_midi_state = DM2_V1_MIDI_BACKEND_DELIVERY_FAILED;
        return 0;
    }
    if (g_dm2_midi_route != DM2_MIDI_ROUTE_COREMIDI) return 0;
    bytes[0] = event->status;
    bytes[1] = event->data1;
    bytes[2] = data2;
    packet = MIDIPacketListInit(&packets);
    if (!MIDIPacketListAdd(&packets, sizeof(packets), packet, 0,
                           (ByteCount)(event->data_size + 1u), bytes)) {
        g_dm2_midi_state = DM2_V1_MIDI_BACKEND_DELIVERY_FAILED;
        return 0;
    }
    status = MIDISend(g_dm2_midi_port, g_dm2_midi_destination, &packets);
    if (status == noErr) {
        return 1;
    }
    g_dm2_midi_state = DM2_V1_MIDI_BACKEND_DELIVERY_FAILED;
#else
    (void)event;
#endif
    return 0;
}

void dm2_v1_midi_backend_close(void)
{
#ifdef __APPLE__
    unsigned channel;
    if (g_dm2_midi_state == DM2_V1_MIDI_BACKEND_READY) {
        /* MIDI All Notes Off prevents a track/session change from leaving
         * a note held on the native destination. */
        for (channel = 0; channel < 16u; ++channel) {
            DM2_V1_MusicScheduledEvent all_notes_off;
            all_notes_off.status = (uint8_t)(0xb0u | channel);
            all_notes_off.data1 = 123u;
            all_notes_off.data2 = 0u;
            all_notes_off.data_size = 2u;
            (void)dm2_v1_midi_backend_send(&all_notes_off);
        }
    }
    if (g_dm2_midi_port != 0) {
        MIDIPortDispose(g_dm2_midi_port);
        g_dm2_midi_port = 0;
    }
    if (g_dm2_midi_client != 0) {
        MIDIClientDispose(g_dm2_midi_client);
        g_dm2_midi_client = 0;
    }
    g_dm2_midi_destination = 0;
    if (g_dm2_audio_graph) {
        (void)AUGraphStop(g_dm2_audio_graph);
        (void)AUGraphUninitialize(g_dm2_audio_graph);
        (void)DisposeAUGraph(g_dm2_audio_graph);
        g_dm2_audio_graph = NULL;
    }
    g_dm2_audio_synth = NULL;
    g_dm2_midi_route = DM2_MIDI_ROUTE_NONE;
#endif
    g_dm2_midi_state = DM2_V1_MIDI_BACKEND_UNAVAILABLE;
}
