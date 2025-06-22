#include "midi_in.hpp"
#include "midi_parser.hpp"
#include <functional>
#include <cstdint>
#include "esp_log.h"

static const char *TAG = "Main";

using namespace midi;

MidiParser parser;
MidiInConfig config = {.pin = GPIO_NUM_5};
MidiIn midiIn(config);

auto controllerCallback = [](const ControllerChange event)
{
    ESP_LOGI(TAG, "ControllerChange value %u", event.value);
};

auto positionCallback = [](const SongPosition event)
{
    ESP_LOGI(TAG, "SongPosition position %u", event.position);
};
auto noteCallback = [](const NoteMessage event)
{
    ESP_LOGI(TAG, "NoteMessage note %u", event.note);
};

auto transportCallback = [](const TransportEvent event)
{
    ESP_LOGI(TAG, "TransportEvent note %u", static_cast<uint8_t>(event.command));
};
auto bpmCallback = [](const uint8_t bpm)
{
    ESP_LOGI(TAG, "BpmEvent value %u", bpm);
};

auto midiInCallback = [](const Packet4 midiPacket){
    parser.feed(midiPacket.data());
};

extern "C" void app_main()
{
    parser.setControllerCallback(controllerCallback);
    parser.setNoteMessageCallback(noteCallback);
    parser.setTransportCallback(transportCallback);
    parser.setBpmCallback(bpmCallback);
    parser.setSongPositionCallback(positionCallback);
    midiIn.init(midiInCallback);

}
