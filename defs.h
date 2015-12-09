// Global definitions

#ifndef DEFS_H
#define DEFS_H

#define F_CPU 16000000UL

#define MINL 2850
#define MAXL 50000

#define BAUD 9600

unsigned long angularPosition;
unsigned int inputs;

// Global OUTPUTs definitions {0 .. 7}
#define LSAW            1   // Left saw
#define RSAW            2   // Right saw
#define HCLAMP          0   // Horizontal clamps
#define VCLAMP          7   // Vertical clamps
#define L45             5   // Left 45 degrees valve
#define L90             3   // Left 90 degrees valve
#define R45             6   // Right 45 degrees valve
#define R90             4   // Right 90 degrees valve

// Global INPUTs definitions {0 .. 15}
#define LBTN            6   // Left button for the saws TWO HAND CONTROL TODO
//#define LBTN            12   // Right button for th saws TWO HAND CONTROL
#define RBTN            7   // Right button for th saws TWO HAND CONTROL


#define LMOTOR          3   // Input to check if the left saw motor is running
#define RMOTOR          4   // -||- right motor

#define STARTBTN        10  // Start button
#define STOPBTN         11  // Stop button

#define LENDSTOP        9   // Left end switch (from the perspective of looking towards the machine from the front)
#define RENDSTOP        8   // Right end switch - the HOME position

#define ESTOP           5   // Emergency stop

#define LSAWEND         1   // Left saw end switch (in the most retracted position)
#define RSAWEND         2   // Right saw end switch

#define OVERPESSURE     0   // Over pressure sensor ??

//#define UNDERPRESSURE   12  // Under pressure sensor TODO

#endif
