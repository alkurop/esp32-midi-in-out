#pragma once

#include <cstdint>
#include <functional>
#include <esp_timer.h>
#include <esp_log.h>
#include "bpm_counter.hpp"
#include "midi_protocol.hpp"

namespace midi
{

    using MidiControllerCallback = std::function<void(const ControllerChange &)>;

    using MidiSongPositionCallback = std::function<void(const SongPosition &)>;

    using MidiNoteMessageCallback = std::function<void(const NoteMessage &)>;

    using MidiTransportCallback = std::function<void(const TransportEvent &)>;

    class MidiInParser
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
        MidiInParser();

        // Feed a 4-byte USB MIDI packet (USB MIDI format)
        void feed(const uint8_t packet[4]);

        // Register callback for MIDI CC messages
        void setControllerCallback(MidiControllerCallback cb) { this->controllerCallback = cb; };
        void setSongPositionCallback(MidiSongPositionCallback cb) { this->songPositionCallback = cb; };
        void setNoteMessageCallback(MidiNoteMessageCallback cb) { this->noteMessageCallback = cb; };
        void setTransportCallback(MidiTransportCallback cb) { this->transportCallback = cb; };
        void setBpmCallback(BpmCounter::BpmCallback callback) { this->bpmCounter.setCallback(callback); };
    };

  

    inline MidiMessageType getMidiMessageType(uint8_t statusByte)
    {
        if (statusByte >= 0xF0)
        {
            return static_cast<MidiMessageType>(statusByte);
        }
        return static_cast<MidiMessageType>(statusByte & 0xF0);
    }

    
}
