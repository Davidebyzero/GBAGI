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
#include <shellapi.h>
#pragma hdrstop

#include "main.h"
#include "main.res.h"
#include "addgame.h"
#include "vocabedit.h"
#include <cstdio>
#include <algorithm>
#include <cctype>
#include <map>
#include <set>
#include <string>
#include <vector>
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"

static const int kMaxInjectGames = 255;

struct WalkItemResult {
    std::string item;
    std::string status;
    int group;
    std::vector<std::string> survivors;
};

static std::string TrimAsciiStr(const std::string &value)
{
    size_t start = 0;
    size_t end = value.size();
    while(start < end && std::isspace((unsigned char)value[start])) start++;
    while(end > start && std::isspace((unsigned char)value[end - 1])) end--;
    return value.substr(start, end - start);
}

static std::string LowerAsciiStr(const std::string &value)
{
    std::string out = value;
    for(size_t i = 0; i < out.size(); ++i)
        out[i] = (char)std::tolower((unsigned char)out[i]);
    return out;
}

static std::string NormalizeWalkWord(const std::string &value)
{
    std::string out;
    bool lastSpace = false;
    for(size_t i = 0; i < value.size(); ++i) {
        unsigned char c = (unsigned char)value[i];
        if(std::isalnum(c)) {
            out.push_back((char)std::tolower(c));
            lastSpace = false;
        } else if(c == '\'' || c == '/' || c == '-') {
            if(!out.empty() && !lastSpace) {
                out.push_back((char)c);
                lastSpace = false;
            }
        } else if(std::isspace(c) || std::strchr(".,!?;:()[]{}<>\"`", c)) {
            if(!out.empty() && !lastSpace) {
                out.push_back(' ');
                lastSpace = true;
            }
        }
    }
    return TrimAsciiStr(out);
}

static bool LoadTextFileUtf8ish(const TCHAR *filename, std::string &out)
{
    FILE *f = _tfopen(filename, _T("rb"));
    long len;
    std::vector<char> buf;
    if(!f) return false;
    fseek(f, 0, SEEK_END);
    len = ftell(f);
    fseek(f, 0, SEEK_SET);
    if(len < 0) {
        fclose(f);
        return false;
    }
    buf.resize((size_t)len);
    if(len > 0)
        fread(&buf[0], 1, (size_t)len, f);
    fclose(f);
    out.assign(buf.begin(), buf.end());
    if(out.size() >= 3 &&
       (unsigned char)out[0] == 0xEF &&
       (unsigned char)out[1] == 0xBB &&
       (unsigned char)out[2] == 0xBF)
        out.erase(0, 3);
    return true;
}

static bool LoadWordsTokMap(const TCHAR *gamePath, std::map<std::string, int> &tokWords)
{
    char path[1024];
    char *tmpPath;
    FILE *f;
    unsigned char *data = NULL;
    long len;
    std::vector<unsigned short> offsets(26, 0);

    tmpPath = strdup_tchar_to_char(gamePath);
    strncpy(path, tmpPath, sizeof(path) - 1);
    path[sizeof(path) - 1] = 0;
    free(tmpPath);
    size_t l = strlen(path);
    if(l && path[l - 1] != '\\' && path[l - 1] != '/')
        strcat(path, "\\");
    strcat(path, "words.tok");

    f = fopen(path, "rb");
    if(!f) return false;
    fseek(f, 0, SEEK_END);
    len = ftell(f);
    fseek(f, 0, SEEK_SET);
    data = (unsigned char*)malloc(len);
    if(!data) {
        fclose(f);
        return false;
    }
    fread(data, 1, len, f);
    fclose(f);

    for(int i = 0; i < 26; ++i) offsets[i] = (unsigned short)((data[i * 2] << 8) | data[i * 2 + 1]);
    tokWords.clear();
    for(int li = 0; li < 26; ++li) {
        unsigned short off = offsets[li];
        int pos, next;
        std::string prev;
        if(!off) continue;
        pos = off;
        next = (int)len;
        for(int ni = li + 1; ni < 26; ++ni) {
            if(offsets[ni]) {
                next = offsets[ni];
                break;
            }
        }
        while(pos < len) {
            int keep = data[pos++];
            std::string suffix;
            if(pos >= len) break;
            while(pos < len) {
                unsigned char c = data[pos++];
                unsigned char decoded = (unsigned char)(c ^ 0x7F);
                suffix.push_back((char)(decoded & 0x7F));
                if(c & 0x80) break;
            }
            if(pos + 1 >= len) break;
            int group = (data[pos] << 8) | data[pos + 1];
            pos += 2;
            prev = prev.substr(0, keep) + suffix;
            if(group != 0 && group != 9999)
                tokWords[LowerAsciiStr(prev)] = group & 0x1FFF;
            if(pos >= next) break;
        }
    }
    free(data);
    return true;
}

static void ExtractWalkthroughData(const std::string &text, std::vector<std::string> &phrases, std::vector<std::string> &words)
{
    std::set<std::string> seenPhrases, seenWords;
    size_t start = 0;
    phrases.clear();
    words.clear();
    while(start <= text.size()) {
        size_t end = text.find_first_of("\r\n", start);
        std::string line = (end == std::string::npos) ? text.substr(start) : text.substr(start, end - start);
        std::string phrase = NormalizeWalkWord(line);
        if(!phrase.empty() && phrase.size() >= 2 && seenPhrases.insert(phrase).second)
            phrases.push_back(phrase);
        size_t wstart = 0;
        while(wstart <= phrase.size()) {
            size_t wend = phrase.find(' ', wstart);
            std::string word = (wend == std::string::npos) ? phrase.substr(wstart) : phrase.substr(wstart, wend - wstart);
            word = TrimAsciiStr(word);
            if(word.size() >= 2 && seenWords.insert(word).second)
                words.push_back(word);
            if(wend == std::string::npos) break;
            wstart = wend + 1;
        }
        if(end == std::string::npos) break;
        start = end + 1;
        if(start < text.size() && text[start] == '\n' && text[end] == '\r') start++;
    }
}

static WalkItemResult ClassifyWalkItem(const std::string &item, const std::map<std::string, int> &effectiveWords, const std::map<int, std::vector<std::string> > &effectiveGroups, const std::map<std::string, int> &tokWords)
{
    WalkItemResult result;
    result.item = item;
    result.group = -1;

    std::map<std::string, int>::const_iterator exact = effectiveWords.find(item);
    if(exact != effectiveWords.end()) {
        result.status = "Exact";
        result.group = exact->second;
        result.survivors.push_back(item);
        return result;
    }

    std::map<std::string, int>::const_iterator tok = tokWords.find(item);
    if(tok != tokWords.end()) {
        std::map<int, std::vector<std::string> >::const_iterator survivors = effectiveGroups.find(tok->second);
        result.group = tok->second;
        if(survivors != effectiveGroups.end() && !survivors->second.empty()) {
            result.status = "Synonym";
            result.survivors = survivors->second;
            return result;
        }
    }

    result.status = "Missing";
    return result;
}

static std::string JoinWords(const std::vector<std::string> &words)
{
    std::string out;
    for(size_t i = 0; i < words.size(); ++i) {
        if(i) out += ", ";
        out += words[i];
    }
    return out;
}

