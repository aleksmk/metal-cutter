// Global definitions

#ifndef DEFS_H
#define DEFS_H

#define F_CPU 16000000UL

#define MINL 2850
#define MAXL 50000

#define BAUD 9600

unsigned long angularPosition;

// Global OUTPUTs definitions {0 .. 7}
#define LSAW            0   // Left saw
#define RSAW            1   // Right saw
#define HCLAMP          2   // Horizontal clamps
#define VCLAMP          3   // Vertical clamps
#define L45             4   // Left 45 degrees valve
#define L90             5   // Left 90 degrees valve
#define R45             6   // Right 45 degrees valve
#define R90             7   // Right 90 degrees valve

// Global INPUTs definitions {0 .. 15}
#define LBTN            0   // Left button for the saws
#define RBTN            1   // Right button for th saws
#define LMOTOR          2   // Input to chech if the left saw motor is running
#define RMOTOR          3   // -||- right motor
#define STARTBTN        4   // Start button
#define STOPBTN         5   // Stop button
#define LENDSTOP        6   // Left end switch (from the perspective of looking towards the machine from the front)
#define RENDSTOP        7   // Right end switch - the HOME position
#define ESTOP           8   // Emergency stop
#define LSAWEND         9   // Left saw end switch (in the most retracted position)
#define RSAWEND         10  // Right saw end switch
#define OVERPESSURE     11  // Over pressure sensor
#define UNDERPRESSURE   12  // Under pressure sensor

#endif
