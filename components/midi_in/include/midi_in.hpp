#pragma once

#include "driver/uart.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <functional>

// MIDI UART and task parameters (fixed)
#define MIDI_UART_NUM UART_NUM_0
#define MIDI_BAUD_RATE 31250
#define MIDI_RX_BUFFER_SIZE 256
#define MIDI_RX_TIMEOUT_MS 20
#define MIDI_TASK_PRIORITY (configMAX_PRIORITIES - 5)
#define MIDI_TASK_STACK_SIZE 2048

namespace midi
{


    // Callback invoked for each received MIDI byte
    using MidiCallback = std::function<void(uint8_t)>;

    // Configuration for the MIDI input component
    struct MidiInConfig
    {
        gpio_num_t pin;                          // RX pin
        uart_port_t uart_num = MIDI_UART_NUM;        // UART port to use
        size_t rx_buffer_size = MIDI_RX_BUFFER_SIZE; // UART RX buffer
        TickType_t rx_timeout = pdMS_TO_TICKS(MIDI_RX_TIMEOUT_MS);
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
    };

} // namespace midi_in
