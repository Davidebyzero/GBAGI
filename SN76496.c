#include "gbagi.h"
#include "SN76496.h"

#define PFEED_SMS 0x8000U
#define WFEED_SMS 0x9000U
#define PFEED_SN  0x4000U
#define WFEED_SN  0x6000U
#define PFEED_NCR 0x4000U
#define WFEED_NCR 0x4400U
#define SN76496_MASTER_STEP 3579545U
#define SN76496_SAMPLE_RATE TANDY_MIX_RATE
#define SN76496_PERIOD_SCALE (32U * SN76496_SAMPLE_RATE)

static const U16 attenuation_table[16] = {
	0xFFFF, 0xCB30, 0xA145, 0x8000,
	0x6598, 0x50A3, 0x4000, 0x32CC,
	0x2851, 0x2000, 0x1966, 0x1428,
	0x1000, 0x0CB3, 0x0A14, 0x0000
};

static U16 sn76496_mix_level(U8 attenuation)
{
	return (U16)(attenuation_table[attenuation & 0x0F] >> 5);
}

static void sn76496_refresh_volumes(SN76496 *chip)
{
	int i;

	chip->ch0Volume = sn76496_mix_level(chip->ch0Att);
	chip->ch1Volume = sn76496_mix_level(chip->ch1Att);
	chip->ch2Volume = sn76496_mix_level(chip->ch2Att);
	chip->ch3Volume = sn76496_mix_level(chip->ch3Att);
	chip->activeMask = 0;
	if (chip->ch0Volume != 0) {
		chip->activeMask |= 0x01U;
	}
	if (chip->ch1Volume != 0) {
		chip->activeMask |= 0x02U;
	}
	if (chip->ch2Volume != 0) {
		chip->activeMask |= 0x04U;
	}
	if (chip->ch3Volume != 0) {
		chip->activeMask |= 0x08U;
	}
	for (i = 0; i < 16; i++) {
		S32 mix;

		mix = 0;
		mix += (i & 0x01) ? chip->ch0Volume : -(S32)chip->ch0Volume;
		mix += (i & 0x02) ? chip->ch1Volume : -(S32)chip->ch1Volume;
		mix += (i & 0x04) ? chip->ch2Volume : -(S32)chip->ch2Volume;
		mix += (i & 0x08) ? chip->ch3Volume : -(S32)chip->ch3Volume;
		mix >>= 8;
		if (mix < -128) {
			mix = -128;
		} else if (mix > 127) {
			mix = 127;
		}
		chip->mixTable[i] = (U8)(S8)mix;
	}
	chip->attChg = 0;
}

static void sn76496_update_noise_freq(SN76496 *chip, U8 reg)
{
	U16 feedback;
	U16 tone2_freq;
	U16 noise_freq;

	chip->ch3Reg = reg & 0x03;
	chip->rng = chip->noiseType & 0xFFFF;
	feedback = (reg & 0x04) ? (U16)(chip->noiseType >> 16) : (U16)(chip->noiseType & 0xFFFF);
	chip->noiseFB = feedback;
	if (chip->ch3Reg == 3) {
		tone2_freq = chip->ch2Frq;
		noise_freq = tone2_freq ? tone2_freq : 0x0010;
	} else {
		noise_freq = (U16)(0x0010U << chip->ch3Reg);
	}
	chip->ch3Frq = noise_freq;
}

static void sn76496_step_tone(U16 freq, U32 *phase, U32 *current_bits, U32 mask)
{
	U32 threshold;

	if (freq == 0 || freq == 0xFFFFU) {
		return;
	}

	threshold = (U32)freq * 16U * SN76496_SAMPLE_RATE;
	*phase += SN76496_MASTER_STEP;
	while (*phase >= threshold) {
		*phase -= threshold;
		*current_bits ^= mask;
	}
}

static void sn76496_step_noise(SN76496 *chip)
{
	U32 threshold;

	if (chip->ch3Frq == 0 || chip->ch3Frq == 0xFFFFU) {
		return;
	}

	threshold = (U32)chip->ch3Frq * 16U * SN76496_SAMPLE_RATE;
	chip->ch3Phase += SN76496_MASTER_STEP;
	while (chip->ch3Phase >= threshold) {
		chip->ch3Phase -= threshold;
		chip->currentBits &= ~0x10U;
		if (chip->rng & 1U) {
			chip->rng >>= 1;
			chip->rng ^= chip->noiseFB;
			chip->currentBits |= 0x10U;
		} else {
			chip->rng >>= 1;
		}
	}
}

