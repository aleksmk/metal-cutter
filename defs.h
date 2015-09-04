// Global definitions

#ifndef DEFS_H
#define DEFS_H

#define F_CPU 16000000UL

#define MINL 2850
#define MAXL 50000

#define BAUD 9600

unsigned long angularPosition;

// Global OUTPUTs definitions {0 .. 7}
#define LSAW    0   // Left saw
#define RSAW    1   // Right saw
#define HCLAMP  2   // Horizontal clamps
#define VCLAMP  3   // Vertical clamps
#define L45     4   // Left 45 degrees valve
#define L90     5   // Left 90 degrees valve
#define R45     6   // Right 45 degrees valve
#define R90     7   // Right 90 degrees valve

// Global INPUTs definitions {0 .. 15}

#endif