static std::string ReplaceFileExtension(const std::string &path, const std::string &suffix)
{
    size_t slash = path.find_last_of("/\\");
    size_t dot = path.find_last_of('.');
    if(dot == std::string::npos || (slash != std::string::npos && dot < slash))
        return path + suffix;
    return path.substr(0, dot) + suffix;
}

static std::string ReadBinaryFileStd(const TCHAR *filename)
{
    std::string data;
    FILE *f = _tfopen(filename, _T("rb"));
    long size;
    if(!f)
        return data;
    fseek(f, 0, SEEK_END);
    size = ftell(f);
    fseek(f, 0, SEEK_SET);
    if(size <= 0) {
        fclose(f);
        return data;
    }
    data.resize((size_t)size);
    fread(&data[0], 1, (size_t)size, f);
    fclose(f);
    return data;
}

static std::string Base64EncodeStd(const unsigned char *data, size_t len)
{
    static const char table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string out;
    size_t i;
    out.reserve(((len + 2) / 3) * 4);
    for(i = 0; i + 2 < len; i += 3) {
        unsigned int value = ((unsigned int)data[i] << 16) | ((unsigned int)data[i + 1] << 8) | data[i + 2];
        out.push_back(table[(value >> 18) & 63]);
        out.push_back(table[(value >> 12) & 63]);
        out.push_back(table[(value >> 6) & 63]);
        out.push_back(table[value & 63]);
    }
    if(i < len) {
        unsigned int value = (unsigned int)data[i] << 16;
        out.push_back(table[(value >> 18) & 63]);
        if(i + 1 < len) {
            value |= (unsigned int)data[i + 1] << 8;
            out.push_back(table[(value >> 12) & 63]);
            out.push_back(table[(value >> 6) & 63]);
            out.push_back('=');
        } else {
            out.push_back(table[(value >> 12) & 63]);
            out.push_back('=');
            out.push_back('=');
        }
    }
    return out;
}

static std::string JsonEscapeStd(const std::string &value)
{
    std::string out;
    for(size_t i = 0; i < value.size(); ++i) {
        unsigned char c = (unsigned char)value[i];
        switch(c) {
            case '\\': out += "\\\\"; break;
            case '\"': out += "\\\""; break;
            case '\r': out += "\\r"; break;
            case '\n': out += "\\n"; break;
            case '\t': out += "\\t"; break;
            default:
                if(c < 32) {
                    char buf[8];
                    sprintf(buf, "\\u%04x", (unsigned int)c);
                    out += buf;
                } else {
                    out.push_back((char)c);
                }
                break;
        }
    }
    return out;
}

static std::string NarrowPathBaseName(const TCHAR *filename)
{
    const TCHAR *base = filename;
    const TCHAR *slash = _tcsrchr(filename, _T('\\'));
    const TCHAR *slash2 = _tcsrchr(filename, _T('/'));
    if(slash2 && (!slash || slash2 > slash))
        slash = slash2;
    if(slash)
        base = slash + 1;
    char *tmp = strdup_tchar_to_char(base);
    std::string out = tmp ? tmp : "";
    if(tmp) free(tmp);
    return out;
}

static std::string NarrowTChar(const TCHAR *value)
{
    char *tmp = strdup_tchar_to_char(value ? value : _T(""));
    std::string out = tmp ? tmp : "";
    if(tmp) free(tmp);
    return out;
}

static int FindPlanPickerVisibilityFor(const GAMEINFO *gameInfo, int group, const char *word)
{
    VOCAB_PLAN_ENTRY *p;
    if(!gameInfo || !gameInfo->vocabPlan || !word)
        return -1;
    for(p = gameInfo->vocabPlan; p; p = p->next) {
        if(!p->word)
            continue;
        if((p->group & 0x1FFF) == (group & 0x1FFF) && strcmp(p->word, word) == 0)
            return p->pickerVisibility;
    }
    return -1;
}

static int FindPlanColumnOverrideFor(const GAMEINFO *gameInfo, int group, const char *word)
{
    VOCAB_PLAN_ENTRY *p;
    if(!gameInfo || !gameInfo->vocabPlan || !word)
        return -1;
    for(p = gameInfo->vocabPlan; p; p = p->next) {
        if(!p->word)
            continue;
        if((p->group & 0x1FFF) == (group & 0x1FFF) && strcmp(p->word, word) == 0)
            return p->columnOverride;
    }
    return -1;
}

static BOOL IsPlanHiddenFor(const GAMEINFO *gameInfo, int group, const char *word)
{
    VOCAB_PLAN_ENTRY *p;
    if(!gameInfo || !gameInfo->vocabPlan || !word)
        return FALSE;
    for(p = gameInfo->vocabPlan; p; p = p->next) {
        if(!p->word)
            continue;
        if((p->group & 0x1FFF) == (group & 0x1FFF) && strcmp(p->word, word) == 0)
            return p->hidden ? TRUE : FALSE;
    }
    return FALSE;
}

struct AnalyzerSnapshotEntry {
    std::string word;
    int group;
    int column;
    int pickerVisibility;
    BOOL hidden;
    BOOL verb;
    int entryOffset;
};

