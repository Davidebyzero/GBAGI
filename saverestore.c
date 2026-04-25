/***************************************************************************
 *  GBAGI: The Game Boy Advance Adventure Game Interpreter
 *  Copyright (C) 2003,2004 Brian Provinciano, http://www.bripro.com
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the GNU General Public License as published by the Free Software Foundation;
 *  either version 2 of the License, or (at your option) any later version.
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
#include "saverestore.h"
#include "text.h"
#include "screen.h"
#include "wingui.h"
#include "logic.h"
#include "commands.h"
#include "views.h"
#include "picture.h"
#include "text.h"
#include "status.h"
#include "menu.h"
#include "input.h"
#include "errmsg.h"
#include "system.h"
#include "invobj.h"
#include "enhanced_audio.h"
/*****************************************************************************/

#ifdef _WINDOWS
FILE* f;
#else
U8* pSaveMem;
#endif

#define MAX_SAVES 8
#define SAVE_FILE_SIZE 4096
#define GLOBAL_AUDIO_PREF_OFFSET (MAX_SAVES * SAVE_FILE_SIZE)

static const char kGlobalAudioPrefTag[4] = { 'A', 'U', 'D', '1' };
static const char kSaveFormatTag[4] = { 'S', 'R', 'V', '2' };

#define BATTERYLESS_COMMIT_NONE	0
#define BATTERYLESS_COMMIT_AUTO	1

#define BATTERYLESS_COMMIT_MODE		BATTERYLESS_COMMIT_NONE
#define BATTERYLESS_COMMIT_SPAN		0x2000
#define BATTERYLESS_COMMIT_PASSES	5
#define BATTERYLESS_DELAY_TICKS		400

const char szSaveHeader[] = "GBAGI/Save_Game";
char szSaveName[MAX_SAVENAME_LEN + 1], szAutoSave[MAX_SAVENAME_LEN + 1];
char szSaveNames[MAX_SAVES][MAX_SAVENAME_LEN + 1];
char szTemp1[16], szTemp2[128];
int saveSlot;
BOOL OK_CLOSE;

const char batteryless_test_tag[] = "BATTERYLESS_TEST_12345";
const char batteryless_commit_tag[] = "BATTERYLESS_COMMIT_AUTO_54321";
const char batteryless_pump_tag[] = "BATTERYLESS_COMMIT_PUMP_2026";
const char* batteryless_test_tag_ptr = batteryless_test_tag;
const char* batteryless_commit_tag_ptr = batteryless_commit_tag;
const char* batteryless_pump_tag_ptr = batteryless_pump_tag;

#ifndef _WINDOWS
static const char szBatterylessCommitLine1[] = "Committing save...";
static const char szBatterylessCommitLine2[] = "Do not power off.";
static BOOL gBatterylessCommitPending = FALSE;
#endif

/*****************************************************************************/
S16 wnSaveRestoreProc(WND* w, U16 msg, U16 wParam, U32 lParam);
static BOOL RestoreGameSlotByIndex(int slot);
static void LoadGlobalAudioPreferences(void);
static void SaveViewObjectsCompact(void);
static void LoadViewObjectsCompact(void);
static int GetSaveDataOffset(int slot);

WND wnSaveRestore = {
	NULL,NULL,NULL,NULL,
	24,22,192,116,
	{0,0,0,0},{0,0,0,0},
	"S/R",
	0,
	0,
	wsRESIZABLE | wsTITLE | wsSELECTABLE,
	(WNPROC)wnSaveRestoreProc
};
WND bnSaveOK = {
	NULL,NULL,&wnSaveRestore,NULL,
	130,8,52,16,
	{0,0,0,0},{0,0,0,0},
	"Save",
	wnBUTTON,
	0,
	bsCAPTION | wsSELECTABLE,
	(WNPROC)wnSaveRestoreProc
};
WND bnSaveCancel = {
	NULL,NULL,&wnSaveRestore,NULL,
	130,30,52,16,
	{0,0,0,0},{0,0,0,0},
	"Cancel",
	wnBUTTON,
	0,
	bsCAPTION | wsSELECTABLE,
	(WNPROC)wnSaveRestoreProc
};
WND lbSaveFiles = {
	NULL,NULL,&wnSaveRestore,NULL,
	0,4,128,68,
	{0,0,0,0},{0,0,0,0},
	"",
	wnLISTBOX,
	0,
	wsSELECTABLE | wsEXTFIXED,
	(WNPROC)wnSaveRestoreProc
};
WND edSaveInput = {
	NULL,NULL,&wnSaveRestore,NULL,
	32,78,96,14,
	{0,0,0,0},{0,0,0,0},
	szSaveName,
	wnEDIT,
	0,
	wsSELECTABLE,
	(WNPROC)wnSaveRestoreProc
};
WND txSaveName = {
	NULL,NULL,&wnSaveRestore,NULL,
	2,82,30,8,
	{0,0,0,0},{0,0,0,0},
	"Name:",
	wnTEXT,
	0,
	0,
	(WNPROC)wnSaveRestoreProc
};

