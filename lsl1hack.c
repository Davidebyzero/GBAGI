/***************************************************************************
 *  GBAGI: The Game Boy Advance Adventure Game Interpreter
 *  Copyright (C) 2003,2004 Brian Provinciano, http://www.bripro.com
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software Foundation,
 *  Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 ***************************************************************************/

/*****************************************************************************/
#include "gbagi.h"
#include "agimain.h"
#include "pcm_music.h"
#include "variables.h"
#include "system.h"
#include "lsl1hack.h"
#include "_generated/ambient_room_table.h"
/*****************************************************************************/
#define LSL1_QUIKIMART_ROOM 10
#define LSL1_PHONE_EASTER_EGG_CLIP_LENGTH 126391U
#define LSL1_PHONE_EASTER_EGG_CLIP_RATE 7680U
#define LSL1_KEN_SENT_ME_CLIP_LENGTH 13640U
#define LSL1_KEN_SENT_ME_CLIP_RATE 7680U
#define SQ2_VOHAUL_INTRO_CLIP_LENGTH 138978U
#define SQ2_VOHAUL_INTRO_CLIP_RATE 7680U
#define SQ2_VOHAUL_PART2_CLIP_LENGTH 121283U
#define SQ2_VOHAUL_PART2_CLIP_RATE 7680U
#define SQ2_VOHAUL_CLIP_NONE 0U
#define SQ2_VOHAUL_CLIP_INTRO 1U
#define SQ2_VOHAUL_CLIP_PART2 2U
static BOOL lsl1NeedSpecialPhoneMessage;
static BOOL lsl1NeedPhoneEasterEggMessage;
static BOOL lsl1KenSentMeClipActive;
static BOOL sq2VohaulIntroClipActive;
static U8 sq2VohaulClipKind;

extern const S8 lsl1_phone_easter_egg_pcm_data[];
extern const S8 lsl1_kensentme_pcm_data[];
extern const S8 sq2_vohaul_intro_pcm_data[];
extern const S8 sq2_vohaul_part2_pcm_data[];

static const pcm_track_asset s_lsl1_phone_easter_egg_track = {
	lsl1_phone_easter_egg_pcm_data,
	LSL1_PHONE_EASTER_EGG_CLIP_LENGTH,
	LSL1_PHONE_EASTER_EGG_CLIP_RATE,
	PCM_CODEC_S8,
	0,
	0,
	FALSE
};

static const pcm_track_asset s_lsl1_kensentme_track = {
	lsl1_kensentme_pcm_data,
	LSL1_KEN_SENT_ME_CLIP_LENGTH,
	LSL1_KEN_SENT_ME_CLIP_RATE,
	PCM_CODEC_S8,
	0,
	0,
	FALSE
};

static const pcm_track_asset s_sq2_vohaul_intro_track = {
	sq2_vohaul_intro_pcm_data,
	SQ2_VOHAUL_INTRO_CLIP_LENGTH,
	SQ2_VOHAUL_INTRO_CLIP_RATE,
	PCM_CODEC_S8,
	0,
	0,
	FALSE
};

static const pcm_track_asset s_sq2_vohaul_part2_track = {
	sq2_vohaul_part2_pcm_data,
	SQ2_VOHAUL_PART2_CLIP_LENGTH,
	SQ2_VOHAUL_PART2_CLIP_RATE,
	PCM_CODEC_S8,
	0,
	0,
	FALSE
};

static char LowerChar(char c)
{
	if(c >= 'A' && c <= 'Z')
		return c | 0x20;
	return c;
}

static BOOL ContainsNoCase(char *text, const char *needle)
{
	char a, b;

	if(!text || !needle || !*needle)
		return FALSE;

	while(*text) {
		char *t = text;
		const char *n = needle;

		while(*t && *n) {
			a = LowerChar(*t);
			b = LowerChar(*n);
			if(a != b)
				break;
			t++;
			n++;
		}
		if(!*n)
			return TRUE;
		text++;
	}

	return FALSE;
}

static BOOL IsLSL1Game(void)
{
	/* Known LSL1 AGI game ID seen in bundled resources/save files. */
	return (strcmp(szGameID, "LLLLL") == 0 || strcmp(szGameID, "LSL1") == 0);
}