static S16 sn76496_mix_sample(const SN76496 *chip)
{
	S32 mix;

	mix = 0;
	mix += (chip->currentBits & 0x02U) ? chip->ch0Volume : -(S32)chip->ch0Volume;
	mix += (chip->currentBits & 0x04U) ? chip->ch1Volume : -(S32)chip->ch1Volume;
	mix += (chip->currentBits & 0x08U) ? chip->ch2Volume : -(S32)chip->ch2Volume;
	mix += (chip->currentBits & 0x10U) ? chip->ch3Volume : -(S32)chip->ch3Volume;

	if (mix < -32768) {
		mix = -32768;
	} else if (mix > 32767) {
		mix = 32767;
	}
	return (S16)mix;
}

static U8 sn76496_mix_u8(U32 current_bits, const U8 *mix_table)
{
	return mix_table[(current_bits >> 1) & 0x0FU];
}

void sn76496Reset(int chiptype, SN76496 *chip)
{
	U32 noise_type;

	memset(chip, 0, sizeof(*chip));

	noise_type = ((U32)WFEED_SMS << 16) | PFEED_SMS;
	if (chiptype == 1) {
		noise_type = ((U32)WFEED_SN << 16) | PFEED_SN;
	} else if (chiptype > 1) {
		noise_type = ((U32)WFEED_NCR << 16) | PFEED_NCR;
	}

	chip->noiseType = noise_type;
	chip->rng = noise_type & 0xFFFF;
	chip->noiseFB = noise_type >> 16;
	chip->ch0Frq = 1;
	chip->ch1Frq = 1;
	chip->ch2Frq = 1;
	chip->ch3Frq = 0x0010;
	chip->ch0Att = 0x0F;
	chip->ch1Att = 0x0F;
	chip->ch2Att = 0x0F;
	chip->ch3Att = 0x0F;
	chip->currentBits = 0x1EU;
	chip->ch0Phase = 0;
	chip->ch1Phase = 0;
	chip->ch2Phase = 0;
	chip->ch3Phase = 0;
	sn76496_refresh_volumes(chip);
}

int sn76496SaveState(void *destination, const SN76496 *chip)
{
	memcpy(destination, chip, sizeof(*chip));
	return (int)sizeof(*chip);
}

int sn76496LoadState(SN76496 *chip, const void *source)
{
	memcpy(chip, source, sizeof(*chip));
	chip->attChg = 1;
	return (int)sizeof(*chip);
}

int sn76496GetStateSize(void)
{
	return (int)sizeof(SN76496);
}

void sn76496Mixer(int count, S16 *dest, SN76496 *chip)
{
	int i;

	if (chip->attChg) {
		sn76496_refresh_volumes(chip);
	}

	for (i = 0; i < count; i++) {
		sn76496_step_tone(chip->ch0Frq, &chip->ch0Phase, &chip->currentBits, 0x02U);
		sn76496_step_tone(chip->ch1Frq, &chip->ch1Phase, &chip->currentBits, 0x04U);
		sn76496_step_tone(chip->ch2Frq, &chip->ch2Phase, &chip->currentBits, 0x08U);
		sn76496_step_noise(chip);
		dest[i] = sn76496_mix_sample(chip);
	}
}

