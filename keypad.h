/***********************\
* 	Keypad.h		*
*	by dovoto		*
*				*
\***********************/

#ifndef KEYPAD_H
#define KEYPAD_H



#define BTN_A 		1
#define BTN_B 		2
#define BTN_SELECT	3//4
#define BTN_START 	4//8
#define BTN_RIGHT 	5//16
#define BTN_LEFT 	6//32
#define BTN_UP 		7//64
#define BTN_DOWN 	8//128
#define BTN_R		9//256
#define BTN_L 		10//512

#define B_A 		1
#define B_B 		2
#define B_SELECT	4
#define B_START 	8
#define B_RIGHT 	16
#define B_LEFT 		32
#define B_UP 		64
#define B_DOWN 		128
#define B_R			256
#define B_L 		512

#define KEYS  ((volatile U32*)0x04000130)

#endif