static BOOL IsSQ2Game(void)
{
	return (BOOL)(
		(strcmp(szGameID, "SQ2") == 0) ||
		(strcmp(szGameID, "sq2") == 0)
	);
}

static BOOL IsSQ2VohaulIntroMessage(char *msg)
{
	return (BOOL)(
		msg &&
		ContainsNoCase(msg, "welcome to my humble fortress") &&
		ContainsNoCase(msg, "the name's vohaul") &&
		ContainsNoCase(msg, "star generator")
	);
}

static BOOL IsSQ2VohaulPart2Message(char *msg)
{
	return (BOOL)(
		msg &&
		ContainsNoCase(msg, "it was to be my ultimate war weapon") &&
		ContainsNoCase(msg, "saving lives rather than destroying them") &&
		ContainsNoCase(msg, "excuse me if i sound bitter")
	);
}

BOOL IsLSL1AmbientRoom(U8 roomNum)
{
	U16 game_index;
	U16 room_index;

	if(!IsLSL1Game())
		return FALSE;

	for(game_index = 0; game_index < K_AMBIENT_ROOM_GAME_LIST_COUNT; game_index++) {
		const ambient_room_game_list *game_list = &kAmbientRoomGameLists[game_index];

		if(strcmp(szGameID, game_list->game_id) != 0)
			continue;

		for(room_index = 0; room_index < game_list->room_count; room_index++) {
			if(game_list->rooms[room_index] == roomNum)
				return TRUE;
		}
	}

	return FALSE;
}

static BOOL IsLSL1WineOrderPrompt(char *prompt)
{
	if(!prompt)
		return FALSE;

	return (
		ContainsNoCase(prompt, "what do you want to buy") ||
		ContainsNoCase(prompt, "what would you like") ||
		ContainsNoCase(prompt, "take your order")
	);
}

static BOOL IsLSL1WineDeliveryPrompt(char *prompt)
{
	if(!prompt)
		return FALSE;

	return (
		ContainsNoCase(prompt, "where do you want it delivered") ||
		(ContainsNoCase(prompt, "deliver") && ContainsNoCase(prompt, "where")) ||
		(ContainsNoCase(prompt, "deliver") && ContainsNoCase(prompt, "send"))
	);
}

static BOOL IsLSL1PhoneNumberPrompt(char *prompt)
{
	if(!prompt)
		return FALSE;

	return (
		ContainsNoCase(prompt, "enter number") ||
		ContainsNoCase(prompt, "please enter number") ||
		ContainsNoCase(prompt, "dial")
	);
}

int LSL1AdjustGetStringMaxLen(char *prompt, int maxLen)
{
	(void)prompt;
	return maxLen;
}

static void CopyAutoFill(char *dest, int maxLen, const char *value, BOOL ignoreMaxLen)
{
	int i, limit;

	if(!dest || !value)
		return;

	limit = ignoreMaxLen ? (MAX_STRINGS_LEN + 1) : maxLen;
	if(limit > MAX_STRINGS_LEN)
		limit = MAX_STRINGS_LEN;
	if(limit <= 0)
		limit = 1;

	for(i = 0; value[i] && i < (limit - 1); i++)
		dest[i] = value[i];
	dest[i] = '\0';
}

BOOL LSL1CheckWineOrderAutofill(char *prompt, char *dest, int maxLen)
{
	if(!IsLSL1Game())
		return FALSE;

	/*
	 * Keep this workaround inert outside the Quikimart phone scene.
	 * Room 10 is the LSL1 exterior convenience-store/phone area.
	 */
	if(vars[vROOMNUM] != LSL1_QUIKIMART_ROOM)
		return FALSE;

	if(IsLSL1WineOrderPrompt(prompt)) {
		CopyAutoFill(dest, maxLen, "wine", FALSE);
		return TRUE;
	}

	if(IsLSL1WineDeliveryPrompt(prompt)) {
		/*
		 * The original puzzle expects "honeymoon suite", but the AGI prompt
		 * length may be shorter than that phrase. Write the full value into
		 * GBAGI's backing string buffer so the later parse(sA) sees the
		 * complete destination text.
		 */
		CopyAutoFill(dest, maxLen, "honeymoon suite", TRUE);
		return TRUE;
	}

	return FALSE;
}

