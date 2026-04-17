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

//---------------------------------------------------------------------------

#include "vcl-shim/vcl-shim.h"
#include "main.h"
#include "addgame.h"
#include "verdef.h"
#include "../gbagi.h"
#include <shellapi.h>
#include <string>
#include <vector>
#include <algorithm>
#include <cwchar>
#include <cwctype>
#include <cstdio>
#pragma hdrstop

//---------------------------------------------------------------------------
static void CliPrint(const wchar_t *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vwprintf(fmt, args);
    va_end(args);
}

static void CliPrintUsage()
{
    CliPrint(
        L"GBAGI injector CLI\n"
        L"Usage:\n"
        L"  gbinject.exe --cli --input <gbagi.bin> --vocab <vocab.bin> --output <out.gba>\n"
        L"               --game-dir <dir> [--game <known title>]\n"
        L"  gbinject.exe --cli --input <gbagi.bin> --vocab <vocab.bin> --output <out.gba>\n"
        L"               --all-games-root <dir>\n"
        L"               [--title <display title>] [--version-index <n>] [--game-id <id>]\n"
        L"\n"
        L"Examples:\n"
        L"  gbinject.exe --cli --input gbagi.bin --vocab vocab.bin --output PQ1.gba \\\n"
        L"    --game-dir H:\\CODEX\\GBAGI\\P-Q1 --game \"Police Quest\"\n"
    );
}

static bool CliEquals(const wchar_t *a, const wchar_t *b)
{
    return _wcsicmp(a, b) == 0;
}

static bool CliStartsCliMode(int argc, wchar_t **argv)
{
    for(int i=1; i<argc; i++) {
        if(CliEquals(argv[i], L"--cli"))
            return true;
    }
    return false;
}

static const GAMEINFO *CliFindKnownGame(const wchar_t *title)
{
    GAMEINFO *gi = games;
    while(gi->title) {
        wchar_t wtitle[256];
        mbstowcs(wtitle, gi->title, sizeof(wtitle) / sizeof(wtitle[0]));
        wtitle[(sizeof(wtitle) / sizeof(wtitle[0])) - 1] = L'\0';
        if(_wcsicmp(wtitle, title) == 0)
            return gi;
        gi++;
    }
    return NULL;
}

static const VERLIST *CliFindVersionByName(const wchar_t *name)
{
    VERLIST *vl = verlist;
    while(vl->name) {
        if(_wcsicmp(vl->name, name) == 0)
            return vl;
        vl++;
    }
    return NULL;
}

static std::wstring CliEnsureTrailingSlash(const wchar_t *path)
{
    std::wstring out(path ? path : L"");
    if(!out.empty() && out[out.size()-1] != L'\\' && out[out.size()-1] != L'/')
        out += L'\\';
    return out;
}

static bool EnsureBatterylessPad16M(const TCHAR *filename)
{
	FILE *f;
	long fileSize;
	char zeros[4096];

	memset(zeros, 0, sizeof(zeros));
	f = _tfopen(filename, _T("ab"));
	if(!f)
		return false;

	fseek(f, 0, SEEK_END);
	fileSize = ftell(f);
	while(fileSize < (16L * 1024L * 1024L)) {
		long remaining = (16L * 1024L * 1024L) - fileSize;
		size_t chunk = (remaining > (long)sizeof(zeros)) ? sizeof(zeros) : (size_t)remaining;
		fwrite(zeros, 1, chunk, f);
		fileSize += (long)chunk;
	}
	fclose(f);
	return true;
}

struct CliGameSpec
{
    std::wstring path;
    std::string title;
    std::string gameId;
    VERLIST *version;
};

static void CliFreeVocabPlan(VOCAB_PLAN_ENTRY *plan)
{
    while(plan) {
        VOCAB_PLAN_ENTRY *next = plan->next;
        if(plan->word)
            free(plan->word);
        free(plan);
        plan = next;
    }
}