static BOOL BuildAnalyzerRomModelJson(const GAMEINFO *sourceGame, const TCHAR *runtimeVocabPath, std::string &outJson)
{
    TCHAR gamePathBuf[1024];
    TCHAR oldFname[1024];
    GAMEINFO tempGame;
    GAMEINFO *oldGi;
    TCHAR *oldVocabName;
    U8 *oldVocabData;
    U8 *oldWordData;
    WORDSET *oldWordset;
    std::vector<AnalyzerSnapshotEntry> entries;
    std::map<int, std::vector<AnalyzerSnapshotEntry> > grouped;

    if(!sourceGame || !sourceGame->path || !runtimeVocabPath || !*runtimeVocabPath)
        return FALSE;

    tempGame = *sourceGame;
    _tcsncpy(gamePathBuf, sourceGame->path, _countof(gamePathBuf) - 2);
    gamePathBuf[_countof(gamePathBuf) - 2] = 0;
    {
        size_t len = _tcslen(gamePathBuf);
        if(len && gamePathBuf[len - 1] != _T('\\') && gamePathBuf[len - 1] != _T('/')) {
            gamePathBuf[len] = _T('\\');
            gamePathBuf[len + 1] = 0;
        }
    }
    tempGame.path = gamePathBuf;

    oldGi = gi;
    oldVocabName = vocabName;
    oldVocabData = vocabData;
    oldWordData = wordData;
    oldWordset = wordset;
    _tcsncpy(oldFname, fname, _countof(oldFname) - 1);
    oldFname[_countof(oldFname) - 1] = 0;

    gi = &tempGame;
    vocabName = const_cast<TCHAR*>(runtimeVocabPath);
    vocabData = NULL;
    wordData = NULL;
    wordset = NULL;

    if(!ProcessWords()) {
        mFree(wordset);
        mFree(vocabData);
        mFree(wordData);
        gi = oldGi;
        vocabName = oldVocabName;
        vocabData = oldVocabData;
        wordData = oldWordData;
        wordset = oldWordset;
        _tcsncpy(fname, oldFname, _countof(fname) - 1);
        fname[_countof(fname) - 1] = 0;
        return FALSE;
    }

    {
        WORDSET *w = wordset;
        int entryOffset = 0;
        while(w && w->group) {
            int baseGroup = w->group & 0x1FFF;
            int encodedGroup = baseGroup;
            int q = vocabData ? FindWordx(w->string, vocabData) : -1;
            int columnOverride = FindPlanColumnOverrideFor(sourceGame, baseGroup, w->string);
            AnalyzerSnapshotEntry entry;

            if((q != -1) && (q & 0x80))
                encodedGroup |= 0x8000;
            if(columnOverride == 0)
                encodedGroup |= 0x8000;
            else if(columnOverride == 1)
                encodedGroup &= 0x7FFF;

            entry.word = w->string ? w->string : "";
            entry.group = baseGroup;
            entry.column = (encodedGroup & 0x8000) ? 0 : 1;
            entry.pickerVisibility = FindPlanPickerVisibilityFor(sourceGame, baseGroup, w->string);
            entry.hidden = IsPlanHiddenFor(sourceGame, baseGroup, w->string);
            entry.verb = (entry.column == 0) ? TRUE : FALSE;
            entry.entryOffset = entryOffset++;
            entries.push_back(entry);
            grouped[entry.group].push_back(entry);
            w++;
        }
    }

    outJson = "{\"words\":{";
    for(size_t i = 0; i < entries.size(); ++i) {
        const AnalyzerSnapshotEntry &entry = entries[i];
        if(i) outJson += ",";
        outJson += "\"" + JsonEscapeStd(entry.word) + "\":{";
        outJson += "\"group\":" + std::to_string(entry.group) + ",";
        outJson += "\"verb\":" + std::string(entry.verb ? "true" : "false") + ",";
        outJson += "\"column\":" + std::to_string(entry.column) + ",";
        outJson += "\"entryOffset\":" + std::to_string(entry.entryOffset) + ",";
        outJson += "\"pickerVisibility\":" + std::to_string(entry.pickerVisibility) + ",";
        outJson += "\"hidden\":" + std::string(entry.hidden ? "true" : "false");
        outJson += "}";
    }
    outJson += "},\"groups\":{";
    {
        BOOL firstGroup = TRUE;
        std::map<int, std::vector<AnalyzerSnapshotEntry> >::const_iterator groupIt;
        for(groupIt = grouped.begin(); groupIt != grouped.end(); ++groupIt) {
            const std::vector<AnalyzerSnapshotEntry> &groupEntries = groupIt->second;
            if(!firstGroup) outJson += ",";
            firstGroup = FALSE;
            outJson += "\"" + std::to_string(groupIt->first) + "\":[";
            for(size_t i = 0; i < groupEntries.size(); ++i) {
                const AnalyzerSnapshotEntry &entry = groupEntries[i];
                if(i) outJson += ",";
                outJson += "{";
                outJson += "\"word\":\"" + JsonEscapeStd(entry.word) + "\",";
                outJson += "\"verb\":" + std::string(entry.verb ? "true" : "false") + ",";
                outJson += "\"column\":" + std::to_string(entry.column) + ",";
                outJson += "\"entryOffset\":" + std::to_string(entry.entryOffset) + ",";
                outJson += "\"pickerVisibility\":" + std::to_string(entry.pickerVisibility) + ",";
                outJson += "\"hidden\":" + std::string(entry.hidden ? "true" : "false");
                outJson += "}";
            }
            outJson += "]";
        }
    }
    outJson += "},\"offset\":0,\"count\":" + std::to_string(entries.size()) + ",\"sigPos\":-1,\"wordDataEnd\":0,\"wordsTablePos\":0,\"wordFlagsPos\":0}";

    mFree(wordset);
    mFree(vocabData);
    mFree(wordData);

    gi = oldGi;
    vocabName = oldVocabName;
    vocabData = oldVocabData;
    wordData = oldWordData;
    wordset = oldWordset;
    _tcsncpy(fname, oldFname, _countof(fname) - 1);
    fname[_countof(fname) - 1] = 0;
    return TRUE;
}

static BOOL WriteAnalyzerBootstrapHtml(const TCHAR *bootstrapPath, const TCHAR *analyzerFileName, const TCHAR *romPath, const TCHAR *tokPath, const std::string *romModelJson, const TCHAR *syncPath)
{
    std::string romData = (romPath && *romPath) ? ReadBinaryFileStd(romPath) : std::string();
    std::string tokData = ReadBinaryFileStd(tokPath);
    FILE *out = _tfopen(bootstrapPath, _T("wb"));
    if(!out || tokData.empty() || (romData.empty() && (!romModelJson || romModelJson->empty()))) {
        if(out) fclose(out);
        return FALSE;
    }

    std::string html;
    html += "<!DOCTYPE html><html><head><meta charset=\"UTF-8\"><title>GBAGI Analyzer Loader</title></head><body><script>\n";
    html += "sessionStorage.setItem('gbagiAnalyzerBootstrap', JSON.stringify({";
    if(!romData.empty()) {
        html += "\"gbaName\":\"" + JsonEscapeStd(NarrowPathBaseName(romPath)) + "\",";
        html += "\"gbaBase64\":\"" + Base64EncodeStd((const unsigned char*)romData.data(), romData.size()) + "\",";
    }
    html += "\"tokName\":\"" + JsonEscapeStd(NarrowPathBaseName(tokPath)) + "\",";
    html += "\"tokBase64\":\"" + Base64EncodeStd((const unsigned char*)tokData.data(), tokData.size()) + "\",";
    if(romModelJson && !romModelJson->empty())
        html += "\"romModel\":" + *romModelJson + ",";
    if(syncPath && *syncPath) {
        html += "\"syncEnabled\":true,";
        html += "\"syncPath\":\"" + JsonEscapeStd(NarrowTChar(syncPath)) + "\",";
        html += "\"syncSuggestedName\":\"gbagi_vocab_sync.tsv\",";
    }
    html += "\"autoAnalyze\":true";
    html += "}));\n";
    html += "location.replace('" + JsonEscapeStd(NarrowPathBaseName(analyzerFileName)) + "');\n";
    html += "</script></body></html>\n";
    fwrite(html.data(), 1, html.size(), out);
    fclose(out);
    return TRUE;
}

static void BuildAnalyzerSyncPath(const TCHAR *programPath, TCHAR *outPath, size_t outCount)
{
    if(!outPath || outCount == 0) return;
    _tcsncpy(outPath, programPath ? programPath : _T(""), outCount - 1);
    outPath[outCount - 1] = 0;
    if(outPath[0]) {
        size_t len = _tcslen(outPath);
        if(len && outPath[len - 1] != '\\' && outPath[len - 1] != '/')
            _tcscat(outPath, _T("\\"));
    }
    _tcscat(outPath, _T("gbagi_vocab_sync.tsv"));
}

static BOOL BuildWordsTokPathForGame(const TAddGameObj *game, TCHAR *outPath, size_t outCount)
{
    if(!game || !game->gameinfo.path || !outPath || outCount == 0)
        return FALSE;
    _tcsncpy(outPath, game->gameinfo.path, outCount - 1);
    outPath[outCount - 1] = 0;
    size_t len = _tcslen(outPath);
    if(len && outPath[len - 1] != '\\' && outPath[len - 1] != '/')
        _tcscat(outPath, _T("\\"));
    _tcscat(outPath, _T("words.tok"));
    return FileExists(outPath);
}