void LSL1NormalizePhoneNumberDigits(char *dest)
{
	char digits[MAX_STRINGS_LEN];
	int i, out;

	if(!dest)
		return;

	if(vars[vROOMNUM] != LSL1_QUIKIMART_ROOM)
		return;

	out = 0;
	for(i = 0; dest[i]; i++) {
		if(dest[i] >= '0' && dest[i] <= '9' && out < (MAX_STRINGS_LEN - 1))
			digits[out++] = dest[i];
	}

	for(i = 0; i < out; i++)
		dest[i] = digits[i];
	dest[out] = '\0';
}

void LSL1TrackPhoneEasterEggInput(char *dest)
{
	char digits[MAX_STRINGS_LEN];
	int i, out;

	if(!dest)
		return;

	out = 0;
	for(i = 0; dest[i]; i++) {
		if(dest[i] >= '0' && dest[i] <= '9' && out < (MAX_STRINGS_LEN - 1))
			digits[out++] = dest[i];
	}
	digits[out] = '\0';

	if(strcmp(digits, "5551987") == 0)
		lsl1NeedPhoneEasterEggMessage = TRUE;
}

void LSL1NormalizePhoneNumberInput(char *prompt, char *dest)
{
	lsl1NeedSpecialPhoneMessage = FALSE;

	if(vars[vROOMNUM] != LSL1_QUIKIMART_ROOM) {
		(void)prompt;
		LSL1NormalizePhoneNumberDigits(dest);
		return;
	}

	LSL1NormalizePhoneNumberDigits(dest);
	LSL1TrackPhoneEasterEggInput(dest);
}

char *LSL1OverridePhoneMessage(char *msg)
{
	if((lsl1NeedSpecialPhoneMessage || lsl1NeedPhoneEasterEggMessage) &&
	   msg &&
	   (ContainsNoCase(msg, "disconnected") ||
	    ContainsNoCase(msg, "no longer in service") ||
	    ContainsNoCase(msg, "reached a number")) &&
	   (ContainsNoCase(msg, "dial again") ||
	    ContainsNoCase(msg, "hang up"))) {
		lsl1NeedSpecialPhoneMessage = FALSE;
		if(lsl1NeedPhoneEasterEggMessage) {
			lsl1NeedPhoneEasterEggMessage = FALSE;
			StartPCMMusicTrack(&s_lsl1_phone_easter_egg_track, 0xFFU);
		}
		return "Are you guys still playing this game in 2026??!!?\nSTOP CALLING THIS NUMBER!";
	}
	return msg;
}

char *LSL1ConsumePhoneEasterEggMessage(void)
{
	if(lsl1NeedPhoneEasterEggMessage) {
		lsl1NeedPhoneEasterEggMessage = FALSE;
		return "Are you guys still playing this game in 2026??!!?\nSTOP CALLING THIS NUMBER!";
	}
	return NULL;
}

void LSL1MaybePlayKenSentMeClip(U8 logic_num, U8 message_num)
{
	if(!IsLSL1Game())
		return;
	if(logic_num != 15U || message_num != 9U)
		return;

	/*
	 * Clear any active AGI sound effect first. Otherwise TIMER2 can finish the
	 * old sound asynchronously and call StopSound(), which also kills PCM.
	 */
	StopLegacySoundEffectsOnly();
	StartPCMMusicTrack(&s_lsl1_kensentme_track, 0xFFU);
	lsl1KenSentMeClipActive = TRUE;
}

BOOL LSL1IsKenSentMeClipActive(void)
{
	return lsl1KenSentMeClipActive;
}

BOOL LSL1ShouldPreserveKenSentMeClip(void)
{
	if(!lsl1KenSentMeClipActive)
		return FALSE;

	/*
	 * Let this one voiced line survive the immediate door-open room transition.
	 * Once PCM has naturally finished, the flag will be cleared by the waiter.
	 */
	return IsPCMMusicPlaying();
}

