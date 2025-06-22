#include "midi_in.hpp"
static const char *TAG = "midi_in";

using namespace midi;

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
                                 config.pin,
                                 UART_PIN_NO_CHANGE,
                                 UART_PIN_NO_CHANGE));

    // 3) Install UART driver with RX buffer
    ESP_ERROR_CHECK(uart_driver_install(config.uart_num,
                                        config.rx_buffer_size * 2,
                                        0,
                                        0,
                                        nullptr,
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
   Packet4 pkt;
    for (;;) {
        // read up to exactly 4 bytes straight into pkt
        int len = uart_read_bytes(
            config.uart_num,
            pkt.data(),          // <- pointer to your 4-byte storage
            pkt.size(),          // <- always 4
            config.rx_timeout
        );
        if (len == static_cast<int>(pkt.size()) && callback) {
            // now you have a full 4-byte USB-MIDI packet in pkt
            callback(pkt);
        }
        // if you get a partial read, you could buffer it until you have 4,
        // or just spin again and let the next iteration top you up.
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}
