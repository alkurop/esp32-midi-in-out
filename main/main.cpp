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
void setup_gpio_for_midi_out()
{
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << GPIO_NUM_10),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE, // MIDI uses external pull-up
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE};
    gpio_config(&io_conf);
}
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
        vTaskDelay(pdMS_TO_TICKS(3000));
        midiOut.setNote({
            .channel = 1,
            .on = true,
            .note = 44,
            .velocity = 127,
        });
        vTaskDelay(pdMS_TO_TICKS(1000));
           midiOut.setNote({
            .channel = 1,
            .on = false,
            .note =45,
            .velocity = 127,
        });
    }
}
