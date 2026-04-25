/*
 * FluBBa SN76496 core, adapted for GBAGI type names/build.
 */
#ifndef SN76496_HEADER
#define SN76496_HEADER

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TANDY_MIX_RATE_HIGH 15360U
#define TANDY_MIX_RATE_BALANCED 13379U
/* Default live-Tandy profile: slightly lower sample rate for less CPU load,
   while keeping the original mixer and chip behavior intact. */
#define TANDY_MIX_RATE TANDY_MIX_RATE_BALANCED

typedef struct {
	U16 ch0Frq;
	U16 ch0Cnt;
	U16 ch1Frq;
	U16 ch1Cnt;
	U16 ch2Frq;
	U16 ch2Cnt;
	U16 ch3Frq;
	U16 ch3Cnt;

	U32 rng;
	U32 currentBits;
	U32 noiseFB;

	U8 attChg;
	U8 ch3Reg;
	U8 activeMask;
	U8 mixTable[16];

	S16 calculatedVolumes[16];

	U16 ch0Volume;
	U8 padding0[1];
	U8 ch0Att;
	U16 ch1Volume;
	U8 padding1[1];
	U8 ch1Att;
	U16 ch2Volume;
	U8 padding2[1];
	U8 ch2Att;
	U16 ch3Volume;
	U8 padding3[1];
	U8 ch3Att;

	U32 lastReg;
	U32 noiseType;
	U32 ch0Phase;
	U32 ch1Phase;
	U32 ch2Phase;
	U32 ch3Phase;
} SN76496;

void sn76496Reset(int chiptype, SN76496 *chip);
int sn76496SaveState(void *destination, const SN76496 *chip);
int sn76496LoadState(SN76496 *chip, const void *source);
int sn76496GetStateSize(void);
void sn76496Mixer(int count, S16 *dest, SN76496 *chip);
void sn76496MixerPacked(int count, U32 *dest, SN76496 *chip);
void sn76496W(U8 val, SN76496 *chip);

#ifdef __cplusplus
}
#endif

#endif
