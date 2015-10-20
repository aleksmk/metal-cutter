#include "defs.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>


ISR(PCINT0_vect) {
    uint8_t DEK0 = 0;
    uint8_t DEK1 = 0;
    uint8_t DEK2 = 0;
    uint8_t DEK3 = 0;
    uint8_t DEK4 = 0;
    angularPosition = 0;

    // Читање на декадите
    DEK0 = (0x0F & PINB);
    DEK1 = (0x0F & (PINB >> 4));
    DEK2 = (0x0F & PINK);
    DEK3 = (0X0F & (PINK >> 4));
    DEK4 = (0x0F & PINJ);

    // Претварање од *код*-от, во декаден броен систем
    switch(DEK0) {
        case 5:
                DEK0 = 0;
                break;
        case 7:
                DEK0 = 1;
                break;
        case 3:
                DEK0 = 2;
                break;
        case 11:
                DEK0 = 3;
                break;
        case 9:
                DEK0 = 4;
                break;
        case 8:
                DEK0 = 5;
                break;
        case 10:
                DEK0 = 6;
                break;
        case 2:
                DEK0 = 7;
                break;
        case 6:
                DEK0 = 8;
                break;
        case 4:
                DEK0 = 9;
                break;
        default:
                DEK0 = 0;
    }

    switch(DEK1) {
        case 5:
                DEK1 = 0;
                break;
        case 7:
                DEK1 = 1;
                break;
        case 3:
                DEK1 = 2;
                break;
        case 11:
                DEK1 = 3;
                break;
        case 9:
                DEK1 = 4;
                break;
        case 8:
                DEK1 = 5;
                break;
        case 10:
                DEK1 = 6;
                break;
        case 2:
                DEK1 = 7;
                break;
        case 6:
                DEK1 = 8;
                break;
        case 4:
                DEK1 = 9;
                break;
        default:
                DEK1 = 0;
    }

    switch(DEK2) {
        case 5:
                DEK2 = 0;
                break;
        case 7:
                DEK2 = 1;
                break;
        case 3:
                DEK2 = 2;
                break;
        case 11:
                DEK2 = 3;
                break;
        case 9:
                DEK2 = 4;
                break;
        case 8:
                DEK2 = 5;
                break;
        case 10:
                DEK2 = 6;
                break;
        case 2:
                DEK2 = 7;
                break;
        case 6:
                DEK2 = 8;
                break;
        case 4:
                DEK2 = 9;
                break;
        default:
                DEK2 = 0;
    }

    switch(DEK3) {
        case 5:
                DEK3 = 0;
                break;
        case 7:
                DEK3 = 1;
                break;
        case 3:
                DEK3 = 2;
                break;
        case 11:
                DEK3 = 3;
                break;
        case 9:
                DEK3 = 4;
                break;
        case 8:
                DEK3 = 5;
                break;
        case 10:
                DEK3 = 6;
                break;
        case 2:
                DEK3 = 7;
                break;
        case 6:
                DEK3 = 8;
                break;
        case 4:
                DEK3 = 9;
                break;
        default:
                DEK3 = 0;
    }

    switch(DEK4) {
        case 5:
                DEK4 = 0;
                break;
        case 7:
                DEK4 = 1;
                break;
        case 3:
                DEK4 = 2;
                break;
        case 11:
                DEK4 = 3;
                break;
        case 9:
                DEK4 = 4;
                break;
        case 8:
                DEK4 = 5;
                break;
        case 10:
                DEK4 = 6;
                break;
        case 2:
                DEK4 = 7;
                break;
        case 6:
                DEK4 = 8;
                break;
        case 4:
                DEK4 = 9;
                break;
        default:
                DEK4 = 0;
    }

    // Доколку повисоката декада има вредност непарен број, тогаш пониската има вредност 9-DEKx
    if((DEK4 % 2) != 0)
        DEK3 = 9-DEK3;

    if((DEK3 % 2) != 0)
        DEK2 = 9-DEK2;

    if((DEK2 % 2) != 0)
        DEK1 = 9-DEK1;

    if((DEK1 % 2) != 0)
        DEK0 = 9-DEK0;


    angularPosition += DEK4;
    angularPosition *= 10;
    angularPosition += DEK3;
    angularPosition *= 10;
    angularPosition += DEK2;
    angularPosition *= 10;
    angularPosition += DEK1;
    angularPosition *= 10;
    angularPosition += DEK0;



    // Принтање
    //printf("angularPosition = %lu\n", angularPosition);
}

ISR(PCINT1_vect, ISR_ALIASOF(PCINT0_vect));
ISR(PCINT2_vect, ISR_ALIASOF(PCINT0_vect));
