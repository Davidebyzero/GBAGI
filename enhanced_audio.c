#include "gbagi.h"
#include "agimain.h"
#include "enhanced_audio.h"
#include "pcm_music.h"
#include "SN76496.h"

#define TANDY_CHUNK_SAMPLES_COMPAT 384U
#define TANDY_CHUNK_SAMPLES_FAST 512U
/* Larger chunks reduce timer/DMA refill overhead for live Tandy at the cost
   of a little more latency. The sample rate and mixer behavior stay the same. */
#define TANDY_SAMPLES_PER_CHUNK TANDY_CHUNK_SAMPLES_FAST
#define TANDY_DMA_WORDS (TANDY_SAMPLES_PER_CHUNK / 4U)
#define TANDY_TIMER0_TICKS ((16777216U + (TANDY_MIX_RATE / 2U)) / TANDY_MIX_RATE)
#define TANDY_TIMER0_RELOAD (0x10000U - TANDY_TIMER0_TICKS)
#define TANDY_TIMER1_RELOAD (0x10000U - TANDY_SAMPLES_PER_CHUNK)
#define DMA_SOUND_FIFO_A 0xB640U

enum audio_mode g_audio_mode = AUDIO_MODE_DEFAULT;
enum audio_music_backend g_audio_music_backend = AUDIO_BACKEND_DEFAULT;

static SN76496 s_tandy_chip;
static U32 s_tandy_dma_buffers[2][TANDY_DMA_WORDS];
static U8 s_tandy_active_buffer;
static BOOL s_tandy_output_enabled;
static BOOL s_tandy_hardware_prepared;
static U16 s_tandy_last_tone_period[3];
static U8 s_tandy_last_volume[4];
static U8 s_tandy_last_noise_control;
static BOOL s_tandy_voice_silent[4];
static BOOL s_tandy_noise_valid;
static U16 s_audio_timer_tick_count;
static U16 s_audio_refill_count;
static const U32 s_semi_tandy_wave[4] = {
	0x56788765U,
	0x56788765U,
	0x56788765U,
	0x56788765U
};

static BOOL UsesTandyMixer(void)
{
	if (g_audio_mode != AUDIO_TANDY) {
		return FALSE;
	}

	if (g_audio_music_backend == AUDIO_BACKEND_TANDY_LIVE) {
		return TRUE;
	}

	return (BOOL)((g_audio_music_backend == AUDIO_BACKEND_TANDY_HYBRID) && !IsPCMMusicPlaying());
}

static BOOL UsesHybridMusicBackend(void)
{
	return (BOOL)(g_audio_music_backend == AUDIO_BACKEND_TANDY_HYBRID);
}

static void clear_tandy_dma_buffers(void)
{
	memset(s_tandy_dma_buffers, 0, sizeof(s_tandy_dma_buffers));
}

static void reset_tandy_voice_state(void)
{
	int i;

	for (i = 0; i < 3; i++) {
		s_tandy_last_tone_period[i] = 0xFFFF;
	}
	for (i = 0; i < 4; i++) {
		s_tandy_last_volume[i] = 0xFF;
		s_tandy_voice_silent[i] = TRUE;
	}
	s_tandy_last_noise_control = 0xFF;
	s_tandy_noise_valid = FALSE;
}

static void stop_psg_voice(U8 voice)
{
	switch (voice) {
	case 0:
		REG_SOUND1CNT_L = 0;
		REG_SOUND1CNT_H = 0;
		REG_SOUND1CNT_X = SOUND1INIT + 0;
		REG_SOUND1CNT_X = 0;
		break;
	case 1:
		REG_SOUND2CNT_L = 0;
		REG_SOUND2CNT_H = SOUND2INIT + 0;
		REG_SOUND2CNT_H = 0;
		break;
	case 2:
		REG_SOUND3CNT_L = 0;
		REG_SOUND3CNT_H = SOUND3INIT + 0;
		REG_SOUND3CNT_H = 0;
		REG_SOUND3CNT_X = 0;
		break;
	default:
		REG_SOUND4CNT_L = 0;
		REG_SOUND4CNT_H = SOUND4INIT + 0;
		REG_SOUND4CNT_H = 0;
		break;
	}
}

