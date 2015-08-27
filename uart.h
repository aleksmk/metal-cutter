// UART debugging interface.
// Aleks - Z32AL

#ifndef UART_H
#define UART_H

//#include "defs.h"

#include <stdio.h>
#include <util/setbaud.h>

void init_uart(void);

int uart_putchar(char, FILE *);

extern FILE uart_output;


#endif
