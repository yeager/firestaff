#include "dm2_v1_midi_backend.h"

#ifdef __APPLE__
#include <CoreMIDI/CoreMIDI.h>

static MIDIClientRef g_dm2_midi_client;
static MIDIPortRef g_dm2_midi_port;
static MIDIEndpointRef g_dm2_midi_destination;
#endif
static DM2_V1_MidiBackendState g_dm2_midi_state;

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
    if (g_dm2_midi_state == DM2_V1_MIDI_BACKEND_READY) {
        return g_dm2_midi_state;
    }
    dm2_v1_midi_backend_close();
    g_dm2_midi_destination = MIDIGetDestination(0);
    if (g_dm2_midi_destination == 0) {
        return g_dm2_midi_state;
    }
    status = MIDIClientCreate(CFSTR("Firestaff DM2 MIDI"), NULL, NULL,
                              &g_dm2_midi_client);
    if (status != noErr) {
        return g_dm2_midi_state;
    }
    status = MIDIOutputPortCreate(g_dm2_midi_client, CFSTR("DM2 HMP output"),
                                  &g_dm2_midi_port);
    if (status != noErr) {
        dm2_v1_midi_backend_close();
        return g_dm2_midi_state;
    }
    g_dm2_midi_state = DM2_V1_MIDI_BACKEND_READY;
#endif
    return g_dm2_midi_state;
}

DM2_V1_MidiBackendState dm2_v1_midi_backend_state(void)
{
    return g_dm2_midi_state;
}

int dm2_v1_midi_backend_send(const DM2_V1_MusicScheduledEvent *event)
{
#ifdef __APPLE__
    MIDIPacketList packets;
    MIDIPacket *packet;
    uint8_t bytes[3];
    OSStatus status;
    if (!event || g_dm2_midi_state != DM2_V1_MIDI_BACKEND_READY ||
        event->status < 0x80u || event->status >= 0xf0u ||
        event->data_size == 0 || event->data_size > 2) {
        return 0;
    }
    bytes[0] = event->status;
    bytes[1] = event->data1;
    bytes[2] = event->data2;
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
#endif
    g_dm2_midi_state = DM2_V1_MIDI_BACKEND_UNAVAILABLE;
}