struct WalkAnalyzerData {
    std::string gameTitle;
    std::map<std::string, int> tokWords;
    std::map<std::string, int> effectiveWords;
    std::map<int, std::vector<std::string> > effectiveGroups;
};

struct WalkAnalysisOutput {
    std::vector<WalkItemResult> wordResults;
    std::vector<WalkItemResult> phraseResults;
    std::string safeWalkthrough;
    std::string report;
    int exactWords;
    int synonymWords;
    int missingWords;
    int exactPhrases;
    int synonymPhrases;
    int missingPhrases;
};

static std::string GetWindowTextFull(HWND hWnd)
{
    int len = ::GetWindowTextLength(hWnd);
    std::vector<TCHAR> buf(len + 1, 0);
    if(len > 0)
        ::GetWindowText(hWnd, &buf[0], len + 1);
#ifdef UNICODE
    char *narrow = strdup_tchar_to_char(&buf[0]);
    std::string out = narrow ? narrow : "";
    if(narrow) free(narrow);
    return out;
#else
    return std::string(&buf[0]);
#endif
}

static void SetWindowTextAscii(HWND hWnd, const std::string &text)
{
#ifdef UNICODE
    VclString s(text.c_str());
    ::SetWindowText(hWnd, s.c_str());
#else
    ::SetWindowText(hWnd, text.c_str());
#endif
}

static bool PromptOpenWalkthrough(HWND owner, VclString &fileName)
{
    OPENFILENAME ofn;
    TCHAR path[1024];
    ZeroMemory(&ofn, sizeof(ofn));
    path[0] = 0;
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = owner;
    ofn.lpstrFilter = _T("Walkthrough Text (*.txt;*.md;*.log)\0*.txt;*.md;*.log\0All Files\0*.*\0");
    ofn.lpstrFile = path;
    ofn.nMaxFile = _countof(path);
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
    ofn.lpstrDefExt = _T("txt");
    if(!GetOpenFileName(&ofn))
        return false;
    fileName = path;
    return true;
}

static bool PromptSaveText(HWND owner, VclString &fileName, const TCHAR *initial)
{
    OPENFILENAME ofn;
    TCHAR path[1024];
    ZeroMemory(&ofn, sizeof(ofn));
    _tcsncpy(path, initial ? initial : _T(""), _countof(path) - 1);
    path[_countof(path) - 1] = 0;
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = owner;
    ofn.lpstrFilter = _T("Text Files (*.txt)\0*.txt\0All Files\0*.*\0");
    ofn.lpstrFile = path;
    ofn.nMaxFile = _countof(path);
    ofn.Flags = OFN_OVERWRITEPROMPT;
    ofn.lpstrDefExt = _T("txt");
    if(!GetSaveFileName(&ofn))
        return false;
    fileName = path;
    return true;
}

static std::string FirstSurvivorFor(const std::string &item, const WalkAnalyzerData &data)
{
    std::map<std::string, int>::const_iterator exact = data.effectiveWords.find(item);
    if(exact != data.effectiveWords.end())
        return item;
    std::map<std::string, int>::const_iterator tok = data.tokWords.find(item);
    if(tok != data.tokWords.end()) {
        std::map<int, std::vector<std::string> >::const_iterator group = data.effectiveGroups.find(tok->second);
        if(group != data.effectiveGroups.end() && !group->second.empty())
            return group->second[0];
    }
    return "";
}

static std::string RewriteWalkthroughLine(const std::string &line, const WalkAnalyzerData &data)
{
    std::string original = line;
    std::string trimmed = TrimAsciiStr(original);
    size_t pos = 0;
    std::string leading;
    std::string phrase;
    if(trimmed.empty())
        return original;
    while(pos < original.size() && std::isspace((unsigned char)original[pos])) {
        leading.push_back(original[pos]);
        pos++;
    }
    phrase = NormalizeWalkWord(original.substr(pos));
    if(phrase.empty())
        return original;
    {
        std::string exactPhrase = FirstSurvivorFor(phrase, data);
        if(!exactPhrase.empty())
            return leading + exactPhrase;
    }
    std::string rebuilt;
    size_t start = 0;
    while(start <= phrase.size()) {
        size_t end = phrase.find(' ', start);
        std::string word = (end == std::string::npos) ? phrase.substr(start) : phrase.substr(start, end - start);
        if(!word.empty()) {
            std::string survivor = FirstSurvivorFor(word, data);
            if(!rebuilt.empty()) rebuilt += " ";
            rebuilt += survivor.empty() ? word : survivor;
        }
        if(end == std::string::npos) break;
        start = end + 1;
    }
    return leading + rebuilt;
}

static std::string RewriteWalkthroughText(const std::string &text, const WalkAnalyzerData &data)
{
    std::string out;
    size_t start = 0;
    while(start <= text.size()) {
        size_t end = text.find_first_of("\r\n", start);
        std::string line = (end == std::string::npos) ? text.substr(start) : text.substr(start, end - start);
        if(!out.empty()) out += "\r\n";
        out += RewriteWalkthroughLine(line, data);
        if(end == std::string::npos) break;
        start = end + 1;
        if(start < text.size() && text[start] == '\n' && text[end] == '\r') start++;
    }
    return out;
}

static WalkAnalysisOutput AnalyzeWalkthroughText(const std::string &text, const WalkAnalyzerData &data)
{
    WalkAnalysisOutput out;
    std::vector<std::string> phrases, words;
    ExtractWalkthroughData(text, phrases, words);
    out.exactWords = out.synonymWords = out.missingWords = 0;
    out.exactPhrases = out.synonymPhrases = out.missingPhrases = 0;

    for(size_t i = 0; i < words.size(); ++i) {
        WalkItemResult r = ClassifyWalkItem(words[i], data.effectiveWords, data.effectiveGroups, data.tokWords);
        if(r.status == "Exact") out.exactWords++;
        else if(r.status == "Synonym") out.synonymWords++;
        else out.missingWords++;
        out.wordResults.push_back(r);
    }
    for(size_t i = 0; i < phrases.size(); ++i) {
        WalkItemResult r = ClassifyWalkItem(phrases[i], data.effectiveWords, data.effectiveGroups, data.tokWords);
        if(r.status == "Exact") out.exactPhrases++;
        else if(r.status == "Synonym") out.synonymPhrases++;
        else out.missingPhrases++;
        out.phraseResults.push_back(r);
    }

    out.safeWalkthrough = RewriteWalkthroughText(text, data);
    out.report += "GBAGI Walkthrough Check\r\n";
    out.report += "======================\r\n\r\n";
    out.report += "Game: " + data.gameTitle + "\r\n\r\n";
    out.report += "Words: exact " + std::to_string(out.exactWords) + ", synonym " + std::to_string(out.synonymWords) + ", missing " + std::to_string(out.missingWords) + "\r\n";
    out.report += "Phrases: exact " + std::to_string(out.exactPhrases) + ", synonym " + std::to_string(out.synonymPhrases) + ", missing " + std::to_string(out.missingPhrases) + "\r\n\r\n";
    out.report += "Walkthrough Words\r\n-----------------\r\n";
    for(size_t i = 0; i < out.wordResults.size(); ++i) {
        out.report += out.wordResults[i].item + " : " + out.wordResults[i].status;
        if(out.wordResults[i].group >= 0)
            out.report += " (G" + std::to_string(out.wordResults[i].group) + ")";
        if(!out.wordResults[i].survivors.empty())
            out.report += " -> " + JoinWords(out.wordResults[i].survivors);
        out.report += "\r\n";
    }
    out.report += "\r\nCommand Phrases\r\n---------------\r\n";
    for(size_t i = 0; i < out.phraseResults.size(); ++i) {
        out.report += out.phraseResults[i].item + " : " + out.phraseResults[i].status;
        if(out.phraseResults[i].group >= 0)
            out.report += " (G" + std::to_string(out.phraseResults[i].group) + ")";
        if(!out.phraseResults[i].survivors.empty())
            out.report += " -> " + JoinWords(out.phraseResults[i].survivors);
        out.report += "\r\n";
    }
    out.report += "\r\nGBAGI Safe Walkthrough\r\n---------------------\r\n";
    out.report += out.safeWalkthrough + "\r\n";
    return out;
}