static void stop_all_psg_voices(void)
{
	stop_psg_voice(0);
	stop_psg_voice(1);
	stop_psg_voice(2);
	stop_psg_voice(3);
}

static void fill_tandy_dma_buffer(U8 index)
{
	/* Render straight into packed FIFO words to keep refill overhead down. */
	sn76496MixerPacked(TANDY_SAMPLES_PER_CHUNK, s_tandy_dma_buffers[index], &s_tandy_chip);
}

static void start_tandy_dma(U8 index)
{
	REG_DMA1CNT_H = 0;
	REG_DMA1SAD = (U32)s_tandy_dma_buffers[index];
	REG_DMA1DAD = (U32)&REG_SGFIFOA;
	REG_DMA1CNT_L = 0;
	REG_DMA1CNT_H = DMA_SOUND_FIFO_A;
	s_tandy_active_buffer = index;
}

static void prepare_tandy_output(void)
{
	if (s_tandy_hardware_prepared) {
		return;
	}

	clear_tandy_dma_buffers();
	REG_SOUNDCNT_X = 0x0080;
	REG_SOUNDCNT_L = 0xFF77;
	REG_SOUNDCNT_H = 0x0B02;
	REG_TM0CNT_H = 0;
	REG_TM1CNT_H = 0;
	REG_DMA1CNT_H = 0;
	s_tandy_hardware_prepared = TRUE;
}

static void start_tandy_output(void)
{
	if (s_tandy_output_enabled) {
		return;
	}

	prepare_tandy_output();
	fill_tandy_dma_buffer(0);
	fill_tandy_dma_buffer(1);
	REG_IF |= INT_TIMER1;
	EnableInterupts(INT_TIMER1);
	start_tandy_dma(0);
	REG_TM0CNT_L = TANDY_TIMER0_RELOAD;
	REG_TM0CNT_H = TIME_FREQUENcySYSTEM | TIME_ENABLE;
	REG_TM1CNT_L = TANDY_TIMER1_RELOAD;
	REG_TM1CNT_H = TIME_OVERFLOW | TIME_ENABLE | TIME_IRQ_ENABLE;
	s_tandy_output_enabled = TRUE;
}

static void stop_tandy_output(void)
{
	s_tandy_output_enabled = FALSE;
	DissableInterupts(INT_TIMER1);
	REG_DMA1CNT_H = 0;
	REG_TM0CNT_H = 0;
	REG_TM1CNT_H = 0;
	REG_IF |= INT_TIMER1;
	REG_SGFIFOA = 0;
	REG_SGFIFOA = 0;
	REG_SOUNDCNT_L = 0xFF77;
	REG_SOUNDCNT_H = 0x0002;
	clear_tandy_dma_buffers();
}

static void write_tandy_tone(U8 voice, U16 period)
{
	if (voice >= 3) {
		return;
	}
	if (s_tandy_last_tone_period[voice] == period) {
		return;
	}
	sn76496W((U8)(0x80 | (voice << 5) | (period & 0x0F)), &s_tandy_chip);
	sn76496W((U8)((period >> 4) & 0x3F), &s_tandy_chip);
	s_tandy_last_tone_period[voice] = period;
}

static void write_tandy_volume(U8 voice, U8 attenuation)
{
	if (s_tandy_last_volume[voice] == attenuation) {
		return;
	}
	sn76496W((U8)(0x90 | (voice << 5) | (attenuation & 0x0F)), &s_tandy_chip);
	s_tandy_last_volume[voice] = attenuation;
}

static void write_tandy_noise(U8 noise_control)
{
	if (s_tandy_noise_valid && s_tandy_last_noise_control == noise_control) {
		return;
	}
	sn76496W((U8)(0xE0 | (noise_control & 0x07)), &s_tandy_chip);
	s_tandy_last_noise_control = noise_control;
	s_tandy_noise_valid = TRUE;
}

static BOOL tandy_voice_is_audible(U8 voice)
{
	return (BOOL)((voice < 4) && !s_tandy_voice_silent[voice] && (s_tandy_last_volume[voice] < 0x0FU));
}

