#include "gbagi.h"
#include "agimain.h"
#include "gamedata.h"
#include "pcm_music.h"
#include "pcm_music_assets.h"

#define PCM_SAMPLES_PER_CHUNK 384U
#define PCM_DMA_WORDS (PCM_SAMPLES_PER_CHUNK / 4U)
#define PCM_TIMER0_RELOAD 0xFBBBU
#define PCM_TIMER1_RELOAD (0x10000U - PCM_SAMPLES_PER_CHUNK)
#define DMA_SOUND_FIFO_A 0xB640U
#define PCM_STREAM_GAIN_NUMERATOR 4
#define PCM_STREAM_GAIN_DENOMINATOR 1

typedef struct {
	char game_id[MAX_ID_LEN + 1];
	U8 sound_num;
	pcm_track_asset track;
} pcm_track_mapping;

#define PCM_TRACK_OUTPUT_RATE 15360U
#define PCM_TRACK_ROW(game_id_value, sound_num_value, symbol_name, track_length, sample_rate_value, codec_value, loop_offset_value, loop_enabled_value) \
	{ game_id_value, sound_num_value, { symbol_name##_pcm_data, track_length, sample_rate_value, codec_value, 0, loop_offset_value, loop_enabled_value } },

/*
 * PCM asset spec for hybrid music mode:
 * - 8-bit signed PCM
 * - mono
 * - 15360 Hz sample rate
 * - stored as ROM-resident const data
 */
static const pcm_track_mapping s_pcm_track_mappings[] = {
#if PCM_MUSIC_TRACK_COUNT > 0
	PCM_MUSIC_TRACKS(PCM_TRACK_ROW)
#else
	{ "", 0, { NULL, 0, 0, PCM_CODEC_S8, 0, 0, FALSE } }
#endif
};

#undef PCM_TRACK_ROW

static U32 s_pcm_dma_buffers[2][PCM_DMA_WORDS];
static const pcm_track_asset *s_pcm_current_track;
static U32 s_pcm_position;
static U8 s_pcm_active_buffer;
static U8 s_pcm_done_flag;
static BOOL s_pcm_playing;
static BOOL s_pcm_hardware_prepared;
static S8 s_pcm_buffers_until_stop;
static U16 s_pcm_refill_count;
static U32 s_pcm_data_offset;
static U32 s_pcm_data_length;
static U32 s_pcm_total_samples;
static U32 s_pcm_decoded_samples;
static U16 s_pcm_sample_repeat;
static U16 s_pcm_repeat_remaining;
static S8 s_pcm_repeat_sample;
static BOOL s_pcm_source_active;
static S32 s_ima_predictor;
static U8 s_ima_step_index;
static U16 s_ima_block_align;
static U16 s_ima_nibbles_remaining;
static U32 s_ima_byte_offset;
static BOOL s_ima_use_high_nibble;
static BOOL s_ima_emit_block_predictor;
static const S16 s_ima_step_table[89] = {
	7, 8, 9, 10, 11, 12, 13, 14, 16, 17,
	19, 21, 23, 25, 28, 31, 34, 37, 41, 45,
	50, 55, 60, 66, 73, 80, 88, 97, 107, 118,
	130, 143, 157, 173, 190, 209, 230, 253, 279, 307,
	337, 371, 408, 449, 494, 544, 598, 658, 724, 796,
	876, 963, 1060, 1166, 1282, 1411, 1552, 1707, 1878, 2066,
	2272, 2499, 2749, 3024, 3327, 3660, 4026, 4428, 4871, 5358,
	5894, 6484, 7132, 7845, 8630, 9493, 10442, 11487, 12635, 13899,
	15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767
};

static const S8 s_ima_index_table[16] = {
	-1, -1, -1, -1, 2, 4, 6, 8,
	-1, -1, -1, -1, 2, 4, 6, 8
};

static void clear_pcm_dma_buffers(void)
{
	memset(s_pcm_dma_buffers, 0, sizeof(s_pcm_dma_buffers));
}

static S8 clamp_s16_to_s8(S32 value)
{
	if (value > 127) {
		return 127;
	}
	if (value < -128) {
		return -128;
	}
	return (S8)value;
}

static S8 apply_pcm_stream_gain(S8 sample)
{
	return clamp_s16_to_s8(
		((S32)sample * PCM_STREAM_GAIN_NUMERATOR) / PCM_STREAM_GAIN_DENOMINATOR
	);
}

static U32 read_le_u32(const U8 *data)
{
	return (U32)data[0] | ((U32)data[1] << 8) | ((U32)data[2] << 16) | ((U32)data[3] << 24);
}

static U16 read_le_u16(const U8 *data)
{
	return (U16)data[0] | ((U16)data[1] << 8);
}

static S16 read_le_s16(const U8 *data)
{
	return (S16)read_le_u16(data);
}

static BOOL pcm_mapping_matches_game(const char *mapping_game_id, const char *requested_game_id)
{
	if (strcmp(mapping_game_id, requested_game_id) == 0) {
		return TRUE;
	}

	if (!GameEnts || !GameEnts->name) {
		return FALSE;
	}

	if ((strcmp(mapping_game_id, "LLLLL") == 0) || (strcmp(mapping_game_id, "LSL1") == 0)) {
		return (BOOL)(strncmp(GameEnts->name, "Leisure Suit Larry", 18) == 0);
	}

	if ((strcmp(mapping_game_id, "PQ1") == 0) || (strcmp(mapping_game_id, "PQ") == 0)) {
		return (BOOL)(strncmp(GameEnts->name, "Police Quest", 12) == 0);
	}

	if (strcmp(mapping_game_id, "KQ1") == 0) {
		return (BOOL)(strncmp(GameEnts->name, "King's Quest", 12) == 0);
	}

	return FALSE;
}

static BOOL is_larry_game_request(const char *game_id)
{
	if ((strcmp(game_id, "LLLLL") == 0) || (strcmp(game_id, "LSL1") == 0)) {
		return TRUE;
	}

	if (GameEnts && GameEnts->name) {
		return (BOOL)(strncmp(GameEnts->name, "Leisure Suit Larry", 18) == 0);
	}

	return FALSE;
}

static const pcm_track_asset *lookup_pcm_track(const char *game_id, U8 sound_num)
{
	U32 i;
	const pcm_track_asset *fallback_track;
	U32 fallback_count;

	fallback_track = NULL;
	fallback_count = 0;

	for (i = 0; i < (U32)PCM_MUSIC_TRACK_COUNT; i++) {
		const pcm_track_mapping *mapping;

		mapping = &s_pcm_track_mappings[i];
		if (mapping->sound_num != sound_num) {
			continue;
		}
		if (pcm_mapping_matches_game(mapping->game_id, game_id)) {
			return &mapping->track;
		}
		if (
			!fallback_track ||
			(fallback_track->data != mapping->track.data) ||
			(fallback_track->length != mapping->track.length) ||
			(fallback_track->loop_offset != mapping->track.loop_offset) ||
			(fallback_track->loop_enabled != mapping->track.loop_enabled)
		) {
			fallback_track = &mapping->track;
			fallback_count++;
		}
	}

	if (fallback_count == 1U) {
		return fallback_track;
	}

	/*
	 * Larry sound 21 is our explicit streamed replacement track. If the
	 * generic matching path misses for any reason, force this one mapping so
	 * hybrid mode still uses the embedded PCM asset instead of dropping back
	 * to live SND playback.
	 */
	if ((sound_num == 21U) && is_larry_game_request(game_id)) {
		for (i = 0; i < (U32)PCM_MUSIC_TRACK_COUNT; i++) {
			const pcm_track_mapping *mapping;

			mapping = &s_pcm_track_mappings[i];
			if (mapping->sound_num != 21U) {
				continue;
			}
			if ((strcmp(mapping->game_id, "LLLLL") == 0) || (strcmp(mapping->game_id, "LSL1") == 0)) {
				return &mapping->track;
			}
		}
	}

	return NULL;
}

static void reset_ima_decoder_state(void)
{
	s_ima_predictor = 0;
	s_ima_step_index = 0;
	s_ima_block_align = 0;
	s_ima_nibbles_remaining = 0;
	s_ima_byte_offset = 0;
	s_ima_use_high_nibble = FALSE;
	s_ima_emit_block_predictor = FALSE;
}

static BOOL parse_ima_wav_track(const pcm_track_asset *track)
{
	const U8 *data;
	U32 offset;
	U16 format_tag;
	U16 channels;
	U16 bits_per_sample;
	U16 cb_size;
	U16 samples_per_block;
	U16 block_align;
	U32 chunk_size;
	BOOL found_fmt;
	BOOL found_data;

	data = (const U8 *)track->data;
	if (track->length < 44U) {
		return FALSE;
	}
	if ((memcmp(data, "RIFF", 4) != 0) || (memcmp(data + 8, "WAVE", 4) != 0)) {
		return FALSE;
	}

	offset = 12U;
	found_fmt = FALSE;
	found_data = FALSE;
	format_tag = 0;
	channels = 0;
	bits_per_sample = 0;
	cb_size = 0;
	samples_per_block = 0;
	block_align = 0;
	s_pcm_data_offset = 0;
	s_pcm_data_length = 0;

	while ((offset + 8U) <= track->length) {
		chunk_size = read_le_u32(data + offset + 4U);
		if ((offset + 8U + chunk_size) > track->length) {
			return FALSE;
		}

		if (memcmp(data + offset, "fmt ", 4) == 0) {
			if (chunk_size < 20U) {
				return FALSE;
			}
			format_tag = read_le_u16(data + offset + 8U);
			channels = read_le_u16(data + offset + 10U);
			block_align = read_le_u16(data + offset + 20U);
			bits_per_sample = read_le_u16(data + offset + 22U);
			cb_size = read_le_u16(data + offset + 24U);
			if ((chunk_size >= 22U) && (cb_size >= 2U)) {
				samples_per_block = read_le_u16(data + offset + 26U);
			}
			found_fmt = TRUE;
		} else if (memcmp(data + offset, "data", 4) == 0) {
			s_pcm_data_offset = offset + 8U;
			s_pcm_data_length = chunk_size;
			found_data = TRUE;
		}

		offset += 8U + chunk_size + (chunk_size & 1U);
	}

	if (!found_fmt || !found_data) {
		return FALSE;
	}
	if ((format_tag != 17U) || (channels != 1U) || (bits_per_sample != 4U) || (block_align < 8U)) {
		return FALSE;
	}
	if ((track->sample_rate == 0U) || (track->sample_rate > PCM_TRACK_OUTPUT_RATE)) {
		return FALSE;
	}
	if ((PCM_TRACK_OUTPUT_RATE % track->sample_rate) != 0U) {
		return FALSE;
	}
	if ((samples_per_block != 0U) && (samples_per_block != (U16)(1U + ((block_align - 4U) * 2U)))) {
		return FALSE;
	}

	s_ima_block_align = block_align;
	s_pcm_sample_repeat = (U16)(PCM_TRACK_OUTPUT_RATE / track->sample_rate);
	return TRUE;
}

static BOOL prepare_track_stream(const pcm_track_asset *track)
{
	s_pcm_data_offset = 0;
	s_pcm_data_length = track->length;
	s_pcm_sample_repeat = 1U;
	s_pcm_repeat_remaining = 0U;
	s_pcm_repeat_sample = 0;
	reset_ima_decoder_state();

	if (track->codec == PCM_CODEC_S8) {
		if ((track->sample_rate == 0U) || (track->sample_rate > PCM_TRACK_OUTPUT_RATE)) {
			return FALSE;
		}
		if ((PCM_TRACK_OUTPUT_RATE % track->sample_rate) != 0U) {
			return FALSE;
		}
		s_pcm_sample_repeat = (U16)(PCM_TRACK_OUTPUT_RATE / track->sample_rate);
		return TRUE;
	}

	if (track->codec == PCM_CODEC_IMA_ADPCM_WAV) {
		return parse_ima_wav_track(track);
	}

	return FALSE;
}

static BOOL advance_to_next_track_segment(void)
{
	if (!s_pcm_current_track) {
		return FALSE;
	}

	if (s_pcm_current_track->loop_enabled && (s_pcm_current_track->loop_offset < s_pcm_current_track->length)) {
		s_pcm_position = s_pcm_current_track->loop_offset;
		s_pcm_repeat_remaining = 0U;
		reset_ima_decoder_state();
		return TRUE;
	}

	return FALSE;
}

static BOOL load_ima_block(void)
{
	const U8 *data;

	if ((s_pcm_position + s_ima_block_align) > s_pcm_data_length) {
		if (!advance_to_next_track_segment()) {
			return FALSE;
		}
		if ((s_pcm_position + s_ima_block_align) > s_pcm_data_length) {
			return FALSE;
		}
	}

	data = (const U8 *)s_pcm_current_track->data + s_pcm_data_offset + s_pcm_position;
	s_ima_predictor = (S32)read_le_s16(data);
	s_ima_step_index = data[2];
	if (s_ima_step_index > 88U) {
		s_ima_step_index = 88U;
	}
	s_ima_byte_offset = s_pcm_position + 4U;
	s_ima_nibbles_remaining = (U16)((s_ima_block_align - 4U) * 2U);
	s_ima_use_high_nibble = FALSE;
	s_ima_emit_block_predictor = TRUE;
	s_pcm_position += s_ima_block_align;
	return TRUE;
}

static S8 decode_ima_nibble(U8 nibble)
{
	S32 step;
	S32 diff;
	S16 next_step_index;

	step = s_ima_step_table[s_ima_step_index];
	diff = step >> 3;
	if ((nibble & 1U) != 0U) {
		diff += step >> 2;
	}
	if ((nibble & 2U) != 0U) {
		diff += step >> 1;
	}
	if ((nibble & 4U) != 0U) {
		diff += step;
	}
	if ((nibble & 8U) != 0U) {
		s_ima_predictor -= diff;
	} else {
		s_ima_predictor += diff;
	}
	if (s_ima_predictor > 32767) {
		s_ima_predictor = 32767;
	} else if (s_ima_predictor < -32768) {
		s_ima_predictor = -32768;
	}

	next_step_index = (S16)s_ima_step_index + s_ima_index_table[nibble & 0x0FU];
	if (next_step_index < 0) {
		s_ima_step_index = 0U;
	} else if (next_step_index > 88) {
		s_ima_step_index = 88U;
	} else {
		s_ima_step_index = (U8)next_step_index;
	}

	return clamp_s16_to_s8(s_ima_predictor >> 8);
}

static BOOL decode_next_track_sample(S8 *sample_out)
{
	if (!s_pcm_current_track || !sample_out) {
		return FALSE;
	}

	if (s_pcm_current_track->codec == PCM_CODEC_S8) {
		if (s_pcm_position >= s_pcm_data_length) {
			if (!advance_to_next_track_segment()) {
				return FALSE;
			}
		}
		*sample_out = ((const S8 *)s_pcm_current_track->data)[s_pcm_data_offset + s_pcm_position++];
		return TRUE;
	}

	if (s_pcm_current_track->codec == PCM_CODEC_IMA_ADPCM_WAV) {
		const U8 *data;
		U8 encoded;
		U8 nibble;

		while (TRUE) {
			if (s_ima_emit_block_predictor) {
				s_ima_emit_block_predictor = FALSE;
				*sample_out = clamp_s16_to_s8(s_ima_predictor >> 8);
				return TRUE;
			}

			if (s_ima_nibbles_remaining == 0U) {
				if (!load_ima_block()) {
					return FALSE;
				}
				continue;
			}

			data = (const U8 *)s_pcm_current_track->data;
			encoded = data[s_pcm_data_offset + s_ima_byte_offset];
			if (s_ima_use_high_nibble) {
				nibble = (U8)((encoded >> 4) & 0x0FU);
				s_ima_byte_offset++;
				s_ima_use_high_nibble = FALSE;
			} else {
				nibble = (U8)(encoded & 0x0FU);
				s_ima_use_high_nibble = TRUE;
			}
			s_ima_nibbles_remaining--;
			*sample_out = decode_ima_nibble(nibble);
			return TRUE;
		}
	}

	return FALSE;
}

static void prepare_pcm_output(void)
{
	if (s_pcm_hardware_prepared) {
		return;
	}

	clear_pcm_dma_buffers();
	REG_SOUNDCNT_X = 0x0080;
	REG_SOUNDCNT_L = 0xFF77;
	REG_SOUNDCNT_H = 0x0B02;
	REG_TM0CNT_H = 0;
	REG_TM1CNT_H = 0;
	REG_DMA1CNT_H = 0;
	s_pcm_hardware_prepared = TRUE;
}

static void start_pcm_dma(U8 index)
{
	REG_DMA1CNT_H = 0;
	REG_DMA1SAD = (U32)s_pcm_dma_buffers[index];
	REG_DMA1DAD = (U32)&REG_SGFIFOA;
	REG_DMA1CNT_L = 0;
	REG_DMA1CNT_H = DMA_SOUND_FIFO_A;
	s_pcm_active_buffer = index;
}

static BOOL fill_pcm_dma_buffer(U8 index)
{
	U32 i;
	U32 out_word;
	U32 out_shift;
	BOOL exhausted;

	out_word = 0;
	out_shift = 0;
	exhausted = FALSE;
	memset(s_pcm_dma_buffers[index], 0, sizeof(s_pcm_dma_buffers[index]));

	if (!s_pcm_current_track || !s_pcm_current_track->data) {
		return TRUE;
	}

	for (i = 0; i < PCM_SAMPLES_PER_CHUNK; i++) {
		S8 sample;

		if (s_pcm_repeat_remaining == 0U) {
			if (!decode_next_track_sample(&sample)) {
				sample = 0;
				exhausted = TRUE;
				s_pcm_source_active = FALSE;
			} else {
				s_pcm_repeat_sample = sample;
				s_pcm_repeat_remaining = s_pcm_sample_repeat;
				s_pcm_source_active = TRUE;
			}
		}

		if (!exhausted && (s_pcm_repeat_remaining > 0U)) {
			sample = s_pcm_repeat_sample;
			s_pcm_repeat_remaining--;
		}

		sample = apply_pcm_stream_gain(sample);

		out_word |= ((U32)(U8)sample) << out_shift;
		out_shift += 8;
		if (out_shift == 32U) {
			s_pcm_dma_buffers[index][i >> 2] = out_word;
			out_word = 0;
			out_shift = 0;
		}
	}

	return exhausted;
}

static void start_pcm_output(void)
{
	BOOL finished0;
	BOOL finished1;

	if (s_pcm_playing) {
		return;
	}

	prepare_pcm_output();
	finished0 = fill_pcm_dma_buffer(0);
	finished1 = fill_pcm_dma_buffer(1);
	if (finished0 || finished1) {
		s_pcm_buffers_until_stop = 2;
	} else {
		s_pcm_buffers_until_stop = -1;
	}
	REG_IF |= INT_TIMER1;
	EnableInterupts(INT_TIMER1);
	start_pcm_dma(0);
	REG_TM0CNT_L = PCM_TIMER0_RELOAD;
	REG_TM0CNT_H = TIME_FREQUENcySYSTEM | TIME_ENABLE;
	REG_TM1CNT_L = PCM_TIMER1_RELOAD;
	REG_TM1CNT_H = TIME_OVERFLOW | TIME_ENABLE | TIME_IRQ_ENABLE;
	s_pcm_playing = TRUE;
}

static void signal_pcm_done(void)
{
	if (s_pcm_done_flag != 0xFFU) {
		SetFlag(s_pcm_done_flag);
		s_pcm_done_flag = 0xFFU;
	}
}

void InitPCMMusicSystem(void)
{
	s_pcm_current_track = NULL;
	s_pcm_position = 0;
	s_pcm_active_buffer = 0;
	s_pcm_done_flag = 0xFFU;
	s_pcm_playing = FALSE;
	s_pcm_hardware_prepared = FALSE;
	s_pcm_buffers_until_stop = -1;
	s_pcm_refill_count = 0;
	s_pcm_data_offset = 0;
	s_pcm_data_length = 0;
	s_pcm_sample_repeat = 1U;
	s_pcm_repeat_remaining = 0U;
	s_pcm_repeat_sample = 0;
	s_pcm_source_active = FALSE;
	reset_ima_decoder_state();
	clear_pcm_dma_buffers();
}

void ResetPCMMusicState(void)
{
	StopPCMMusic();
	InitPCMMusicSystem();
}

void StopPCMMusic(void)
{
	if (!s_pcm_playing && !s_pcm_hardware_prepared && !s_pcm_current_track) {
		return;
	}

	s_pcm_playing = FALSE;
	DissableInterupts(INT_TIMER1);
	REG_DMA1CNT_H = 0;
	REG_TM0CNT_H = 0;
	REG_TM1CNT_H = 0;
	REG_IF |= INT_TIMER1;
	REG_SGFIFOA = 0;
	REG_SGFIFOA = 0;
	REG_SOUNDCNT_L = 0xFF77;
	REG_SOUNDCNT_H = 0x0002;
	s_pcm_current_track = NULL;
	s_pcm_position = 0;
	s_pcm_active_buffer = 0;
	s_pcm_hardware_prepared = FALSE;
	s_pcm_buffers_until_stop = -1;
	s_pcm_data_offset = 0;
	s_pcm_data_length = 0;
	s_pcm_sample_repeat = 1U;
	s_pcm_repeat_remaining = 0U;
	s_pcm_repeat_sample = 0;
	s_pcm_source_active = FALSE;
	reset_ima_decoder_state();
	clear_pcm_dma_buffers();
	signal_pcm_done();
}

BOOL HasPCMMusicForSound(const char *game_id, U8 sound_num)
{
	return (BOOL)(lookup_pcm_track(game_id, sound_num) != NULL);
}

BOOL StartPCMMusicTrack(const pcm_track_asset *track, U8 done_flag)
{
	if (!track) {
		return FALSE;
	}

	StopPCMMusic();
	s_pcm_current_track = track;
	s_pcm_position = 0;
	s_pcm_done_flag = done_flag;
	if (done_flag != 0xFFU) {
		ResetFlag(done_flag);
	}
	if (!prepare_track_stream(track)) {
		s_pcm_current_track = NULL;
		signal_pcm_done();
		return FALSE;
	}
	start_pcm_output();
	return TRUE;
}

BOOL StartPCMMusicForSound(const char *game_id, U8 sound_num, U8 done_flag)
{
	const pcm_track_asset *track;

	track = lookup_pcm_track(game_id, sound_num);
	if (!track) {
		return FALSE;
	}

	return StartPCMMusicTrack(track, done_flag);
}

BOOL StartLSL1Sound21PCMMusic(U8 done_flag)
{
	const pcm_track_asset *track;

	track = lookup_pcm_track("LSL1", 21U);
	if (!track) {
		return FALSE;
	}

	return StartPCMMusicTrack(track, done_flag);
}

void PCMMusicTimerTick(void)
{
	U8 next_buffer;
	BOOL finished;

	if (!s_pcm_playing) {
		return;
	}

	if (s_pcm_buffers_until_stop >= 0) {
		s_pcm_buffers_until_stop--;
		if (s_pcm_buffers_until_stop <= 0) {
			StopPCMMusic();
			return;
		}
	}

	next_buffer = s_pcm_active_buffer ^ 1U;
	finished = fill_pcm_dma_buffer(next_buffer);
	if (finished && (s_pcm_buffers_until_stop < 0)) {
		s_pcm_buffers_until_stop = 2;
	}
	s_pcm_refill_count++;
	start_pcm_dma(next_buffer);
}

BOOL IsPCMMusicPlaying(void)
{
	return s_pcm_playing;
}

BOOL IsCurrentPCMMusicLongerThanSeconds(U16 seconds)
{
	U32 minimum_length;

	if (
		!s_pcm_playing ||
		!s_pcm_source_active ||
		!s_pcm_current_track ||
		(s_pcm_current_track->sample_rate == 0U)
	) {
		return FALSE;
	}

	minimum_length = (U32)s_pcm_current_track->sample_rate * (U32)seconds;
	return (BOOL)(s_pcm_current_track->length > minimum_length);
}

U16 GetPCMMusicRefillCount(void)
{
	return s_pcm_refill_count;
}

void ResetPCMMusicRefillCount(void)
{
	s_pcm_refill_count = 0;
}
