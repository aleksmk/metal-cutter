#include "pomosni.h"
#include "hd44780.h"
#include "defs.h"

#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>

static unsigned int oldKeyboardState = 0x00;

void initKeypadPins(void) {
    // Употребени се 3x5 тастери - 3 редици со 5 колони.
    // Колоните се влезови, редиците се излези и се скенира низ нив.
    //

    // Колони
    DDRL &= ~((1 << PL0) | (1 << PL1) | (1 << PL2) | (1 << PL3) | (1 << PL4)); // Прва, втора ..

    // Редици
    DDRL |= (1 << PL5) | (1 << PL6) | (1 << PL7); // Прва, втора, трета

}


unsigned char scanKeypad(void) {
    // Притискање на тастер се регистрира на кликање. Мора да има повторна промена - пуштање тастер - притискање за да се регистрира ново кликнато копче.
    unsigned int newKeyboardState = 0;
    unsigned char tmp = 0;

    // Што има на првата колона:
    PORTL |= (1 << PL5);
    _delay_us(100);
    tmp = (PINL & 0x1F);
    newKeyboardState = tmp;
    PORTL &= ~(1 << PL5);
//    printf("Prva redica : %X\n", tmp);
//    printf("newKeyboardState = %X\n", newKeyboardState);

    // Што има на втората колона:
    PORTL |= (1 << PL6);
    _delay_us(100);
    tmp = (PINL & 0x1F);
    newKeyboardState += (tmp << 5);
    PORTL &= ~(1 << PL6);
//    printf("Vtora redica : %X\n", tmp);
//    printf("newKeyboardState = %X\n", newKeyboardState);

    // Што има на третата колона:
    PORTL |= (1 << PL7);
    _delay_us(100);
    tmp = (PINL & 0x1F);
    newKeyboardState += (tmp << 10);
    PORTL &= ~(1 << PL7);
//    printf("Treta redica : %X\n", tmp);

//    printf("newKeyboardState = %X\n", newKeyboardState);

    // Прво провери дали само еден бит е подесен :
    tmp = newKeyboardState && !(newKeyboardState & (newKeyboardState - 1)); // Враќа 0 доколку само едно копче е кликнато.
    if(0 == tmp) { // Има повеќе од едно копче кликнато. Врати 0xFF
        oldKeyboardState = 0;
        return 0xFF;
    }

    // Доколку новата состојба е иста со старата, значи се држи некое копче. Нема промена
    if(newKeyboardState == oldKeyboardState)
        return 0xFF;

    // Доколку сегашната состојба е 0x00, стави ја во oldKeyboardState - се отпуштило копче.
    if(0 == newKeyboardState) {
        oldKeyboardState = 0;
        return 0xFF;
    }

    // Доколку сѐ е во ред, може да видиме кое копче е кликнато.
    printf("newKeyboardState = %d\n", newKeyboardState);
    switch(newKeyboardState) {
        case 0x4000 :
                    tmp = 0x0C;
                    break;
        case 0x2000 :
                    tmp = 0x0A;
                    break;
        case 0x1000 :
                    tmp = 0x03;
                    break;
        case 0x800 :
                    tmp = 0x06;
                    break;
        case 0x400 :
                    tmp = 0x09;
                    break;
        case 0x200 :
                    tmp = 0x0D;
                    break;
        case 0x100 :
                    tmp = 0x00;
                    break;
        case 0x80 :
                    tmp = 0x02;
                    break;
        case 0x40 :
                    tmp = 0x05;
                    break;
        case 0x20 :
                    tmp = 0x08;
                    break;
        case 0x10 :
                    tmp = 0x0B;
                    break;
        case 0x8 :
                    tmp = 0x0E;
                    break;
        case 0x4 :
                    tmp = 0x01;
                    break;
        case 0x2 :
                    tmp = 0x04;
                    break;
        case 0x1 :
                    tmp = 0x07;
                    break;
        default :
                    tmp = 0xFF;
                    break;
    }
    oldKeyboardState = newKeyboardState;
    printf("tmp = %d\n", tmp);
    return tmp;
}

void lcd_printStatus(char *statusMsg) {
    // Терминирај ја statusMsg на 20-тиот карактер со NULL за да нема overflow
    //statusMsg[10] = 0;
    lcd_goto(0x54);
    lcd_clearChars(20);
    lcd_goto(0x54);
    lcd_puts(statusMsg);
}

void lcd_clearChars(unsigned char n) {
    for(uint8_t i=0; i<n; i++) {
        lcd_putc(0x11);
    }
}


// Функции за влезовите и излезовите

void initOutputs(void) {
    // Подеси го DDRF = 0b11111100 и DDRG = 0b11
    DDRF = 0xFC;
    PORTF = 0x00;
    
    DDRG = 0x03;
    PORTF &= ~(0x03);
}

void initInputs(void) {
    // Влезови се портите C и D.

    DDRC = 0x00;
    DDRD = 0x00;
}

