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
    // Buffer on stack to avoid per-iteration allocation
    uint8_t buf[MIDI_RX_BUFFER_SIZE];
    while (true)
    {
        int len = uart_read_bytes(
            config.uart_num,
            buf,
            config.rx_buffer_size,
            config.rx_timeout);
        if (len > 0 && callback)
        {
            for (int i = 0; i < len; ++i)
            {
                callback(buf[i]);
            }
        }
        // Yield to other tasks / avoid blocking
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}
