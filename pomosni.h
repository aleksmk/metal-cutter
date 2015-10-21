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

// TODO // Функции за влезно/излезните плочки
void initOutputs(void);
void initInputs(void);
unsigned int getInputs(void);
void setOutput(unsigned char, unsigned char); // Output, State{ON, OFF}
unsigned char getOutput(void); // Прочитај ја портата. Треба ова воопшто?

// Функции за AD конверторот за читење на струјата на моторот и тахометарот
void initADC(void);
unsigned int getrawADC(unsigned char);
float getMotorCurrent(void);
float getTachoSpeed(void);

// Функции за глобална безбедност
unsigned char globalOK(unsigned int);

// Функција за подесување на влезните пинови за енкодерот и соодветните интерапти
void initEncoder(void);

// Функции за мотор
void initPWM(void);

#endif
