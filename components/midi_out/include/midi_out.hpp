#pragma once
#include "driver/gpio.h"
#include "driver/uart.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <functional>

#include "midi_protocol.hpp"

namespace midi
{

    struct MidiOutConfig
    {
        uart_port_t uart_num = UART_NUM_1;
        gpio_num_t tx_pin = GPIO_NUM_17;
        int baud_rate = 31250;
    };

    class MidiOut

    {
    public:
        MidiOut(const MidiOutConfig &config);
        void init();
        void sendControllerChange(ControllerChange event);
        void setSongPosition(SongPosition event);
        void setNote(NoteMessage event);
        void setTransportEvent(TransportEvent event);
        void sendTimingClock();

    private:
        TaskHandle_t task_handle = nullptr;
        MidiOutConfig config;
    };

}