class TFormWalkCheck : public TForm
{
public:
    TLabel *lblGame;
    TLabel *lblSummary;
    TLabel *lblWords;
    TLabel *lblPhrases;
    TLabel *lblSafe;
    TEdit *edWalk;
    TEdit *edSafe;
    TListBox *listWords;
    TListBox *listPhrases;
    TButton *btnLoad;
    TButton *btnAnalyze;
    TButton *btnSaveSafe;
    TButton *btnSaveReport;
    TButton *btnClose;
    WalkAnalyzerData data;
    VclString walkFileName;
    WalkAnalysisOutput lastOutput;

    __fastcall TFormWalkCheck(TComponent *Owner) : TForm(Owner) { CreateControls(); }

    void SetUp(const WalkAnalyzerData &src)
    {
        data = src;
        if(lblGame)
            lblGame->Caption = VclString(_T("Game: ")) + VclString(data.gameTitle.c_str());
        if(lblSummary)
            lblSummary->Caption = _T("Load a walkthrough, then analyze against the current injector vocabulary.");
        if(lblGame) lblGame->Update();
        if(lblSummary) lblSummary->Update();
    }

    virtual void CreateControls()
    {
        this->hWnd = CreateDialogParam(NULL, MAKEINTRESOURCE(IDC_TFORMWALKCHECK), this->Owner->hWnd, TFormDialogProc, (LPARAM)this);
    }

    virtual void OnInitDialog(HWND hWnd)
    {
        Attach(hWnd);
        lblGame = new TLabel(this); lblGame->Attach(GetDlgItem(hWnd, IDC_WALK_LBLGAME));
        lblSummary = new TLabel(this); lblSummary->Attach(GetDlgItem(hWnd, IDC_WALK_LBLSUMMARY));
        lblWords = new TLabel(this); lblWords->Attach(GetDlgItem(hWnd, IDC_WALK_LBLWORDS));
        lblPhrases = new TLabel(this); lblPhrases->Attach(GetDlgItem(hWnd, IDC_WALK_LBLPHRASES));
        lblSafe = new TLabel(this); lblSafe->Attach(GetDlgItem(hWnd, IDC_WALK_LBLSAFE));
        edWalk = new TEdit(this); edWalk->Attach(GetDlgItem(hWnd, IDC_WALK_EDITTEXT));
        edSafe = new TEdit(this); edSafe->Attach(GetDlgItem(hWnd, IDC_WALK_EDITSAFE));
        listWords = new TListBox(this); listWords->Attach(GetDlgItem(hWnd, IDC_WALK_LISTWORDS));
        listPhrases = new TListBox(this); listPhrases->Attach(GetDlgItem(hWnd, IDC_WALK_LISTPHRASES));
        btnLoad = new TButton(this); btnLoad->Attach(GetDlgItem(hWnd, IDC_WALK_BTNLOAD));
        btnAnalyze = new TButton(this); btnAnalyze->Attach(GetDlgItem(hWnd, IDC_WALK_BTNANALYZE));
        btnSaveSafe = new TButton(this); btnSaveSafe->Attach(GetDlgItem(hWnd, IDC_WALK_BTNSAVESAFE));
        btnSaveReport = new TButton(this); btnSaveReport->Attach(GetDlgItem(hWnd, IDC_WALK_BTNSAVEREPORT));
        btnClose = new TButton(this); btnClose->Attach(GetDlgItem(hWnd, IDC_WALK_BTNCLOSE));
    }

    virtual void OnCommand(WPARAM wParam, LPARAM lParam)
    {
        TForm::OnCommand(wParam, lParam);
        if(LOWORD(wParam) == IDC_WALK_BTNLOAD && HIWORD(wParam) == BN_CLICKED) btnLoadClick();
        if(LOWORD(wParam) == IDC_WALK_BTNANALYZE && HIWORD(wParam) == BN_CLICKED) btnAnalyzeClick();
        if(LOWORD(wParam) == IDC_WALK_BTNSAVESAFE && HIWORD(wParam) == BN_CLICKED) btnSaveSafeClick();
        if(LOWORD(wParam) == IDC_WALK_BTNSAVEREPORT && HIWORD(wParam) == BN_CLICKED) btnSaveReportClick();
        if(LOWORD(wParam) == IDC_WALK_BTNCLOSE && HIWORD(wParam) == BN_CLICKED) btnCloseClick();
    }

    void btnLoadClick()
    {
        VclString path;
        if(!PromptOpenWalkthrough(this->hWnd, path))
            return;
        walkFileName = path;
        std::string text;
        if(!LoadTextFileUtf8ish(path.c_str(), text)) {
            ShowMessage(_T("Could not read walkthrough file."));
            return;
        }
        SetWindowTextAscii(edWalk->hWnd, text);
    }

    void btnAnalyzeClick()
    {
        std::string text = GetWindowTextFull(edWalk->hWnd);
        char buf[256];
        if(TrimAsciiStr(text).empty()) {
            ShowMessage(_T("Paste or load a walkthrough first."));
            return;
        }
        lastOutput = AnalyzeWalkthroughText(text, data);
        listWords->Items->Clear();
        listPhrases->Items->Clear();
        for(size_t i = 0; i < lastOutput.wordResults.size(); ++i) {
            std::string line = lastOutput.wordResults[i].item + " | " + lastOutput.wordResults[i].status;
            if(lastOutput.wordResults[i].group >= 0)
                line += " | G" + std::to_string(lastOutput.wordResults[i].group);
            if(!lastOutput.wordResults[i].survivors.empty())
                line += " | " + JoinWords(lastOutput.wordResults[i].survivors);
            listWords->Items->Add(line.c_str());
        }
        for(size_t i = 0; i < lastOutput.phraseResults.size(); ++i) {
            std::string line = lastOutput.phraseResults[i].item + " | " + lastOutput.phraseResults[i].status;
            if(lastOutput.phraseResults[i].group >= 0)
                line += " | G" + std::to_string(lastOutput.phraseResults[i].group);
            if(!lastOutput.phraseResults[i].survivors.empty())
                line += " | " + JoinWords(lastOutput.phraseResults[i].survivors);
            listPhrases->Items->Add(line.c_str());
        }
        SetWindowTextAscii(edSafe->hWnd, lastOutput.safeWalkthrough);
        sprintf(buf, "Words: exact %d, synonym %d, missing %d | Phrases: exact %d, synonym %d, missing %d",
            lastOutput.exactWords, lastOutput.synonymWords, lastOutput.missingWords,
            lastOutput.exactPhrases, lastOutput.synonymPhrases, lastOutput.missingPhrases);
        lblSummary->Caption = buf;
        lblSummary->Update();
    }

