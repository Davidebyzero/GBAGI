#ifndef _LSL1HACK_H
#define _LSL1HACK_H

#include "types.h"

/*
 * Leisure Suit Larry 1 compatibility workaround.
 * This only targets the Quikimart phone wine-order prompts that are known
 * to be unreliable on GBA virtual-keyboard input.
 */
BOOL LSL1CheckWineOrderAutofill(char *prompt, char *dest, int maxLen);
int LSL1AdjustGetStringMaxLen(char *prompt, int maxLen);
void LSL1NormalizePhoneNumberInput(char *prompt, char *dest);
void LSL1NormalizePhoneNumberDigits(char *dest);
char *LSL1OverridePhoneMessage(char *msg);

#endif
