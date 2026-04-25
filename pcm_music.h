#ifndef _PCM_MUSIC_H
#define _PCM_MUSIC_H

#include "types.h"

enum pcm_track_codec {
	PCM_CODEC_S8 = 0,
	PCM_CODEC_IMA_ADPCM_WAV = 1
};

typedef struct {
	const S8 *data;
	U32 length;
	U16 sample_rate;
	U8 codec;
	U8 reserved;
	U32 loop_offset;
	BOOL loop_enabled;
} pcm_track_asset;

void InitPCMMusicSystem(void);
void ResetPCMMusicState(void);
void StopPCMMusic(void);
BOOL StartPCMMusicTrack(const pcm_track_asset *track, U8 done_flag);
BOOL StartPCMMusicForSound(const char *game_id, U8 sound_num, U8 done_flag);
BOOL StartLSL1Sound21PCMMusic(U8 done_flag);
void PCMMusicTimerTick(void);
BOOL IsPCMMusicPlaying(void);
BOOL IsCurrentPCMMusicLongerThanSeconds(U16 seconds);
BOOL HasPCMMusicForSound(const char *game_id, U8 sound_num);
U16 GetPCMMusicRefillCount(void);
void ResetPCMMusicRefillCount(void);

#endif
