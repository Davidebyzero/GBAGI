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

/******************************************************************************/
#include "makerom.h"
#include "decompress.h"
#include "commands.h"
#include <conio.h>
#include <ctype.h>
/******************************************************************************/

/******************************************************************************/
GAMEINFO *gi;

DIRENT dirs[4][256];
const char *dirNames[4] = {"logdir","picdir","viewdir","snddir"};

char fname[1024];
char wordbuf[128];

BOOL VUSED[16];
U8 *volData;
U32 volSize;

const char *objNames[256];
char *objNameData;
U8 objRoomsStart[256];

const char *words[26];
U8 *vocabData, *wordData;
U32 wordsSize;

WORDSET *wordset,*pwords;
FILE *fout;
U32 offs, giPos;


int lwfsize;
U8 *lwPtrs[256];
BOOL LOG_DONE[256];
int strSize;   
U8 *code;
/******************************************************************************/
const char *solidWords[] = {
	"look","marry", "take", "button", "phone", "hallway", "drinks", "hole", "cigarette",
	"yes", "no", "wall", "area", "wallet", "earth", "ground", "stairs", "window", "open", "move", "climb", "talk",
	"garbage", "ledge", "under", "over", "drawer", "carpet", "out", "key", "give",
	"lie down", "rub", "touch", "empty", "find", "behind", "door", "clothes", "lock", "break",
	"listen", "table", "shelf", "book", "page", "thank you", "box", "jump", "gnome", "dwarf", "neptune", "man", "grandma", "mermaid", "woman",
	"start", "ignite", "fire", "meter", "lips", "basket", "wine", "food", "steal", "knife",
	"cut", "sink", "sit down", "hit", "knock on", "cover", "bottle", "television", "put on",
	"ashtray", "bill", "pass", "ladder", "capsule", "negotiate", "flower", "elevator",
	"paper", "ken sent me", "feel", "bartender", "sleep", "pills", "powder", "rope", "stake", "shower", "radio", "extender", "laura", "water",
	"ring", "blow up", "release", "booze", "remote control", "whiskey", "bathroom",  "mat",
	"notes", "wash", "candy", "doll", "magazine", "old", "building", "get up", "change",
	"channel", "jukebox", "wipe", "attatch", "clothes line", "decrease", "increase",
	"spray", "bar", "church", "lint", "blackjack", "slots" , "dog", "pole", "rough",
	"plants", "boo", "music", "junk food", "gamble", "lever", "split", "rules",
	"swim suit", "moose", "undress", "dress", "artwork", "fan", "yourself", "fire hydrant",
	"enjoy", "disc jockey", "roof", "weapon", "onto", "person", "watch", "waiter", "spit",
	"shout", "swim", "dive", "under water", "bucket", "leg", "bubbles", "credit card", "pause",
	"suit", "cooler", "information", "cheer", "stop", "inventory", "crack", "crap", "leak",
	"screw", "fart", "boobs", "ass", "dong", "up yours", "business card", "work",
	"computer console", "transporter", "screen", "fuel pump", "close", "catch", "rock",
	"push", "position", "forest", "help", "climb", "ground", "plants", "smell", "stand up",
    "knife", "torch", "soup", "catch", "opening", "bag", "get in", "tree", "walnut", "key", "sun", "light",
    "knight", "pump", "eat", "silence", "name", "bird", "feed", "diamond", "throw", "drop",
    "cast", "fence", "stove", "fly", "move", "woman", "remove", "waterfall", "river", "guitar", "initialize",
    "hat", "chair", "keyhole", "duck", "cloud", "suite", "body", ""
};

#define SG_TOTAL 19
#define SG_WORDMAX 20