    void btnSaveSafeClick()
    {
        std::string safeText = GetWindowTextFull(edSafe->hWnd);
        VclString path;
        std::string initial;
        if(safeText.empty()) {
            ShowMessage(_T("Analyze a walkthrough first."));
            return;
        }
        if(walkFileName.Length()) {
            char *walkPath = strdup_tchar_to_char(walkFileName.c_str());
            initial = ReplaceFileExtension(walkPath ? walkPath : "walkthrough", "_gbagi_safe.txt");
            if(walkPath) free(walkPath);
        } else initial = "walkthrough_gbagi_safe.txt";
        if(!PromptSaveText(this->hWnd, path, VclString(initial.c_str()).c_str()))
            return;
        {
            char *savePath = strdup_tchar_to_char(path.c_str());
            FILE *f = fopen(savePath, "wb");
            if(savePath) free(savePath);
            if(!f) {
                ShowMessage(_T("Could not save GBAGI-safe walkthrough."));
                return;
            }
            fwrite(safeText.data(), 1, safeText.size(), f);
            fclose(f);
        }
    }

    void btnSaveReportClick()
    {
        VclString path;
        std::string initial;
        if(lastOutput.report.empty()) {
            ShowMessage(_T("Analyze a walkthrough first."));
            return;
        }
        if(walkFileName.Length()) {
            char *walkPath = strdup_tchar_to_char(walkFileName.c_str());
            initial = ReplaceFileExtension(walkPath ? walkPath : "walkthrough", "_gbagi_check.txt");
            if(walkPath) free(walkPath);
        } else initial = "walkthrough_gbagi_check.txt";
        if(!PromptSaveText(this->hWnd, path, VclString(initial.c_str()).c_str()))
            return;
        {
            char *savePath = strdup_tchar_to_char(path.c_str());
            FILE *f = fopen(savePath, "wb");
            if(savePath) free(savePath);
            if(!f) {
                ShowMessage(_T("Could not save walkthrough report."));
                return;
            }
            fwrite(lastOutput.report.data(), 1, lastOutput.report.size(), f);
            fclose(f);
        }
    }

    void btnCloseClick()
    {
        Close();
    }
};