void LSL1WaitForKenSentMeClip(void)
{
	U16 frames_waited;
	U16 max_frames;

	if(!lsl1KenSentMeClipActive)
		return;

	/*
	 * This specific door-reply clip gets cut off because the room transition
	 * logic continues immediately after the message. Keep the workaround narrow:
	 * only for the Ken-sent-me clip and only long enough for the PCM track to
	 * finish, with a conservative safety cap.
	 */
	max_frames = 180U;
	frames_waited = 0U;
	while(IsPCMMusicPlaying() && (frames_waited < max_frames)) {
		WaitForFrames(1);
		frames_waited++;
	}

	lsl1KenSentMeClipActive = FALSE;
}

void SQ2MaybePlayVohaulIntroClip(U8 logic_num, U8 message_num)
{
	if(!IsSQ2Game())
		return;
	if(logic_num != 6U || message_num != 3U)
		return;

	StopLegacySoundEffectsOnly();
	StartPCMMusicTrack(&s_sq2_vohaul_intro_track, 0xFFU);
	sq2VohaulIntroClipActive = TRUE;
}

void SQ2MaybePlayVohaulIntroClipForMessage(char *msg)
{
	if(IsSQ2VohaulIntroMessage(msg)) {
		if(sq2VohaulIntroClipActive &&
		   sq2VohaulClipKind == SQ2_VOHAUL_CLIP_INTRO &&
		   IsPCMMusicPlaying())
			return;

		StopLegacySoundEffectsOnly();
		StartPCMMusicTrack(&s_sq2_vohaul_intro_track, 0xFFU);
		sq2VohaulIntroClipActive = TRUE;
		sq2VohaulClipKind = SQ2_VOHAUL_CLIP_INTRO;
		return;
	}

	if(IsSQ2VohaulPart2Message(msg)) {
		if(sq2VohaulIntroClipActive &&
		   sq2VohaulClipKind == SQ2_VOHAUL_CLIP_PART2 &&
		   IsPCMMusicPlaying())
			return;

		StopLegacySoundEffectsOnly();
		StartPCMMusicTrack(&s_sq2_vohaul_part2_track, 0xFFU);
		sq2VohaulIntroClipActive = TRUE;
		sq2VohaulClipKind = SQ2_VOHAUL_CLIP_PART2;
	}
}

void SQ2MaybeStopVohaulIntroClipForMessage(char *msg)
{
	if(!sq2VohaulIntroClipActive)
		return;

	/*
	 * SQ2 should not block on this voiceover, but it also should not keep
	 * talking underneath the next textbox. As soon as a different messagebox
	 * opens, stop this one-off clip cleanly.
	 */
	if(IsSQ2VohaulIntroMessage(msg) &&
	   sq2VohaulClipKind == SQ2_VOHAUL_CLIP_INTRO &&
	   IsPCMMusicPlaying())
		return;

	if(IsSQ2VohaulPart2Message(msg) &&
	   sq2VohaulClipKind == SQ2_VOHAUL_CLIP_PART2 &&
	   IsPCMMusicPlaying())
		return;

	StopPCMMusic();
	sq2VohaulIntroClipActive = FALSE;
	sq2VohaulClipKind = SQ2_VOHAUL_CLIP_NONE;
}

BOOL SQ2IsVohaulIntroClipActive(void)
{
	return sq2VohaulIntroClipActive;
}

BOOL SQ2ShouldPreserveVohaulIntroClip(void)
{
	if(!sq2VohaulIntroClipActive)
		return FALSE;

	return IsPCMMusicPlaying();
}

void SQ2WaitForVohaulIntroClip(void)
{
	U16 frames_waited;
	U16 max_frames;

	if(!sq2VohaulIntroClipActive)
		return;

	max_frames = 2400U;
	frames_waited = 0U;
	while(IsPCMMusicPlaying() && (frames_waited < max_frames)) {
		WaitForFrames(1);
		frames_waited++;
	}

	sq2VohaulIntroClipActive = FALSE;
	sq2VohaulClipKind = SQ2_VOHAUL_CLIP_NONE;
}