static void silence_tandy_voice(U8 voice)
{
	if (s_tandy_voice_silent[voice]) {
		return;
	}
	write_tandy_volume(voice, 0x0F);
	s_tandy_voice_silent[voice] = TRUE;
}

static void play_sound_original(const note_event *note)
{
	int i;

	if (note->is_end || note->is_silent || note->volume == 0) {
		stop_psg_voice(note->voice);
		return;
	}

	switch (note->voice) {
	case 0:
		REG_SOUND1CNT_L = 0;
		REG_SOUND1CNT_H = (note->volume << 12) + (1 << 11) + (7 << 8) + (2 << 6);
		REG_SOUND1CNT_X = SOUND1INIT + note->gba_frequency;
		break;
	case 1:
		REG_SOUND2CNT_L = (note->volume << 12) + (1 << 11) + (7 << 8) + (2 << 6);
		REG_SOUND2CNT_H = SOUND2INIT + note->gba_frequency;
		break;
	case 2:
		REG_SOUND3CNT_L = SOUND3SETBANK1 + SOUND3BANK32;
		for (i = 0; i < 4; i++) {
			(&REG_WAVE_RAM0)[i] = (i & 1) ? 0 : 0xFFFFFFFF;
		}
		REG_SOUND3CNT_L = SOUND3PLAY + SOUND3SETBANK0 + SOUND3BANK32;
		REG_SOUND3CNT_H = (note->volume << 13) + 0;
		REG_SOUND3CNT_X = SOUND3INIT + SOUND3PLAYLOOP + note->gba_frequency;
		break;
	default:
		REG_SOUND4CNT_L = (note->volume << 12) + (1 << 11) + (7 << 8);
		REG_SOUND4CNT_H = SOUND4INIT + (4 << 4) + note->noise_period;
		break;
	}
}

static U8 semi_tandy_volume(U8 voice, U8 attenuation)
{
	static const U8 semi_voice0_table[16] = {
		12, 11, 10, 9, 8, 7, 6, 5,
		4, 3, 3, 2, 2, 1, 1, 0
	};
	static const U8 semi_voice1_table[16] = {
		11, 10, 9, 8, 7, 6, 6, 5,
		4, 3, 2, 2, 1, 1, 1, 0
	};
	static const U8 semi_voice2_table[16] = {
		7, 7, 6, 6, 5, 5, 4, 4,
		3, 3, 2, 2, 2, 1, 1, 0
	};
	static const U8 semi_noise_table[16] = {
		6, 6, 5, 5, 4, 4, 3, 3,
		2, 2, 2, 1, 1, 1, 0, 0
	};

	switch (voice) {
	case 0:
		return semi_voice0_table[attenuation & 0x0F];
	case 1:
		return semi_voice1_table[attenuation & 0x0F];
	case 2:
		return semi_voice2_table[attenuation & 0x0F];
	default:
		return semi_noise_table[attenuation & 0x0F];
	}
}

static void play_sound_semi_tandy(const note_event *note)
{
	int i;
	U8 volume;

	if (note->is_end) {
		stop_psg_voice(note->voice);
		return;
	}

	volume = semi_tandy_volume(note->voice, note->attenuation);
	if (note->is_silent || volume == 0) {
		stop_psg_voice(note->voice);
		return;
	}

	switch (note->voice) {
	case 0:
		REG_SOUND1CNT_L = 0;
		REG_SOUND1CNT_H = SOUND1ENVINIT(volume) + SOUND1ENVDEC + SOUND1ENVSTEPS(0) + SOUNDDUTY50;
		REG_SOUND1CNT_X = SOUND1INIT + note->gba_frequency;
		break;
	case 1:
		REG_SOUND2CNT_L = SOUND2ENVINIT(volume) + SOUND2ENVDEC + SOUND2ENVSTEPS(0) + SOUNDDUTY50;
		REG_SOUND2CNT_H = SOUND2INIT + note->gba_frequency;
		break;
	case 2:
		REG_SOUND3CNT_L = SOUND3SETBANK1 + SOUND3BANK32;
		for (i = 0; i < 4; i++) {
			(&REG_WAVE_RAM0)[i] = s_semi_tandy_wave[i];
		}
		REG_SOUND3CNT_L = SOUND3PLAY + SOUND3SETBANK0 + SOUND3BANK32;
		REG_SOUND3CNT_H = SOUND3OUTPUT14;
		REG_SOUND3CNT_X = SOUND3INIT + SOUND3PLAYLOOP + note->gba_frequency;
		break;
	default:
		REG_SOUND4CNT_L = SOUND4ENVINIT(volume) + SOUND4ENVDEC + SOUND4ENVSTEPS(0);
		REG_SOUND4CNT_H = SOUND4INIT +
			((note->noise_control & 0x04) ? SOUND4STEPS7 : SOUND4STEPS15) +
			(4 << 4) +
			note->noise_period;
		break;
	}
}