//---------------------------------------------------------------------------
__fastcall TFormMain::TFormMain(TComponent* Owner)
	: TForm(Owner)
{
    CreateControls();

    _tcscpy(szPath,GetCurrentDir().c_str());
    if(!FileExists(ProgramDir+_T("\\gbinjectb.exe")))
    	GetProgramPath();
    int l=_tcslen(szPath);
    if(szPath[l-1]=='\\')
    	szPath[l-1]='\0'; 
    ProgramDir=VclString(GetProgramPath());

    tbInput->Text	= ProgramDir+_T("\\gbagi.bin");
    tbVocab->Text	= ProgramDir+_T("\\vocab.bin");
    tbOutput->Text	= ProgramDir+_T("\\agigames.gba");

	DirDialog = new TDirDialog;

    addGameFirst	=
    addGamePtr 		= NULL;

	FormResize(this);
    UpdateControls();
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::FormDestroy(TObject *Sender)
{
	delete DirDialog;
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::UpdateControls()
{
	BOOL hasBuiltRom = (tbOutput->Text != _T("") && FileExists(tbOutput->Text));
	btnBuild->Enabled 	= (tbOutput->Text!=_T("") && listbox->Items->Count > 0);
	btnRemove->Enabled 	= (listbox->Items->Count > 0);
	btnWords->Enabled   = (listbox->ItemIndex != -1);
	btnWalkTest->Enabled = (listbox->ItemIndex != -1) || hasBuiltRom;

	// Generated;
	this->Update();
	btnBuild->Update();
	btnRemove->Update();
	btnWords->Update();
	btnWalkTest->Update();
	tbOutput->Update();
	tbInput->Update();
	tbVocab->Update();
	txStatus->Update();
	listbox->Update();
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::FormResize(TObject *Sender)
{
	tbInput->Width	= ClientWidth-tbInput->Left - btnBrowseInp->Width;
	tbVocab->Width 	= ClientWidth-tbVocab->Left - btnBrowseVoc->Width;
	tbOutput->Width	= ClientWidth-tbOutput->Left- btnBrowseOut->Width;
}
//---------------------------------------------------------------------------
void __fastcall TFormMain::btnAddClick(TObject *Sender)
{
	if(listbox->Items->Count >= kMaxInjectGames) {
     	ShowMessage(_T("Maximum games added to game list"));
        return;
    }

	// Get the directory which the game is located
    DirDialog->Title = _T("Add Game");
    DirDialog->Caption = _T("Please select the directory which the game you wish to add is located in.");
    DirDialog->MAP_CHECK = TRUE;
    if(!DirDialog->Execute()) {
        return;
    }
	DirDialog->InitialDir = DirDialog->FullPath;

    TFormAddGame *ag = new TFormAddGame(this);
    ag->SetUp(DirDialog->FullPath);
    ag->ShowModal();

    if(ag->okClose) {
    	TAddGameObj *go = new TAddGameObj();
        go->prev = addGamePtr;
        go->next = NULL;
      	if(!addGameFirst) {
          	addGameFirst = go;
        }
      	if(addGamePtr) {
          	addGamePtr->next = go;
        }
        addGamePtr = go;

        memcpy(&go->gameinfo,&ag->gameinfo,sizeof(go->gameinfo));
        go->gameinfo.vocabPlan = NULL;
        go->itemindex = listbox->Items->Add(go->gameinfo.title);

        UpdateControls();
    }

    delete ag;
}
//---------------------------------------------------------------------------
void __fastcall TFormMain::btnBrowseInpClick(TObject *Sender)
{
	if(dlgOpenInp->Execute()) {
     	tbInput->Text = dlgOpenInp->FileName;
    }
}
//---------------------------------------------------------------------------
void __fastcall TFormMain::btnBrowseVocClick(TObject *Sender)
{
	if(dlgOpenVoc->Execute()) {
     	tbVocab->Text = dlgOpenVoc->FileName;
    }
}
//---------------------------------------------------------------------------
void __fastcall TFormMain::btnBrowseOutClick(TObject *Sender)
{
	if(dlgSaveOut->Execute()) {
     	tbOutput->Text = dlgSaveOut->FileName;
    }
}
//---------------------------------------------------------------------------
void __fastcall TFormMain::btnRemoveClick(TObject *Sender)
{
	if(listbox->ItemIndex != -1) {
    	RemoveAddGame(listbox->ItemIndex);
        listbox->Items->Delete(listbox->ItemIndex);
    }
}
//---------------------------------------------------------------------------
TAddGameObj *TFormMain::FindAddGame(int num)
{
 	TAddGameObj *o = addGameFirst;
    while(o) {
     	if(!num--)
        	break;
        o = o->next;
    }
    return o;
}
//---------------------------------------------------------------------------
void TFormMain::FreeVocabPlan(VOCAB_PLAN_ENTRY *plan)
{
	while(plan) {
		VOCAB_PLAN_ENTRY *next = plan->next;
		if(plan->word) free(plan->word);
		free(plan);
		plan = next;
	}
}
//---------------------------------------------------------------------------
VOCAB_PLAN_ENTRY *TFormMain::LoadVocabPlanFromPresetFile(const TCHAR *filename)
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
    } else if(strncmp(line, "GBAGI_VOCAB_PRESET_V3", 21) == 0)
        isV3 = true;
    else if(strncmp(line, "GBAGI_VOCAB_PRESET_V2", 21) != 0) {
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
        VOCAB_PLAN_ENTRY *entry;

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

        entry = (VOCAB_PLAN_ENTRY*)calloc(1, sizeof(VOCAB_PLAN_ENTRY));
        if(!entry)
            continue;

        entry->group = atoi(groupS);
        entry->word = _strdup(wordS);
        entry->forceKeep = atoi(keepS) ? TRUE : FALSE;
        entry->forceRemove = atoi(removeS) ? TRUE : FALSE;
        entry->hidden = atoi(hideS) ? TRUE : FALSE;
        entry->addAlias = isV3 && atoi(addS) ? TRUE : FALSE;
        entry->replaceGroup = isV3 && atoi(replaceS) ? TRUE : FALSE;
        entry->pickerVisibility = (isV4 && showS) ? atoi(showS) : -1;
        entry->columnOverride = atoi(colS);
        entry->next = NULL;

        if(!head) head = entry;
        else tail->next = entry;
        tail = entry;
    }

    fclose(f);
    return head;
}
//---------------------------------------------------------------------------
void TFormMain::ReloadGamePresetIfPresent(TAddGameObj *game)
{
    TCHAR presetPath[1024];
    size_t pathLen;
    VOCAB_PLAN_ENTRY *plan;

    if(!game || !game->gameinfo.path)
        return;

    _tcsncpy(presetPath, game->gameinfo.path, _countof(presetPath) - 1);
    presetPath[_countof(presetPath) - 1] = 0;
    pathLen = _tcslen(presetPath);
    if(pathLen && presetPath[pathLen - 1] != '\\' && presetPath[pathLen - 1] != '/')
        _tcscat(presetPath, _T("\\"));
    _tcscat(presetPath, _T("gbagi_vocab_preset.tsv"));

    if(!FileExists(presetPath))
        return;

    plan = LoadVocabPlanFromPresetFile(presetPath);
    if(!plan)
        return;

    FreeVocabPlan(game->gameinfo.vocabPlan);
    game->gameinfo.vocabPlan = plan;
}
//---------------------------------------------------------------------------
void TFormMain::TryImportAnalyzerSyncForSelectedGame()
{
    TCHAR syncPath[MAX_PATH * 4];
    TAddGameObj *game;
    VOCAB_PLAN_ENTRY *plan;

    if(listbox->ItemIndex < 0)
        return;
    game = FindAddGame(listbox->ItemIndex);
    if(!game)
        return;

    BuildAnalyzerSyncPath(GetProgramPath(), syncPath, _countof(syncPath));
    if(!FileExists(syncPath))
        return;

    plan = LoadVocabPlanFromPresetFile(syncPath);
    if(!plan)
        return;

    FreeVocabPlan(game->gameinfo.vocabPlan);
    game->gameinfo.vocabPlan = plan;
    DeleteFile(syncPath);
    txStatus->Caption = _T("Imported analyzer changes for ") + VclString(game->gameinfo.title ? game->gameinfo.title : "");
    UpdateControls();
}
//---------------------------------------------------------------------------
int TFormMain::RemoveAddGame(int num)
{
	TAddGameObj *o = FindAddGame(num);

    if(o==NULL) return -1;

    if(o->prev)
    	o->prev->next = o->next;
    if(o->next)
    	o->next->prev = o->prev;
    if(o == addGameFirst)
    	addGameFirst = o->next;
    if(o == addGamePtr)
    	addGamePtr = o->next?o->next:o->prev;


    free((void*) o->gameinfo.title );
    free((void*) o->gameinfo.path );
    free((void*) o->gameinfo.vID );
    FreeVocabPlan(o->gameinfo.vocabPlan);

    delete o;

    UpdateControls();

    return 0;
}
//---------------------------------------------------------------------------
void TFormMain::RemoveAddGames()
{
	while(RemoveAddGame(0) != -1)
        listbox->Items->Delete(0);
    addGameFirst	=
    addGamePtr 		= NULL;
}
//---------------------------------------------------------------------------
BOOL TFormMain::PackGames()
{
	int i, l;
	FILE *fin;

	int totalGames = listbox->Items->Count;

	inromName	= _tcsdup(tbInput->Text.c_str());
	outromName	= _tcsdup(tbOutput->Text.c_str());
	vocabName	= _tcsdup(tbVocab->Text.c_str());

	if(FileExists(vocabName) == false || (fin=_tfopen(vocabName,_T("rb")))==NULL) {
 		ShowMessage(_T("Error opening vocab definition file! Please specify the location of the file included with this program by clicking on the \"Browse...\" button."));
		return FALSE;
	}
	fclose(fin);
	if(FileExists(inromName) == false || (fin=_tfopen(inromName,_T("rb")))==NULL) {
 		ShowMessage(_T("Error opening input rom! Please specify the location of the file included with this program by clicking on the \"Browse...\" button."));
		return FALSE;
	}
	if((fout=_tfopen(outromName,_T("wb")))==NULL) {
    	fclose(fin);
 		ShowMessage(_T("Error opening file for output!"));
		return FALSE;
	}

	fseek(fin,0,SEEK_END);
	l=ftell(fin);
	fseek(fin,0,SEEK_SET);
	for(i=0;i<l;i++)
    	fputc(fgetc(fin),fout);
	fclose(fin);
	S32 BASEx0X = (l + AGI_DATA_ALIGNMENT-1) & -AGI_DATA_ALIGNMENT;
	for(i=l;i<BASEx0X;i++)
    	fputc(0xFF,fout);
	U32 BASE80X = BASE800 + BASEx0X;

	offs = BASE80X;

  	fwrite(agiid,IDSIZE,1,fout);
	fputc(totalGames,fout);   
	for(i=IDSIZE+1;i<0x20;i++)
    	fputc(0,fout);
	offs += 0x20;

	giPos = ftell(fout);

	for(i=totalGames*80;i>0;i--)
    	fputc(0,fout);
	offs += totalGames*80;

	GAMEINFO ginm;
	listbox->ItemIndex = 0;
    TAddGameObj *gameobj = addGameFirst;
	while(gameobj) {
        ReloadGamePresetIfPresent(gameobj);
		//printf("Packing game: %s...\t\t\t",games[i].title);
        Repaint();
        Update();
		ginm.path = gameobj->gameinfo.path;
		ginm.title = gameobj->gameinfo.title;
		ginm.version = gameobj->gameinfo.version;
		ginm.vID = gameobj->gameinfo.vID;
		ginm.vocabPlan = gameobj->gameinfo.vocabPlan;
		txStatus->Caption = _T("Adding Game: ")+VclString(gameobj->gameinfo.title);
		if(!ProcessGame(&ginm)) {
    		FreeGame();
    		fclose(fout);
			ShowMessage(_T("Error adding game: ")+VclString(gameobj->gameinfo.title));
			return FALSE;
		}
    	FreeGame();
		if(gameobj->next)
			listbox->ItemIndex++;
     	gameobj = gameobj->next;
	}

	fclose(fout);
	if(!FixOutputRomForHardware(outromName, NULL))
	{
		ShowMessage(_T("Warning: ROM was built, but hardware-header fixing failed."));
	}

	return TRUE;
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::btnWordsClick(TObject *Sender)
{
    if(listbox->ItemIndex < 0)
        return;

    TryImportAnalyzerSyncForSelectedGame();

    TAddGameObj *game = FindAddGame(listbox->ItemIndex);
    if(!game)
        return;

    inromName = _tcsdup(tbInput->Text.c_str());
    outromName = _tcsdup(tbOutput->Text.c_str());
    vocabName = _tcsdup(tbVocab->Text.c_str());

    TFormVocabEdit *ve = new TFormVocabEdit(this);
    ve->SetUp(&game->gameinfo);
    ve->ShowModal();

    if(ve->okClose) {
        FreeVocabPlan(game->gameinfo.vocabPlan);
        game->gameinfo.vocabPlan = ve->DetachPlan();
    }

    delete ve;
    delete inromName; inromName = NULL;
    delete outromName; outromName = NULL;
    delete vocabName; vocabName = NULL;
    UpdateControls();
}
//---------------------------------------------------------------------------
void __fastcall TFormMain::btnWalkTestClick(TObject *Sender)
{
    TCHAR analyzerPath[MAX_PATH * 4];
    TCHAR bootstrapPath[MAX_PATH * 4];
    TCHAR syncPath[MAX_PATH * 4];
    TCHAR tokPath[MAX_PATH * 4];
    LPCTSTR programPath = GetProgramPath();
    TAddGameObj *game = NULL;
    std::string romModelJson;
    BOOL haveSnapshot = FALSE;
    BOOL haveOutputRom = FALSE;

    _stprintf_s(analyzerPath, _countof(analyzerPath), _T("%s\\gbagi_vocab_analyzer.html"), programPath);
    if(!FileExists(analyzerPath))
        _stprintf_s(analyzerPath, _countof(analyzerPath), _T("%s\\..\\..\\gbagi_vocab_analyzer.html"), programPath);
    if(!FileExists(analyzerPath))
        _stprintf_s(analyzerPath, _countof(analyzerPath), _T("%s\\..\\gbagi_vocab_analyzer.html"), programPath);

    if(!FileExists(analyzerPath)) {
        ShowMessage(_T("Could not find gbagi_vocab_analyzer.html in Release or the injector project."));
        return;
    }

    TryImportAnalyzerSyncForSelectedGame();
    game = FindAddGame(listbox->ItemIndex >= 0 ? listbox->ItemIndex : 0);
    if(game)
        haveSnapshot = BuildAnalyzerRomModelJson(&game->gameinfo, tbVocab->Text.c_str(), romModelJson);
    haveOutputRom = FileExists(tbOutput->Text.c_str());
    _tcsncpy(bootstrapPath, analyzerPath, _countof(bootstrapPath) - 1);
    bootstrapPath[_countof(bootstrapPath) - 1] = 0;
    {
        TCHAR *slash = _tcsrchr(bootstrapPath, _T('\\'));
        if(slash) {
            *(slash + 1) = 0;
            _tcscat(bootstrapPath, _T("gbagi_vocab_analyzer_autoload.html"));
        }
    }
    BuildAnalyzerSyncPath(programPath, syncPath, _countof(syncPath));

    if(BuildWordsTokPathForGame(game, tokPath, _countof(tokPath)) &&
       (haveSnapshot || haveOutputRom) &&
       WriteAnalyzerBootstrapHtml(bootstrapPath, analyzerPath, haveOutputRom ? tbOutput->Text.c_str() : NULL, tokPath, haveSnapshot ? &romModelJson : NULL, syncPath)) {
        ShellExecute(NULL, NULL, bootstrapPath, NULL, NULL, SW_SHOWNORMAL);
        return;
    }

    ShellExecute(NULL, NULL, analyzerPath, NULL, NULL, SW_SHOWNORMAL);
}
//---------------------------------------------------------------------------
void __fastcall TFormMain::btnBuildClick(TObject *Sender)
{
    TryImportAnalyzerSyncForSelectedGame();
	if(tbOutput->Text == _T("") || tbInput->Text == _T("")) {
		ShowMessage(_T("You must specify an input and output file name!"));
	} else {
	if(listbox->Items->Count == 0) {
		ShowMessage(_T("You must add games to embed in the ROM!"));
	} else {
		Enabled = false;
		if(PackGames()) {
			ShowMessage(_T("ROM Build finished. Enjoy!"));
    		UpdateControls();
		}
		txStatus->Caption = _T("");
		delete inromName;
		delete outromName;
		delete vocabName;
		Enabled = true;
	}
	}
}
//---------------------------------------------------------------------------
void __fastcall TFormMain::btnExitClick(TObject *Sender)
{
	Close();
}
//---------------------------------------------------------------------------
void __fastcall TFormMain::tbOutputChange(TObject *Sender)
{
	UpdateControls();	
}
//---------------------------------------------------------------------------
// Get path where the current process (.EXE) is located.
// This may be different from the current directory.
LPCTSTR TFormMain::GetProgramPath()
{
	int i;
	BOOL bInString = FALSE;
	static TCHAR szPath[4096];

	_tcscpy(szPath, GetCommandLine());

	// Extract first argument
	for (i = 0; i < (int)_tcslen(szPath); i++)
	{
		if (szPath[i] == '"')
			bInString = !bInString;

		if (szPath[i] == ' ' && !bInString)
		{
			szPath[i] = 0;
			break;
		}
	}

	// Remove file name
	bool bFoundPath = FALSE;

	for (i = _tcslen(szPath) - 1; i >= 0; i--)
		if (szPath[i] == '\\')
		{
			szPath[i] = 0;
			bFoundPath = TRUE;
			break;
		}

	if (!bFoundPath)
	{
		// Command line does not contain path.
		// Assume working directory.
		GetCurrentDirectory(MAX_PATH, szPath);
	}

	// Remove leading quote, if any
	if (szPath[0] == '"')
		return szPath + 1;
	else
		return szPath;
}
//---------------------------------------------------------------------------
void __fastcall TFormMain::FormShow(TObject *Sender)
{
    tbOutput->SetFocus();
}
//---------------------------------------------------------------------------
void __fastcall TFormMain::Label3Click(TObject *Sender)
{
	// This URL was originally "http://www.bripro.com", but that is a broken link now.
	// An archive of its last state can be seen at:
	// https://web.archive.org/web/20090512184610/http://www.bripro.com:80/gbagi/index.php
	ShellExecute(NULL, NULL, _T("https://github.com/VisionaiR3D/GBAGI"), NULL, NULL, 0);
}
//---------------------------------------------------------------------------