static VOCAB_PLAN_ENTRY *CliLoadVocabPlanFromPresetFile(const wchar_t *filename)
{
    FILE *f = _tfopen(filename, _T("rb"));
    char line[2048];
    bool isV3 = false;
    bool isV4 = false;
    VOCAB_PLAN_ENTRY *head = NULL;
    VOCAB_PLAN_ENTRY *tail = NULL;

    if(!f)
        return NULL;
    if(!fgets(line, sizeof(line), f)) {
        fclose(f);
        return NULL;
    }
    if(strncmp(line, "GBAGI_VOCAB_PRESET_V4", 21) == 0) {
        isV3 = true;
        isV4 = true;
    } else if(strncmp(line, "GBAGI_VOCAB_PRESET_V3", 21) == 0) {
        isV3 = true;
    } else if(strncmp(line, "GBAGI_VOCAB_PRESET_V2", 21) != 0) {
        fclose(f);
        return NULL;
    }

    while(fgets(line, sizeof(line), f)) {
        char *ctx = NULL;
        char *groupS = strtok_s(line, "\t\r\n", &ctx);
        char *keepS = strtok_s(NULL, "\t\r\n", &ctx);
        char *removeS = strtok_s(NULL, "\t\r\n", &ctx);
        char *hideS = strtok_s(NULL, "\t\r\n", &ctx);
        char *addS = NULL;
        char *replaceS = NULL;
        char *showS = NULL;
        char *colS = NULL;
        char *wordS = NULL;

        if(isV3) {
            addS = strtok_s(NULL, "\t\r\n", &ctx);
            replaceS = strtok_s(NULL, "\t\r\n", &ctx);
            if(isV4) {
                showS = strtok_s(NULL, "\t\r\n", &ctx);
                colS = strtok_s(NULL, "\t\r\n", &ctx);
            } else {
                colS = strtok_s(NULL, "\t\r\n", &ctx);
            }
            wordS = strtok_s(NULL, "\r\n", &ctx);
        } else {
            colS = strtok_s(NULL, "\t\r\n", &ctx);
            wordS = strtok_s(NULL, "\r\n", &ctx);
        }

        if(!groupS || !keepS || !removeS || !hideS || !colS)
            continue;
        if(isV3 && (!addS || !replaceS))
            continue;
        if(!wordS)
            wordS = (char*)"";

        VOCAB_PLAN_ENTRY *entry = (VOCAB_PLAN_ENTRY*)calloc(1, sizeof(VOCAB_PLAN_ENTRY));
        if(!entry) {
            CliFreeVocabPlan(head);
            fclose(f);
            return NULL;
        }

        entry->group = atoi(groupS);
        entry->forceKeep = atoi(keepS) ? TRUE : FALSE;
        entry->forceRemove = atoi(removeS) ? TRUE : FALSE;
        entry->hidden = atoi(hideS) ? TRUE : FALSE;
        entry->addAlias = isV3 && atoi(addS) ? TRUE : FALSE;
        entry->replaceGroup = isV3 && atoi(replaceS) ? TRUE : FALSE;
        entry->pickerVisibility = (isV4 && showS) ? atoi(showS) : -1;
        entry->columnOverride = atoi(colS);
        entry->word = _strdup(wordS);
        if(!entry->word) {
            free(entry);
            CliFreeVocabPlan(head);
            fclose(f);
            return NULL;
        }

        if(!head)
            head = entry;
        else
            tail->next = entry;
        tail = entry;
    }

    fclose(f);
    return head;
}

