#include "midi_out.hpp"
#include "midi_out_parser.hpp"

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
    ESP_ERROR_CHECK(uart_set_pin(config.uart_num, config.tx_pin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    ESP_ERROR_CHECK(uart_driver_install(config.uart_num, 0, 256, 0, nullptr, 0));
}

void MidiOut::sendControllerChange(ControllerChange event)
{
    uint8_t packet[4];
    to_usb_packet(event, packet);
    uart_write_bytes(config.uart_num, &packet[1], 3);
}

void MidiOut::setNote(NoteMessage event)
{
    uint8_t packet[4];
    to_usb_packet(event, packet);
    uart_write_bytes(config.uart_num, &packet[1], 3);
}

void MidiOut::setTransportEvent(TransportEvent event)
{
    uint8_t packet[4];
    to_usb_packet(event, packet);
    uart_write_bytes(config.uart_num, &packet[1], 1); // system real-time messages are single byte
}

void MidiOut::setSongPosition(SongPosition event)
{
    uint8_t packet[4];
    to_usb_packet(event, packet);
    uart_write_bytes(config.uart_num, &packet[1], 3);
}

void MidiOut::sendTimingClock()
{
    uint8_t data = 0xF8;
    uart_write_bytes(config.uart_num, &data, 1);
}