static void play_sound_tandy(const note_event *note)
{
	BOOL tone_is_rest;
	BOOL retune_active_voice;

	tone_is_rest = (BOOL)(note->voice < 3 && note->raw_frequency == 0);
	if (note->is_end || note->is_silent || note->attenuation >= 0x0F || tone_is_rest) {
		silence_tandy_voice(note->voice);
		return;
	}

	retune_active_voice = FALSE;
	if (note->voice < 3) {
		retune_active_voice = (BOOL)(
			tandy_voice_is_audible(note->voice) &&
			(s_tandy_last_tone_period[note->voice] != note->raw_frequency)
		);
		if (retune_active_voice) {
			/* Briefly mute before retuning to reduce clicks from abrupt phase jumps. */
			write_tandy_volume(note->voice, 0x0F);
		}
		write_tandy_tone(note->voice, note->raw_frequency);
	} else {
		retune_active_voice = (BOOL)(
			tandy_voice_is_audible(note->voice) &&
			(!s_tandy_noise_valid || (s_tandy_last_noise_control != note->noise_control))
		);
		if (retune_active_voice) {
			write_tandy_volume(note->voice, 0x0F);
		}
		write_tandy_noise(note->noise_control);
	}
	write_tandy_volume(note->voice, note->attenuation & 0x0F);
	s_tandy_voice_silent[note->voice] = FALSE;
	start_tandy_output();
}

void InitEnhancedAudioHardware(void)
{
	sn76496Reset(1, &s_tandy_chip);
	clear_tandy_dma_buffers();
	s_tandy_active_buffer = 0;
	s_tandy_output_enabled = FALSE;
	s_tandy_hardware_prepared = FALSE;
	reset_tandy_voice_state();
	s_audio_timer_tick_count = 0;
	s_audio_refill_count = 0;
}

void InitAudioMode(void)
{
	InitPCMMusicSystem();
	ResetEnhancedAudioState();
	ApplyAudioModeRouting();
}

void SetAudioMode(enum audio_mode mode)
{
	BOOL had_audio;
	U8 sound_num;
	U8 sound_done_flag;
	U8 music_num;
	U8 music_done_flag;

	had_audio = (BOOL)((GetCurrentMusicNumber() != 0xFFU) || (GetCurrentSoundNumber() != 0xFFU));
	sound_num = GetCurrentSoundNumber();
	sound_done_flag = GetCurrentSoundDoneFlag();
	music_num = GetCurrentMusicNumber();
	music_done_flag = GetCurrentMusicDoneFlag();
	g_audio_mode = mode;
	ResetEnhancedAudioState();
	ApplyAudioModeRouting();
	if (had_audio) {
		if (music_num != 0xFFU) {
			StartSound((int)music_num, (int)music_done_flag);
		} else if (sound_num != 0xFFU) {
			StartSound((int)sound_num, (int)sound_done_flag);
		}
	}
}

enum audio_mode GetAudioMode(void)
{
	return g_audio_mode;
}

void SetAudioMusicBackend(enum audio_music_backend mode)
{
	BOOL had_audio;
	U8 sound_num;
	U8 sound_done_flag;
	U8 music_num;
	U8 music_done_flag;

	if (g_audio_music_backend == mode) {
		return;
	}

	had_audio = (BOOL)((GetCurrentMusicNumber() != 0xFFU) || (GetCurrentSoundNumber() != 0xFFU));
	sound_num = GetCurrentSoundNumber();
	sound_done_flag = GetCurrentSoundDoneFlag();
	music_num = GetCurrentMusicNumber();
	music_done_flag = GetCurrentMusicDoneFlag();
	StopAudioHardware();
	StopPCMMusic();
	g_audio_music_backend = mode;
	ResetAudioDebugStats();
	ApplyAudioModeRouting();
	if (had_audio) {
		if (music_num != 0xFFU) {
			StartSound((int)music_num, (int)music_done_flag);
		} else if (sound_num != 0xFFU) {
			StartSound((int)sound_num, (int)sound_done_flag);
		}
	}
}

