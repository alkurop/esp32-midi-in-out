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

    // 1) Configure UART parameters (8N1, MIDI baud)
    uart_config_t uart_cfg = {
        .baud_rate = MIDI_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE};
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
    // ESP_ERROR_CHECK(uart_set_line_inverse(config.uart_num, UART_SIGNAL_RXD_INV));

    // 4) Launch MIDI reader task
    xTaskCreate(
        [](void *arg)
        {
            auto *self = static_cast<MidiIn *>(arg);
            self->taskLoop();
        },
        "midi_in_task",
        MIDI_TASK_STACK_SIZE,
        this,
        MIDI_TASK_PRIORITY,
        &task_handle);
}

void MidiIn::taskLoop()
{
    uart_event_t event;
    uint8_t midi_packet[3];

    while (true)
    {
        // Block until a UART event is received
        if (xQueueReceive(uart_queue, &event, portMAX_DELAY))
        {
            ESP_LOGI(TAG, "Event %d", static_cast<int>(event.type));

            if (event.type == UART_DATA)
            {
                uint8_t byte;
                Packet4 pkt = {0};    // pkt[0] = dummy CIN
                int packet_index = 1; // start from pkt[1]
                int midi_index = 0;

                while (uart_read_bytes(config.uart_num, &byte, 1, 0) == 1)
                {
                    midi_packet[midi_index++] = byte;

                    if (midi_index == 3) // Standard MIDI message
                    {
                        ESP_LOGI(TAG, "Receiving: %02X %02X %02X", midi_packet[0], midi_packet[1], midi_packet[2]);
                        ESP_LOGI(TAG, "Receiving (bin): %s %s %s",
                                 toBinary(midi_packet[0]).c_str(),
                                 toBinary(midi_packet[1]).c_str(),
                                 toBinary(midi_packet[2]).c_str());

                        // Optional: wrap into a Packet4 with dummy CIN = 0
                        Packet4 pkt = {0, midi_packet[0], midi_packet[1], midi_packet[2]};
                        if (callback)
                            callback(pkt);

                        midi_index = 0;
                    }
                }
            }
        }
    }
}
