#ifndef _MIDI_h
#define _MIDI_h

#include <stdint.h>
#include "FS.h"
// #include "event.hpp"

#define SWAP_SHORT(s) ((s & 0xFF) << 8 | (s & 0xFF00) >> 8)
#define SWAP_INT(i) ((i & 0xFF) << 24 | (i & 0xFF00) << 8 | (i & 0xFF0000) >> 8 | (i & 0xFF000000) >> 24)
#define CHECK_BIT(var, pos) ((var) & (1 << (pos)))
#define MIDI_HZ(midiNote) (440.0 * pow(2.0,(midiNote - 69.0)/12.0))
namespace midi
{
    typedef struct Event
    {
        uint32_t time;
        uint8_t statusByte;
        uint8_t *buf;
        uint32_t len;
        Event() : buf(nullptr) {}
        ~Event()
        {
            if (buf != nullptr)
            {
                free(buf);
            }
        };
    } Event_t;

    typedef struct EventHandler
    {
        EventHandler() :

                         NoteOffHandler([](uint8_t chan, uint8_t note, uint8_t velocity) {}),
                         NoteOnHandler([](uint8_t chan, uint8_t note, uint8_t velocity) {}),
                         AfterTouchHandler([](uint8_t chan, uint8_t pressure, uint8_t value) {}),
                         ControlChangeHandler([](uint8_t chan, uint8_t control, uint8_t value) {}),
                         ProgramChangeHandler([](uint8_t chan, uint8_t program) {}),
                         ChannelPressureHandler([](uint8_t chan, uint8_t pressure) {}),
                         PitchBendHandler([](uint8_t chan, uint16_t pitch) {}),
                         MTCQFHandler([](uint8_t type, uint16_t value) {}),
                         SongPositionPointerHandler([](uint16_t position) {}),
                         SongSelectHandler([](uint8_t song) {}),
                         TuneRequestHandler([]() {}),
                         EndOfSystemExclusiveHandler([]() {}),
                         TimingClockHandler([]() {}),
                         StartHandler([]() {}),
                         ContinueHandler([]() {}),
                         StopHandler([]() {}),
                         ActiveSensingHandler([]() {}),
                         SystemResetHandler([]() {}),
                         SystemExclusiveHandler([](const uint8_t *buf, uint32_t len) {}){};
        void (*NoteOffHandler)(uint8_t chan, uint8_t note, uint8_t velocity);
        void (*NoteOnHandler)(uint8_t chan, uint8_t note, uint8_t velocity);
        void (*AfterTouchHandler)(uint8_t chan, uint8_t pressure, uint8_t value);
        void (*ControlChangeHandler)(uint8_t chan, uint8_t control, uint8_t value);
        void (*ProgramChangeHandler)(uint8_t chan, uint8_t program);
        void (*ChannelPressureHandler)(uint8_t chan, uint8_t pressure);
        void (*PitchBendHandler)(uint8_t chan, uint16_t pitch);

        void (*MTCQFHandler)(uint8_t type, uint16_t value);
        void (*SongPositionPointerHandler)(uint16_t position);
        void (*SongSelectHandler)(uint8_t song);
        void (*TuneRequestHandler)();
        void (*EndOfSystemExclusiveHandler)();

        // SYSTEM REAL-TIME
        void (*TimingClockHandler)();
        void (*StartHandler)();
        void (*ContinueHandler)();
        void (*StopHandler)();
        void (*ActiveSensingHandler)();
        void (*SystemResetHandler)();

        // SYSTEM EXCLUSIVE
        void (*SystemExclusiveHandler)(const uint8_t *buf, uint32_t len);
    } EventHandler_t;
    void ProcessEvent(const Event *event, const EventHandler_t *eventHandler);

    class File
    {
    public:
        File(fs::File *file);
        void BufferPlayTrack(uint8_t track, const midi::EventHandler* eventHandler);
        void PlayTrack(uint8_t track, const midi::EventHandler *eventHandler);
        bool SeekTrack(uint8_t);
        bool TrackRemain();
        void TrackInfo(uint8_t);
        void Stop();
        void QueueTrack(uint8_t track);

        uint32_t SkipBytes(uint32_t num);
        uint32_t SkipDataBytes();
        uint32_t ReadVarLen(uint32_t &bytes_read);
        uint32_t ReadVarLen();
        uint32_t ReadInt();
        uint16_t ReadShort();
        uint8_t ReadByte();
        uint32_t ReadBytesToBuffer(uint8_t *buf, uint32_t len);
    
        uint16_t GetFormat(){return format_spec;};
        uint16_t GetNumTracks(){return num_tracks;};
        uint16_t GetDeltaTimeLen(){return delta_time_len;};
    private:
        fs::File *midiFile;
        bool TryGetEvent(Event &event);
        void HandleMetaEvent(Event *event);

        void ReadHeader();
        void Play();
        bool play = false;
        bool stop = true;
        bool pause = false;

        // HEADER DATA
        uint32_t header;
        uint32_t header_len;
        uint16_t format_spec;
        uint16_t num_tracks;
        uint16_t delta_time_len;
    };
}
#endif