enum audio_music_backend GetAudioMusicBackend(void)
{
	return g_audio_music_backend;
}

BOOL IsHybridMusicBackendActive(void)
{
	return UsesHybridMusicBackend();
}

void ResetAudioDebugStats(void)
{
	s_audio_timer_tick_count = 0;
	s_audio_refill_count = 0;
	ResetPCMMusicRefillCount();
}

void CaptureAudioDebugStats(audio_debug_stats *stats)
{
	if (!stats) {
		return;
	}

	stats->timer_ticks = s_audio_timer_tick_count;
	stats->buffer_refills = UsesHybridMusicBackend() ? GetPCMMusicRefillCount() : s_audio_refill_count;
	stats->backend_pcm_active = (U8)(IsPCMMusicPlaying() ? 1 : 0);
	stats->reserved = 0;
}

void ApplyAudioModeRouting(void)
{
	if (g_audio_mode == AUDIO_ORIGINAL) {
		stop_all_psg_voices();
		stop_tandy_output();
		REG_SOUNDCNT_X = 0x0080;
		REG_SOUNDCNT_L = 0xFF77;
		REG_SOUNDCNT_H = 0x0002;
	} else if (g_audio_mode == AUDIO_SEMI_TANDY) {
		stop_all_psg_voices();
		stop_tandy_output();
		REG_SOUNDCNT_X = 0x0080;
		REG_SOUNDCNT_L = 0xFF77;
		REG_SOUNDCNT_H = 0x0002;
	} else {
		stop_all_psg_voices();
	}
}

void ResetEnhancedAudioState(void)
{
	stop_tandy_output();
	sn76496Reset(1, &s_tandy_chip);
	s_tandy_active_buffer = 0;
	s_tandy_hardware_prepared = FALSE;
	reset_tandy_voice_state();
	if (!UsesHybridMusicBackend()) {
		ResetPCMMusicState();
	}
}

void StopLegacyAudioHardware(void)
{
	stop_all_psg_voices();
	stop_tandy_output();
	sn76496Reset(1, &s_tandy_chip);
	s_tandy_active_buffer = 0;
	s_tandy_hardware_prepared = FALSE;
	reset_tandy_voice_state();
	if (g_audio_mode == AUDIO_ORIGINAL) {
		ApplyAudioModeRouting();
	}
}

void StopAudioHardware(void)
{
	StopLegacyAudioHardware();
	StopPCMMusic();
}

void AudioBackendTimerTick(void)
{
	U8 next_buffer;

	s_audio_timer_tick_count++;

	if (UsesHybridMusicBackend() && IsPCMMusicPlaying()) {
		PCMMusicTimerTick();
		return;
	}

	if (!UsesTandyMixer() || !s_tandy_output_enabled) {
		return;
	}

	/* Refill the inactive half and then flip DMA to it on the next chunk boundary. */
	next_buffer = s_tandy_active_buffer ^ 1;
	fill_tandy_dma_buffer(next_buffer);
	s_audio_refill_count++;
	start_tandy_dma(next_buffer);
}

void play_sound_event(const note_event *note)
{
	if (g_audio_mode == AUDIO_ORIGINAL) {
		play_sound_original(note);
	} else if (g_audio_mode == AUDIO_SEMI_TANDY) {
		play_sound_semi_tandy(note);
	} else if (UsesHybridMusicBackend() && IsPCMMusicPlaying()) {
		/* Hybrid mode keeps DMA free for PCM music, so Tandy SFX fall back to the
		   lightweight PSG approximation rather than the live SN76496 mixer. */
		play_sound_semi_tandy(note);
	} else if (UseOriginalForCurrentSound()) {
		play_sound_original(note);
	} else {
		play_sound_tandy(note);
	}
}
