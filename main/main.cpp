#include "midi_in.hpp"
#include "midi_in_parser.hpp"
#include "midi_out.hpp"
#include <functional>
#include <cstdint>
#include "esp_log.h"

static const char *TAG = "Main";

using namespace midi;

MidiInParser parser;
MidiInConfig inConfig = {.receivePin = GPIO_NUM_5, .uart_num = UART_NUM_1};
MidiOutConfig outConfig = {.sendPin = GPIO_NUM_10, .receivePin = GPIO_NUM_0, .uart_num = UART_NUM_0};
MidiIn midiIn(inConfig);
MidiOut midiOut(outConfig);

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

auto midiInCallback = [](const Packet4 midiPacket)
{
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
    midiOut.init();

    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(2000));
        midiOut.setNote({
            .channel = 1,
            .on = true,
            .note = 3,
            .velocity = 127,
        });
    }
}
