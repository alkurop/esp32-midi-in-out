#pragma once

#include <cstdint>
#include <functional>
#include <esp_timer.h>
#include <esp_log.h>
#include "bpm_counter.hpp"

namespace midi
{
    struct ControllerChange
    {
        uint8_t channel;    // MIDI channel (0–15)
        uint8_t controller; // Controller number (e.g. 1 = mod wheel)
        uint8_t value;      // Value (0–127)
    };

    using MidiControllerCallback = std::function<void(const ControllerChange &)>;

    struct SongPosition
    {
        uint16_t position; // Position in MIDI beats (16th notes)
    };
    using MidiSongPositionCallback = std::function<void(const SongPosition &)>;

    struct NoteMessage
    {
        uint8_t channel;  // MIDI channel (0–15)
        bool on;          // true = Note On, false = Note Off
        uint8_t note;     // MIDI note number (0–127)
        uint8_t velocity; // Velocity or release velocity
    };

    using MidiNoteMessageCallback = std::function<void(const NoteMessage &)>;

    enum class TransportCommand : uint8_t
    {
        Start = 0xFA,
        Continue = 0xFB,
        Stop = 0xFC,
        Unknown = 0x00
    };

    struct TransportEvent
    {
        TransportCommand command;
    };

    using MidiTransportCallback = std::function<void(const TransportEvent &)>;

    class MidiParser
    {
    private:
        MidiControllerCallback controllerCallback;
        MidiSongPositionCallback songPositionCallback;
        MidiNoteMessageCallback noteMessageCallback;
        MidiTransportCallback transportCallback;
        BpmCounter bpmCounter;

        void parseControllerChange(const uint8_t packet[4]);
        void parseSongPosition(const uint8_t packet[4]);
        void parseNoteMessage(const uint8_t packet[4], bool on);
        void parseTransportCommand(const uint8_t packet[4]);
        void parseTimingClock(const uint8_t packet[4]); // new use of BpmCounter

    public:
        MidiParser();

        // Feed a 4-byte USB MIDI packet (USB MIDI format)
        void feed(const uint8_t packet[4]);

        // Register callback for MIDI CC messages
        void setControllerCallback(MidiControllerCallback cb) { this->controllerCallback = cb; };
        void setSongPositionCallback(MidiSongPositionCallback cb) { this->songPositionCallback = cb; };
        void setNoteMessageCallback(MidiNoteMessageCallback cb) { this->noteMessageCallback = cb; };
        void setTransportCallback(MidiTransportCallback cb) { this->transportCallback = cb; };
        void setBpmCallback(BpmCounter::BpmCallback callback) { this->bpmCounter.setCallback(callback); };
    };

    enum class MidiMessageType : uint8_t
    {
        NoteOff = 0x80,
        NoteOn = 0x90,
        PolyAftertouch = 0xA0,
        ControlChange = 0xB0,
        ProgramChange = 0xC0,
        ChannelPressure = 0xD0,
        PitchBend = 0xE0,

        // System messages (0xF0–0xFF)
        SystemExclusive = 0xF0,
        TimeCodeQuarter = 0xF1,
        SongPosition = 0xF2,
        SongSelect = 0xF3,
        TuneRequest = 0xF6,
        EndOfExclusive = 0xF7,
        TimingClock = 0xF8,
        Start = 0xFA,
        Continue = 0xFB,
        Stop = 0xFC,
        ActiveSensing = 0xFE,
        SystemReset = 0xFF,

        Unknown = 0x00
    };

    inline MidiMessageType getMidiMessageType(uint8_t statusByte)
    {
        if (statusByte >= 0xF0)
        {
            return static_cast<MidiMessageType>(statusByte);
        }
        return static_cast<MidiMessageType>(statusByte & 0xF0);
    }

    inline const char *to_string(MidiMessageType type)
    {
        switch (type)
        {
        case MidiMessageType::NoteOff:
            return "Note Off";
        case MidiMessageType::NoteOn:
            return "Note On";
        case MidiMessageType::PolyAftertouch:
            return "Poly Aftertouch";
        case MidiMessageType::ControlChange:
            return "Control Change";
        case MidiMessageType::ProgramChange:
            return "Program Change";
        case MidiMessageType::ChannelPressure:
            return "Channel Pressure";
        case MidiMessageType::PitchBend:
            return "Pitch Bend";
        case MidiMessageType::SystemExclusive:
            return "System Exclusive";
        case MidiMessageType::TimeCodeQuarter:
            return "Time Code Quarter";
        case MidiMessageType::SongPosition:
            return "Song Position";
        case MidiMessageType::SongSelect:
            return "Song Select";
        case MidiMessageType::TuneRequest:
            return "Tune Request";
        case MidiMessageType::EndOfExclusive:
            return "End of SysEx";
        case MidiMessageType::TimingClock:
            return "Timing Clock";
        case MidiMessageType::Start:
            return "Start";
        case MidiMessageType::Continue:
            return "Continue";
        case MidiMessageType::Stop:
            return "Stop";
        case MidiMessageType::ActiveSensing:
            return "Active Sensing";
        case MidiMessageType::SystemReset:
            return "System Reset";
        default:
            return "Unknown";
        }
    }
}