const char *solidGroups[SG_TOTAL][SG_WORDMAX] = {
	{"enter","exit", ""},
	{"taste", "eat", "drink", ""},
	{"hug", "kiss", ""},
	{"cabinet","closet","cupboard","wardrobe", ""}, 
	{"bureau","chest","dresser", ""},
	{"purchase", "pay", ""},
	{"car", "taxi", ""},
	{"girl", "woman", ""},
	{"press", "move", ""},
	{"earth", "ground", ""},
	{"sun", "light", ""},
	{"cloud", "sky", ""},
	{"rub", "touch", ""},
	{"book", "bookcase", "book shelf", ""},
	{"dwarf", "man", ""},
	{"neptune", "man", ""},
	{"grandma", "woman", ""},
	{"mermaid", "woman", ""},
	{""}
};
/******************************************************************************/
static const U8 kGbaNintendoLogo[0x9C] = {
	0x24, 0xFF, 0xAE, 0x51, 0x69, 0x9A, 0xA2, 0x21, 0x3D, 0x84, 0x82, 0x0A,
	0x84, 0xE4, 0x09, 0xAD, 0x11, 0x24, 0x8B, 0x98, 0xC0, 0x81, 0x7F, 0x21,
	0xA3, 0x52, 0xBE, 0x19, 0x93, 0x09, 0xCE, 0x20, 0x10, 0x46, 0x4A, 0x4A,
	0xF8, 0x27, 0x31, 0xEC, 0x58, 0xC7, 0xE8, 0x33, 0x82, 0xE3, 0xCE, 0xBF,
	0x85, 0xF4, 0xDF, 0x94, 0xCE, 0x4B, 0x09, 0xC1, 0x94, 0x56, 0x8A, 0xC0,
	0x13, 0x72, 0xA7, 0xFC, 0x9F, 0x84, 0x4D, 0x73, 0xA3, 0xCA, 0x9A, 0x61,
	0x58, 0x97, 0xA3, 0x27, 0xFC, 0x03, 0x98, 0x76, 0x23, 0x1D, 0xC7, 0x61,
	0x03, 0x04, 0xAE, 0x56, 0xBF, 0x38, 0x84, 0x00, 0x40, 0xA7, 0x0E, 0xFD,
	0xFF, 0x52, 0xFE, 0x03, 0x6F, 0x95, 0x30, 0xF1, 0x97, 0xFB, 0xC0, 0x85,
	0x60, 0xD6, 0x80, 0x25, 0xA9, 0x63, 0xBE, 0x03, 0x01, 0x4E, 0x38, 0xE2,
	0xF9, 0xA2, 0x34, 0xFF, 0xBB, 0x3E, 0x03, 0x44, 0x78, 0x00, 0x90, 0xCB,
	0x88, 0x11, 0x3A, 0x94, 0x65, 0xC0, 0x7C, 0x63, 0x87, 0xF0, 0x3C, 0xAF,
	0xD6, 0x25, 0xE4, 0x8B, 0x38, 0x0A, 0xAC, 0x72, 0x21, 0xD4, 0xF8, 0x07
};
static const char *kSaveSignatures[] = {
	"SRAM_V113",
	"SRAM_F_V103",
	"SRAM_F_VA1"
};
#define BATTERYLESS_PAD_TARGET_SIZE (16L*1024L*1024L)
/******************************************************************************/
static U8 CalcGbaHeaderComplement(const U8 *header)
{
	int i;
	int total = 0;
	for(i = 0xA0; i < 0xBD; i++)
		total += header[i];
	return (U8)((-total - 0x19) & 0xFF);
}
/******************************************************************************/
static void FillGbaHeaderText(U8 *dest, int len, const char *text, const char *fallback)
{
	int i = 0;
	memset(dest, 0, len);
	if(!text)
		text = "";
	while(*text && i < len) {
		char ch = *text++;
		if(isalnum((unsigned char)ch) || ch == ' ' || ch == '_' || ch == '-') {
			if(ch >= 'a' && ch <= 'z')
				ch = (char)(ch - 'a' + 'A');
			dest[i++] = (U8)ch;
		}
	}
	for(i = 0; fallback && fallback[i] && i < len && dest[i] == 0; i++)
		dest[i] = (U8)fallback[i];
}
/******************************************************************************/
static BOOL FileContainsTag(FILE *f, const char *tag)
{
	U8 chunk[256];
	size_t readCount;
	size_t tagLen;
	size_t i;

	if(!f || !tag)
		return FALSE;

	tagLen = strlen(tag);
	if(tagLen == 0)
		return FALSE;

	fseek(f, 0, SEEK_SET);
	while((readCount = fread(chunk, 1, sizeof(chunk), f)) > 0) {
		if(readCount < tagLen)
			continue;
		for(i = 0; i + tagLen <= readCount; i++) {
			if(memcmp(chunk + i, tag, tagLen) == 0)
				return TRUE;
		}
	}

	return FALSE;
}
/******************************************************************************/
BOOL FixOutputRomForHardware(const char *filename, const char *titleText)
{
	FILE *f;
	long fileSize;
	U8 header[0xC0];
	U8 zero = 0;
	int sigIndex;

	if(!filename || !*filename)
		return FALSE;

	f = fopen(filename, "r+b");
	if(!f)
		return FALSE;

	if(fread(header, 1, sizeof(header), f) != sizeof(header)) {
		fclose(f);
		return FALSE;
	}

	memcpy(header + 0x04, kGbaNintendoLogo, sizeof(kGbaNintendoLogo));
	FillGbaHeaderText(header + 0xA0, 12, titleText, "GBAGI");
	memcpy(header + 0xAC, "GBAG", 4);
	memcpy(header + 0xB0, "01", 2);
	header[0xB2] = 0x96;
	header[0xBD] = CalcGbaHeaderComplement(header);

	fseek(f, 0, SEEK_SET);
	fwrite(header, 1, sizeof(header), f);

	fseek(f, 0, SEEK_END);
	fileSize = ftell(f);
	for(sigIndex = 0; sigIndex < (int)(sizeof(kSaveSignatures) / sizeof(kSaveSignatures[0])); sigIndex++) {
		const char *sig = kSaveSignatures[sigIndex];
		size_t sigLen = strlen(sig);
		if(FileContainsTag(f, sig))
			continue;
		fseek(f, 0, SEEK_END);
		fwrite(sig, 1, sigLen, f);
		fileSize += (long)sigLen;
		while((fileSize & 3) != 0) {
			fwrite(&zero, 1, 1, f);
			fileSize++;
		}
	}
	if(fileSize < BATTERYLESS_PAD_TARGET_SIZE) {
		fseek(f, 0, SEEK_END);
		while(fileSize < BATTERYLESS_PAD_TARGET_SIZE) {
			fwrite(&zero, 1, 1, f);
			fileSize++;
		}
	}

	fclose(f);
	return TRUE;
}
/******************************************************************************/
static BOOL IsPoliceQuest(void)
{
	return gi && gi->title && strcmp(gi->title, "Police Quest") == 0;
}
/******************************************************************************/
static BOOL IsKingsQuest1(void)
{
	return gi && gi->title && strcmp(gi->title, "King's Quest 1") == 0;
}
/******************************************************************************/
static BOOL IsKingsQuest2(void)
{
	return gi && gi->title && strcmp(gi->title, "King's Quest 2") == 0;
}
/******************************************************************************/
static BOOL IsKingsQuest3(void)
{
	return gi && gi->title && strcmp(gi->title, "King's Quest 3") == 0;
}
/******************************************************************************/
static BOOL IsKingsQuest4(void)
{
	return gi && gi->title && strcmp(gi->title, "King's Quest 4") == 0;
}
/******************************************************************************/
static BOOL IsGoldRush(void)
{
	return gi && gi->title && strcmp(gi->title, "Gold Rush!") == 0;
}
/******************************************************************************/
static BOOL IsSpaceQuest2(void)
{
	return gi && gi->title &&
		((strcmp(gi->title, "Space Quest 2") == 0) ||
		 (strcmp(gi->title, "Space Quest II") == 0));
}
/******************************************************************************/
static BOOL IsSpaceQuest1(void)
{
	return gi && gi->title &&
		((strcmp(gi->title, "Space Quest 1") == 0) ||
		 (strcmp(gi->title, "Space Quest I") == 0) ||
		 (strncmp(gi->title, "Space Quest", 11) == 0 &&
		  strstr(gi->title, "0") == NULL &&
		  strstr(gi->title, "2") == NULL));
}
/******************************************************************************/
static void RemoveAddedWord(const char *string)
{
	WORDSET *w = wordset;
	if(!w || !pwords)
		return;

	while(w < pwords) {
		if(w->group && w->string && strcmp(w->string, string) == 0) {
			WORDSET *next = w + 1;
			while(next < pwords) {
				*(next - 1) = *next;
				next++;
			}
			pwords--;
			memset(pwords, 0, sizeof(*pwords));
			continue;
		}
		w++;
	}
}
/******************************************************************************/
static BOOL HasAddedWord(int group, const char *string)
{
	WORDSET *w = wordset;
	if(!w || !pwords || !string)
		return FALSE;
	while(w < pwords) {
		if(w->group == group && w->string && strcmp(w->string, string) == 0)
			return TRUE;
		w++;
	}
	return FALSE;
}
/******************************************************************************/
int CountGameSpecificAliases()
{
	if(IsKingsQuest1())
		return 128;
	if(IsKingsQuest2())
		return 96;
	if(IsKingsQuest3())
		return 192;
	if(IsKingsQuest4())
		return 320;
	if(IsGoldRush())
		return 384;
	if(IsSpaceQuest2())
		return 1024;
	if(IsSpaceQuest1())
		return 320;
	if(IsPoliceQuest())
		return 320;
	return 0;
}
/******************************************************************************/
void DoGameSpecificAliases()
{
	if(IsKingsQuest1()) {
		AddWord(2, "examine");
		AddWord(2, "look");
		ClearGroup(2);
		AddWord(3, "swim");
		ClearGroup(3);
		AddWord(5, "get");
		AddWord(5, "take");
		ClearGroup(5);
		AddWord(9, "tree");
		ClearGroup(9);
		AddWord(11, "bush");
		AddWord(11, "plants");
		ClearGroup(11);
		AddWord(12, "alligator");
		ClearGroup(12);
		AddWord(16, "blossoms");
		AddWord(16, "flower");
		ClearGroup(16);
		AddWord(21, "building");
		AddWord(21, "house");
		ClearGroup(21);
		AddWord(24, "dagger");
		ClearGroup(24);
		AddWord(27, "chicken broth");
		AddWord(27, "soup");
		ClearGroup(27);
		AddWord(29, "torch");
		ClearGroup(29);
		AddWord(37, "boulder");
		AddWord(37, "rock");
		ClearGroup(37);
		AddWord(40, "give");
		AddWord(40, "present");
		ClearGroup(40);
		AddWord(48, "lion");
		ClearGroup(48);
		AddWord(51, "bag");
		AddWord(51, "pouch");
		ClearGroup(51);
		AddWord(55, "banner");
		AddWord(55, "flag");
		ClearGroup(55);
		AddWord(58, "dwarf");
		AddWord(58, "enchanter");
		AddWord(58, "gnome");
		AddWord(58, "man");
		ClearGroup(58);
		AddWord(61, "carrot");
		ClearGroup(61);
		AddWord(62, "get in");
		ClearGroup(62);
		AddWord(67, "mountain");
		ClearGroup(67);
		AddWord(69, "branch");
		AddWord(69, "log");
		ClearGroup(69);
		AddWord(74, "fourleaf clover");
		ClearGroup(74);
		AddWord(75, "key");
		ClearGroup(75);
		AddWord(76, "cave");
		AddWord(76, "chamber");
		ClearGroup(76);
		AddWord(80, "attack");
		AddWord(80, "hit");
		ClearGroup(80);
		AddWord(82, "armor");
		AddWord(82, "knight");
		ClearGroup(82);
		AddWord(86, "axe");
		ClearGroup(86);
		AddWord(89, "slingshot");
		ClearGroup(89);
		AddWord(90, "gold nut");
		AddWord(90, "walnut");
		ClearGroup(90);
		AddWord(94, "wrote");
		ClearGroup(94);
		AddWord(95, "stairs");
		AddWord(95, "steps");
		ClearGroup(95);
		AddWord(97, "mushroom");
		AddWord(97, "toadstool");
		ClearGroup(97);
		AddWord(98, "stalactite");
		ClearGroup(98);
		AddWord(101, "cabinet");
		AddWord(101, "cupboard");
		ClearGroup(101);
		AddWord(108, "egg");
		ClearGroup(108);
		AddWord(115, "bird nest");
		AddWord(115, "nest");
		ClearGroup(115);
		AddWord(118, "bird");
		AddWord(118, "eagle");
		ClearGroup(118);
		AddWord(122, "beans");
		ClearGroup(122);
		AddWord(124, "assist");
		AddWord(124, "help");
		ClearGroup(124);
		AddWord(126, "feed");
		AddWord(126, "feed soup");
		ClearGroup(126);
		AddWord(127, "diamond");
		ClearGroup(127);
		AddWord(129, "cheese");
		ClearGroup(129);
		AddWord(132, "graham");
		ClearGroup(132);
		AddWord(149, "hello");
		ClearGroup(149);
		AddWord(164, "fly");
		ClearGroup(164);
		AddWord(168, "move");
		AddWord(168, "push");
		ClearGroup(168);
		AddWord(169, "rumpelstiltskin");
		ClearGroup(169);
		AddWord(178, "fairy");
		AddWord(178, "godmother");
		AddWord(178, "woman");
		ClearGroup(178);
		AddWord(195, "fiddle");
		ClearGroup(195);
		AddWord(196, "leprechaun");
		ClearGroup(196);
		AddWord(199, "gold");
		ClearGroup(199);
		AddWord(203, "look in");
		ClearGroup(203);
		AddWord(204, "thank");
		ClearGroup(204);
		AddWord(208, "king");
		ClearGroup(208);
		AddWord(210, "initialize");
		ClearGroup(210);
		AddWord(213, "coin");
		ClearGroup(213);
		AddWord(219, "chair");
		ClearGroup(219);
		AddWord(225, "stump");
		ClearGroup(225);
		AddWord(230, "keyhole");
		ClearGroup(230);
		AddWord(242, "finger");
		ClearGroup(242);
		AddWord(247, "mikel knight");
		ClearGroup(247);
		AddWord(252, "curtain");
		ClearGroup(252);
		AddWord(257, "beaver");
		ClearGroup(257);
		RemoveAddedWord("acquire");
		RemoveAddedWord("check");
		RemoveAddedWord("swimming");
		RemoveAddedWord("knife");
		RemoveAddedWord("guitar");
		RemoveAddedWord("trees");
		RemoveAddedWord("alligators");
		RemoveAddedWord("bushes");
		RemoveAddedWord("broth");
		RemoveAddedWord("boulders");
		RemoveAddedWord("offer");
		RemoveAddedWord("lions");
		RemoveAddedWord("bag of jewels");
		RemoveAddedWord("banners");
		RemoveAddedWord("carrots");
		RemoveAddedWord("mountains");
		RemoveAddedWord("clover");
		RemoveAddedWord("four leaf clover");
		RemoveAddedWord("cavern");
		RemoveAddedWord("fight");
		RemoveAddedWord("kinghts");
		RemoveAddedWord("ax");
		RemoveAddedWord("sling");
		RemoveAddedWord("gold walnut");
		RemoveAddedWord("programmed");
		RemoveAddedWord("stairway");
		RemoveAddedWord("mushrooms");
		RemoveAddedWord("stalactites");
		RemoveAddedWord("gold egg");
		RemoveAddedWord("birds nest");
		RemoveAddedWord("big bird");
		RemoveAddedWord("bean");
		RemoveAddedWord("aid");
		RemoveAddedWord("feed broth");
		RemoveAddedWord("diamonds");
		RemoveAddedWord("grahams");
		RemoveAddedWord("rumplestiltskin");
		RemoveAddedWord("fairy godmother");
		RemoveAddedWord("leprechauns");
		RemoveAddedWord("golden");
		RemoveAddedWord("look inside");
		RemoveAddedWord("thank you");
		RemoveAddedWord("king edward");
		RemoveAddedWord("format");
		RemoveAddedWord("init");
		RemoveAddedWord("coins");
		RemoveAddedWord("chairs");
		RemoveAddedWord("stumps");
		RemoveAddedWord("key hole");
		RemoveAddedWord("fingers");
		RemoveAddedWord("mikel");
		RemoveAddedWord("curtains");
		RemoveAddedWord("beavers");
	}

	if(IsKingsQuest2()) {
		AddWord(2, "examine");
		AddWord(2, "look");
		ClearGroup(2);
		AddWord(3, "area");
		AddWord(3, "around");
		AddWord(3, "cavern");
		AddWord(3, "forest");
		AddWord(3, "hallway");
		ClearGroup(3);
		AddWord(5, "get");
		AddWord(5, "take");
		ClearGroup(5);
		AddWord(11, "bushes");
		AddWord(11, "plants");
		ClearGroup(11);
		AddWord(18, "girl");
		AddWord(18, "lady");
		AddWord(18, "mermaid");
		AddWord(18, "woman");
		ClearGroup(18);
		AddWord(21, "building");
		AddWord(21, "house");
		AddWord(21, "palace");
		ClearGroup(21);
		AddWord(23, "box");
		AddWord(23, "mailbox");
		ClearGroup(23);
		AddWord(31, "restart");
		ClearGroup(31);
		AddWord(32, "cross and chain");
		ClearGroup(32);
		AddWord(37, "boulder");
		AddWord(37, "rock");
		ClearGroup(37);
		AddWord(47, "trident");
		ClearGroup(47);
		AddWord(48, "sea horse");
		ClearGroup(48);
		AddWord(50, "thorns");
		ClearGroup(50);
		AddWord(51, "lion");
		ClearGroup(51);
		AddWord(52, "ghost");
		ClearGroup(52);
		AddWord(58, "dracula");
		AddWord(58, "dwarf");
		AddWord(58, "king");
		AddWord(58, "man");
		AddWord(58, "neptune");
		AddWord(58, "wizard");
		ClearGroup(58);
		AddWord(59, "clam");
		AddWord(59, "shell");
		ClearGroup(59);
		AddWord(60, "bracelet");
		ClearGroup(60);
		AddWord(65, "boat");
		ClearGroup(65);
		AddWord(69, "branch");
		AddWord(69, "log");
		ClearGroup(69);
		AddWord(70, "necklace");
		ClearGroup(70);
		AddWord(72, "hammer");
		ClearGroup(72);
		AddWord(77, "skull");
		ClearGroup(77);
		AddWord(86, "bridle");
		ClearGroup(86);
		AddWord(94, "wrote");
		ClearGroup(94);
		AddWord(98, "tiara");
		ClearGroup(98);
		AddWord(106, "coffin");
		ClearGroup(106);
		AddWord(111, "lamp");
		ClearGroup(111);
		AddWord(116, "birdcage");
		ClearGroup(116);
		AddWord(127, "antique");
		ClearGroup(127);
		AddWord(129, "cape");
		AddWord(129, "cloak");
		ClearGroup(129);
		AddWord(132, "graham");
		ClearGroup(132);
		AddWord(141, "save game");
		ClearGroup(141);
		AddWord(144, "cliff");
		AddWord(144, "mountain");
		ClearGroup(144);
		AddWord(147, "restore game");
		ClearGroup(147);
		AddWord(149, "hello");
		ClearGroup(149);
		AddWord(152, "bouquet");
		ClearGroup(152);
		AddWord(163, "suntan");
		ClearGroup(163);
		AddWord(167, "wave");
		ClearGroup(167);
		AddWord(186, "book shelf");
		ClearGroup(186);
		AddWord(189, "horse");
		ClearGroup(189);
		AddWord(199, "gold");
		ClearGroup(199);
		AddWord(203, "look in");
		ClearGroup(203);
		AddWord(204, "thank");
		ClearGroup(204);
		AddWord(208, "flower box");
		ClearGroup(208);
		AddWord(225, "stump");
		ClearGroup(225);
		AddWord(248, "pause game");
		ClearGroup(248);

		RemoveAddedWord("restart game");
		RemoveAddedWord("cross");
		RemoveAddedWord("rusty trident");
		RemoveAddedWord("magic seahorse");
		RemoveAddedWord("brambles");
		RemoveAddedWord("briars");
		RemoveAddedWord("hungry lion");
		RemoveAddedWord("ghosts");
		RemoveAddedWord("clam shell");
		RemoveAddedWord("diamond and sapphire bracelet");
		RemoveAddedWord("rowboat");
		RemoveAddedWord("branches");
		RemoveAddedWord("diamond and sapphire necklace");
		RemoveAddedWord("diamond necklace");
		RemoveAddedWord("mallet");
		RemoveAddedWord("skulls");
		RemoveAddedWord("leather bridle");
		RemoveAddedWord("programmed");
		RemoveAddedWord("diamond and sapphire tiara");
		RemoveAddedWord("diamond tiara");
		RemoveAddedWord("casket");
		RemoveAddedWord("magic lamp");
		RemoveAddedWord("bird cage");
		RemoveAddedWord("antiques");
		RemoveAddedWord("black cape");
		RemoveAddedWord("black cloak");
		RemoveAddedWord("graham's");
		RemoveAddedWord("rescue");
		RemoveAddedWord("save");
		RemoveAddedWord("cliffs");
		RemoveAddedWord("restore");
		RemoveAddedWord("hi");
		RemoveAddedWord("bouquet of flowers");
		RemoveAddedWord("tan");
		RemoveAddedWord("waves");
		RemoveAddedWord("bookcase");
		RemoveAddedWord("flying horse");
		RemoveAddedWord("golden");
		RemoveAddedWord("look inside");
		RemoveAddedWord("thank you");
		RemoveAddedWord("window box");
		RemoveAddedWord("stumps");
		RemoveAddedWord("pause");
	}

	if(IsKingsQuest3()) {
		AddWord(2, "examine");
		AddWord(2, "look");
		ClearGroup(2);
		AddWord(3, "cast");
		AddWord(3, "do");
		ClearGroup(3);
		AddWord(16, "banner");
		AddWord(16, "flag");
		ClearGroup(16);
		AddWord(18, "bye");
		ClearGroup(18);
		AddWord(28, "laboratory");
		ClearGroup(28);
		AddWord(31, "earth");
		AddWord(31, "floor");
		AddWord(31, "ground");
		ClearGroup(31);
		AddWord(34, "glass");
		AddWord(34, "window");
		ClearGroup(34);
		AddWord(37, "move");
		AddWord(37, "push");
		ClearGroup(37);
		AddWord(40, "wizard");
		ClearGroup(40);
		AddWord(41, "say");
		AddWord(41, "talk");
		ClearGroup(41);
		AddWord(45, "bed");
		ClearGroup(45);
		AddWord(49, "treasure");
		ClearGroup(49);
		AddWord(50, "cabinet");
		AddWord(50, "cupboard");
		ClearGroup(50);
		AddWord(56, "add");
		AddWord(56, "drop");
		AddWord(56, "put");
		ClearGroup(56);
		AddWord(60, "empty");
		AddWord(60, "pour");
		AddWord(60, "remove");
		ClearGroup(60);
		AddWord(61, "picture");
		AddWord(61, "tapestry");
		ClearGroup(61);
		AddWord(68, "all");
		AddWord(68, "inventory");
		AddWord(68, "items");
		ClearGroup(68);
		AddWord(71, "bookshelf");
		ClearGroup(71);
		AddWord(80, "fireplace");
		ClearGroup(80);
		AddWord(85, "kettle");
		AddWord(85, "pan");
		AddWord(85, "pot");
		ClearGroup(85);
		AddWord(86, "nightstand");
		ClearGroup(86);
		AddWord(89, "churn");
		ClearGroup(89);
		AddWord(91, "beam");
		ClearGroup(91);
		AddWord(92, "herb");
		AddWord(92, "spice");
		ClearGroup(92);
		AddWord(95, "apple");
		AddWord(95, "fruit");
		AddWord(95, "grapes");
		ClearGroup(95);
		AddWord(97, "drink");
		AddWord(97, "eat");
		ClearGroup(97);
		AddWord(107, "poison");
		ClearGroup(107);
		AddWord(111, "beaker");
		AddWord(111, "flask");
		ClearGroup(111);
		AddWord(112, "bone");
		AddWord(112, "fishbone");
		AddWord(112, "skull");
		ClearGroup(112);
		AddWord(113, "equipment");
		AddWord(113, "instruments");
		ClearGroup(113);
		AddWord(115, "compound");
		AddWord(115, "dough");
		AddWord(115, "mixture");
		ClearGroup(115);
		AddWord(116, "capsule");
		AddWord(116, "vial");
		ClearGroup(116);
		AddWord(118, "nightshade");
		ClearGroup(118);
		AddWord(120, "mushroom");
		AddWord(120, "toadstool");
		ClearGroup(120);
		AddWord(123, "mortar");
		ClearGroup(123);
		AddWord(124, "brazier");
		ClearGroup(124);
		AddWord(125, "crumble");
		AddWord(125, "crush");
		AddWord(125, "grind");
		ClearGroup(125);
		AddWord(127, "coal");
		ClearGroup(127);
		AddWord(129, "cup");
		ClearGroup(129);
		AddWord(130, "ball");
		ClearGroup(130);
		AddWord(131, "brew");
		ClearGroup(131);
		AddWord(135, "skin");
		AddWord(135, "snakeskin");
		ClearGroup(135);
		AddWord(138, "hand");
		AddWord(138, "handful");
		ClearGroup(138);
		AddWord(149, "alexander");
		AddWord(149, "boy");
		AddWord(149, "me");
		ClearGroup(149);
		AddWord(150, "salt");
		ClearGroup(150);
		AddWord(152, "rock");
		AddWord(152, "stone");
		ClearGroup(152);
		AddWord(156, "press");
		AddWord(156, "squash");
		ClearGroup(156);
		AddWord(159, "cookie");
		ClearGroup(159);
		AddWord(162, "pinch");
		ClearGroup(162);
		AddWord(164, "release");
		AddWord(164, "untie");
		ClearGroup(164);
		AddWord(167, "cactus");
		ClearGroup(167);
		AddWord(171, "spell");
		ClearGroup(171);
		AddWord(174, "look in");
		ClearGroup(174);
		AddWord(177, "pouch");
		ClearGroup(177);
		AddWord(180, "fish");
		ClearGroup(180);
		AddWord(187, "coin");
		ClearGroup(187);
		AddWord(188, "furniture");
		ClearGroup(188);
		AddWord(190, "magic");
		ClearGroup(190);
		AddWord(193, "cat");
		ClearGroup(193);
		AddWord(195, "storm");
		ClearGroup(195);
		AddWord(196, "shop");
		ClearGroup(196);
		AddWord(200, "boat");
		ClearGroup(200);
		AddWord(201, "feather");
		ClearGroup(201);
		AddWord(232, "wake");
		ClearGroup(232);
		AddWord(233, "knob");
		ClearGroup(233);
		AddWord(234, "wrote");
		ClearGroup(234);
		AddWord(239, "barrel");
		ClearGroup(239);
		AddWord(240, "crowsnest");
		ClearGroup(240);
		AddWord(246, "cliff");
		AddWord(246, "hill");
		AddWord(246, "mountain");
		ClearGroup(246);
		AddWord(252, "play");
		ClearGroup(252);
		AddWord(257, "river");
		AddWord(257, "waterfall");
		ClearGroup(257);
		AddWord(258, "bear");
		ClearGroup(258);
		AddWord(262, "web");
		ClearGroup(262);
		AddWord(263, "cave");
		ClearGroup(263);
		AddWord(264, "spider");
		ClearGroup(264);
		AddWord(268, "ocean");
		AddWord(268, "sea");
		ClearGroup(268);
		AddWord(274, "crystal ball");
		ClearGroup(274);
		AddWord(277, "rob");
		AddWord(277, "steal");
		ClearGroup(277);
		AddWord(278, "ale");
		AddWord(278, "beer");
		AddWord(278, "booze");
		AddWord(278, "liquor");
		AddWord(278, "wine");
		ClearGroup(278);
		AddWord(281, "sword");
		ClearGroup(281);
		AddWord(284, "bandit");
		AddWord(284, "captain");
		AddWord(284, "dwarf");
		AddWord(284, "ghost");
		AddWord(284, "gnome");
		AddWord(284, "king");
		AddWord(284, "man");
		AddWord(284, "pirate");
		AddWord(284, "rogue");
		AddWord(284, "sailor");
		AddWord(284, "sentry");
		ClearGroup(284);
		AddWord(285, "barmaid");
		AddWord(285, "girl");
		AddWord(285, "medusa");
		AddWord(285, "princess");
		AddWord(285, "queen");
		AddWord(285, "statue");
		AddWord(285, "woman");
		ClearGroup(285);
		AddWord(286, "animal");
		AddWord(286, "squirrel");
		ClearGroup(286);
		AddWord(287, "lizard");
		ClearGroup(287);
		AddWord(298, "find");
		AddWord(298, "search");
		ClearGroup(298);
		AddWord(300, "hello");
		ClearGroup(300);
		AddWord(301, "dragon");
		AddWord(301, "monster");
		AddWord(301, "snowman");
		ClearGroup(301);
		AddWord(401, "mouse");
		AddWord(401, "rat");
		ClearGroup(401);
		AddWord(402, "shark");
		ClearGroup(402);

		RemoveAddedWord("feel");
		RemoveAddedWord("banners");
		RemoveAddedWord("good bye");
		RemoveAddedWord("lab");
		RemoveAddedWord("window");
		RemoveAddedWord("enchanter");
		RemoveAddedWord("magician");
		RemoveAddedWord("beds");
		RemoveAddedWord("treasure chest");
		RemoveAddedWord("closet");
		RemoveAddedWord("wardrobe");
		RemoveAddedWord("drapery");
		RemoveAddedWord("bookcase");
		RemoveAddedWord("hearth");
		RemoveAddedWord("chamber pot");
		RemoveAddedWord("vanity");
		RemoveAddedWord("butter churn");
		RemoveAddedWord("beams");
		RemoveAddedWord("herbs");
		RemoveAddedWord("taste");
		RemoveAddedWord("beakers");
		RemoveAddedWord("bones");
		RemoveAddedWord("implements");
		RemoveAddedWord("toad stool");
		RemoveAddedWord("mortar and pestle");
		RemoveAddedWord("burner");
		RemoveAddedWord("charcoal");
		RemoveAddedWord("empty cup");
		RemoveAddedWord("balls");
		RemoveAddedWord("storm brew");
		RemoveAddedWord("reptile skin");
		RemoveAddedWord("fingers");
		RemoveAddedWord("grains of salt");
		RemoveAddedWord("patty");
		RemoveAddedWord("bit");
		RemoveAddedWord("cactii");
		RemoveAddedWord("spells");
		RemoveAddedWord("look inside");
		RemoveAddedWord("pouches");
		RemoveAddedWord("fishes");
		RemoveAddedWord("coins");
		RemoveAddedWord("furnishings");
		RemoveAddedWord("magical");
		RemoveAddedWord("kitty");
		RemoveAddedWord("storms");
		RemoveAddedWord("store");
		RemoveAddedWord("lifeboat");
		RemoveAddedWord("feathers");
		RemoveAddedWord("wake up");
		RemoveAddedWord("knobs");
		RemoveAddedWord("programmed");
		RemoveAddedWord("barrels");
		RemoveAddedWord("crows nest");
		RemoveAddedWord("bluff");
		RemoveAddedWord("bluffs");
		RemoveAddedWord("playing");
		RemoveAddedWord("bears");
		RemoveAddedWord("spiderweb");
		RemoveAddedWord("cavern");
		RemoveAddedWord("spiders");
		RemoveAddedWord("bay");
		RemoveAddedWord("crystal");
		RemoveAddedWord("swords");
		RemoveAddedWord("animals");
		RemoveAddedWord("lizards");
		RemoveAddedWord("hi");
		RemoveAddedWord("mice");
		RemoveAddedWord("sharks");
	}

	if(IsKingsQuest4()) {
		AddWord(8, "rock");
		AddWord(8, "stone");
		ClearGroup(8);
		AddWord(2, "examine");
		AddWord(2, "look");
		ClearGroup(2);
		AddWord(3, "get");
		AddWord(3, "take");
		ClearGroup(3);
		AddWord(12, "grass");
		ClearGroup(12);
		AddWord(13, "bush");
		ClearGroup(13);
		AddWord(25, "all");
		AddWord(25, "inventory");
		AddWord(25, "items");
		ClearGroup(25);
		AddWord(27, "mantelpiece");
		ClearGroup(27);
		AddWord(33, "gold");
		ClearGroup(33);
		AddWord(36, "crumbs");
		ClearGroup(36);
		AddWord(43, "look in");
		ClearGroup(43);
		AddWord(48, "fish");
		ClearGroup(48);
		AddWord(66, "give");
		AddWord(66, "offer");
		AddWord(66, "present");
		ClearGroup(66);
		AddWord(67, "lute");
		ClearGroup(67);
		AddWord(68, "cupid");
		ClearGroup(68);
		AddWord(69, "arrow");
		AddWord(69, "bow");
		ClearGroup(69);
		AddWord(74, "wade");
		ClearGroup(74);
		AddWord(78, "column");
		ClearGroup(78);
		AddWord(82, "cottage");
		AddWord(82, "home");
		AddWord(82, "house");
		ClearGroup(82);
		AddWord(83, "cockatoo");
		AddWord(83, "parrot");
		ClearGroup(83);
		AddWord(85, "wood");
		ClearGroup(85);
		AddWord(88, "unlock");
		ClearGroup(88);
		AddWord(92, "skeleton");
		ClearGroup(92);
		AddWord(101, "branch");
		ClearGroup(101);
		AddWord(102, "axe");
		ClearGroup(102);
		AddWord(103, "cut");
		AddWord(103, "hack");
		ClearGroup(103);
		AddWord(105, "arm");
		ClearGroup(105);
		AddWord(107, "creepy");
		ClearGroup(107);
		AddWord(111, "peacock");
		ClearGroup(111);
		AddWord(113, "cliff");
		AddWord(113, "mountain");
		ClearGroup(113);
		AddWord(114, "palm");
		ClearGroup(114);
		AddWord(115, "hello");
		ClearGroup(115);
		AddWord(118, "barrel");
		ClearGroup(118);
		AddWord(123, "place");
		AddWord(123, "put");
		AddWord(123, "set");
		ClearGroup(123);
		AddWord(124, "worm");
		ClearGroup(124);
		AddWord(127, "cute");
		AddWord(127, "pretty");
		ClearGroup(127);
		AddWord(130, "dwarf");
		AddWord(130, "giant");
		AddWord(130, "man");
		AddWord(130, "minstrel");
		AddWord(130, "ogre");
		AddWord(130, "troll");
		ClearGroup(130);
		AddWord(132, "sing");
		ClearGroup(132);
		AddWord(137, "coconut");
		ClearGroup(137);
		AddWord(138, "cemetary");
		ClearGroup(138);
		AddWord(139, "grave");
		ClearGroup(139);
		AddWord(140, "gravestone");
		ClearGroup(140);
		AddWord(141, "lion");
		AddWord(141, "monument");
		AddWord(141, "statue");
		ClearGroup(141);
		AddWord(146, "move");
		AddWord(146, "pull");
		AddWord(146, "push");
		ClearGroup(146);
		AddWord(147, "rattle");
		ClearGroup(147);
		AddWord(150, "pouch of diamonds");
		ClearGroup(150);
		AddWord(151, "bag of gold");
		ClearGroup(151);
		AddWord(153, "badge");
		AddWord(153, "medal");
		ClearGroup(153);
		AddWord(160, "picture");
		AddWord(160, "portrait");
		ClearGroup(160);
		AddWord(161, "ghoul");
		AddWord(161, "ghouls");
		AddWord(161, "monster");
		AddWord(161, "zombie");
		ClearGroup(161);
		AddWord(164, "charm");
		ClearGroup(164);
		AddWord(168, "tower");
		ClearGroup(168);
		AddWord(171, "ghost");
		AddWord(171, "spirit");
		ClearGroup(171);
		AddWord(172, "seagull");
		ClearGroup(172);
		AddWord(173, "lilies");
		ClearGroup(173);
		AddWord(174, "pad");
		ClearGroup(174);
		AddWord(179, "crown");
		ClearGroup(179);
		AddWord(207, "fairy");
		ClearGroup(207);
		AddWord(213, "feather");
		ClearGroup(213);
		AddWord(217, "crone");
		AddWord(217, "genesta");
		AddWord(217, "girl");
		AddWord(217, "hag");
		AddWord(217, "lady");
		AddWord(217, "witch");
		AddWord(217, "woman");
		ClearGroup(217);
		AddWord(223, "bed");
		ClearGroup(223);
		AddWord(226, "shipwreck");
		ClearGroup(226);
		AddWord(227, "boat");
		ClearGroup(227);
		AddWord(229, "dolphin");
		ClearGroup(229);
		AddWord(230, "whistle");
		ClearGroup(230);
		AddWord(232, "whale");
		ClearGroup(232);
		AddWord(233, "molar");
		AddWord(233, "tooth");
		ClearGroup(233);
		AddWord(236, "kick");
		AddWord(236, "punch");
		ClearGroup(236);
		AddWord(245, "bone");
		ClearGroup(245);
		AddWord(246, "bye");
		ClearGroup(246);
		AddWord(247, "rib");
		ClearGroup(247);
		AddWord(256, "chamber");
		ClearGroup(256);
		AddWord(257, "room");
		ClearGroup(257);
		AddWord(259, "cat");
		ClearGroup(259);
		AddWord(263, "candle");
		ClearGroup(263);
		AddWord(270, "chicken");
		ClearGroup(270);
		AddWord(274, "magic");
		ClearGroup(274);
		AddWord(278, "fireplace");
		ClearGroup(278);
		AddWord(280, "seven");
		ClearGroup(280);
		AddWord(281, "pillow");
		ClearGroup(281);
		AddWord(286, "dishes");
		ClearGroup(286);
		AddWord(295, "dough");
		ClearGroup(295);
		AddWord(298, "swan");
		ClearGroup(298);
		AddWord(299, "brew");
		ClearGroup(299);
		AddWord(300, "glass eye");
		ClearGroup(300);
		AddWord(306, "cobweb");
		AddWord(306, "web");
		ClearGroup(306);
		AddWord(319, "tapestry");
		ClearGroup(319);
		AddWord(324, "entry");
		AddWord(324, "hallway");
		ClearGroup(324);
		AddWord(334, "shovel");
		ClearGroup(334);
		AddWord(338, "couch");
		ClearGroup(338);
		AddWord(347, "epitaph");
		AddWord(347, "inscription");
		ClearGroup(347);
		AddWord(349, "pandora");
		ClearGroup(349);
		AddWord(356, "fruit");
		ClearGroup(356);
		AddWord(361, "rose");
		ClearGroup(361);
		AddWord(365, "wake");
		ClearGroup(365);
		AddWord(371, "wrote");
		ClearGroup(371);
		AddWord(375, "amulet");
		ClearGroup(375);
		AddWord(376, "squirrel");
		ClearGroup(376);
		AddWord(396, "chain");
		ClearGroup(396);
		AddWord(398, "coin");
		ClearGroup(398);
		AddWord(404, "letter");
		AddWord(404, "paper");
		ClearGroup(404);
		AddWord(439, "screw");
		AddWord(439, "shit");
		ClearGroup(439);

		RemoveAddedWord("green grass");
		RemoveAddedWord("bushes");
		RemoveAddedWord("mantel");
		RemoveAddedWord("golden");
		RemoveAddedWord("crumb");
		RemoveAddedWord("look inside");
		RemoveAddedWord("dead fish");
		RemoveAddedWord("wood lute");
		RemoveAddedWord("baby");
		RemoveAddedWord("arrows");
		RemoveAddedWord("wade in");
		RemoveAddedWord("columns");
		RemoveAddedWord("cockatoos");
		RemoveAddedWord("wooden");
		RemoveAddedWord("unlatch");
		RemoveAddedWord("james");
		RemoveAddedWord("branches");
		RemoveAddedWord("ax");
		RemoveAddedWord("scary");
		RemoveAddedWord("cliffs");
		RemoveAddedWord("hi");
		RemoveAddedWord("barrels");
		RemoveAddedWord("earthworm");
		RemoveAddedWord("glittering");
		RemoveAddedWord("hum");
		RemoveAddedWord("cemetery");
		RemoveAddedWord("graves");
		RemoveAddedWord("gravestones");
		RemoveAddedWord("baby rattle");
		RemoveAddedWord("pouch");
		RemoveAddedWord("bag");
		RemoveAddedWord("badge of honor");
		RemoveAddedWord("pictures");
		RemoveAddedWord("obsidian charm");
		RemoveAddedWord("towers");
		RemoveAddedWord("ghosts");
		RemoveAddedWord("gull");
		RemoveAddedWord("gulls");
		RemoveAddedWord("seagulls");
		RemoveAddedWord("lily");
		RemoveAddedWord("pads");
		RemoveAddedWord("gold crown");
		RemoveAddedWord("fairies");
		RemoveAddedWord("peacock feather");
		RemoveAddedWord("beds");
		RemoveAddedWord("shipwrecks");
		RemoveAddedWord("boats");
		RemoveAddedWord("dolphins");
		RemoveAddedWord("silver whistle");
		RemoveAddedWord("whale's");
		RemoveAddedWord("molars");
		RemoveAddedWord("hit");
		RemoveAddedWord("good bye");
		RemoveAddedWord("ribs");
		RemoveAddedWord("bed chamber");
		RemoveAddedWord("stone room");
		RemoveAddedWord("feline");
		RemoveAddedWord("candles");
		RemoveAddedWord("hen");
		RemoveAddedWord("magical");
		RemoveAddedWord("hearth");
		RemoveAddedWord("seven little");
		RemoveAddedWord("pillows");
		RemoveAddedWord("dish");
		RemoveAddedWord("bread dough");
		RemoveAddedWord("swans");
		RemoveAddedWord("broth");
		RemoveAddedWord("eye");
		RemoveAddedWord("cobwebs");
		RemoveAddedWord("entryway");
		RemoveAddedWord("hall");
		RemoveAddedWord("tapestries");
		RemoveAddedWord("broken shovel");
		RemoveAddedWord("sofa");
		RemoveAddedWord("epitaphs");
		RemoveAddedWord("pandora's");
		RemoveAddedWord("magic fruit");
		RemoveAddedWord("red rose");
		RemoveAddedWord("awaken");
		RemoveAddedWord("programmed");
		RemoveAddedWord("magic amulet");
		RemoveAddedWord("squirrels");
		RemoveAddedWord("chains");
		RemoveAddedWord("coins");
	}

	if(IsGoldRush()) {
		AddWord(4, "account");
		ClearGroup(4);
		AddWord(6, "animal");
		ClearGroup(6);
		AddWord(9, "antler");
		ClearGroup(9);
		AddWord(12, "axe");
		ClearGroup(12);
		AddWord(19, "boat");
		AddWord(19, "ship");
		ClearGroup(19);
		AddWord(20, "box");
		AddWord(20, "crate");
		ClearGroup(20);
		AddWord(29, "clip");
		ClearGroup(29);
		AddWord(30, "chain");
		ClearGroup(30);
		AddWord(32, "coins");
		ClearGroup(32);
		AddWord(36, "california");
		ClearGroup(36);
		AddWord(37, "cape horn");
		ClearGroup(37);
		AddWord(40, "graveyard");
		ClearGroup(40);
		AddWord(42, "candle");
		ClearGroup(42);
		AddWord(44, "clippings");
		ClearGroup(44);
		AddWord(47, "dock");
		ClearGroup(47);
		AddWord(61, "enter");
		AddWord(61, "get in");
		ClearGroup(61);
		AddWord(62, "engine");
		ClearGroup(62);
		AddWord(65, "entrance");
		ClearGroup(65);
		AddWord(67, "eleven");
		ClearGroup(67);
		AddWord(81, "fort");
		ClearGroup(81);
		AddWord(84, "fruit");
		AddWord(84, "orange");
		ClearGroup(84);
		AddWord(91, "grass");
		ClearGroup(91);
		AddWord(95, "gauge");
		ClearGroup(95);
		AddWord(96, "grave");
		ClearGroup(96);
		AddWord(99, "guard");
		ClearGroup(99);
		AddWord(100, "gun");
		ClearGroup(100);
		AddWord(105, "hello");
		ClearGroup(105);
		AddWord(106, "house");
		ClearGroup(106);
		AddWord(113, "horn");
		ClearGroup(113);
		AddWord(114, "horse");
		ClearGroup(114);
		AddWord(122, "island");
		ClearGroup(122);
		AddWord(123, "iceberg");
		ClearGroup(123);
		AddWord(137, "journal");
		ClearGroup(137);
		AddWord(139, "kick");
		ClearGroup(139);
		AddWord(145, "keg");
		ClearGroup(145);
		AddWord(147, "leonard");
		ClearGroup(147);
		AddWord(149, "let");
		ClearGroup(149);
		AddWord(150, "stable");
		ClearGroup(150);
		AddWord(153, "log");
		ClearGroup(153);
		AddWord(155, "letter");
		ClearGroup(155);
		AddWord(166, "beam");
		AddWord(166, "lumber");
		AddWord(166, "wood");
		ClearGroup(166);
		AddWord(167, "lantern");
		ClearGroup(167);
		AddWord(171, "cash");
		AddWord(171, "money");
		ClearGroup(171);
		AddWord(174, "mailbox");
		ClearGroup(174);
		AddWord(176, "metal");
		AddWord(176, "scraps");
		ClearGroup(176);
		AddWord(177, "hill");
		AddWord(177, "mountain");
		ClearGroup(177);
		AddWord(178, "donkey");
		ClearGroup(178);
		AddWord(180, "message");
		ClearGroup(180);
		AddWord(181, "fertilizer");
		ClearGroup(181);
		AddWord(186, "new york");
		ClearGroup(186);
		AddWord(194, "mosquito net");
		ClearGroup(194);
		AddWord(195, "nugget");
		ClearGroup(195);
		AddWord(204, "bacon");
		AddWord(204, "bait");
		AddWord(204, "ham");
		ClearGroup(204);
		AddWord(203, "oxen");
		ClearGroup(203);
		AddWord(205, "postmark");
		ClearGroup(205);
		AddWord(207, "photo");
		AddWord(207, "picture");
		ClearGroup(207);
		AddWord(209, "painting");
		ClearGroup(209);
		AddWord(212, "postoffice");
		ClearGroup(212);
		AddWord(214, "cop");
		AddWord(214, "policeman");
		ClearGroup(214);
		AddWord(219, "mailman");
		ClearGroup(219);
		AddWord(222, "note pad");
		ClearGroup(222);
		AddWord(232, "desktop");
		ClearGroup(232);
		AddWord(238, "railing");
		ClearGroup(238);
		AddWord(240, "sign");
		ClearGroup(240);
		AddWord(241, "path");
		AddWord(241, "road");
		ClearGroup(241);
		AddWord(247, "saving account");
		ClearGroup(247);
		AddWord(255, "shovel");
		ClearGroup(255);
		AddWord(259, "sacramento");
		ClearGroup(259);
		AddWord(260, "shop");
		ClearGroup(260);
		AddWord(268, "thank");
		ClearGroup(268);
		AddWord(270, "gravestone");
		AddWord(270, "tombstone");
		ClearGroup(270);
		AddWord(275, "team");
		ClearGroup(275);
		AddWord(277, "twelve");
		ClearGroup(277);
		AddWord(278, "ten");
		ClearGroup(278);
		AddWord(279, "thirteen");
		ClearGroup(279);
		AddWord(283, "disconnect");
		AddWord(283, "disengage");
		ClearGroup(283);
		AddWord(285, "pressure valve");
		ClearGroup(285);
		AddWord(287, "vegetables");
		ClearGroup(287);
		AddWord(291, "withdraw");
		ClearGroup(291);
		AddWord(297, "cart");
		AddWord(297, "wagon");
		ClearGroup(297);
		AddWord(298, "wheel");
		ClearGroup(298);
		AddWord(307, "young");
		ClearGroup(307);
		AddWord(320, "plain");
		AddWord(320, "prairie");
		ClearGroup(320);
		AddWord(321, "field");
		ClearGroup(321);
		AddWord(322, "pasture");
		ClearGroup(322);
		AddWord(326, "thing");
		ClearGroup(326);
		AddWord(330, "loom");
		ClearGroup(330);
		AddWord(331, "barrel");
		ClearGroup(331);
		AddWord(335, "shoes");
		ClearGroup(335);
		AddWord(336, "boots");
		ClearGroup(336);
		AddWord(350, "cannon");
		ClearGroup(350);
		AddWord(359, "match");
		ClearGroup(359);
		AddWord(361, "cave");
		ClearGroup(361);
		AddWord(367, "pan");
		ClearGroup(367);
		AddWord(372, "bean");
		ClearGroup(372);
		AddWord(373, "pea");
		ClearGroup(373);
		AddWord(374, "pattern");
		ClearGroup(374);
		AddWord(377, "butterchurn");
		ClearGroup(377);
		AddWord(381, "psalm 23");
		ClearGroup(381);
		AddWord(382, "city");
		AddWord(382, "settlement");
		AddWord(382, "town");
		ClearGroup(382);
		AddWord(384, "bushes");
		ClearGroup(384);
		AddWord(386, "captain");
		ClearGroup(386);
		AddWord(388, "bed");
		ClearGroup(388);
		AddWord(392, "brick");
		ClearGroup(392);
		AddWord(394, "bulletin");
		ClearGroup(394);
		AddWord(396, "cobblestones");
		ClearGroup(396);
		AddWord(398, "cross");
		ClearGroup(398);
		AddWord(399, "pipe");
		ClearGroup(399);
		AddWord(400, "brake");
		ClearGroup(400);
		AddWord(405, "fast");
		ClearGroup(405);
		AddWord(409, "post");
		ClearGroup(409);
		AddWord(411, "pillar");
		ClearGroup(411);
		AddWord(420, "format");
		ClearGroup(420);
		AddWord(426, "tent");
		ClearGroup(426);
		AddWord(434, "rafter");
		ClearGroup(434);
		AddWord(452, "time");
		ClearGroup(452);
		AddWord(455, "slow");
		ClearGroup(455);
		AddWord(461, "coat");
		ClearGroup(461);
		AddWord(469, "plank");
		ClearGroup(469);
		AddWord(470, "pocket");
		ClearGroup(470);
		AddWord(478, "vine");
		ClearGroup(478);
		AddWord(482, "hut");
		AddWord(482, "shack");
		ClearGroup(482);
		AddWord(483, "snake");
		ClearGroup(483);
		AddWord(484, "cliff");
		ClearGroup(484);
		AddWord(487, "cactus");
		ClearGroup(487);
		AddWord(496, "bin");
		ClearGroup(496);
		AddWord(497, "leaves");
		ClearGroup(497);
		AddWord(501, "smokestack");
		ClearGroup(501);
		AddWord(502, "word");
		ClearGroup(502);
		AddWord(503, "quarter");
		ClearGroup(503);
		AddWord(505, "memory");
		ClearGroup(505);
		AddWord(509, "pillow");
		ClearGroup(509);
		AddWord(510, "object");
		ClearGroup(510);
		AddWord(514, "bench");
		ClearGroup(514);
		AddWord(516, "worker");
		ClearGroup(516);
		AddWord(520, "priority");
		ClearGroup(520);
		AddWord(521, "crank");
		ClearGroup(521);
		AddWord(529, "buddy");
		ClearGroup(529);
		AddWord(533, "mr weest");
		ClearGroup(533);
		AddWord(537, "rio de janeiro");
		ClearGroup(537);
		AddWord(547, "fastest");
		ClearGroup(547);
		AddWord(556, "rand");
		ClearGroup(556);
		AddWord(563, "sutters");
		ClearGroup(563);
		AddWord(566, "doormat");
		ClearGroup(566);
		AddWord(567, "route");
		ClearGroup(567);
		AddWord(569, "ants");
		ClearGroup(569);
		AddWord(572, "weight");
		ClearGroup(572);
		AddWord(580, "alligator");
		ClearGroup(580);

		RemoveAddedWord("account balance");
		RemoveAddedWord("animals");
		RemoveAddedWord("antlers");
		RemoveAddedWord("ax");
		RemoveAddedWord("boats");
		RemoveAddedWord("boxes");
		RemoveAddedWord("coin");
		RemoveAddedWord("calif");
		RemoveAddedWord("cape");
		RemoveAddedWord("cemetery");
		RemoveAddedWord("candles");
		RemoveAddedWord("clipping");
		RemoveAddedWord("pier");
		RemoveAddedWord("engines");
		RemoveAddedWord("entry");
		RemoveAddedWord("room 11");
		RemoveAddedWord("sutters fort");
		RemoveAddedWord("citrus fruit");
		RemoveAddedWord("lawn");
		RemoveAddedWord("pressure gauge");
		RemoveAddedWord("graves");
		RemoveAddedWord("guards");
		RemoveAddedWord("guns");
		RemoveAddedWord("hi");
		RemoveAddedWord("home");
		RemoveAddedWord("horns");
		RemoveAddedWord("horses");
		RemoveAddedWord("islands");
		RemoveAddedWord("icebergs");
		RemoveAddedWord("journals");
		RemoveAddedWord("kicks");
		RemoveAddedWord("kegs");
		RemoveAddedWord("leonards");
		RemoveAddedWord("lets");
		RemoveAddedWord("livery");
		RemoveAddedWord("logs");
		RemoveAddedWord("letters");
		RemoveAddedWord("beams");
		RemoveAddedWord("bucks");
		RemoveAddedWord("metal pieces");
		RemoveAddedWord("foot hill");
		RemoveAddedWord("foot hills");
		RemoveAddedWord("donkeys");
		RemoveAddedWord("messages");
		RemoveAddedWord("bull manure");
		RemoveAddedWord("new york city");
		RemoveAddedWord("mosquito netting");
		RemoveAddedWord("nuggets");
		RemoveAddedWord("ox");
		RemoveAddedWord("post mark");
		RemoveAddedWord("photograph");
		RemoveAddedWord("paintings");
		RemoveAddedWord("post office");
		RemoveAddedWord("lawman");
		RemoveAddedWord("mail man");
		RemoveAddedWord("note pads");
		RemoveAddedWord("desk top");
		RemoveAddedWord("rail");
		RemoveAddedWord("signs");
		RemoveAddedWord("paths");
		RemoveAddedWord("saving");
		RemoveAddedWord("shovels");
		RemoveAddedWord("sacramento city");
		RemoveAddedWord("shops");
		RemoveAddedWord("thanks");
		RemoveAddedWord("gravestones");
		RemoveAddedWord("teams");
		RemoveAddedWord("room 12");
		RemoveAddedWord("room 10");
		RemoveAddedWord("room 13");
		RemoveAddedWord("diconnect");
		RemoveAddedWord("pressure relief valve");
		RemoveAddedWord("vegetable");
		RemoveAddedWord("withdrawing");
		RemoveAddedWord("covered wagon");
		RemoveAddedWord("wheels");
		RemoveAddedWord("youthful");
		RemoveAddedWord("plains");
		RemoveAddedWord("fields");
		RemoveAddedWord("pastures");
		RemoveAddedWord("things");
		RemoveAddedWord("looms");
		RemoveAddedWord("barrels");
		RemoveAddedWord("shoe");
		RemoveAddedWord("boot");
		RemoveAddedWord("cannons");
		RemoveAddedWord("matches");
		RemoveAddedWord("cavern");
		RemoveAddedWord("panning");
		RemoveAddedWord("beans");
		RemoveAddedWord("peas");
		RemoveAddedWord("patterns");
		RemoveAddedWord("butter churn");
		RemoveAddedWord("psalm");
		RemoveAddedWord("bush");
		RemoveAddedWord("captains");
		RemoveAddedWord("beds");
		RemoveAddedWord("bricks");
		RemoveAddedWord("bulletins");
		RemoveAddedWord("cobblestone");
		RemoveAddedWord("crosses");
		RemoveAddedWord("pipes");
		RemoveAddedWord("brakes");
		RemoveAddedWord("f");
		RemoveAddedWord("posts");
		RemoveAddedWord("pillars");
		RemoveAddedWord("init");
		RemoveAddedWord("tents");
		RemoveAddedWord("rafters");
		RemoveAddedWord("times");
		RemoveAddedWord("s");
		RemoveAddedWord("coats");
		RemoveAddedWord("planks");
		RemoveAddedWord("pockets");
		RemoveAddedWord("huts");
		RemoveAddedWord("boa");
		RemoveAddedWord("cacti");
		RemoveAddedWord("bins");
		RemoveAddedWord("leaf");
		RemoveAddedWord("stack");
		RemoveAddedWord("words");
		RemoveAddedWord("quarters");
		RemoveAddedWord("mem");
		RemoveAddedWord("pillows");
		RemoveAddedWord("obj");
		RemoveAddedWord("benches");
		RemoveAddedWord("workers");
		RemoveAddedWord("pri");
		RemoveAddedWord("cranks");
		RemoveAddedWord("bud");
		RemoveAddedWord("mr weest jr");
		RemoveAddedWord("rio");
		RemoveAddedWord("ff");
		RemoveAddedWord("rands");
		RemoveAddedWord("sutter");
		RemoveAddedWord("doormats");
		RemoveAddedWord("routes");
		RemoveAddedWord("ant");
		RemoveAddedWord("weights");
		RemoveAddedWord("gator");
	}

	if(IsSpaceQuest2()) {
		AddWord(8, "hose");
		ClearGroup(8);
		AddWord(10, "keyboard");
		ClearGroup(10);
		AddWord(16, "monitor");
		AddWord(16, "screen");
		ClearGroup(16);
		AddWord(43, "clothes");
		AddWord(43, "spacesuit");
		AddWord(43, "suit");
		AddWord(43, "uniform");
		ClearGroup(43);
		AddWord(46, "craft");
		AddWord(46, "escape pod");
		AddWord(46, "ship");
		ClearGroup(46);
		AddWord(48, "body");
		AddWord(48, "corpse");
		ClearGroup(48);
		AddWord(50, "condom");
		ClearGroup(50);
		AddWord(53, "base");
		AddWord(53, "platform");
		ClearGroup(53);
		AddWord(55, "banner");
		AddWord(55, "flag");
		ClearGroup(55);
		AddWord(58, "being");
		AddWord(58, "crewman");
		AddWord(58, "guard");
		AddWord(58, "hunter");
		AddWord(58, "man");
		AddWord(58, "sarien");
		ClearGroup(58);
		AddWord(67, "cm");
		ClearGroup(67);
		AddWord(68, "sn");
		ClearGroup(68);
		AddWord(72, "lever");
		AddWord(72, "throttle");
		ClearGroup(72);
		AddWord(78, "droid");
		AddWord(78, "robot");
		ClearGroup(78);
		AddWord(82, "moon");
		AddWord(82, "planet");
		AddWord(82, "star");
		ClearGroup(82);
		AddWord(83, "explore");
		AddWord(83, "frisk");
		AddWord(83, "search");
		ClearGroup(83);
		AddWord(86, "backstage");
		ClearGroup(86);
		AddWord(87, "scott murphy");
		ClearGroup(87);
		AddWord(91, "athletic supporter");
		AddWord(91, "jock strap");
		ClearGroup(91);
		AddWord(94, "programmed");
		ClearGroup(94);
		AddWord(105, "hit");
		AddWord(105, "kick");
		AddWord(105, "punch");
		ClearGroup(105);
		AddWord(114, "cash");
		AddWord(114, "money");
		ClearGroup(114);
		AddWord(115, "pockets");
		ClearGroup(115);
		AddWord(120, "leaves");
		ClearGroup(120);
		AddWord(121, "branch");
		ClearGroup(121);
		AddWord(122, "buckazoids");
		ClearGroup(122);
		AddWord(128, "baydoors");
		ClearGroup(128);
		AddWord(129, "airlock");
		ClearGroup(129);
		AddWord(137, "cave");
		AddWord(137, "hallway");
		AddWord(137, "room");
		ClearGroup(137);
		AddWord(140, "drop");
		AddWord(140, "put");
		ClearGroup(140);
		AddWord(142, "alien");
		ClearGroup(142);
		AddWord(144, "cliff");
		AddWord(144, "mountain");
		ClearGroup(144);
		AddWord(145, "eyes");
		ClearGroup(145);
		AddWord(149, "hello");
		ClearGroup(149);
		AddWord(154, "follow");
		ClearGroup(154);
		AddWord(163, "bomb");
		AddWord(163, "grenade");
		ClearGroup(163);
		AddWord(173, "grate");
		AddWord(173, "vent");
		ClearGroup(173);
		AddWord(176, "vine");
		ClearGroup(176);
		AddWord(191, "fire");
		AddWord(191, "flame");
		ClearGroup(191);
		AddWord(199, "device");
		AddWord(199, "gadget");
		AddWord(199, "translator");
		ClearGroup(199);
		AddWord(225, "berries");
		ClearGroup(225);
		AddWord(228, "generator");
		ClearGroup(228);
		AddWord(230, "spore");
		ClearGroup(230);
		AddWord(233, "belt");
		AddWord(233, "seatbelt");
		ClearGroup(233);
		AddWord(236, "afix");
		AddWord(236, "attach");
		AddWord(236, "tie");
		ClearGroup(236);
		AddWord(241, "jetpack");
		ClearGroup(241);
		AddWord(246, "sound");
		ClearGroup(246);
		AddWord(248, "foot step");
		AddWord(248, "foot steps");
		AddWord(248, "footsteps");
		ClearGroup(248);
		AddWord(252, "railing");
		ClearGroup(252);
		AddWord(254, "attitude");
		AddWord(254, "dial");
		ClearGroup(254);
		AddWord(260, "arch");
		ClearGroup(260);
		AddWord(261, "spider");
		ClearGroup(261);
		AddWord(265, "appendage");
		AddWord(265, "claw");
		AddWord(265, "hands");
		ClearGroup(265);
		AddWord(277, "drip");
		AddWord(277, "drops");
		ClearGroup(277);
		AddWord(278, "beam");
		AddWord(278, "laser");
		ClearGroup(278);
		AddWord(281, "walkway");
		ClearGroup(281);
		AddWord(282, "mailbox");
		ClearGroup(282);
		AddWord(283, "breathe");
		AddWord(283, "inhale");
		AddWord(283, "take breath");
		ClearGroup(283);
		AddWord(284, "dunes");
		ClearGroup(284);
		AddWord(285, "animal");
		AddWord(285, "beast");
		AddWord(285, "creature");
		AddWord(285, "monster");
		ClearGroup(285);
		AddWord(288, "city");
		AddWord(288, "settlement");
		AddWord(288, "town");
		ClearGroup(288);
		AddWord(293, "customer");
		ClearGroup(293);
		AddWord(296, "cabinet");
		AddWord(296, "compartment");
		AddWord(296, "locker");
		ClearGroup(296);
		AddWord(302, "cube");
		AddWord(302, "cubix rube");
		AddWord(302, "puzzle");
		ClearGroup(302);
		AddWord(311, "nozzle");
		ClearGroup(311);
		AddWord(316, "windshield");
		ClearGroup(316);
		AddWord(326, "lips");
		AddWord(326, "mouth");
		ClearGroup(326);
		AddWord(339, "carving");
		AddWord(339, "face");
		AddWord(339, "image");
		ClearGroup(339);
		AddWord(346, "communicator");
		AddWord(346, "watch");
		ClearGroup(346);
		AddWord(349, "naked");
		ClearGroup(349);
		AddWord(357, "digital readout");
		ClearGroup(357);
		AddWord(358, "plunger");
		ClearGroup(358);
		AddWord(361, "toilet");
		ClearGroup(361);
		AddWord(364, "stall");
		ClearGroup(364);
		AddWord(365, "use toilet");
		ClearGroup(365);
		AddWord(368, "overalls");
		ClearGroup(368);
		AddWord(369, "lighter");
		ClearGroup(369);
		AddWord(370, "fire sprinkler");
		ClearGroup(370);
		AddWord(371, "fixture");
		ClearGroup(371);
		AddWord(372, "gem");
		ClearGroup(372);
		AddWord(373, "cable");
		AddWord(373, "cord");
		AddWord(373, "wire");
		ClearGroup(373);
		AddWord(378, "switch off");
		ClearGroup(378);
		AddWord(381, "sledge vohaul");
		AddWord(381, "vohaul");
		ClearGroup(381);
		AddWord(382, "tits");
		ClearGroup(382);
		AddWord(386, "above");
		ClearGroup(386);
		AddWord(388, "underwater");
		ClearGroup(388);
		AddWord(389, "parasite");
		ClearGroup(389);
		AddWord(390, "landing platform");
		ClearGroup(390);
		AddWord(392, "camera");
		ClearGroup(392);
		AddWord(397, "clone");
		AddWord(397, "insurance salesmen");
		AddWord(397, "salesman");
		ClearGroup(397);
		AddWord(400, "life support system");
		ClearGroup(400);
		AddWord(405, "detach");
		AddWord(405, "disconnect");
		AddWord(405, "unplug");
		ClearGroup(405);
		AddWord(408, "vomit");
		ClearGroup(408);
		AddWord(414, "thrusters");
		ClearGroup(414);
		AddWord(417, "hill");
		AddWord(417, "plateau");
		ClearGroup(417);
		AddWord(423, "fungus");
		AddWord(423, "mushroom");
		ClearGroup(423);
		AddWord(432, "ashes");
		ClearGroup(432);
		AddWord(433, "pipe");
		AddWord(433, "plumbing");
		ClearGroup(433);

		RemoveAddedWord("hoses");
		RemoveAddedWord("key board");
		RemoveAddedWord("automaton");
		RemoveAddedWord("pedestal");
		RemoveAddedWord("banners");
		RemoveAddedWord("c");
		RemoveAddedWord("s");
		RemoveAddedWord("stick");
		RemoveAddedWord("dbg");
		RemoveAddedWord("scott");
		RemoveAddedWord("jock");
		RemoveAddedWord("programmer");
		RemoveAddedWord("credit");
		RemoveAddedWord("pocket");
		RemoveAddedWord("leaf");
		RemoveAddedWord("branches");
		RemoveAddedWord("buck");
		RemoveAddedWord("buckazoid");
		RemoveAddedWord("bay door");
		RemoveAddedWord("bay doors");
		RemoveAddedWord("airlock door");
		RemoveAddedWord("aliens");
		RemoveAddedWord("cliffs");
		RemoveAddedWord("eye");
		RemoveAddedWord("hi");
		RemoveAddedWord("go after");
		RemoveAddedWord("bombs");
		RemoveAddedWord("grates");
		RemoveAddedWord("vines");
		RemoveAddedWord("dialect translator");
		RemoveAddedWord("berrie");
		RemoveAddedWord("star generator");
		RemoveAddedWord("spores");
		RemoveAddedWord("seat belt");
		RemoveAddedWord("jet pack");
		RemoveAddedWord("sounds");
		RemoveAddedWord("rail");
		RemoveAddedWord("attitude dial");
		RemoveAddedWord("arches");
		RemoveAddedWord("spider droid");
		RemoveAddedWord("appendages");
		RemoveAddedWord("drips");
		RemoveAddedWord("beams");
		RemoveAddedWord("catwalk");
		RemoveAddedWord("mail box");
		RemoveAddedWord("breath");
		RemoveAddedWord("dune");
		RemoveAddedWord("flats");
		RemoveAddedWord("customers");
		RemoveAddedWord("nozzles");
		RemoveAddedWord("wind shield");
		RemoveAddedWord("face");
		RemoveAddedWord("nude");
		RemoveAddedWord("digital read-out");
		RemoveAddedWord("plunge");
		RemoveAddedWord("throne");
		RemoveAddedWord("stalls");
		RemoveAddedWord("crap");
		RemoveAddedWord("over all");
		RemoveAddedWord("over alls");
		RemoveAddedWord("igniter");
		RemoveAddedWord("fire sprinklers");
		RemoveAddedWord("fixtures");
		RemoveAddedWord("glowing gem");
		RemoveAddedWord("cables");
		RemoveAddedWord("switch up");
		RemoveAddedWord("sledge vohauls");
		RemoveAddedWord("shsr");
		RemoveAddedWord("over head");
		RemoveAddedWord("under water");
		RemoveAddedWord("parasites");
		RemoveAddedWord("landing");
		RemoveAddedWord("television camera");
		RemoveAddedWord("clones");
		RemoveAddedWord("lss");
		RemoveAddedWord("mess");
		RemoveAddedWord("puke");
		RemoveAddedWord("thruster");
		RemoveAddedWord("mesa");
		RemoveAddedWord("fungi");
		RemoveAddedWord("ash");
		RemoveAddedWord("pipes");
	}

	if(IsSpaceQuest1()) {
		RemoveAddedWord("tp");
		RemoveAddedWord("sp");
		RemoveAddedWord("var");
		RemoveAddedWord("xy");
		RemoveAddedWord("access tubes");
		AddWord(8, "access tube");
		AddWord(8, "dome");
		ClearGroup(8);
		AddWord(12, "pilot droid");
		ClearGroup(12);
		AddWord(43, "clothes");
		AddWord(43, "spacesuit");
		AddWord(43, "suit");
		AddWord(43, "uniform");
		ClearGroup(43);
		AddWord(46, "craft");
		AddWord(46, "escape pod");
		AddWord(46, "ship");
		ClearGroup(46);
		AddWord(48, "body");
		AddWord(48, "corpse");
		ClearGroup(48);
		RemoveAddedWord("pedestal");
		AddWord(53, "base");
		AddWord(53, "platform");
		ClearGroup(53);
		RemoveAddedWord("banners");
		AddWord(55, "banner");
		ClearGroup(55);
		RemoveAddedWord("scott");
		AddWord(87, "scott murphy");
		ClearGroup(87);
		RemoveAddedWord("wrote");
		AddWord(94, "programmed");
		ClearGroup(94);
		RemoveAddedWord("chunk");
		AddWord(101, "fragment");
		AddWord(101, "part");
		ClearGroup(101);
		AddWord(58, "crewman");
		AddWord(58, "guard");
		AddWord(58, "keronian");
		AddWord(58, "man");
		AddWord(58, "sarien");
		AddWord(58, "scientist");
		AddWord(58, "technician");
		ClearGroup(58);
		AddWord(72, "lever");
		AddWord(72, "throttle");
		ClearGroup(72);
		RemoveAddedWord("i.d.");
		RemoveAddedWord("i.d. card");
		AddWord(76, "id card");
		ClearGroup(76);
		AddWord(78, "droid");
		AddWord(78, "robot");
		ClearGroup(78);
		AddWord(82, "planet");
		AddWord(82, "star");
		ClearGroup(82);
		AddWord(85, "don't touch");
		ClearGroup(85);
		AddWord(86, "bike");
		AddWord(86, "skimmer");
		ClearGroup(86);
		AddWord(111, "ale");
		AddWord(111, "beer");
		AddWord(111, "keronian ale");
		ClearGroup(111);
		AddWord(119, "credit card");
		AddWord(119, "keycard");
		ClearGroup(119);
		RemoveAddedWord("slots");
		RemoveAddedWord("slot");
		AddWord(127, "cartridge slot");
		AddWord(127, "slot machine");
		ClearGroup(127);
		AddWord(128, "bay doors");
		ClearGroup(128);
		AddWord(129, "airlock");
		ClearGroup(129);
		AddWord(115, "pocket");
		ClearGroup(115);
		AddWord(137, "area");
		AddWord(137, "cave");
		AddWord(137, "hallway");
		AddWord(137, "room");
		ClearGroup(137);
		AddWord(142, "alien");
		ClearGroup(142);
		AddWord(144, "cliff");
		AddWord(144, "mountain");
		ClearGroup(144);
		AddWord(163, "bomb");
		AddWord(163, "grenade");
		ClearGroup(163);
		AddWord(168, "move");
		AddWord(168, "pull");
		ClearGroup(168);
		AddWord(173, "grate");
		AddWord(173, "vent");
		ClearGroup(173);
		AddWord(185, "remove");
		AddWord(185, "take off");
		AddWord(185, "unbuckle");
		ClearGroup(185);
		AddWord(140, "drop");
		AddWord(140, "put");
		ClearGroup(140);
		AddWord(149, "hello");
		ClearGroup(149);
		AddWord(157, "exit");
		AddWord(157, "get out");
		AddWord(157, "leave");
		ClearGroup(157);
		AddWord(195, "laundry");
		AddWord(195, "machine");
		AddWord(195, "washer");
		ClearGroup(195);
		AddWord(199, "gadget");
		AddWord(199, "translator");
		ClearGroup(199);
		AddWord(203, "look in");
		ClearGroup(203);
		AddWord(209, "board");
		AddWord(209, "get on");
		AddWord(209, "ride");
		ClearGroup(209);
		AddWord(228, "generator");
		ClearGroup(228);
		AddWord(233, "belt");
		AddWord(233, "seatbelt");
		ClearGroup(233);
		RemoveAddedWord("jet pack");
		AddWord(241, "jetpack");
		ClearGroup(241);
		AddWord(248, "footstep");
		ClearGroup(248);
		AddWord(251, "box");
		AddWord(251, "retrieval unit");
		ClearGroup(251);
		AddWord(252, "railing");
		ClearGroup(252);
		AddWord(260, "arch");
		ClearGroup(260);
		AddWord(261, "spider");
		ClearGroup(261);
		AddWord(262, "antenna");
		AddWord(262, "tower");
		ClearGroup(262);
		AddWord(264, "supplies");
		AddWord(264, "survival kit");
		ClearGroup(264);
		AddWord(266, "raygun");
		AddWord(266, "weapon");
		ClearGroup(266);
		AddWord(267, "device");
		AddWord(267, "remote control");
		ClearGroup(267);
		AddWord(279, "piston");
		ClearGroup(279);
		AddWord(278, "beam");
		AddWord(278, "laser");
		ClearGroup(278);
		AddWord(281, "walkway");
		ClearGroup(281);
		AddWord(284, "dune");
		ClearGroup(284);
		AddWord(285, "animal");
		AddWord(285, "beast");
		AddWord(285, "monster");
		AddWord(285, "tentacle");
		ClearGroup(285);
		AddWord(288, "city");
		AddWord(288, "settlement");
		ClearGroup(288);
		AddWord(290, "pile");
		ClearGroup(290);
		AddWord(291, "band");
		AddWord(291, "musician");
		ClearGroup(291);
		AddWord(301, "give me");
		ClearGroup(301);
		AddWord(304, "plug in");
		ClearGroup(304);
		AddWord(316, "windshield");
		ClearGroup(316);
		AddWord(326, "coupon");
		ClearGroup(326);
		AddWord(332, "bay");
		ClearGroup(332);
		AddWord(345, "reflective");
		ClearGroup(345);
		AddWord(293, "customer");
		ClearGroup(293);
		AddWord(296, "salesman");
		ClearGroup(296);
		AddWord(297, "dust");
		AddWord(297, "powder");
		ClearGroup(297);
		AddWord(339, "archive");
		AddWord(339, "books");
		AddWord(339, "library");
		ClearGroup(339);
		AddWord(342, "ledge");
		AddWord(342, "path");
		ClearGroup(342);
	}

	if(IsPoliceQuest()) {
		AddWord(268, "extender");
		AddWord(139, "poster");
		AddWord(276, "nightstick");

		AddWord(2, "examine");
		AddWord(2, "investigate");
		AddWord(2, "look");
		AddWord(2, "read");
		ClearGroup(2);
		AddWord(3, "do");
		AddWord(3, "fix");
		AddWord(3, "prepare");
		ClearGroup(3);
		AddWord(16, "kick");
		AddWord(16, "punch");
		ClearGroup(16);
		AddWord(17, "bye");
		ClearGroup(17);
		AddWord(19, "accident");
		AddWord(19, "bar");
		AddWord(19, "blue room");
		AddWord(19, "cafe");
		AddWord(19, "casino");
		AddWord(19, "court");
		AddWord(19, "hallway");
		AddWord(19, "joint");
		AddWord(19, "lounge");
		AddWord(19, "office");
		AddWord(19, "park");
		AddWord(19, "room");
		AddWord(19, "store");
		AddWord(19, "tavern");
		AddWord(19, "willys");
		ClearGroup(19);
		AddWord(21, "freeze");
		AddWord(21, "stop");
		ClearGroup(21);
		AddWord(32, "move");
		AddWord(32, "press");
		AddWord(32, "push");
		AddWord(32, "turn");
		ClearGroup(32);
		AddWord(33, "enter");
		AddWord(33, "exit");
		AddWord(33, "leave");
		ClearGroup(33);
		AddWord(45, "drop");
		AddWord(45, "place");
		AddWord(45, "put");
		ClearGroup(45);
		AddWord(49, "empty");
		AddWord(49, "remove");
		ClearGroup(49);
		AddWord(56, "inventory");
		AddWord(56, "objects");
		ClearGroup(56);
		AddWord(57, "cover");
		AddWord(57, "hide");
		ClearGroup(57);
		AddWord(59, "bookcase");
		AddWord(59, "shelf");
		ClearGroup(59);
		AddWord(62, "find");
		AddWord(62, "search");
		ClearGroup(62);
		AddWord(63, "clean");
		AddWord(63, "wash");
		AddWord(63, "wipe");
		ClearGroup(63);
		AddWord(66, "light");
		AddWord(66, "pole");
		ClearGroup(66);
		AddWord(67, "fire");
		AddWord(67, "kill");
		AddWord(67, "shoot");
		ClearGroup(67);
		AddWord(68, "cocaine");
		AddWord(68, "dope");
		AddWord(68, "drugs");
		AddWord(68, "marijuana");
		AddWord(68, "narcotics");
		ClearGroup(68);
		AddWord(69, "fence");
		AddWord(69, "wall");
		ClearGroup(69);
		AddWord(75, "eat");
		AddWord(75, "lick");
		ClearGroup(75);
		AddWord(84, "banister");
		AddWord(84, "railing");
		ClearGroup(84);
		AddWord(89, "bike");
		AddWord(89, "motorcycle");
		ClearGroup(89);
		AddWord(92, "dui");
		AddWord(92, "influence");
		ClearGroup(92);
		AddWord(94, "make");
		AddWord(94, "wants");
		ClearGroup(94);
		AddWord(96, "use");
		AddWord(96, "with");
		ClearGroup(96);
		AddWord(99, "bonds");
		AddWord(99, "i'm");
		AddWord(99, "me");
		AddWord(99, "sonny");
		ClearGroup(99);
		AddWord(105, "shower");
		AddWord(105, "water");
		ClearGroup(105);
		AddWord(106, "gamble");
		AddWord(106, "poker");
		ClearGroup(106);
		AddWord(107, "citation");
		AddWord(107, "pinch");
		AddWord(107, "ticket");
		ClearGroup(107);
		AddWord(112, "wastebasket");
		ClearGroup(112);
		AddWord(116, "look in");
		ClearGroup(116);
		AddWord(123, "cash");
		AddWord(123, "money");
		ClearGroup(123);
		AddWord(124, "furniture");
		ClearGroup(124);
		AddWord(133, "number");
		ClearGroup(133);
		AddWord(136, "emergency");
		ClearGroup(136);
		AddWord(141, "drive");
		ClearGroup(141);
		AddWord(142, "cuff");
		AddWord(142, "handcuff");
		ClearGroup(142);
		AddWord(143, "crime");
		AddWord(143, "felony");
		AddWord(143, "theft");
		ClearGroup(143);
		AddWord(146, "smell");
		AddWord(146, "snort");
		ClearGroup(146);
		AddWord(149, "city");
		AddWord(149, "lytton");
		AddWord(149, "town");
		ClearGroup(149);
		AddWord(153, "blind");
		AddWord(153, "curtains");
		ClearGroup(153);
		AddWord(163, "ante");
		AddWord(163, "deal");
		AddWord(163, "sell");
		ClearGroup(163);
		AddWord(167, "notebook");
		ClearGroup(167);
		AddWord(180, "ambulance");
		AddWord(180, "coroner");
		AddWord(180, "hospital");
		ClearGroup(180);
		AddWord(183, "bystanders");
		AddWord(183, "crowd");
		AddWord(183, "people");
		ClearGroup(183);
		AddWord(194, "complaint");
		AddWord(194, "difficulty");
		AddWord(194, "problem");
		ClearGroup(194);
		AddWord(197, "change");
		AddWord(197, "put on");
		AddWord(197, "uncover");
		AddWord(197, "undress");
		AddWord(197, "wear");
		ClearGroup(197);
		AddWord(198, "envelope");
		AddWord(198, "letter");
		AddWord(198, "mail");
		ClearGroup(198);
		AddWord(205, "bug");
		AddWord(205, "bugs");
		AddWord(205, "microphone");
		ClearGroup(205);
		AddWord(210, "caffeine");
		AddWord(210, "coffee");
		AddWord(210, "milk");
		AddWord(210, "soda");
		ClearGroup(210);
		AddWord(226, "play");
		AddWord(226, "select");
		ClearGroup(226);
		AddWord(229, "rights");
		ClearGroup(229);
		AddWord(230, "convict");
		AddWord(230, "criminal");
		AddWord(230, "prisoner");
		ClearGroup(230);
		AddWord(238, "dispatch");
		AddWord(238, "headquarters");
		AddWord(238, "station");
		ClearGroup(238);
		AddWord(240, "hello");
		ClearGroup(240);
		AddWord(244, "information");
		AddWord(244, "tip");
		ClearGroup(244);
		AddWord(247, "compartment");
		AddWord(247, "hole");
		AddWord(247, "mailbox");
		ClearGroup(247);
		AddWord(252, "ammo");
		AddWord(252, "bullet");
		ClearGroup(252);
		AddWord(253, "breasts");
		AddWord(253, "nipples");
		ClearGroup(253);
		AddWord(258, "gun");
		AddWord(258, "weapon");
		ClearGroup(258);
		AddWord(262, "blackboard");
		ClearGroup(262);
		AddWord(272, "bartender");
		AddWord(272, "biker");
		AddWord(272, "clerk");
		AddWord(272, "dealer");
		AddWord(272, "dude");
		AddWord(272, "man");
		AddWord(272, "officer");
		AddWord(272, "police");
		AddWord(272, "victim");
		AddWord(272, "witness");
		ClearGroup(272);
		AddWord(284, "leak");
		AddWord(284, "piss");
		ClearGroup(284);
		AddWord(285, "shit");
		ClearGroup(285);
		RemoveAddedWord("judge");
		AddWord(286, "girl");
		AddWord(286, "hooker");
		AddWord(286, "judge");
		AddWord(286, "lady");
		AddWord(286, "palmer");
		AddWord(286, "woman");
		ClearGroup(286);
		AddWord(309, "panties");
		AddWord(309, "underwear");
		ClearGroup(309);
		AddWord(318, "pussy");
		ClearGroup(318);
		AddWord(321, "beer");
		AddWord(321, "gin");
		AddWord(321, "rum");
		AddWord(321, "wine");
		ClearGroup(321);
		AddWord(328, "id card");
		AddWord(328, "license");
		ClearGroup(328);
		AddWord(332, "eye");
		AddWord(332, "face");
		ClearGroup(332);
		AddWord(351, "planter");
		ClearGroup(351);
		AddWord(363, "screw");
		AddWord(363, "seduce");
		ClearGroup(363);
		AddWord(543, "previous");
		ClearGroup(543);
		AddWord(548, "seatbelt");
		ClearGroup(548);
		AddWord(549, "fasten");
		ClearGroup(549);
		AddWord(554, "carvings");
		AddWord(554, "initials");
		ClearGroup(554);

		RemoveAddedWord("looking");
		RemoveAddedWord("reading");
		RemoveAddedWord("hit");
		RemoveAddedWord("goodbye");
		RemoveAddedWord("moving");
		RemoveAddedWord("using");
		RemoveAddedWord("dwi");
		RemoveAddedWord("makes");
		RemoveAddedWord("driver");
		RemoveAddedWord("blinds");
		RemoveAddedWord("notes");
		RemoveAddedWord("dispatcher");
		RemoveAddedWord("furnishings");
		RemoveAddedWord("bills");
		RemoveAddedWord("right");
		RemoveAddedWord("crap");
	}
}
/******************************************************************************/
int ErrorMessage(const char *s, ...)
{
	va_list argptr;
	int cnt;

	va_start(argptr, s);
	cnt = vprintf(s, argptr);
	va_end(argptr);

#ifndef GBAGI_NO_GETCH
    getch();
#endif
    exit(1);

	return(cnt);
}
/******************************************************************************/
U16 fgetw(FILE *f)
{
	return (fgetc(f))+(fgetc(f)<<8);
}
/******************************************************************************/
U32 fgett(FILE *f)
{
	return (fgetc(f))+(fgetc(f)<<8)+(fgetc(f)<<16);
}
/******************************************************************************/
U32 fgetl(FILE *f)
{
	return (fgetc(f))+(fgetc(f)<<8)+(fgetc(f)<<16)+(fgetc(f)<<24);
}
/******************************************************************************/
void fputw(U16 l, FILE *f)
{
	fputc(l&0xFF,f);
	fputc(l>>8,f);
}
/******************************************************************************/
void fputt(U32 l, FILE *f)
{
	fputc(l&0xFF,f);
	fputc((l>>8)&0xFF,f);
	fputc((l>>16)&0xFF,f);
}
/******************************************************************************/
void fputl(U32 l, FILE *f)
{
	fputc(l&0xFF,f);
	fputc((l>>8)&0xFF,f);
	fputc((l>>16)&0xFF,f);
	fputc((l>>24)&0xFF,f);
}
/******************************************************************************/
U16 bGetW(const U8 *p)
{
	return (*p)+(p[1]<<8);
}
/******************************************************************************/
U16 beGetW(const U8 *p)
{
	return (p[1])+(p[0]<<8);
}
/******************************************************************************/
// scan the directory file(s) for the locations of the resources
BOOL LoadDir(BOOL SINGLE, int num)
{
	FILE *f;
	U32 offs;
	int totalEnts, i;
	if(SINGLE) {
		sprintf(fname,"%s%s%s",gi->path,gi->vID,gi->vID[0]?"dir":"dirs");
		if((f=fopen(fname,"rb"))==NULL) {
			ErrorMessage("Unable to open file: %s, for reading!",fname);
			return FALSE;
		}
		fseek(f,num<<1,SEEK_SET);
		offs = fgetw(f);
		if(num==3) {
			fseek(f,0,SEEK_END);
			totalEnts = (ftell(f)-offs)/3;
		} else
			totalEnts = (fgetw(f)-offs)/3;
		fseek(f,offs,SEEK_SET);
	} else {
		sprintf(fname,"%s%s",gi->path,dirNames[num]);
		if((f=fopen(fname,"rb"))==NULL) {
			ErrorMessage("Unable to open file: %s, for reading!",fname);
			return FALSE;
		}
		fseek(f,0,SEEK_END);
		totalEnts = ftell(f)/3;
		fseek(f,0,SEEK_SET);
	}
    if(totalEnts>256) totalEnts=0;
	memset(dirs[num],-1,sizeof(dirs[num]));
	for(i=0;i<totalEnts;i++) {
		if((offs = (fgetc(f)<<16)|(fgetc(f)<<8)|fgetc(f))!=0xFFFFFF) {
			dirs[num][i].offset		= offs&0x0FFFFF;
			dirs[num][i].vol		= (offs>>20)&0xF;
			VUSED[dirs[num][i].vol]	= TRUE;
		}
	}

	fclose(f);
	return TRUE;
}
/******************************************************************************/
// scan the vol files to find out how much space to allocate for all the files
BOOL PrepDirs()
{
	int vn,t,i;
	FILE *f;

	volSize=0;
	for(vn=0;vn<16;vn++) {
    	if(!VUSED[vn]) continue;
		sprintf(fname,"%s%svol.%d",gi->path,gi->vID,vn);
        printf("PrepDirs: checking %s\n", fname);
		if((f=fopen(fname,"rb"))==NULL) {
        	VUSED[vn] = FALSE;
            printf("PrepDirs: missing volume %d\n", vn);
        	continue;
			//ErrorMessage("Unable to open file: \"%s\", for reading!",fname);
			//return FALSE;
		}
		for(t=0;t<4;t++) {
			for(i=0;i<256;i++) {
				if(dirs[t][i].vol == vn) {
					fseek(f,dirs[t][i].offset,SEEK_SET);
                    if((fgetw(f)==0x3412)&&((fgetc(f)&0x7F)==vn)) {
						dirs[t][i].length = fgetw(f)+5;
						volSize += dirs[t][i].length;
                    } else {
                        printf("PrepDirs: rejected entry t=%d i=%d vol=%d off=%ld\n", t, i, vn, (long)dirs[t][i].offset);
                    	memset(&dirs[t][i],-1,sizeof(DIRENT));
                    }
				}
			}
		}
		fclose(f);
	}
    printf("PrepDirs: total volSize=%lu\n", (unsigned long)volSize);
	return TRUE;
}
/******************************************************************************/
static BOOL IsLarry1(void);
static U16 PatchGameSpecificLogicMessages(U8 *logicData, U16 logicLen, U8 *scratch, size_t scratchSize);
/******************************************************************************/
// scan the vol files to find out how much space to allocate for all the files
BOOL ProcessDirs()
{
	int vn, vi, t,i,msgTotal;
	FILE *f;
	U8 *pvol,*p;
	U16 enclen,declen;
	U8 *fbuf,*logicPatchBuf;
    char *msg;
	size_t allocVolSize;

	if(!volSize) return FALSE;
	allocVolSize = (size_t)volSize + 64u;
	if((volData = (U8*)malloc(allocVolSize))==NULL)
		return FALSE;
	if((fbuf = (U8*)malloc(65535))==NULL)
		return FALSE;
	if((logicPatchBuf = (U8*)malloc(65535))==NULL) {
		mFree(fbuf);
		return FALSE;
	}
	
	pvol = volData;
	for(vn=0;vn<16;vn++) {
    	if(!VUSED[vn]) continue;
		sprintf(fname,"%s%svol.%d",gi->path,gi->vID,vn);
		if((f=fopen(fname,"rb"))==NULL) {
			ErrorMessage("Unable to open file: %s, for reading!",fname);
			mFree(fbuf);
			return FALSE;	
		}
		for(t=0;t<4;t++) {
			for(i=0;i<256;i++) {
				if(dirs[t][i].vol == vn) {
					fseek(f,dirs[t][i].offset+2,SEEK_SET);

					dirs[t][i].offset = (intptr_t)((intptr_t)pvol-(intptr_t)volData);
					vi 		= fgetc(f);
					declen 	= fgetw(f);
					enclen 	= (gi->version->flags&PACKED_DIRS)?fgetw(f):declen;

					fread(fbuf,enclen,1,f);
					if(enclen==declen) { // not compressed
		               	if(t==0) { // logic
		            		msg = (char*)(fbuf+bGetW(fbuf)+2);	// pointer to messages
				    		msgTotal = (U8)*msg++;			// number of messages
							if(msgTotal) {
								DecryptBlock(msg + ((msgTotal + 1)<<1),msg+bGetW((U8*)msg));
								{
									U16 patchedDeclen = PatchGameSpecificLogicMessages(fbuf, declen, logicPatchBuf, 65535u);
									if(patchedDeclen != declen) {
										memcpy(fbuf, logicPatchBuf, patchedDeclen);
										declen = patchedDeclen;
									}
								}
							}
		                }
	                    *pvol++ = 0x34;
    	                *pvol++ = 0x12;
        	            *pvol++ = vi;
            	        *pvol++ = declen&0xFF;
                	    *pvol++ = declen>>8;
						memcpy(pvol,fbuf,declen);
					} else if(vi&0x80) { // compressed picture file
	                    *pvol++ = 0x34;
    	                *pvol++ = 0x12;
        	            *pvol++ = vi;
            	        *pvol++ = declen&0xFF;
                	    *pvol++ = declen>>8;
           				PIC_expand(fbuf, pvol, declen);
					} else { // compressed LZW
	                    *pvol++ = 0x34;
    	                *pvol++ = 0x12;
        	            *pvol++ = vi;
            	        *pvol++ = declen&0xFF;
                	    *pvol++ = declen>>8;
           				LZW_expand(fbuf, pvol, declen);
					}
					pvol += declen;
				}
			}
		}
		fclose(f);
	}
	volSize = (U32)(pvol - volData);
	mFree(logicPatchBuf);
	mFree(fbuf);
	return TRUE;
}
/******************************************************************************/
BOOL ProcessObject()
{
	FILE *f;
    U16 l;
    int q, nameStart;
    int objCount, i, entSize;
    U8 *p, *iPtr, *wPtr, *u;

	sprintf(fname,"%sobject",gi->path);
	if((f=fopen(fname,"rb")) == NULL) {
		ErrorMessage("Unable to open file: \"%s\" for reading!", fname);
		return FALSE;
	}
	fseek(f,0,SEEK_END);
	l=ftell(f);
	fseek(f,0,SEEK_SET);

	p = (U8*) malloc(l);
	fread(p, l, 1, f);
	fclose(f);

	q=bGetW(p);
	if( q>l || q<3 )// || (gi->version->flags & ENCRYPT_OBJ))
		DecryptBlock((char*)p,(char*)p+l);

    f=fopen("object.bin","wb");
    fwrite(p,l,1,f);
    fclose(f);

    nameStart 	= bGetW(p);
    entSize 	= (gi->version->flags&AMIGA)?4:3;
    objCount	= nameStart/entSize;
    iPtr		= p+entSize;
    strSize		= l-nameStart;
	if((objNameData = (char*) malloc(strSize))==NULL) {
     	ErrorMessage("Unable to allocate %d bytes of memory!", strSize);
     	return FALSE;
    }
    memcpy(objNameData,iPtr+nameStart, strSize);

    u=objRoomsStart;
    for(i=0;i<objCount;i++) {
        objNames[i] = objNameData+(bGetW(iPtr)-nameStart);
    	*u++ = iPtr[2];
     	iPtr+=entSize;
    }

	free(p);

    return TRUE;
}
/******************************************************************************/
static BOOL IsLarry1(void)
{
	return gi && gi->title && strcmp(gi->title, "Leisure Suit Larry") == 0;
}
/******************************************************************************/
static U16 PatchGameSpecificLogicMessages(U8 *logicData, U16 logicLen, U8 *scratch, size_t scratchSize)
{
	const char *originalText = "\"This is a computer.\"";
	const char *replacementText = "\"The line disconnected.\"";
	U16 codeLen, msgSize;
	U8 msgTotal;
	U8 *msgBase, *msg, *outMsgBase, *outMsg;
	size_t tableBytes, writePos, maxMessageBytes;
	int i;
	BOOL changed = FALSE;

	if(!IsLarry1() || !logicData || !scratch || logicLen < 5)
		return logicLen;

	codeLen = bGetW(logicData);
	if((size_t)codeLen + 3u > logicLen)
		return logicLen;

	msgBase = logicData + codeLen + 2;
	msgTotal = *msgBase;
	if(!msgTotal)
		return logicLen;

	msg = msgBase + 1;
	msgSize = bGetW(msg);
	tableBytes = ((size_t)msgTotal + 1u) << 1;
	if(msgSize < tableBytes || (size_t)codeLen + 3u + msgSize > logicLen)
		return logicLen;

	memcpy(scratch, logicData, (size_t)codeLen + 3u);
	outMsgBase = scratch + codeLen + 2;
	outMsg = outMsgBase + 1;
	writePos = tableBytes;
	maxMessageBytes = scratchSize - ((size_t)codeLen + 3u);

	for(i = 1; i <= msgTotal; ++i) {
		U16 offset = bGetW(msg + (i << 1));
		const char *src;
		const char *text;
		size_t remaining, textLen;

		if(offset == 0) {
			outMsg[i << 1] = 0;
			outMsg[(i << 1) + 1] = 0;
			continue;
		}
		if(offset < tableBytes || offset >= msgSize)
			return logicLen;

		src = (const char *)(msg + offset);
		remaining = (size_t)msgSize - offset;
		text = src;
		textLen = 0;
		while(textLen < remaining && src[textLen] != '\0')
			textLen++;
		if(textLen >= remaining)
			return logicLen;

		if(strcmp(src, originalText) == 0) {
			text = replacementText;
			textLen = strlen(replacementText);
			changed = TRUE;
		}

		if(writePos + textLen + 1u > maxMessageBytes)
			return logicLen;

		outMsg[i << 1] = (U8)(writePos & 0xFF);
		outMsg[(i << 1) + 1] = (U8)((writePos >> 8) & 0xFF);
		memcpy(outMsg + writePos, text, textLen + 1u);
		writePos += textLen + 1u;
	}

	if(!changed)
		return logicLen;

	outMsg[0] = (U8)(writePos & 0xFF);
	outMsg[1] = (U8)((writePos >> 8) & 0xFF);
	return (U16)((size_t)codeLen + 3u + writePos);
}
/******************************************************************************/
U8 *LoadFile(BOOL G_PATH, const char *name, int *len)
{
	FILE *f;
	int l;
    U8 *p;

	if(G_PATH)
    	sprintf(fname,"%s%s",gi->path,name);
    else
    	strcpy(fname,name);
	if((f=fopen(fname,"rb")) == NULL) {
		ErrorMessage("Unable to open file: \"%s\" for reading!", fname);
		return FALSE;
	}
	fseek(f,0,SEEK_END);
	l=ftell(f);
	fseek(f,0,SEEK_SET);

	p = (U8*) malloc(l);
	fread(p, l, 1, f);
	fclose(f);

    if(len)
    	*len = l;

    return p;
}
/******************************************************************************/
int FindWordx(const char *s,U8 *b)
{
	const char *s1,*s2;
	while(*b) {
    	s1 = (char*)b+3;
        s2 = s;
        if(strcmp(s1,s2)==0)
        	return (b[1]+(b[2]<<8));
        b+=b[0]+1;
    }
    return -1;
}   
/******************************************************************************/
void AddWord(int group, const char *string)
{
	if(HasAddedWord(group, string))
		return;
	pwords->group = group;
	pwords->string = string;
	pwords++;
}
/******************************************************************************/
const char *FindWord(int group)
{
	int a;
    U8 *p;
	for(a=0;a<26;a++) {
		p=(U8*)words[a];
        if(!p) continue;
        while(*p!=0) {
         	if((p[1]+(p[2]<<8))==group)
            	return (char*)p+3;
            p+=*p;
        }
    }
    return "^";
}
/******************************************************************************/
int FindTotalWordsInGroup(int group)
{
	int a;
    int groupcount = 0;
    U8 *p;
	for(a=0;a<26;a++) {
		p=(U8*)words[a];
        if(!p) continue;
        while(*p!=0) {
         	if((p[1]+(p[2]<<8))==group)
            	groupcount++;
            p+=*p;
        }
    }
    return groupcount;
}
/******************************************************************************/
const char **FindExtraWordInGroup(int group)
{
	int a;
    U8 *p;
    const char **slist;

    int maxgroups = FindTotalWordsInGroup(group);
    int gcnt = 0;
    if(maxgroups<2) return NULL;

    slist = (const char **)malloc(sizeof(const char **)*(maxgroups+1));

	for(a=0;a<26;a++) {
		p=(U8*)words[a];
        if(!p) continue;
        while(*p!=0) {
         	if((p[1]+(p[2]<<8))==group) {
                if(gcnt) {
                 	slist[gcnt-1] = ((char*)p+3);
                }
             	gcnt++;
            }
            p+=*p;
        }
    }
    slist[gcnt-1] = NULL;
    return slist;
}
/******************************************************************************/
const char *FindWord2(int group)
{
	WORDSET *w;
	w = wordset;
    while(w->group) {
     	if(w->group==group)
        	return(w->string);
        w++;
    }
    return "^";
}
/******************************************************************************/
int FindWordStr(const char *s)
{
	int group, a;
    U8 *p;
	for(a=0;a<26;a++) {
		p=(U8*)words[a];
        if(!p) continue;
        while(*p!=0) {
         	if(strcmp(s,(char*)(p+3))==0) {
            	group = (p[1]+(p[2]<<8))&0x1FFF;
                return(group);
            }
            p+=*p;
        }
    }
    return 0;
}
/******************************************************************************/
void ClearGroup(int group)
{
	U8 *p;
    int a;
	if(group)
	for(a=0;a<26;a++) {
		p=(U8*)words[a];
        if(!p) continue;
        while(*p!=0) {
         	if(((p[1]+(p[2]<<8))&0x1FFF)==group) {
            	p[1]=0;
                p[2]=0;
            }
            p+=*p;
        }
    }
}
/******************************************************************************/
void DoSolidGroups()
{
	int set=0,idx,groups[5];
    const char *s;

    while(solidGroups[set][0][0]) {
    	idx=0;
    	while((s=solidGroups[set][idx])[0]) {
    		if((groups[idx] = FindWordStr(s))!=0) {
            	AddWord(groups[idx],s);
        	}
        	idx++;
    	}
        while(idx)
        	ClearGroup(groups[--idx]);
        set++;
    }
}
/******************************************************************************/
void DoSolidWords()
{
	int idx=0,group;
    const char *s;

    while((s=solidWords[idx])[0]) {
    	if((group = FindWordStr(s))!=0) {
        	AddWord(group,s);
            ClearGroup(group);
        }
        idx++;
    }
}
/******************************************************************************/
void DoRemainingWords()
{
	const char *s;
    int i;
	for(i=1;i<9999;i++) {
		if((s=FindWord(i))[0]!='^') {
        	AddWord(i,s);
        }
	}
	for(i=1;i<9999;i++) {
    	const char **slist = FindExtraWordInGroup(i);
        if(slist) {
        	const char **sl = slist;
        	if(*sl) {
        		AddWord(i,*sl);
        	    sl++;
        	}
        	free(slist);
        }
	}
}
/******************************************************************************/
void AlphaSortWords()
{
	WORDSET *w1 = wordset, *w0, wt;
            /*
    int q=0;
    Form1->ListBox2->Items->Clear();
    while(w1->group) {
    	q++;
    	Form1->ListBox2->Items->Add(AnsiString(w1->string));
		w1++;
    }        */
    w1 = wordset;
    w0 = w1;   
    if(!w1) return;
    w1++;        
    if(!w1) return;
    while(w1->group) {
        if(strcmp(w1->string,w0->string)<0) {
       		memcpy(&wt, w0,sizeof(WORDSET));
       		memcpy( w0, w1,sizeof(WORDSET));
       		memcpy( w1,&wt,sizeof(WORDSET));
    		w1 = wordset;
        }
		w0 = w1;
        w1++;
    }
    w1 = wordset;
    while(w1->group) {
        w1++;
    }         /*
    w1 = wordset;
    while(w1->group) {
    	Form1->ListBox3->Items->Add(AnsiString(w1->string));
		w1++;
    }   */
}
/******************************************************************************/
BOOL ProcessWords()
{
	FILE *f;
    U16 l, wc;
    int strSize, objCount, i, offs,g;
    U8 *msg,*tokData,*wPtr;
	WORDSET *w;

	if((vocabData = LoadFile(FALSE, "vocab.bin", NULL))==NULL)
    	return FALSE;
	if((tokData = LoadFile(TRUE, "words.tok", NULL))==NULL)
    	return FALSE;

    wordData = (U8*)malloc(64000);
    memset(words,0,sizeof(words));
    wPtr = wordData;
    wc=0;
    for(i=0;i<26;i++) {
     	offs = 0;
        if(bGetW(tokData+(i<<1))==0) continue;
        msg = tokData + ((tokData[i<<1]<<8)|(tokData[(i<<1)+1]));
        words[i] = (const char *)wPtr;
        do {
        	offs = *msg++;
        	do
        		wordbuf[offs++] = (*msg&0x7F)^0x7F;
        	while(*msg++<0x80);
       		wordbuf[offs++] = 0;
            g=beGetW(msg);
            if(g!=1&&g!=9999) {
            	l=strlen(wordbuf)+1;
               	wPtr[0] = l+3;
            	wPtr[1] = g&0xFF;
            	wPtr[2] = g>>8;
            	memcpy(wPtr+3,wordbuf,l);
            	wPtr+=l+3;
            	wc++;
            }
            msg+=2;
        } while(msg[0]);
        *wPtr++=0;
            	wc++;
    }
    wc += CountGameSpecificAliases();
    wordsSize = wPtr-wordData;
	mFree(tokData);

    if(!wc) return TRUE;
	wordset = (WORDSET*)calloc(sizeof(WORDSET),wc);
 	pwords = wordset;
	if(!IsKingsQuest1())
		DoSolidGroups();
    if(!IsKingsQuest1())
        DoSolidWords();
    DoGameSpecificAliases();
    DoRemainingWords();
    //if(pwords!=wordset)
    	AlphaSortWords();
                     /*
    f=fopen("words.txt","w");

	w = wordset;
    while(w->group) {
     	fprintf(f,"%05d:%s\n",w->group,w->string);
        w++;
    }
    fclose(f);  */

    return TRUE;
}
/******************************************************************************/
int sfcount;
int curlogsX[256], *curlogs;
U8 vVals[256][8];
void DumpLog(int num)
{
	U8 *xcode=code,*log,*end,*flags,*logDir;
    int pc,op,g,a,i;
    char *p;

	if(LOG_DONE[num]) return;
    LOG_DONE[num]=TRUE;

	if(dirs[0][num].vol==-1) return;
	if((logDir = &volData[dirs[0][num].offset]) == NULL) return;

	log=logDir+5;
    end=logDir+7+(logDir[5]+(logDir[6]<<8));

    code=log+2;

    if(lwPtrs[num]&&curlogs==curlogsX) return;
    sfcount = 0;
    *curlogs++ = num;

    while(code<end) {
        op = *code++;
     	if(op==0xFF) {
        	ExecuteIF();
        } else {
        	switch(op) {
             	case 3: // assignn
            		for(i=0;i<8;i++) {
                    	if(vVals[*code][i]==code[1])
                        	break;
                		if(!vVals[*code][i]) {
                        	vVals[*code][i] = code[1];
                            break;
                        }
                    }
                    break;
             	case 22:
             	 	DumpLog(*code);
                    break;
                case 23:
            		for(i=0;i<8;i++) {
                		if(!vVals[*code][i]) break;
             			 DumpLog(vVals[*code][i]);
                	}
                    break;
            }
            code += agiCommands[op].nParams;
        }
    }

    *curlogs--=-1;
	code=xcode;
}
/******************************************************************************/
void ExecuteIF()
{
	int op;
    int saidCnt,g;
    int *m;
    int n;
	for(;;) {
    	op = *code++;
        if(op==0xFF) break;
    	if(op<0xFC) {
        	if(op==0xE) {
             	saidCnt=*code++;
               	//fprintf(lf,"( ");
                while(saidCnt) {
                	g = (code[0]+(code[1]<<8));
                    if(g&&g!=9999) {
                    	for(m=curlogsX;m<curlogs;m++) {
                        	n=*m;
                        	if(n<0||n>255) {
                            	m=m;
                            	ErrorMessage("M");
                            }
                            if(!lwPtrs[n])
                            	lwPtrs[n] = (U8*)calloc(lwfsize,1);
                        	if(!lwPtrs[n])
                            	lwPtrs[n]=lwPtrs[n];
                            if((g>=4096)||((g>>3)>=lwfsize)) {
                            	g=g;
                            	//ErrorMessage("G");
                            } else {
                            	lwPtrs[n][g>>2]|=((*curlogsX==0||m==curlogsX)?1:2)<<((g&3)<<1);
                            }
                        }
                    }
                    	//fprintf(lf,"\"%s\" ",FindWord2(g));
                    code+=2;
                    saidCnt--;
                }
               	//fprintf(lf,")\n");
                sfcount++;
            } else code += testCommands[op].nParams;
		}
	}
	code+=2;
    //fprintf(lf,"-\n");
}
/******************************************************************************/
U32 ptrs[14];
U32 wm[256];
BOOL OutputGame()
{
	int t,i,j,len,q,a,g;
    U32 fstart;
	WORDSET *w;
    char ccc;
    int wSize;
    int maxgrp, ps, nWc;
    char *wBuf=NULL,*wPtr;

    memset(ptrs,0,sizeof(ptrs));

    ptrs[1] = offs;
    fwrite(volData,volSize,1,fout);
	offs += volSize;
    ALIGN(fout);

    for(t=0;t<4;t++) {
    	ptrs[2+t] = offs;
    	for(i=0;i<256;i++) {
    		fputl(
        		(dirs[t][i].vol==-1)?0:ptrs[1]+dirs[t][i].offset,
                fout
            );
    	}
    	offs += 1024;
    }
    ALIGN(fout);

    ptrs[6] = offs;
    fwrite(objNameData,strSize,1,fout);
	offs += strSize;
    ALIGN(fout);

    ptrs[7] = offs;
    for(i=0;i<256;i++) {
        fputl(
          	(objNames[i])?
            	(ptrs[6]+(intptr_t)((intptr_t)objNames[i]-(intptr_t)objNameData)):0,
            fout
        );
    }
    offs += 1024;

    ptrs[8] = offs;
    fwrite(objRoomsStart,256,1,fout);
	offs += 256;

    maxgrp=0;
    wSize = 1;
	w = wordset;
    if(w) {
    if(w->group) {
    	ccc = w->string[0];
    	while(w->group) {
        	if(w->string[0]!=ccc) {
        		wSize++;
        	}
    		wSize += strlen(w->string)+1;
        	wSize += 3;
        	w++;
    	}
    }
    wBuf = (char*)calloc(wSize,1);
    wPtr = wBuf;
    w = wordset;
    if(w->group) {
    ccc = w->string[0];
    ps=offs;
    nWc=0;
    while(w->group) {
        g = w->group & 0x1FFF;
        if(g > maxgrp)
            maxgrp = g;
        q = FindWordx(w->string, vocabData);
        if((q != -1) && (q & 0x80))
            w->group = g | 0x8000;
        else
            w->group = g;
        w++;
    }
    w = wordset;
    while(w->group) {
        if(w->string[0]!=ccc) {
        	ccc = w->string[0];
        	*wPtr++ = 0;
            ps++;
        }
        w->addr = ps;
    	len = strlen(w->string)+1;

        *wPtr++ = (len+3);
        //q=FindWordx(w->string,vocabData);

        *wPtr++ = (w->group&0xFF);
        *wPtr++ = (w->group>>8);

    	memcpy(wPtr,w->string,len);
        w->string = wPtr;
        wPtr += len;
        ps += len+3;
        nWc++;
        w++;
    } }
    *wPtr++ = 0;
    maxgrp++;
	}

    ptrs[9] = offs;
    if(wBuf) {
    	fwrite(wBuf,wSize,1,fout);
		offs += wSize;
    } else {
     	fputl(0,fout);
        offs+=4;
    }
    ALIGN(fout);


    ptrs[10] = offs;
    if(wBuf) {

    ccc='a';
    memset(words,0,sizeof(words));
    for(a=0;a<26;a++) {
		w = wordset;
    	while(w->group&0x1FFF) {
    		if(w->string[0]==ccc) {
        		words[a] = w->string-3;
                break;
            }
        	w++;
    	}
        ccc++;
    }

    	for(i=0;i<26;i++) {
        	q = (words[i])?
        		(ptrs[9]+(intptr_t)((intptr_t)words[i]-(intptr_t)wBuf)):0;
        	fwrite(&q,4,1,fout);
    	}
    } else {
     	for(i=0;i<26;i++) {
         	fputl(0,fout);
        }
    }
    offs += 26*4;
    ALIGN(fout);

    mFree(wBuf);



    lwfsize = (maxgrp>>2)+1;

    memset(vVals,0,sizeof(vVals));
    if(lwfsize)
    	for(i=0;i<256;i++) {
    		memset(LOG_DONE,0,sizeof(LOG_DONE));
   			curlogs = curlogsX;
    		code = NULL;
    		DumpLog(i);
            if(i&&lwPtrs[i]&&lwPtrs[0]) {
             	for(j=0;j<lwfsize;j++) {
                 	lwPtrs[i][j]|=lwPtrs[0][j]<<1;
                }
            }
    	}

    ptrs[11] = offs;
    for(i=0;i<256;i++) {
        if(lwPtrs[i]) {
        	wm[i] = offs;
        	fwrite(lwPtrs[i],lwfsize,1,fout);
            offs += lwfsize;
        } else
        	wm[i] = 0;
    }
    ALIGN(fout);

    ptrs[12] = offs;
    fwrite(wm,sizeof(wm),1,fout);
    offs += sizeof(wm);

    ptrs[13] = maxgrp;

    fstart = ftell(fout);

    fseek(fout,giPos,SEEK_SET);
    giPos += 80;

  	fwrite("AGi",3,1,fout);
    fputc(gi->version->major,fout);
    fputw(gi->version->minor,fout);
  	fwrite(gi->title,strlen(gi->title)+1,1,fout);
    for(i=strlen(gi->title)+1;i<26;i++)
    	fputc(0,fout);

    fwrite(ptrs+1,sizeof(ptrs)-8,1,fout);
    fseek(fout,fstart,SEEK_SET);   

   	return TRUE;
}
/******************************************************************************/
BOOL ProcessGame(GAMEINFO *gmInfo)
{
	int i;
    FILE *f;

	memset(VUSED,0,sizeof(VUSED));
	volData = NULL;
    memset(objNames,0,sizeof(objNames));
    memset(lwPtrs,0,sizeof(lwPtrs));
    objNameData = NULL;
    vocabData = NULL;
    wordData = NULL;
    wordset = NULL;

    gi = gmInfo;

	for(i=0;i<4;i++) {
		if(!LoadDir(gi->version->flags&SINGLE_DIR, i)) {
            printf("ProcessGame failed: LoadDir(%d)\n", i);
			return FALSE;
        }
	}
	if(!PrepDirs()) {
        printf("ProcessGame failed: PrepDirs\n");
		return FALSE;
    }
	if(!ProcessDirs()) {
        printf("ProcessGame failed: ProcessDirs\n");
		return FALSE;
    }
	if(!ProcessObject()) {
        printf("ProcessGame failed: ProcessObject\n");
		return FALSE;
    }
	if(!ProcessWords()) {
        printf("ProcessGame failed: ProcessWords\n");
		return FALSE;
    }

    if(!OutputGame()) {
        printf("ProcessGame failed: OutputGame\n");
    	return FALSE;
    }
                         /*
    f=fopen("vols.bin","wb");
    fwrite(volData,volSize,1,f);
    fclose(f);     */
		
	return TRUE;
}
/******************************************************************************/
void FreeGame()
{
	int i;

	mFree(volData);
	mFree(objNameData);
    mFree(vocabData);
    mFree(wordData);
    mFree(wordset);
 
    for(i=0;i<256;i++) {
    	mFree(lwPtrs[i]);
    }
}
/******************************************************************************/
