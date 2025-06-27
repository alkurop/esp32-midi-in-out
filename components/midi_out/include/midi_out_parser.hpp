#pragma once
#include "midi_protocol.hpp"

namespace midi
{
    inline void to_usb_packet(const ControllerChange &msg, uint8_t out[4])
    {
        out[0] = (0 << 4) | 0xB;
        out[1] = 0xB0 | (msg.channel & 0x0F);
        out[2] = msg.controller;
        out[3] = msg.value;
    }

    inline void to_usb_packet(const NoteMessage &msg, uint8_t out[4])
    {
        const uint8_t status = (msg.on ? 0x90 : 0x80) | (msg.channel & 0x0F);
        const uint8_t cin = msg.on ? 0x9 : 0x8;
        out[0] = (0 << 4) | cin;
        out[1] = status;
        out[2] = msg.note;
        out[3] = msg.velocity;
    }

    inline void to_usb_packet(const TransportEvent &msg, uint8_t out[4])
    {
        out[0] = (0 << 4) | 0x5;
        out[1] = static_cast<uint8_t>(msg.command);
        out[2] = 0x00;
        out[3] = 0x00;
    }

    inline void to_usb_packet(const SongPosition &msg, uint8_t out[4])
    {
        out[0] = (0 << 4) | 0x3;
        out[1] = 0xF2;
        out[2] = static_cast<uint8_t>(msg.position & 0x7F);
        out[3] = static_cast<uint8_t>((msg.position >> 7) & 0x7F);
    }


}
