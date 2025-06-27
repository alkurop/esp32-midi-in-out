#include "midi_in.hpp"
#include "esp_log.h"

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
    Packet4 pkt;
    int packet_index = 0;

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

                while (uart_read_bytes(config.uart_num, &byte, 1, 0) == 1)
                {
                    pkt[packet_index++] = byte;

                    if (packet_index == 4) // pkt[1..3] filled
                    {
                        if (callback)
                        {
                            ESP_LOGI(TAG, "Receiving: %02X %02X %02X", pkt[1], pkt[2], pkt[3]);

                            callback(pkt); // USB-style 4-byte packet
                        }
                        packet_index = 1; // reset to pkt[1]
                    }
                }
            }
        }
    }
}
