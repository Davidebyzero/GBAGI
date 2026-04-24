#ifndef _ENHANCED_AUDIO_H
#define _ENHANCED_AUDIO_H

#include "types.h"

enum audio_mode {
	AUDIO_ORIGINAL = 0,
	AUDIO_SEMI_TANDY = 1,
	AUDIO_TANDY = 2
};

enum audio_music_backend {
	AUDIO_BACKEND_TANDY_LIVE = 0,
	AUDIO_BACKEND_TANDY_HYBRID = 1
};

#define AUDIO_MODE_DEFAULT AUDIO_TANDY
#define AUDIO_BACKEND_DEFAULT AUDIO_BACKEND_TANDY_HYBRID

typedef struct note_event {
	U8 voice;
	U8 is_noise;
	U8 is_end;
	U8 is_silent;
	U16 duration;
	U16 raw_frequency;
	U16 gba_frequency;
	U8 attenuation;
	U8 volume;
	U8 noise_period;
	U8 noise_control;
} note_event;

typedef struct {
	U16 timer_ticks;
	U16 buffer_refills;
	U8 backend_pcm_active;
	U8 reserved;
} audio_debug_stats;

extern enum audio_mode g_audio_mode;
extern enum audio_music_backend g_audio_music_backend;

void InitAudioMode(void);
void SetAudioMode(enum audio_mode mode);
enum audio_mode GetAudioMode(void);
void SetAudioMusicBackend(enum audio_music_backend mode);
enum audio_music_backend GetAudioMusicBackend(void);
BOOL IsHybridMusicBackendActive(void);
void ResetAudioDebugStats(void);
void CaptureAudioDebugStats(audio_debug_stats *stats);
void InitEnhancedAudioHardware(void);
void ApplyAudioModeRouting(void);
void ResetEnhancedAudioState(void);
void StopLegacyAudioHardware(void);
void StopAudioHardware(void);
void AudioBackendTimerTick(void);
void play_sound_event(const note_event *note);

#endif