void sn76496MixerPacked(int count, U32 *dest, SN76496 *chip)
{
	U32 current_bits;
	U32 rng;
	U32 noise_fb;
	U32 phase0;
	U32 phase1;
	U32 phase2;
	U32 phase3;
	U32 threshold0;
	U32 threshold1;
	U32 threshold2;
	U32 threshold3;
	U16 freq0;
	U16 freq1;
	U16 freq2;
	U16 freq3;
	const U8 *mix_table;
	int words;
	int i;

	if (chip->attChg) {
		sn76496_refresh_volumes(chip);
	}

	if (chip->activeMask == 0) {
		memset(dest, 0, (size_t)(count >> 2) * sizeof(*dest));
		return;
	}

	freq0 = chip->ch0Frq;
	freq1 = chip->ch1Frq;
	freq2 = chip->ch2Frq;
	freq3 = chip->ch3Frq;
	current_bits = chip->currentBits;
	rng = chip->rng;
	noise_fb = chip->noiseFB;
	phase0 = chip->ch0Phase;
	phase1 = chip->ch1Phase;
	phase2 = chip->ch2Phase;
	phase3 = chip->ch3Phase;
	mix_table = chip->mixTable;

	threshold0 = (freq0 == 0 || freq0 == 0xFFFFU) ? 0 : (U32)freq0 * SN76496_PERIOD_SCALE;
	threshold1 = (freq1 == 0 || freq1 == 0xFFFFU) ? 0 : (U32)freq1 * SN76496_PERIOD_SCALE;
	threshold2 = (freq2 == 0 || freq2 == 0xFFFFU) ? 0 : (U32)freq2 * SN76496_PERIOD_SCALE;
	threshold3 = (freq3 == 0 || freq3 == 0xFFFFU) ? 0 : (U32)freq3 * SN76496_PERIOD_SCALE;
	if ((chip->activeMask & 0x01U) == 0) {
		threshold0 = 0;
	}
	if ((chip->activeMask & 0x02U) == 0) {
		threshold1 = 0;
	}
	if ((chip->activeMask & 0x04U) == 0) {
		threshold2 = 0;
	}
	if ((chip->activeMask & 0x08U) == 0) {
		threshold3 = 0;
	}

	/* The audio backend consumes 8-bit FIFO samples packed four at a time. */
	words = count >> 2;
	for (i = 0; i < words; i++) {
		U32 packed;
		int j;

		packed = 0;
		for (j = 0; j < 4; j++) {
			if (threshold0 != 0) {
				phase0 += SN76496_MASTER_STEP;
				while (phase0 >= threshold0) {
					phase0 -= threshold0;
					current_bits ^= 0x02U;
				}
			}
			if (threshold1 != 0) {
				phase1 += SN76496_MASTER_STEP;
				while (phase1 >= threshold1) {
					phase1 -= threshold1;
					current_bits ^= 0x04U;
				}
			}
			if (threshold2 != 0) {
				phase2 += SN76496_MASTER_STEP;
				while (phase2 >= threshold2) {
					phase2 -= threshold2;
					current_bits ^= 0x08U;
				}
			}
			if (threshold3 != 0) {
				phase3 += SN76496_MASTER_STEP;
				while (phase3 >= threshold3) {
					phase3 -= threshold3;
					current_bits &= ~0x10U;
					if (rng & 1U) {
						rng >>= 1;
						rng ^= noise_fb;
						current_bits |= 0x10U;
					} else {
						rng >>= 1;
					}
				}
			}
			packed |= (U32)sn76496_mix_u8(current_bits, mix_table) << (j * 8);
		}
		dest[i] = packed;
	}

	chip->currentBits = current_bits;
	chip->rng = rng;
	chip->ch0Phase = phase0;
	chip->ch1Phase = phase1;
	chip->ch2Phase = phase2;
	chip->ch3Phase = phase3;
}

void sn76496W(U8 val, SN76496 *chip)
{
	U8 incoming;
	U8 reg;
	U8 channel;

	incoming = val;
	if (val & 0x80U) {
		chip->lastReg = val;
	}

	reg = (U8)(chip->lastReg >> 4);
	channel = (U8)((reg >> 1) & 0x03);

	if (reg & 0x01U) {
		U8 attenuation;

		attenuation = chip->lastReg & 0x0F;
		switch (channel) {
		case 0:
			chip->ch0Att = attenuation;
			break;
		case 1:
			chip->ch1Att = attenuation;
			break;
		case 2:
			chip->ch2Att = attenuation;
			break;
		default:
			chip->ch3Att = attenuation;
			break;
		}
		chip->attChg = 1;
		return;
	}

	if (channel < 3) {
		U16 raw;
		U16 *freq_ptr;

		switch (channel) {
		case 0:
			freq_ptr = &chip->ch0Frq;
			break;
		case 1:
			freq_ptr = &chip->ch1Frq;
			break;
		default:
			freq_ptr = &chip->ch2Frq;
			break;
		}
		raw = *freq_ptr;
		if (incoming & 0x80U) {
			raw = (U16)((raw & 0x03F0U) | (chip->lastReg & 0x0FU));
		} else {
			raw = (U16)((raw & 0x000FU) | ((incoming & 0x3FU) << 4));
		}
		if (raw < 6U) {
			raw = 1U;
		}
		*freq_ptr = raw;
		if (channel == 2 && chip->ch3Reg == 3) {
			chip->ch3Frq = chip->ch2Frq;
		}
		return;
	}

	sn76496_update_noise_freq(chip, chip->lastReg & 0x07U);
}