/*****************************************************************************/
S16 wnSaveRestoreProc(WND* w, U16 msg, U16 wParam, U32 lParam)
{
	switch (msg) {
	case wmLISTBOX_CHANGE:
		edSaveInput.caption = ((LISTITEM*)lParam)->text;
		wDrawWnd(&edSaveInput, 0);
		break;
	case wmEDIT_CHANGE:
		wDrawWnd(&lbSaveFiles, 0);
		break;
	case wmBUTTON_CLICK:
		if (w == &bnSaveOK) {
			OK_CLOSE = TRUE;
			WndDispose(&wnSaveRestore);
		}
		else if (w == &bnSaveCancel) {
			OK_CLOSE = FALSE;
			WndDispose(&wnSaveRestore);
		}
		break;
	}
	return 1;
}

/*****************************************************************************/
void InitSaveRestore()
{
	szAutoSave[0] = '\0';
	LoadGlobalAudioPreferences();
}

/*****************************************************************************/
void SaveGlobalAudioPreferences(void)
{
	U8 saved_mode;
	U8 saved_backend;
	U8 saved_toggle_flags;

	OPEN_SAVE_FILE();
	saved_mode = (U8)GetAudioMode();
	saved_backend = (U8)GetAudioMusicBackend();
	saved_toggle_flags = (U8)(
		(IsLarryMusicSpeedHackEnabled() ? 0x01U : 0U) |
		(IsAudioBoostEnabled() ? 0x02U : 0U) |
		(IsCharacterBoostEnabled() ? 0x04U : 0U) |
		(IsComboBoostEnabled() ? 0x08U : 0U)
	);

	SAVE_SEEK_SET(GLOBAL_AUDIO_PREF_OFFSET);
	FWRITEN(kGlobalAudioPrefTag, 4);
	FPUTB(saved_mode);
	FPUTB(saved_backend);
	FPUTB(saved_toggle_flags);
	FPUTB(4);
	CLOSE_SAVE_FILE();

#ifndef _WINDOWS
	BatterylessNotifySaveDirty();
#endif
}

