#include "midi_in.hpp"
#include "midi_parser.hpp"
#include <functional>
#include <cstdint>
#include "esp_log.h"

static const char *TAG = "Main";

using namespace midi_module;

MidiParser parser;

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
    ESP_LOGI(TAG, "TransportEvent note %u", event.command);
};
auto bpmCallback = [](const uint8_t bpm)
{
    ESP_LOGI(TAG, "BpmEvent value %u", bpm);
};

extern "C" void app_main()
{
    parser.setControllerCallback(controllerCallback);
    parser.setNoteMessageCallback(noteCallback);
    parser.setTransportCallback(transportCallback);
    parser.setBpmCallback(bpmCallback);
    parser.setSongPositionCallback(positionCallback);
}
