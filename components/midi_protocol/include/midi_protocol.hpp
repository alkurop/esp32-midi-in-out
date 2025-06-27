#pragma once
#include <cstdint>
#include <string.h>
#include <string>
#define MIDI_BAUD_RATE 31250

namespace midi
{
    struct ControllerChange
    {
        uint8_t channel;    // MIDI channel (0–15)
        uint8_t controller; // Controller number (e.g. 1 = mod wheel)
        uint8_t value;      // Value (0–127)
    };

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

    struct NoteMessage
    {
        uint8_t channel;  // MIDI channel (0–15)
        bool on;          // true = Note On, false = Note Off
        uint8_t note;     // MIDI note number (0–127)
        uint8_t velocity; // Velocity or release velocity
    };

    struct SongPosition
    {
        uint16_t position; // Position in MIDI beats (16th notes)
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

    inline std::string toBinary(uint8_t byte)
    {
        char buf[9];
        for (int i = 7; i >= 0; --i)
            buf[7 - i] = (byte & (1 << i)) ? '1' : '0';
        buf[8] = '\0';
        return std::string(buf);
    }

    inline int getMidiMessageSize(MidiMessageType type)
    {
        using T = MidiMessageType;
        switch (type)
        {
        // 3-byte channel voice messages
        case T::NoteOff:
        case T::NoteOn:
        case T::PolyAftertouch:
        case T::ControlChange:
        case T::PitchBend:
            return 3;

        // 2-byte channel voice messages
        case T::ProgramChange:
        case T::ChannelPressure:
            return 2;

        // System Common
        case T::SongPosition:
            return 3;
        case T::TimeCodeQuarter:
        case T::SongSelect:
            return 2;
        case T::TuneRequest:
        case T::EndOfExclusive:
            return 1;

        // System Real-Time (always 1 byte)
        case T::TimingClock:
        case T::Start:
        case T::Continue:
        case T::Stop:
        case T::ActiveSensing:
        case T::SystemReset:
            return 1;

        // System Exclusive (variable length)
        case T::SystemExclusive:
            return -1;

        case T::Unknown:
        default:
            return 1; // Safe fallback
        }
    }

    inline MidiMessageType getMessageType(uint8_t statusByte)
    {
        using T = MidiMessageType;

        if (statusByte >= 0xF8)
            return static_cast<T>(statusByte); // Real-time messages (single-byte)
        if (statusByte >= 0xF0 && statusByte <= 0xF7)
            return static_cast<T>(statusByte); // System Common messages
        if ((statusByte & 0x80) == 0)
            return T::Unknown; // Not a status byte (data byte)

        return static_cast<T>(statusByte & 0xF0); // Channel Voice messages (high nibble identifies type)
    }
}
