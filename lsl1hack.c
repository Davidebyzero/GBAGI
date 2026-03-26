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
#include "variables.h"
#include "lsl1hack.h"
/*****************************************************************************/
#define LSL1_QUIKIMART_ROOM 10

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

	if(!dest || !IsLSL1Game())
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

void LSL1NormalizePhoneNumberInput(char *prompt, char *dest)
{
	(void)prompt;
	LSL1NormalizePhoneNumberDigits(dest);
}
