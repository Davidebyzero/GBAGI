#ifndef _LSL1HACK_H
#define _LSL1HACK_H

#include "types.h"

/*
 * Leisure Suit Larry 1 compatibility workaround.
 * This only targets the Quikimart phone wine-order prompts that are known
 * to be unreliable on GBA virtual-keyboard input.
 */
BOOL IsLSL1AmbientRoom(U8 roomNum);
BOOL LSL1CheckWineOrderAutofill(char *prompt, char *dest, int maxLen);
int LSL1AdjustGetStringMaxLen(char *prompt, int maxLen);
void LSL1NormalizePhoneNumberInput(char *prompt, char *dest);
void LSL1NormalizePhoneNumberDigits(char *dest);
void LSL1TrackPhoneEasterEggInput(char *dest);
char *LSL1OverridePhoneMessage(char *msg);
char *LSL1ConsumePhoneEasterEggMessage(void);
void LSL1MaybePlayKenSentMeClip(U8 logic_num, U8 message_num);
BOOL LSL1IsKenSentMeClipActive(void);
BOOL LSL1ShouldPreserveKenSentMeClip(void);
void LSL1WaitForKenSentMeClip(void);
void SQ2MaybePlayVohaulIntroClip(U8 logic_num, U8 message_num);
void SQ2MaybePlayVohaulIntroClipForMessage(char *msg);
void SQ2MaybeStopVohaulIntroClipForMessage(char *msg);
BOOL SQ2IsVohaulIntroClipActive(void);
BOOL SQ2ShouldPreserveVohaulIntroClip(void);
void SQ2WaitForVohaulIntroClip(void);

#endif
