#include "midi_in.hpp"
#include "midi_protocol.hpp"
#include "esp_log.h"
#include <soc/uart_reg.h>

using namespace midi;
static const char *TAG = "MidiReceives";

MidiIn::MidiIn(const MidiInConfig &config)
    : config(config), callback(nullptr), task_handle(nullptr)
{
}

void MidiIn::init(MidiCallback cb)
{
    callback = cb;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
    // 1) Configure UART parameters (8N1, MIDI baud)
    uart_config_t uart_cfg = {
        .baud_rate = MIDI_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE};
#pragma GCC diagnostic pop

    ESP_ERROR_CHECK(uart_param_config(config.uart_num, &uart_cfg));

    // 2) Set UART pins (TX unused)
    ESP_ERROR_CHECK(uart_set_pin(config.uart_num,
                                 UART_PIN_NO_CHANGE,
                                 config.receivePin,
                                 UART_PIN_NO_CHANGE,
                                 UART_PIN_NO_CHANGE));

    // 3) Install UART driver with RX buffer
    ESP_ERROR_CHECK(uart_driver_install(config.uart_num,
                                        config.rx_buffer_size * 2,
                                        0,
                                        10, // queue size
                                        &uart_queue,
                                        0));
    uart_intr_config_t intr_conf = {
        .intr_enable_mask = UART_RXFIFO_FULL_INT_ENA_M | UART_RXFIFO_TOUT_INT_ENA_M,
        .rx_timeout_thresh = 2, // ~2 character durations (for idle detection)
        .txfifo_empty_intr_thresh = 10,
        .rxfifo_full_thresh = 3, // Trigger interrupt on every byte
    };
    ESP_ERROR_CHECK(uart_intr_config(config.uart_num, &intr_conf));

    // 4) Launch MIDI reader task
    xTaskCreate(
        [](void *arg)
        {
            auto *self = static_cast<MidiIn *>(arg);
            self->taskLoop();
        },
        "midi_in_task",
        4098,
        this,
        configMAX_PRIORITIES - 5,
        &task_handle);
}

void MidiIn::taskLoop()
{
    uart_event_t event;
    uint8_t midi_packet[3];

    while (true)
    {
        if (xQueueReceive(uart_queue, &event, portMAX_DELAY))
        {
            ESP_LOGI(TAG, "Event %d", static_cast<int>(event.type));

            if (event.type == UART_DATA)
            {
                uint8_t byte;
                int midi_index = 0;
                int expected_length = 0;
                while (uart_read_bytes(config.uart_num, &byte, 1, 0) == 1)
                {
                    if (byte & 0x80) // Status byte
                    {
                        midi_index = 0;
                        midi_packet[midi_index++] = byte;

                        // Determine expected message length
                        auto type = getMessageType(byte);
                        expected_length = getMidiMessageSize(type);
                        if (expected_length <= 0)
                        {
                            ESP_LOGW(TAG, "Skipping unsupported/variable-length MIDI msg: 0x%02X", byte);
                            midi_index = 0;
                            expected_length = 0;
                        }
                    }
                    else if (midi_index > 0 && midi_index < 3) // Data byte
                    {
                        midi_packet[midi_index++] = byte;
                    }

                    if (midi_index == expected_length)
                    {
                        ESP_LOGI(TAG, "Receiving: %02X %02X %02X", midi_packet[0], midi_packet[1], midi_packet[2]);

                        Packet4 pkt = {0, midi_packet[0], midi_packet[1], midi_packet[2]};
                        if (callback)
                            callback(pkt);

                        midi_index = 0;
                        expected_length = 0;
                    }
                }
            }
        }
    }
}