/*****************************************************************************/
static void LoadGlobalAudioPreferences(void)
{
	char tag[4];
	U8 saved_mode;
	U8 saved_backend;
	U8 saved_toggle_flags;
	U8 pref_version;

	OPEN_SAVE_FILE();
	SAVE_SEEK_SET(GLOBAL_AUDIO_PREF_OFFSET);
	FREADN(tag, 4);
	if (memcmp(tag, kGlobalAudioPrefTag, 4) != 0) {
		CLOSE_SAVE_FILE();
		return;
	}

	FGETB(saved_mode);
	FGETB(saved_backend);
	FGETB(saved_toggle_flags);
	FGETB(pref_version);
	CLOSE_SAVE_FILE();
	if (saved_mode > AUDIO_TANDY) {
		saved_mode = (U8)AUDIO_MODE_DEFAULT;
	}
	if (saved_backend > AUDIO_BACKEND_TANDY_HYBRID) {
		saved_backend = (U8)AUDIO_BACKEND_DEFAULT;
	}

	SetAudioMode((enum audio_mode)saved_mode);
	SetAudioMusicBackend((enum audio_music_backend)saved_backend);
	if (pref_version == 1U) {
		SetLarryMusicSpeedHackEnabled(saved_toggle_flags ? TRUE : FALSE);
		SetAudioBoostEnabled(FALSE);
		SetCharacterBoostEnabled(TRUE);
		SetComboBoostEnabled(TRUE);
	} else if (pref_version == 2U) {
		SetLarryMusicSpeedHackEnabled((saved_toggle_flags & 0x01U) ? TRUE : FALSE);
		SetAudioBoostEnabled((saved_toggle_flags & 0x02U) ? TRUE : FALSE);
		SetCharacterBoostEnabled(TRUE);
		SetComboBoostEnabled(TRUE);
	} else if (pref_version >= 3U) {
		SetLarryMusicSpeedHackEnabled((saved_toggle_flags & 0x01U) ? TRUE : FALSE);
		SetAudioBoostEnabled((saved_toggle_flags & 0x02U) ? TRUE : FALSE);
		SetCharacterBoostEnabled((saved_toggle_flags & 0x04U) ? TRUE : FALSE);
		if (pref_version >= 4U) {
			SetComboBoostEnabled((saved_toggle_flags & 0x08U) ? TRUE : FALSE);
		} else {
			SetComboBoostEnabled(TRUE);
		}
	}
}

/*****************************************************************************/
static int GetSaveDataOffset(int slot)
{
	return (slot * SAVE_FILE_SIZE) + sizeof(szSaveHeader) + sizeof(szGameID) + MAX_SAVENAME_LEN + 1;
}

/*****************************************************************************/
static void SaveViewObjectsCompact(void)
{
	int i;
	VOBJ* v;

	for (i = 0; i < MAX_VOBJ; i++) {
		v = &ViewObjs[i];

		FPUTB(v->num);
		FPUTW(v->x);
		FPUTW(v->y);
		FPUTW(v->prevX);
		FPUTW(v->prevY);
		FPUTB(v->width);
		FPUTB(v->height);
		FPUTB(v->prevWidth);
		FPUTB(v->prevHeight);
		FPUTB(v->view);
		FPUTB(v->loop);
		FPUTB(v->totalLoops);
		FPUTB(v->cel);
		FPUTB(v->totalCels);
		FPUTB(v->direction);
		FPUTB(v->motion);
		FPUTB(v->priority);
		FPUTW(v->flags);
		FPUTB(v->stepTime);
		FPUTB(v->stepCount);
		FPUTB(v->stepSize);
		FPUTB(v->cycle);
		FPUTB(v->cycleTime);
		FPUTB(v->cycleCount);
		FPUTW(v->move.x);
		FPUTW(v->move.y);
		FPUTB(v->move.stepSize);
		FPUTB(v->move.flag);
	}
}

/*****************************************************************************/
static void LoadViewObjectsCompact(void)
{
	int i;
	VOBJ* v;

	for (i = 0; i < MAX_VOBJ; i++) {
		v = &ViewObjs[i];

		FGETB(v->num);
		FGETW(v->x);
		FGETW(v->y);
		FGETW(v->prevX);
		FGETW(v->prevY);
		FGETB(v->width);
		FGETB(v->height);
		FGETB(v->prevWidth);
		FGETB(v->prevHeight);
		FGETB(v->view);
		FGETB(v->loop);
		FGETB(v->totalLoops);
		FGETB(v->cel);
		FGETB(v->totalCels);
		FGETB(v->direction);
		FGETB(v->motion);
		FGETB(v->priority);
		FGETW(v->flags);
		FGETB(v->stepTime);
		FGETB(v->stepCount);
		FGETB(v->stepSize);
		FGETB(v->cycle);
		FGETB(v->cycleTime);
		FGETB(v->cycleCount);
		FGETW(v->move.x);
		FGETW(v->move.y);
		FGETB(v->move.stepSize);
		FGETB(v->move.flag);
	}
}