static std::wstring CliFindPresetPathForGameDir(const wchar_t *gameDir)
{
    std::wstring base = CliEnsureTrailingSlash(gameDir);
    std::wstring preferred = base + L"gbagi_vocab_preset.tsv";
    if(FileExists(preferred.c_str()))
        return preferred;

    std::vector<std::wstring> matches;
    WIN32_FIND_DATAW findData;
    HANDLE hFind = FindFirstFileW((base + L"*.tsv").c_str(), &findData);
    if(hFind == INVALID_HANDLE_VALUE)
        return L"";

    do {
        if(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            continue;
        matches.push_back(base + findData.cFileName);
    } while(FindNextFileW(hFind, &findData));
    FindClose(hFind);

    if(matches.empty())
        return L"";

    std::sort(matches.begin(), matches.end(), [](const std::wstring &a, const std::wstring &b) {
        return _wcsicmp(a.c_str(), b.c_str()) < 0;
    });
    return matches[0];
}

static std::wstring CliNormalizeName(const wchar_t *text)
{
    std::wstring out;
    if(!text)
        return out;

    while(*text) {
        wchar_t ch = (wchar_t)towlower(*text++);
        if((ch >= L'a' && ch <= L'z') || (ch >= L'0' && ch <= L'9'))
            out += ch;
    }
    return out;
}

static std::wstring CliGetLeafName(const std::wstring &path)
{
    if(path.empty())
        return path;

    size_t end = path.size();
    while(end > 0 && (path[end-1] == L'\\' || path[end-1] == L'/'))
        end--;

    size_t start = path.find_last_of(L"\\/", end == 0 ? 0 : end - 1);
    if(start == std::wstring::npos)
        start = 0;
    else
        start++;

    return path.substr(start, end - start);
}

static std::string CliWideToAnsi(const std::wstring &text)
{
    char buf[256];
    wcstombs(buf, text.c_str(), sizeof(buf));
    buf[sizeof(buf) - 1] = '\0';
    return std::string(buf);
}

static const GAMEINFO *CliFindKnownGameByFolderName(const wchar_t *folderName)
{
    std::wstring key = CliNormalizeName(folderName);

    if(key == L"kq1") return CliFindKnownGame(L"King's Quest 1");
    if(key == L"kq2") return CliFindKnownGame(L"King's Quest 2");
    if(key == L"kq3") return CliFindKnownGame(L"King's Quest 3");
    if(key == L"kq4") return CliFindKnownGame(L"King's Quest 4");
    if(key == L"larry1" || key == L"lsl1") return CliFindKnownGame(L"Leisure Suit Larry");
    if(key == L"pq1" || key == L"pq") return CliFindKnownGame(L"Police Quest");
    if(key == L"sq1") return CliFindKnownGame(L"Space Quest 1");
    if(key == L"sq2") return CliFindKnownGame(L"Space Quest 2");

    GAMEINFO *gi = games;
    while(gi->title) {
        wchar_t wtitle[256];
        mbstowcs(wtitle, gi->title, sizeof(wtitle) / sizeof(wtitle[0]));
        wtitle[(sizeof(wtitle) / sizeof(wtitle[0])) - 1] = L'\0';
        if(CliNormalizeName(wtitle) == key)
            return gi;
        gi++;
    }
    return NULL;
}

static std::string CliFriendlyTitleForFolderName(const std::wstring &folderName)
{
    std::wstring key = CliNormalizeName(folderName.c_str());

    if(key == L"kq6")
        return "King's Quest 6";
    if(key == L"sq0")
        return "Space Quest 0";

    return CliWideToAnsi(CliGetLeafName(folderName));
}

static VERLIST *CliDetectVersionFromFolder(const wchar_t *gameDir)
{
    static const wchar_t *knownFiles[] = {
        L"agidata.ovl",
        L"AGIDATA.OVL",
        L"agi",
        L"AGI",
        L"sierra.com",
        L"SIERRA.COM",
        L"KQ6AGI.EXE",
        L"KQ6AGI.exe",
        NULL
    };

    std::wstring base = CliEnsureTrailingSlash(gameDir);
    for(int i = 0; knownFiles[i]; i++) {
        std::wstring candidate = base + knownFiles[i];
        if(FileExists(candidate.c_str())) {
            VERLIST *version = FindAGIVersion((wchar_t*)candidate.c_str());
            if(version)
                return version;
        }
    }

    WIN32_FIND_DATAW findData;
    HANDLE hFind = FindFirstFileW((base + L"*").c_str(), &findData);
    if(hFind == INVALID_HANDLE_VALUE)
        return NULL;

    do {
        if(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            continue;

        std::wstring filename = findData.cFileName;
        std::wstring normalized = CliNormalizeName(filename.c_str());
        if(normalized.find(L"agi") == std::wstring::npos)
            continue;

        std::wstring candidate = base + filename;
        VERLIST *version = FindAGIVersion((wchar_t*)candidate.c_str());
        if(version) {
            FindClose(hFind);
            return version;
        }
    } while(FindNextFileW(hFind, &findData));

    FindClose(hFind);

    if(
        FileExists((base + L"LOGDIR").c_str()) || FileExists((base + L"logdir").c_str())
    ) {
        return &verlist[1];
    }

    return NULL;
}

static bool CliPackGameList(
    const wchar_t *inputPath,
    const wchar_t *vocabPath,
    const wchar_t *outputPath,
    const std::vector<CliGameSpec> &gamesToPack
)
{
    FILE *fin;
    int i, l;

    inromName = _wcsdup(inputPath);
    outromName = _wcsdup(outputPath);
    vocabName = _wcsdup(vocabPath);

    if(FileExists(vocabName) == false || (fin = _tfopen(vocabName, _T("rb"))) == NULL) {
        CliPrint(L"Error opening vocab file: %ls\n", vocabName);
        return false;
    }
    fclose(fin);
    if(FileExists(inromName) == false || (fin = _tfopen(inromName, _T("rb"))) == NULL) {
        CliPrint(L"Error opening input ROM: %ls\n", inromName);
        return false;
    }
    if((fout = _tfopen(outromName, _T("wb"))) == NULL) {
        fclose(fin);
        CliPrint(L"Error opening output file: %ls\n", outromName);
        return false;
    }

    fseek(fin, 0, SEEK_END);
    l = ftell(fin);
    fseek(fin, 0, SEEK_SET);
    for(i = 0; i < l; i++)
        fputc(fgetc(fin), fout);
    fclose(fin);

    S32 BASEx0X = (l + AGI_DATA_ALIGNMENT - 1) & -AGI_DATA_ALIGNMENT;
    for(i = l; i < BASEx0X; i++)
        fputc(0xFF, fout);
    U32 BASE80X = BASE800 + BASEx0X;

    offs = BASE80X;

    fwrite(agiid, IDSIZE, 1, fout);
    fputc((int)gamesToPack.size(), fout);
    for(i = IDSIZE + 1; i < 0x20; i++)
        fputc(0, fout);
    offs += 0x20;

    giPos = ftell(fout);
    for(i = (int)gamesToPack.size() * 80; i > 0; i--)
        fputc(0, fout);
    offs += (U32)gamesToPack.size() * 80;

    for(size_t gameIndex = 0; gameIndex < gamesToPack.size(); gameIndex++) {
        const CliGameSpec &spec = gamesToPack[gameIndex];
        std::wstring presetPath = CliFindPresetPathForGameDir(spec.path.c_str());
        VOCAB_PLAN_ENTRY *vocabPlan = NULL;
        if(!presetPath.empty()) {
            vocabPlan = CliLoadVocabPlanFromPresetFile(presetPath.c_str());
            if(vocabPlan)
                CliPrint(L"  Using vocab preset: %ls\n", CliGetLeafName(presetPath).c_str());
            else
                CliPrint(L"  Warning: failed to load vocab preset: %ls\n", presetPath.c_str());
        }
        GAMEINFO cliGame = {
            spec.version,
            spec.gameId.c_str(),
            spec.title.c_str(),
            spec.path.c_str(),
            vocabPlan
        };

        CliPrint(L"Packing game %d/%d: %S\n", (int)gameIndex + 1, (int)gamesToPack.size(), spec.title.c_str());
        if(!ProcessGame(&cliGame)) {
            FreeGame();
            CliFreeVocabPlan(vocabPlan);
            fclose(fout);
            fout = NULL;
            CliPrint(L"Error processing game: %S\n", spec.title.c_str());
            return false;
        }
        FreeGame();
        CliFreeVocabPlan(vocabPlan);
    }

    fclose(fout);
    fout = NULL;
	if(!FixOutputRomForHardware(outromName, NULL))
	{
		CliPrint(L"Warning: built ROM, but hardware-header fixing failed.\n");
	}
	if(!EnsureBatterylessPad16M(outromName))
	{
		CliPrint(L"Warning: built ROM, but 16MB padding failed.\n");
	}
	return true;
}

static bool CliPackOneGame(
    const wchar_t *inputPath,
    const wchar_t *vocabPath,
    const wchar_t *outputPath,
    const wchar_t *gameDir,
    const char *title,
    const char *gameId,
    VERLIST *version
)
{
    CliGameSpec spec;
    spec.path = CliEnsureTrailingSlash(gameDir);
    spec.title = title;
    spec.gameId = gameId ? gameId : "";
    spec.version = version;

    std::vector<CliGameSpec> oneGame;
    oneGame.push_back(spec);
    return CliPackGameList(inputPath, vocabPath, outputPath, oneGame);
}

static int RunCliMode()
{
    int argc = 0;
    wchar_t **argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    if(argv == NULL)
        return 1;

    const wchar_t *inputPath = NULL;
    const wchar_t *vocabPath = NULL;
    const wchar_t *outputPath = NULL;
    const wchar_t *gameDir = NULL;
    const wchar_t *allGamesRoot = NULL;
    const wchar_t *knownGame = NULL;
    const wchar_t *titleOverride = NULL;
    const wchar_t *versionName = NULL;
    const wchar_t *gameIdOverride = NULL;
    int versionIndex = -1;

    for(int i = 1; i < argc; i++) {
        const wchar_t *arg = argv[i];
        if(CliEquals(arg, L"--cli")) {
            continue;
        } else if(CliEquals(arg, L"--help") || CliEquals(arg, L"-h")) {
            CliPrintUsage();
            LocalFree(argv);
            return 0;
        } else if(i + 1 < argc && CliEquals(arg, L"--input")) {
            inputPath = argv[++i];
        } else if(i + 1 < argc && CliEquals(arg, L"--vocab")) {
            vocabPath = argv[++i];
        } else if(i + 1 < argc && CliEquals(arg, L"--output")) {
            outputPath = argv[++i];
        } else if(i + 1 < argc && CliEquals(arg, L"--game-dir")) {
            gameDir = argv[++i];
        } else if(i + 1 < argc && CliEquals(arg, L"--all-games-root")) {
            allGamesRoot = argv[++i];
        } else if(i + 1 < argc && CliEquals(arg, L"--game")) {
            knownGame = argv[++i];
        } else if(i + 1 < argc && CliEquals(arg, L"--title")) {
            titleOverride = argv[++i];
        } else if(i + 1 < argc && CliEquals(arg, L"--version-name")) {
            versionName = argv[++i];
        } else if(i + 1 < argc && CliEquals(arg, L"--version-index")) {
            versionIndex = _wtoi(argv[++i]);
        } else if(i + 1 < argc && CliEquals(arg, L"--game-id")) {
            gameIdOverride = argv[++i];
        } else {
            CliPrint(L"Unknown or incomplete argument: %ls\n\n", arg);
            CliPrintUsage();
            LocalFree(argv);
            return 2;
        }
    }

    if(!inputPath || !vocabPath || !outputPath || (!gameDir && !allGamesRoot)) {
        CliPrint(L"Missing required arguments.\n\n");
        CliPrintUsage();
        LocalFree(argv);
        return 2;
    }

    if(gameDir && allGamesRoot) {
        CliPrint(L"Use either --game-dir or --all-games-root, not both.\n");
        LocalFree(argv);
        return 2;
    }

    if(allGamesRoot) {
        std::wstring root = CliEnsureTrailingSlash(allGamesRoot);
        WIN32_FIND_DATAW findData;
        HANDLE hFind = FindFirstFileW((root + L"*").c_str(), &findData);
        if(hFind == INVALID_HANDLE_VALUE) {
            CliPrint(L"Unable to open games root: %ls\n", root.c_str());
            LocalFree(argv);
            return 2;
        }

        std::vector<std::wstring> folders;
        do {
            if(!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
                continue;
            if(wcscmp(findData.cFileName, L".") == 0 || wcscmp(findData.cFileName, L"..") == 0)
                continue;
            folders.push_back(findData.cFileName);
        } while(FindNextFileW(hFind, &findData));
        FindClose(hFind);

        std::sort(folders.begin(), folders.end(), [](const std::wstring &a, const std::wstring &b) {
            return _wcsicmp(a.c_str(), b.c_str()) < 0;
        });

        if(folders.empty()) {
            CliPrint(L"No game folders found in: %ls\n", root.c_str());
            LocalFree(argv);
            return 2;
        }

        std::vector<CliGameSpec> gamesToPack;
        gamesToPack.reserve(folders.size());

        for(size_t i = 0; i < folders.size(); i++) {
            std::wstring folder = folders[i];
            std::wstring fullPath = root + folder;
            const GAMEINFO *known = CliFindKnownGameByFolderName(folder.c_str());
            VERLIST *version = known ? known->version : CliDetectVersionFromFolder(fullPath.c_str());
            if(!version) {
                CliPrint(L"Unable to determine AGI version for folder: %ls\n", fullPath.c_str());
                LocalFree(argv);
                return 2;
            }

            CliGameSpec spec;
            spec.path = CliEnsureTrailingSlash(fullPath.c_str());
            spec.version = version;
            if(known) {
                spec.title = known->title;
                spec.gameId = known->vID ? known->vID : "";
            } else {
                spec.title = CliFriendlyTitleForFolderName(folder);
                spec.gameId = "";
            }
            gamesToPack.push_back(spec);
        }

        bool ok = CliPackGameList(inputPath, vocabPath, outputPath, gamesToPack);
        mFree(inromName);
        mFree(outromName);
        mFree(vocabName);
        LocalFree(argv);
        return ok ? 0 : 1;
    }

    const GAMEINFO *known = NULL;
    if(knownGame) {
        known = CliFindKnownGame(knownGame);
        if(!known) {
            CliPrint(L"Unknown game title: %ls\n", knownGame);
            LocalFree(argv);
            return 2;
        }
    } else if(gameDir) {
        std::wstring gameLeaf = CliGetLeafName(gameDir);
        known = CliFindKnownGameByFolderName(gameLeaf.c_str());
    }

    VERLIST *version = NULL;
    if(versionIndex >= 0) {
        if(verlist[versionIndex].name == NULL) {
            CliPrint(L"Invalid version index: %d\n", versionIndex);
            LocalFree(argv);
            return 2;
        }
        version = &verlist[versionIndex];
    } else if(versionName) {
        version = const_cast<VERLIST*>(CliFindVersionByName(versionName));
        if(!version) {
            CliPrint(L"Unknown version name: %ls\n", versionName);
            LocalFree(argv);
            return 2;
        }
    } else if(known) {
        version = known->version;
    } else {
        std::wstring agiData = CliEnsureTrailingSlash(gameDir) + L"agidata.ovl";
        version = FindAGIVersion((wchar_t*)agiData.c_str());
        if(!version) {
            CliPrint(L"Unable to autodetect AGI version from %ls\n", agiData.c_str());
            CliPrint(L"Pass --game, --version-index, or --version-name.\n");
            LocalFree(argv);
            return 2;
        }
    }

    const char *gameId = "";
    std::string gameIdStorage;
    if(gameIdOverride) {
        char buf[64];
        wcstombs(buf, gameIdOverride, sizeof(buf));
        buf[sizeof(buf) - 1] = '\0';
        gameIdStorage = buf;
        gameId = gameIdStorage.c_str();
    } else if(known) {
        gameId = known->vID;
    }

    std::string titleStorage;
    const char *title = NULL;
    if(titleOverride) {
        char buf[128];
        wcstombs(buf, titleOverride, sizeof(buf));
        buf[sizeof(buf) - 1] = '\0';
        titleStorage = buf;
        title = titleStorage.c_str();
    } else if(known) {
        title = known->title;
    } else {
        const wchar_t *base = wcsrchr(gameDir, L'\\');
        if(!base) base = gameDir;
        else base++;
        char buf[128];
        wcstombs(buf, base, sizeof(buf));
        buf[sizeof(buf) - 1] = '\0';
        titleStorage = buf;
        title = titleStorage.c_str();
    }

    bool ok = CliPackOneGame(inputPath, vocabPath, outputPath, gameDir, title, gameId, version);
    mFree(inromName);
    mFree(outromName);
    mFree(vocabName);
    LocalFree(argv);
    return ok ? 0 : 1;
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    int argc = 0;
    wchar_t **argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    bool cliMode = (argv != NULL) ? CliStartsCliMode(argc, argv) : false;
    if(argv != NULL)
        LocalFree(argv);

    if(cliMode)
        return RunCliMode();

	TFormMain* FormMain = new TFormMain(NULL);
	FormMain->CreateForm();
	MSG msg = { };
	while (GetMessage(&msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	delete FormMain;

	return 0;
}
//---------------------------------------------------------------------------
