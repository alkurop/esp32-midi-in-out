#pragma once

#include "driver/gpio.h"
#include "driver/uart.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <functional>
#include <array>

// MIDI UART and task parameters (fixed)

namespace midi
{

    // Callback invoked for each received MIDI byte
    using Packet4 = std::array<uint8_t, 4>;

    using MidiCallback = std::function<void(Packet4)>;

    // Configuration for the MIDI input component
    struct MidiInConfig
    {
        gpio_num_t receivePin;                              // RX pin
        uart_port_t uart_num;        // UART port to use
        size_t rx_buffer_size = 256; // UART RX buffer
        TickType_t rx_timeout = pdMS_TO_TICKS(20);
    };

    class MidiIn
    {
    public:
        // Construct with config (but does not start I/O until init)
        explicit MidiIn(const MidiInConfig &config);

        // Set the callback and start the MIDI input task
        void init(MidiCallback cb);

    private:
        void taskLoop();

        MidiInConfig config;   // UART and timing configuration
        MidiCallback callback; // User callback for each byte
        TaskHandle_t task_handle = nullptr;
        QueueHandle_t uart_queue = nullptr;
    };

} // namespace midi_in
