// Преправка на машина за сечење на лимови.
// Функции:
// * Читање на тастатура
// * Запишување на LCD
// * Движење на мотор
// * ..

#ifndef POMOSNI_H
#define POMOSNI_H

#include <inttypes.h>

// Функции за тастатурата
void initKeypadPins(void); // Иницијализација на пиновите за тастатурата
unsigned char scanKeypad(void);

// Функции за дисплејот
void lcd_printStatus(char *);
void lcd_clearChars(unsigned char);

// Функции за влезно/излезните плочки - ТЕСТИРАНО
void initOutputs(void); //
void initInputs(void); //
unsigned int getInputs(void); //
void setOutput(unsigned char, unsigned char); // Output, State{ON, OFF}
unsigned char getOutput(void); // Прочитај ја портата. Треба ова воопшто?

// Функции за AD конверторот за читење на струјата на моторот и тахометарот
void initADC(void);
unsigned int getrawADC(unsigned char);
float getMotorCurrent(void); // TODO
float getTachoSpeed(void); // TODO

// Функции за глобална безбедност - дали оваа функција воопшто ни е потребна ?
unsigned char globalOK(unsigned int); // TODO
void ITNOWORKY(unsigned char); // IT NO WORKY ! The argument is the error code. This is a blocking function !

// Функција за подесување на влезните пинови за енкодерот и соодветните интерапти
void initEncoder(void);

// Функции за мотор
void initPWM(void);
unsigned char gotoPosition(unsigned int); // TODO - треба да оди на зададената позиција во аргумент. Треба да се имплементира профил на забрзување! :S

// Функција за стегање на стегите
void stegniStegi(unsigned int);
void otpustiStegi(void);

// Ардуино-like millis()
void initMillis();
unsigned long millis();



#endif