/*****************************************************************************/
BOOL ExecuteSaveDialog(const char* szTitle, const char* szType)
{
	wnSaveRestore.caption = (char*)szTitle;
	bnSaveOK.caption = (char*)szType;

	AddWindow(&wnSaveRestore);
	AddWindow(&bnSaveCancel);
	AddWindow(&bnSaveOK);
	edSaveInput.ext.edit.maxLen = 15;
	AddWindow(&edSaveInput);
	AddWindow(&lbSaveFiles);
	AddWindow(&txSaveName);

	WinGUIDoit();

	saveSlot = lbSaveFiles.ext.listbox.itemActive->index;

	return OK_CLOSE;
}

/*****************************************************************************/
int FillSaveList(BOOL RES, int maxsaves)
{
	int i = 0;
	for (saveSlot = 0;saveSlot < maxsaves;saveSlot++) {
		SAVE_SEEK_SET(saveSlot * SAVE_FILE_SIZE);

		FREADN(szTemp1, sizeof(szSaveHeader));
		if (strcmp(szTemp1, szSaveHeader))
			continue;
		FREADN(szTemp1, sizeof(szGameID));
		if (strcmp(szTemp1, szGameID))
			continue;

		FREAD(szSaveNames[saveSlot]);
		if (RES)ListBoxAdd(&lbSaveFiles, (char*)szSaveNames[saveSlot]);
		i++;
	}

	ListBoxSelect(&lbSaveFiles, 0);

	return (i);
}

/*****************************************************************************/
void SRamMemCpy(U8* a, U8* b, int len)
{
	while (len--)
		*a++ = *b++;
}

/*****************************************************************************/
#ifndef _WINDOWS
static void BatterylessBusyDelay(void)
{
	U32 frames;

	frames = (((U32)((64 * BATTERYLESS_DELAY_TICKS) + 1) * 60) + 16383) / 16384;
	if (frames == 0) {
		frames = 1;
	}
	WaitForFrames((U16)frames);
}

/*****************************************************************************/
static void BatterylessShowCommitMessage(void)
{
	BoxNBorder(24, 52, 216, 92, 0x4F);
	DrawStringAbs(42, 62, (char*)szBatterylessCommitLine1, 0xF0);
	DrawStringAbs(42, 74, (char*)szBatterylessCommitLine2, 0xF0);
}

/*****************************************************************************/
static void BatterylessCommitSRAM(void)
{
#if BATTERYLESS_COMMIT_MODE == BATTERYLESS_COMMIT_AUTO
	volatile U8* sram = (volatile U8*)GAMEPAK_RAM;
	int pass, i;
	BOOL wasGUIActive = GUI_ACTIVE;

	if (!wasGUIActive)
		gfxGUIEnter();

	BatterylessShowCommitMessage();
	GBA_Flip();

	for (pass = 0; pass < BATTERYLESS_COMMIT_PASSES; pass++) {
		for (i = 0; i < BATTERYLESS_COMMIT_SPAN; i++) {
			U8 original = sram[i];
			sram[i] = (U8)(original ^ 0xFF);
			sram[i] = original;
		}
	}

	BatterylessBusyDelay();

	if (!wasGUIActive) {
		RedrawScreen(TEXT_MODE);
		GBA_Flip();
		GUI_ACTIVE = FALSE;
		if (REG_DISPCNT & BACKBUFFER)
			vidPtr = ((U16*)0x600A000);
		else
			vidPtr = ((U16*)0x6000000);
	}
#endif
}

/*****************************************************************************/
void BatterylessNotifySaveDirty(void)
{
	BatterylessManualFlush();
}

/*****************************************************************************/
void BatterylessUpdateCommitPump(void)
{
#if BATTERYLESS_COMMIT_MODE == BATTERYLESS_COMMIT_AUTO
	if (!gBatterylessCommitPending)
		return;
	gBatterylessCommitPending = FALSE;
	BatterylessCommitSRAM();
#endif
}
#endif

