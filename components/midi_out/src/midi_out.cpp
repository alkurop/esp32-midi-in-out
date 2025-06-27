#include "midi_out.hpp"
#include "midi_out_parser.hpp"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "MidiSends";
using namespace midi;

MidiOut::MidiOut(const MidiOutConfig &cfg) : config(cfg) {}

void MidiOut::init()
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
    uart_config_t uart_config = {
        .baud_rate = config.baud_rate,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    };
#pragma GCC diagnostic pop

    ESP_ERROR_CHECK(uart_param_config(config.uart_num, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(config.uart_num, config.sendPin, config.receivePin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    ESP_ERROR_CHECK(uart_driver_install(config.uart_num, 256, 256, 0, nullptr, 0));
    ESP_ERROR_CHECK(uart_flush(config.uart_num));

    tx_queue = xQueueCreate(32, sizeof(MidiTxMessage));

    xTaskCreate(
        [](void *arg)
        {
            auto *self = static_cast<MidiOut *>(arg);
            self->txLoop();
        },
        "midi_tx_task",
        2048,
        this,
        5,
        &tx_task);
}

void MidiOut::setNote(NoteMessage event)
{
    uint8_t packet[4];
    to_usb_packet(event, packet);
    sendBytes(&packet[1], 3);
}

void MidiOut::sendControllerChange(ControllerChange event)
{
    uint8_t packet[4];
    to_usb_packet(event, packet);

    sendBytes(&packet[1], 3);
}

void MidiOut::setTransportEvent(TransportEvent event)
{
    uint8_t packet[4];
    to_usb_packet(event, packet);
    sendBytes(&packet[1], 1); // real-time MIDI is 1 byte
}

void MidiOut::setSongPosition(SongPosition event)
{
    uint8_t packet[4];
    to_usb_packet(event, packet);
    sendBytes(&packet[1], 3);
}

void MidiOut::sendTimingClock()
{
    uint8_t data = 0xF8;
    sendBytes(&data, 1);
}

void MidiOut::txLoop()
{
    MidiTxMessage msg;
    while (true)
    {
        if (xQueueReceive(tx_queue, &msg, portMAX_DELAY))
        {
            int res = uart_write_bytes(config.uart_num, msg.data, msg.length);
            if (res < 0)
            {
                ESP_LOGE(TAG, "MIDI send failed: %s", esp_err_to_name(res));
            }
            else
            {
                ESP_LOGI(TAG, "MIDI send len=%u, wrote=%d", (unsigned int)msg.length, res);
                uart_wait_tx_done(config.uart_num, pdMS_TO_TICKS(10)); // ✅ Wait until TX is fully flushed
            }

        }
    }
}

void MidiOut::sendBytes(const uint8_t *data, size_t length)
{
    MidiTxMessage msg;
    memcpy(msg.data, data, length);
    msg.length = length;
    ESP_LOGI(TAG, "Sending: %02X %02X %02X", msg.data[1], msg.data[2], msg.data[3]);

    if (xQueueSend(tx_queue, &msg, 0) != pdTRUE)
    {
        ESP_LOGW(TAG, "MIDI TX queue full — message dropped");
    }
}
