#include "defs.h"
#include "uart.h"

#include <avr/io.h>
#include <stdio.h>

FILE uart_output = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);

void init_uart(void) {
    // Set BAUD
    UBRR0H = UBRRH_VALUE;
    UBRR0L = UBRRL_VALUE;

    // Enable RX and TX
    UCSR0B |= (1<<RXEN0) | (1<<TXEN0);

    // 8-bit data, no parity, one stop bit
    UCSR0C |= (1<<UCSZ01) | (1<<UCSZ00);

    stdout = &uart_output;

}

int uart_putchar(char c, FILE *stream) {
    if(c == '\n') {
        uart_putchar('\r', stream);
    }
    loop_until_bit_is_set(UCSR0A, UDRE0); // wait until we have a clean DATA register !
    UDR0 = c;
    return 0;
}