unsigned int getInputs(void) {
    // Прочитај ги двата порти. Врати го резултатот како 16битна вредност.
    // Формат : (MSB) PORTC{7..0} PORTD{7..0} (LSB)

    unsigned int tmp;
    tmp = (PINC << 8);
    tmp |= ((PIND) & 0x00FF);

    return tmp;
}

void setOutput(unsigned char pin, unsigned char state) { //
    if(0 == state) {
        if(pin >= 2) {
            PORTF &= ~(1 << pin);
        } else {
            PORTG &= ~(1 << pin);
        }
    } else {
        if(pin >= 2) {
            PORTF |= (1 << pin);
        } else {
            PORTG |= (1 << pin);
        }
    }
}

unsigned char getOutput(void) { //
    return (PORTF & 0xFC) + (PORTG & 0x03);
}

unsigned char globalOK(unsigned int inputStates) {  // Return 0 if there is a problem

    /*
    unsigned int MASK = (1 << UNDERPRESSURE) | (1 << OVERPESSURE);
    unsigned int tmp = inputStates & MASK;
    if(tmp == MASK)
        return 0;

    MASK = (1 << LENDSTOP) | (1 << RENDSTOP);
    tmp = inputStates & MASK;
    if(tmp == MASK)
        return 0;


    return 1;
    */

}

void initEncoder(void) {
    DDRB = 0x00;
    DDRK = 0x00;
    DDRJ &= ~((1 << PJ0) | (1 << PJ1) | (1 << PJ2) | (1 << PJ3));

    // Интерапти :
    PCMSK0 = 0xFF;
    PCMSK1 = (1 << PCINT9) | (1 << PCINT10) | (1 << PCINT11) | (1 << PCINT12);
    PCMSK2 = 0xFF;

    // Interrupt banks :
    PCICR |= (1 << PCIE0) | (1 << PCIE1) | (1 << PCIE2);
}

// ADC functions

void initADC(void) {
    // Set AVcc reference for the ADC
    ADMUX |= (1 << REFS0);

    // Enable the ADC
    ADCSRA |= (1<<ADEN);

    // Set the ADC prescaler to 1:128
    ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);

    // Disable the digital buffers for ADC0 and ADC1 pins
    DIDR0 |= (1 << ADC0D) | (1 << ADC1D);
}

// Initiate and get a RAW ADC value from the A/D
unsigned int getrawADC(unsigned char channel) {
    unsigned int readOut = 0;
    unsigned long value = 0;

    // First, select the channel
    if(channel == 0) {
        ADMUX &= ~(1 << 0);
    } else if (channel == 1) {
        ADMUX |= (1 << MUX0);
    }

    // Make 10 conversions
    for(unsigned char i=0; i<10; i++) {
        // Start the ADC conversion
        ADCSRA |= (1<<ADSC);
        while(ADCSRA & (1<<ADSC)){};
        readOut = ADC;
        value += readOut;
    }

    // Return the average value
    return round(value/10);
}



// PWM & Motor functions

void initPWM(void) {
    // Тајмер број : 3

    // Што сакам:
    // 1. fOUT = ~1000Hz
    // 2. Phase correct PWM
    // 3. Ваљда сакам WGM3{3..0} == 1000, TOP е во ICR3, OCR3x се апдејтува на BOTTOM
    // 4. Сакам да ми работи во неинвертирачки режим на споредба - COM3x1:0 = 2
    // 5. DDRE треба соодветно да се подеси за излезните пинови
    // 6. Пресметка : N = 8; ICR3 = 1000;

    // Подесување на пиновите за излезни
    DDRE |= (1 << PE3) | (1 << PE4) | (1 << PE5) | (1 << PE6);
    PORTE &= ~((1 << PE3) | (1 << PE4) | (1 << PE5) | (1 << PE6));

    // Подесување на режимот за канал A и B
    TCCR3A |= (1 << COM3A1) | (1 << COM3B1);

    // Подесување на модот на работа WGM33 set
    TCCR3B |= (1 << WGM33);

    // TOP вредноста = 1000 во ICR3
    ICR3 = 1000UL;

    // Подеси ги излезните компараторски вредности на 0 - нема излез.
    OCR3A = 0;
    OCR3B = 0;

    // Прескалер - N = 8 и почни со броење !
    TCCR3B |= (1 << CS31);
}

void stegniStegi(unsigned int kodPili) {
    if(kodPili <= 4) { // Стегни само хоризонтално
        setOutput(HCLAMP, 1);
    } else if((kodPili >=5) && (kodPili <= 8)) { // Стегни само вертикално
        setOutput(VCLAMP, 1);
    } else if((kodPili >=9) && (kodPili <= 12)) { // Стегни и вертикално и хоризонтално
        setOutput(HCLAMP, 1);
        setOutput(VCLAMP, 1);
    }
}

void otpustiStegi(void) {
    setOutput(HCLAMP, 0);
    setOutput(VCLAMP, 0);
}