/*****************************************************************************/
BOOL SaveGame()
{
	int i, totalOverlays, totalPViews;
	BOOL isAutoSave = FALSE;

	OPEN_SAVE_FILE();

	if (szAutoSave[0]) {
		strcpy(szSaveName, szAutoSave);
		saveSlot = 0;
		isAutoSave = TRUE;
	}
	else {
		ListBoxClear(&lbSaveFiles);
		for (i = 0;i < MAX_SAVES;i++) {
			strcpy(szSaveNames[i], "-");
			ListBoxAdd(&lbSaveFiles, (char*)szSaveNames[i]);
		}
		if (!FillSaveList(FALSE, MAX_SAVES)) {
		}

		edSaveInput.style |= wsSELECTABLE;

		if (!ExecuteSaveDialog("Save Game", "Save")) {
			CLOSE_SAVE_FILE();
			return FALSE;
		}

		sprintf(szSaveName, szSaveNames[saveSlot]);
	}

	SAVE_SEEK_SET(saveSlot * SAVE_FILE_SIZE);

	FWRITE(szSaveHeader);
	FWRITE(szGameID);
	FWRITE(szSaveName);
	FWRITEN(kSaveFormatTag, 4);

	FWRITE(vars);
	FWRITE(flags);
	FWRITE(strings);

	FPUTB(PLAYER_CONTROL);
	FPUTB(TEXT_MODE);
	FPUTB(REFRESH_SCREEN);
	FPUTB(MENU_SET);
	FPUTB(INPUT_ENABLED);
	FPUTB(SOUND_ON);
	FPUTB(PIC_VISIBLE);
	FPUTB(PRI_VISIBLE);
	FPUTB(STATUS_VISIBLE);
	FPUTB(VOBJ_BLOCKING);
	FPUTB(WALK_HOLD);

	FPUTB(oldScore);
	FPUTB(horizon);
	FPUTB(picNum);
	FPUTB(minRow);
	FPUTB(inputPos);
	FPUTB(statusRow);
	FPUTB(textColour);
	FPUTB(textAttr);
	FPUTB(textRow);
	FPUTB(textCol);
	FPUTB(minRowY);
	FPUTB(ticks);
	FPUTB(cursorChar);
	FPUTB(IF_RESULT);

	FWRITE(objBlock);
	SaveViewObjectsCompact();
	FWRITE(logScan);
	FWRITE(invObjRooms);

	FPUTW(msgX);
	FPUTW(msgY);
	FPUTW(msgHeight);
	FPUTW(msgWidth);
	FPUTW(maxWidth);
	FWRITE(wndDraw);

	totalPViews = (pPView - pViews);
	FPUTB((U8)totalPViews);
	for (i = 0;i < totalPViews;i++) {
		FPUTB(pViews[i].view);
		FPUTB(pViews[i].loop);
		FPUTB(pViews[i].cel);
		FPUTB(pViews[i].x);
		FPUTB(pViews[i].y);
		FPUTB(pViews[i].pri);
		FPUTW(0);
	}
	totalOverlays = (pOverlay - overlays);
	FPUTB((U8)totalOverlays);
	for (i = 0;i < totalOverlays;i++) {
		FPUTB(overlays[i]);
	}

	CLOSE_SAVE_FILE();

	BatterylessNotifySaveDirty();

	return TRUE;
}

/*****************************************************************************/
BOOL RestoreGame()
{
	int i;
	int selectedIndex;

	OPEN_SAVE_FILE();

	ListBoxClear(&lbSaveFiles);
	if (!FillSaveList(TRUE, MAX_SAVES)) {
		MessageBox("There are no previously saved games to restore.");
		CLOSE_SAVE_FILE();
		return FALSE;
	}

	edSaveInput.style &= ~wsSELECTABLE;

	if (!ExecuteSaveDialog("Restore Game", "Restore")) {
		CLOSE_SAVE_FILE();
		return FALSE;
	}

	selectedIndex = saveSlot;
	for (i = 0;i < MAX_SAVES;i++) {
		SAVE_SEEK_SET(i * SAVE_FILE_SIZE);

		FREADN(szTemp1, sizeof(szSaveHeader));
		if (strcmp(szTemp1, szSaveHeader))
			continue;
		FREADN(szTemp1, sizeof(szGameID));
		if (strcmp(szTemp1, szGameID))
			continue;

		SAVE_SEEK_CUR(MAX_SAVENAME_LEN + 1);

		if (selectedIndex == 0) {
			saveSlot = i;
			break;
		}
		selectedIndex--;
	}
	CLOSE_SAVE_FILE();

	return RestoreGameSlotByIndex(saveSlot);
}

/*****************************************************************************/
static BOOL RestoreGameSlotByIndex(int slot)
{
	int i;
	int totalPViews, totalOverlays;
	VOBJ* v;
	char formatTag[4];
	BOOL isCompactSave = FALSE;

	OPEN_SAVE_FILE();
	SAVE_SEEK_SET(slot * SAVE_FILE_SIZE);

	FREADN(szTemp1, sizeof(szSaveHeader));
	if (strcmp(szTemp1, szSaveHeader)) {
		CLOSE_SAVE_FILE();
		return FALSE;
	}
	FREADN(szTemp1, sizeof(szGameID));
	if (strcmp(szTemp1, szGameID)) {
		CLOSE_SAVE_FILE();
		return FALSE;
	}

	SAVE_SEEK_CUR(MAX_SAVENAME_LEN + 1);
	FREADN(formatTag, 4);
	if (memcmp(formatTag, kSaveFormatTag, 4) == 0) {
		isCompactSave = TRUE;
	}
	else {
		SAVE_SEEK_SET(GetSaveDataOffset(slot));
	}

	EraseBlitLists();
	InitViewSystem();

	FREAD(vars);
	FREAD(flags);
	FREAD(strings);

	FGETB(PLAYER_CONTROL);
	FGETB(TEXT_MODE);
	FGETB(REFRESH_SCREEN);
	FGETB(MENU_SET);
	FGETB(INPUT_ENABLED);
	FGETB(SOUND_ON);
	FGETB(PIC_VISIBLE);
	FGETB(PRI_VISIBLE);
	FGETB(STATUS_VISIBLE);
	FGETB(VOBJ_BLOCKING);
	FGETB(WALK_HOLD);

	FGETB(oldScore);
	FGETB(horizon);
	FGETB(picNum);
	FGETB(minRow);
	FGETB(inputPos);
	FGETB(statusRow);
	FGETB(textColour);
	FGETB(textAttr);
	FGETB(textRow);
	FGETB(textCol);
	FGETB(minRowY);
	FGETB(ticks);
	FGETB(cursorChar);
	FGETB(IF_RESULT);

	FREAD(objBlock);
	if (isCompactSave) {
		LoadViewObjectsCompact();
	}
	else {
		FREAD(ViewObjs);
	}
	FREAD(logScan);
	FREAD(invObjRooms);

	FGETW(msgX);
	FGETW(msgY);
	FGETW(msgHeight);
	FGETW(msgWidth);
	FGETW(maxWidth);
	FREAD(wndDraw);

	FGETB(totalPViews);
	for (i = 0;i < totalPViews;i++) {
		FGETB(pViews[i].view);
		FGETB(pViews[i].loop);
		FGETB(pViews[i].cel);
		FGETB(pViews[i].x);
		FGETB(pViews[i].y);
		FGETB(pViews[i].pri);
		FSKIPW();
	}
	FGETB(totalOverlays);
	for (i = 0;i < totalOverlays;i++) {
		FGETB(overlays[i]);
	}

	CLOSE_SAVE_FILE();

	AGIInitVars();

	for (v = ViewObjs; v < &ViewObjs[MAX_VOBJ]; v++)
		SetObjView(v, v->view);

	DrawPic(picNum);

	pPView = pViews;
	pOverlay = overlays;

	for (i = 0;i < totalOverlays;i++)
		OverlayPic(overlays[i]);
	for (i = 0;i < totalPViews;i++)
		AddToPic(pViews[i].view, pViews[i].loop, pViews[i].cel, pViews[i].x, pViews[i].y, pViews[i].pri);

	PIC_VISIBLE = TRUE;
	ShowPic();

	cCancelLine();
	WriteStatusLine();

	ClearControllers();

	SetFlag(fRESTORE);

	code = NULL;

	return TRUE;
}
/*****************************************************************************/
BOOL RestoreLastSavedGameOnBoot(void)
{
	if (!RestoreGameSlotByIndex(0))
		return FALSE;

	/* Boot path starts with fNEWROOM set; clear it so restored state survives
	   the first AGI main-loop tick. */
	ResetFlag(fNEWROOM);
	return TRUE;
}
/*****************************************************************************/
