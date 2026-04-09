#include "vcl-shim/vcl-shim.h"
#pragma hdrstop
#include "vocabedit.h"
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstring>
#include <commdlg.h>
#if defined(_MSC_VER)
#define strdup _strdup
#endif

#pragma package(smart_init)
#pragma resource "*.dfm"

static unsigned short ReadBE16(const unsigned char *p) {
    return (unsigned short)((p[0] << 8) | p[1]);
}

static std::string TrimAscii(const std::string &value) {
    size_t start = 0;
    size_t end = value.size();
    while(start < end && std::isspace((unsigned char)value[start])) start++;
    while(end > start && std::isspace((unsigned char)value[end - 1])) end--;
    return value.substr(start, end - start);
}

static std::string LowerAscii(const std::string &value) {
    std::string out = value;
    for(size_t i=0; i<out.size(); ++i)
        out[i] = (char)std::tolower((unsigned char)out[i]);
    return out;
}

__fastcall TFormVocabEdit::TFormVocabEdit(TComponent* Owner)
    : TForm(Owner)
{
    CreateControls();
    okClose = FALSE;
    gameInfo = NULL;
    workingPlan = NULL;
    previewMoreMode = false;
    searchFilter.clear();
}

std::string TFormVocabEdit::MakeWordKey(int group, const char *word) const
{
    char buf[64];
    sprintf(buf, "%d\t", group);
    return std::string(buf) + (word ? word : "");
}

bool TFormVocabEdit::IsAutoUsed(int group, const char *word) const
{
    std::map<std::string, AutoWordState>::const_iterator it = autoStates.find(MakeWordKey(group, word));
    return it != autoStates.end() && it->second.used;
}

int TFormVocabEdit::GetAutoColumn(int group, const char *word) const
{
    std::map<std::string, AutoWordState>::const_iterator it = autoStates.find(MakeWordKey(group, word));
    if(it == autoStates.end())
        return 0;
    return it->second.column;
}

bool TFormVocabEdit::IsEffectivelyUsed(int group, const char *word) const
{
    const VOCAB_PLAN_ENTRY *e = FindPlanEntry(group, word);
    if(e) {
        if(e->forceRemove)
            return false;
        if(e->forceKeep)
            return true;
    }
    return IsAutoUsed(group, word);
}

int TFormVocabEdit::GetEffectiveColumn(int group, const char *word) const
{
    const VOCAB_PLAN_ENTRY *e = FindPlanEntry(group, word);
    if(e && (e->columnOverride == 0 || e->columnOverride == 1))
        return e->columnOverride;
    return GetAutoColumn(group, word);
}

int TFormVocabEdit::GetEffectivePickerVisibility(int group, const char *word) const
{
    const VOCAB_PLAN_ENTRY *e = FindPlanEntry(group, word);
    if(e && (e->pickerVisibility == 0 || e->pickerVisibility == 1))
        return e->pickerVisibility;
    return -1;
}

void TFormVocabEdit::SetUp(GAMEINFO *game)
{
    gameInfo = game;
    FreePlan(workingPlan);
    workingPlan = ClonePlan(game ? game->vocabPlan : NULL);
    groups.clear();
    autoStates.clear();

    if(game && game->title)
        lblGame->Caption = VclString(_T("Game: ")) + VclString(game->title);
    else
        lblGame->Caption = _T("Game: <unknown>");

    if(!game || !game->path || !LoadWordsTok(game->path)) {
        ShowMessage(_T("Could not load words.tok for this game."));
        lblStatus->Caption = _T("words.tok could not be loaded.");
        return;
    }

    AnalyzeCurrentWordset();
    if(tbSearch) {
        tbSearch->Text = _T("");
        tbSearch->Update();
    }
    searchFilter.clear();
    PopulateGroups();
    if(!visibleGroupIndices.empty()) {
        PopulateWords();
    } else {
        PopulatePreview();
    }
    UpdateStatus();
    UpdateActionButtons();
}

VOCAB_PLAN_ENTRY *TFormVocabEdit::DetachPlan()
{
    VOCAB_PLAN_ENTRY *plan = workingPlan;
    workingPlan = NULL;
    return plan;
}

bool TFormVocabEdit::LoadWordsTok(const TCHAR *gamePath)
{
    char path[1024];
    FILE *f;
    unsigned char *data = NULL;
    long len;
    std::vector<unsigned short> offsets(26, 0);
    char *tmpPath = strdup_tchar_to_char(gamePath);
    strncpy(path, tmpPath, sizeof(path)-1);
    path[sizeof(path)-1] = 0;
    free(tmpPath);
    size_t l = strlen(path);
    if(l && path[l-1] != '\\' && path[l-1] != '/') strcat(path, "\\");
    strcat(path, "words.tok");
    f = fopen(path, "rb");
    if(!f) return false;
    fseek(f, 0, SEEK_END);
    len = ftell(f);
    fseek(f, 0, SEEK_SET);
    data = (unsigned char*)malloc(len);
    if(!data) { fclose(f); return false; }
    fread(data, 1, len, f);
    fclose(f);

    for(int i=0;i<26;i++) offsets[i] = ReadBE16(data + i*2);
    groups.clear();
    for(int li=0; li<26; ++li) {
        unsigned short off = offsets[li];
        if(!off) continue;
        int pos = off;
        int next = (int)len;
        std::string prev;
        for(int ni=li+1; ni<26; ++ni) if(offsets[ni]) { next = offsets[ni]; break; }
        while(pos < len) {
            int keep = data[pos++];
            if(pos >= len) break;
            std::string suffix;
            while(pos < len) {
                unsigned char c = data[pos++];
                unsigned char decoded = (unsigned char)(c ^ 0x7F);
                suffix.push_back((char)(decoded & 0x7F));
                if(c & 0x80) break;
            }
            if(pos + 1 >= len) break;
            int group = ReadBE16(data + pos); pos += 2;
            std::string word = prev.substr(0, keep) + suffix;
            prev = word;
            if(group != 0 && group != 9999) {
                std::vector<TokGroup>::iterator it = std::find_if(groups.begin(), groups.end(), [group](const TokGroup& g){ return g.group == group; });
                if(it == groups.end()) {
                    TokGroup tg; tg.group = group; tg.words.push_back(word); groups.push_back(tg);
                } else {
                    it->words.push_back(word);
                }
            }
            if(pos >= next) break;
        }
    }
    free(data);
    std::sort(groups.begin(), groups.end(), [](const TokGroup&a,const TokGroup&b){ return a.group < b.group; });
    return true;
}

bool TFormVocabEdit::AnalyzeCurrentWordset()
{
    GAMEINFO *oldGi = gi;
    VOCAB_PLAN_ENTRY *oldPlan = gameInfo ? gameInfo->vocabPlan : NULL;
    if(!gameInfo || !vocabName || !FileExists(vocabName))
        return false;

    FreeGame();
    gi = gameInfo;
    gameInfo->vocabPlan = NULL;
    autoStates.clear();

    if(!ProcessWords()) {
        gameInfo->vocabPlan = oldPlan;
        gi = oldGi;
        FreeGame();
        return false;
    }

    for(WORDSET *w = wordset; w && w->group; ++w) {
        AutoWordState st;
        int q = (vocabData && w->string) ? FindWordx(w->string, vocabData) : -1;
        st.used = true;
        if(q != -1)
            st.column = (q & 0x80) ? 0 : 1;
        else
            st.column = (w->group & 0x8000) ? 0 : 1;
        autoStates[MakeWordKey(w->group & 0x1FFF, w->string)] = st;
    }

    FreeGame();
    gameInfo->vocabPlan = oldPlan;
    gi = oldGi;
    return true;
}

int TFormVocabEdit::GetSelectedGroupVectorIndex() const
{
    int idx = listGroups ? listGroups->ItemIndex : -1;
    if(idx < 0 || idx >= (int)visibleGroupIndices.size())
        return -1;
    return visibleGroupIndices[idx];
}

void TFormVocabEdit::PopulateGroups()
{
    int keepGroup = -1;
    int selectedIndex = GetSelectedGroupVectorIndex();
    if(selectedIndex >= 0 && selectedIndex < (int)groups.size())
        keepGroup = groups[selectedIndex].group;

    visibleGroupIndices.clear();
    listGroups->Items->Clear();
    for(size_t i=0;i<groups.size();++i) {
        bool matches = searchFilter.empty();
        if(!matches) {
            for(size_t wi=0; wi<groups[i].words.size(); ++wi) {
                if(LowerAscii(groups[i].words[wi]).find(searchFilter) != std::string::npos) {
                    matches = true;
                    break;
                }
            }
        }
        if(!matches) {
            for(VOCAB_PLAN_ENTRY *p = workingPlan; p; p = p->next) {
                if(p->group != groups[i].group || !p->word || !p->word[0])
                    continue;
                if(LowerAscii(p->word).find(searchFilter) != std::string::npos) {
                    matches = true;
                    break;
                }
            }
        }
        if(!matches)
            continue;
        visibleGroupIndices.push_back((int)i);
        char buf[128];
        sprintf(buf, "G%d (%d words)", groups[i].group, (int)groups[i].words.size());
        listGroups->Items->Add(buf);
    }
    if(!visibleGroupIndices.empty()) {
        int listIndex = 0;
        if(keepGroup >= 0) {
            for(size_t i=0; i<visibleGroupIndices.size(); ++i) {
                if(groups[visibleGroupIndices[i]].group == keepGroup) {
                    listIndex = (int)i;
                    break;
                }
            }
        }
        listGroups->ItemIndex = listIndex;
    } else {
        listGroups->ItemIndex = -1;
    }
    listGroups->Update();
}

void TFormVocabEdit::PopulateWords()
{
    std::string keepWord;
    int previousIndex = (listWords ? listWords->ItemIndex : -1);

    if(previousIndex >= 0 && previousIndex < (int)visibleWords.size())
        keepWord = visibleWords[previousIndex];

    listWords->Items->Clear();
    visibleWords.clear();
    int groupIndex = GetSelectedGroupVectorIndex();
    if(groupIndex < 0 || groupIndex >= (int)groups.size()) return;
    TokGroup &g = groups[groupIndex];
    for(size_t i=0;i<g.words.size();++i)
        visibleWords.push_back(g.words[i]);
    for(VOCAB_PLAN_ENTRY *p = workingPlan; p; p = p->next) {
        if(p->group != g.group || !p->word || !p->word[0] || !p->addAlias)
            continue;
        if(std::find(visibleWords.begin(), visibleWords.end(), p->word) == visibleWords.end())
            visibleWords.push_back(p->word);
    }
    std::sort(visibleWords.begin(), visibleWords.end());
    for(size_t i=0;i<visibleWords.size();++i) {
        const char *word = visibleWords[i].c_str();
        VOCAB_PLAN_ENTRY *e = FindPlanEntry(g.group, word);
        char buf[512];
        const char *used = IsEffectivelyUsed(g.group, word) ? "U" : "-";
        const char *autoUsed = IsAutoUsed(g.group, word) ? "A" : "-";
        const char *col = GetEffectiveColumn(g.group, word) ? "R" : "L";
        const char *mode = "   ";
        const char *colMode = "   ";
        const char *hidden = "   ";
        const char *showMode = "    ";
        if(e) {
            if(e->addAlias) mode = "[+]";
            else if(e->forceKeep) mode = "[K]";
            else if(e->forceRemove) mode = "[R]";
            if(e->columnOverride == 0) colMode = "[CL]";
            else if(e->columnOverride == 1) colMode = "[CR]";
            if(e->hidden) hidden = "[H]";
            if(e->pickerVisibility == 0) showMode = "[SN]";
            else if(e->pickerVisibility == 1) showMode = "[SM]";
        }
        sprintf(buf, "[%s%s|%s] %s %s %s %s %s", used, autoUsed, col, mode, colMode, hidden, showMode, word);
        listWords->Items->Add(buf);
    }
    if(listWords->Items->Count > 0) {
        int selectedIndex = 0;
        if(!keepWord.empty()) {
            for(size_t i=0; i<visibleWords.size(); ++i) {
                if(visibleWords[i] == keepWord) {
                    selectedIndex = (int)i;
                    break;
                }
            }
        } else if(!searchFilter.empty()) {
            for(size_t i=0; i<visibleWords.size(); ++i) {
                if(LowerAscii(visibleWords[i]).find(searchFilter) != std::string::npos) {
                    selectedIndex = (int)i;
                    break;
                }
            }
        }
        listWords->ItemIndex = selectedIndex;
    }
    listWords->Update();
    PopulatePreview();
    UpdateStatus();
    UpdateActionButtons();
}

void TFormVocabEdit::PopulatePreview()
{
    std::vector<std::string> leftWords;
    std::vector<std::string> rightWords;
    std::vector<std::string> leftAuto;
    std::vector<std::string> rightAuto;
    std::vector<std::string> leftNormal;
    std::vector<std::string> rightNormal;
    std::vector<std::string> leftMore;
    std::vector<std::string> rightMore;

    if(listPreviewLeft)
        listPreviewLeft->Items->Clear();
    if(listPreviewRight)
        listPreviewRight->Items->Clear();

    for(size_t giIdx = 0; giIdx < groups.size(); ++giIdx) {
        std::vector<std::string> groupWords = groups[giIdx].words;
        for(VOCAB_PLAN_ENTRY *p = workingPlan; p; p = p->next) {
            if(p->group != groups[giIdx].group || !p->word || !p->word[0] || !p->addAlias)
                continue;
            if(std::find(groupWords.begin(), groupWords.end(), p->word) == groupWords.end())
                groupWords.push_back(p->word);
        }
        for(size_t wi = 0; wi < groupWords.size(); ++wi) {
            const char *word = groupWords[wi].c_str();
            const VOCAB_PLAN_ENTRY *e = FindPlanEntry(groups[giIdx].group, word);
            int pickerVisibility;
            if(!IsEffectivelyUsed(groups[giIdx].group, word))
                continue;
            if(e && e->hidden)
                continue;
            pickerVisibility = GetEffectivePickerVisibility(groups[giIdx].group, word);
            if(GetEffectiveColumn(groups[giIdx].group, word) == 0) {
                if(pickerVisibility == 0)
                    leftNormal.push_back(groupWords[wi]);
                else if(pickerVisibility == 1)
                    leftMore.push_back(groupWords[wi]);
                else
                    leftAuto.push_back(groupWords[wi]);
            } else {
                if(pickerVisibility == 0)
                    rightNormal.push_back(groupWords[wi]);
                else if(pickerVisibility == 1)
                    rightMore.push_back(groupWords[wi]);
                else
                    rightAuto.push_back(groupWords[wi]);
            }
        }
    }

    std::sort(leftAuto.begin(), leftAuto.end());
    leftAuto.erase(std::unique(leftAuto.begin(), leftAuto.end()), leftAuto.end());
    std::sort(rightAuto.begin(), rightAuto.end());
    rightAuto.erase(std::unique(rightAuto.begin(), rightAuto.end()), rightAuto.end());
    std::sort(leftNormal.begin(), leftNormal.end());
    leftNormal.erase(std::unique(leftNormal.begin(), leftNormal.end()), leftNormal.end());
    std::sort(rightNormal.begin(), rightNormal.end());
    rightNormal.erase(std::unique(rightNormal.begin(), rightNormal.end()), rightNormal.end());
    std::sort(leftMore.begin(), leftMore.end());
    leftMore.erase(std::unique(leftMore.begin(), leftMore.end()), leftMore.end());
    std::sort(rightMore.begin(), rightMore.end());
    rightMore.erase(std::unique(rightMore.begin(), rightMore.end()), rightMore.end());

    if(previewMoreMode) {
        leftWords = leftNormal;
        rightWords = rightNormal;
        leftWords.insert(leftWords.end(), leftAuto.begin(), leftAuto.end());
        rightWords.insert(rightWords.end(), rightAuto.begin(), rightAuto.end());
        leftWords.insert(leftWords.end(), leftMore.begin(), leftMore.end());
        rightWords.insert(rightWords.end(), rightMore.begin(), rightMore.end());
    } else {
        leftWords = leftNormal;
        rightWords = rightNormal;
        leftWords.insert(leftWords.end(), leftAuto.begin(), leftAuto.end());
        rightWords.insert(rightWords.end(), rightAuto.begin(), rightAuto.end());
    }

    std::sort(leftWords.begin(), leftWords.end());
    leftWords.erase(std::unique(leftWords.begin(), leftWords.end()), leftWords.end());
    std::sort(rightWords.begin(), rightWords.end());
    rightWords.erase(std::unique(rightWords.begin(), rightWords.end()), rightWords.end());

    if(leftWords.empty() && rightWords.empty()) {
        std::string placeholder = previewMoreMode ? "(No more-page words)" : "(No preview words)";
        leftWords.push_back(placeholder);
    }

    if(listPreviewLeft) {
        for(size_t i = 0; i < leftWords.size(); ++i)
            listPreviewLeft->Items->Add(leftWords[i].c_str());
        listPreviewLeft->Update();
    }
    if(listPreviewRight) {
        for(size_t i = 0; i < rightWords.size(); ++i)
            listPreviewRight->Items->Add(rightWords[i].c_str());
        listPreviewRight->Update();
    }
    if(btnPreviewToggle && btnPreviewToggle->hWnd)
        ::SetWindowText(btnPreviewToggle->hWnd, previewMoreMode ? _T("< Less") : _T("More >"));
}

void TFormVocabEdit::SyncPreviewSelection()
{
    if(listPreviewLeft) {
        if(listPreviewLeft->hWnd)
            ::SendMessage(listPreviewLeft->hWnd, LB_SETCURSEL, (WPARAM)-1, 0);
        listPreviewLeft->ItemIndex = -1;
        listPreviewLeft->Update();
    }
    if(listPreviewRight) {
        if(listPreviewRight->hWnd)
            ::SendMessage(listPreviewRight->hWnd, LB_SETCURSEL, (WPARAM)-1, 0);
        listPreviewRight->ItemIndex = -1;
        listPreviewRight->Update();
    }
}

VOCAB_PLAN_ENTRY *TFormVocabEdit::FindPlanEntry(int group, const char *word)
{
    VOCAB_PLAN_ENTRY *p = workingPlan;
    while(p) {
        if(p->group == group && p->word && p->word[0] && strcmp(p->word, word) == 0) return p;
        p = p->next;
    }
    return NULL;
}

const VOCAB_PLAN_ENTRY *TFormVocabEdit::FindPlanEntry(int group, const char *word) const
{
    const VOCAB_PLAN_ENTRY *p = workingPlan;
    while(p) {
        if(p->group == group && p->word && p->word[0] && strcmp(p->word, word) == 0) return p;
        p = p->next;
    }
    return NULL;
}

VOCAB_PLAN_ENTRY *TFormVocabEdit::FindGroupMetaEntry(int group)
{
    VOCAB_PLAN_ENTRY *p = workingPlan;
    while(p) {
        if(p->group == group && p->word && !p->word[0]) return p;
        p = p->next;
    }
    return NULL;
}

const VOCAB_PLAN_ENTRY *TFormVocabEdit::FindGroupMetaEntry(int group) const
{
    const VOCAB_PLAN_ENTRY *p = workingPlan;
    while(p) {
        if(p->group == group && p->word && !p->word[0]) return p;
        p = p->next;
    }
    return NULL;
}

VOCAB_PLAN_ENTRY *TFormVocabEdit::GetOrCreatePlanEntry(int group, const char *word)
{
    VOCAB_PLAN_ENTRY *p = FindPlanEntry(group, word);
    if(p) return p;
    p = (VOCAB_PLAN_ENTRY*)calloc(1, sizeof(VOCAB_PLAN_ENTRY));
    p->group = group;
    p->word = strdup(word);
    p->columnOverride = -1;
    p->pickerVisibility = -1;
    p->next = workingPlan;
    workingPlan = p;
    return p;
}

VOCAB_PLAN_ENTRY *TFormVocabEdit::GetOrCreateGroupMetaEntry(int group)
{
    VOCAB_PLAN_ENTRY *p = FindGroupMetaEntry(group);
    if(p) return p;
    p = (VOCAB_PLAN_ENTRY*)calloc(1, sizeof(VOCAB_PLAN_ENTRY));
    p->group = group;
    p->word = strdup("");
    p->columnOverride = -1;
    p->pickerVisibility = -1;
    p->next = workingPlan;
    workingPlan = p;
    return p;
}

void TFormVocabEdit::SetWordMode(int group, const char *word, BOOL forceKeep, BOOL forceRemove)
{
    VOCAB_PLAN_ENTRY *p = GetOrCreatePlanEntry(group, word);
    BOOL wasAlias = p->addAlias;
    if(forceRemove && wasAlias)
        p->addAlias = FALSE;
    else if(forceKeep && wasAlias)
        p->addAlias = TRUE;
    else if(!wasAlias)
        p->addAlias = FALSE;
    p->forceKeep = forceKeep;
    p->forceRemove = forceRemove;
    RemovePlanEntryIfEmpty(p);
    UpdateStatus();
    PopulateWords();
}

void TFormVocabEdit::ToggleHidden(int group, const char *word)
{
    VOCAB_PLAN_ENTRY *p = GetOrCreatePlanEntry(group, word);
    p->hidden = !p->hidden;
    RemovePlanEntryIfEmpty(p);
    UpdateStatus();
    PopulateWords();
}

void TFormVocabEdit::SetColumnOverride(int group, const char *word, int columnOverride)
{
    VOCAB_PLAN_ENTRY *p = GetOrCreatePlanEntry(group, word);
    p->columnOverride = columnOverride;
    RemovePlanEntryIfEmpty(p);
    UpdateStatus();
    PopulateWords();
}

static void SetPickerVisibilityInternal(VOCAB_PLAN_ENTRY *p, int pickerVisibility)
{
    if(p)
        p->pickerVisibility = pickerVisibility;
}

void TFormVocabEdit::AddAliasWord(int group, const char *word)
{
    if(!word || !word[0]) return;
    VOCAB_PLAN_ENTRY *p = GetOrCreatePlanEntry(group, word);
    p->addAlias = TRUE;
    p->forceKeep = TRUE;
    p->forceRemove = FALSE;
    if(p->columnOverride != 0 && p->columnOverride != 1)
        p->columnOverride = 0;
    if(p->pickerVisibility != 0 && p->pickerVisibility != 1)
        p->pickerVisibility = 0;
    UpdateStatus();
    PopulateWords();
}

void TFormVocabEdit::ClearWordState(int group, const char *word)
{
    VOCAB_PLAN_ENTRY *p = FindPlanEntry(group, word);
    if(!p) return;
    p->addAlias = FALSE;
    p->forceKeep = FALSE;
    p->forceRemove = FALSE;
    p->hidden = FALSE;
    p->pickerVisibility = -1;
    p->columnOverride = -1;
    RemovePlanEntryIfEmpty(p);
    UpdateStatus();
    PopulateWords();
}

void TFormVocabEdit::ClearGroupState(int group)
{
    VOCAB_PLAN_ENTRY *p = workingPlan;
    while(p) {
        if(p->group == group) {
            p->addAlias = FALSE;
            p->forceKeep = FALSE;
            p->forceRemove = FALSE;
            p->hidden = FALSE;
            p->replaceGroup = FALSE;
            p->pickerVisibility = -1;
            p->columnOverride = -1;
        }
        p = p->next;
    }
    p = workingPlan;
    while(p) {
        VOCAB_PLAN_ENTRY *next = p->next;
        RemovePlanEntryIfEmpty(p);
        p = next;
    }
    UpdateStatus();
    PopulateWords();
}

void TFormVocabEdit::ResetToAuto()
{
    FreePlan(workingPlan);
    workingPlan = NULL;
    if(tbAlias) {
        tbAlias->Text = _T("");
        tbAlias->Update();
    }
    PopulateWords();
}

void TFormVocabEdit::UpdateStatus()
{
    int keep = 0, remove = 0, hide = 0, col = 0, show = 0, added = 0, replaced = 0, autoUsed = 0, effectiveUsed = 0;
    for(std::map<std::string, AutoWordState>::const_iterator it = autoStates.begin(); it != autoStates.end(); ++it)
        if(it->second.used) autoUsed++;
    for(size_t giIdx=0; giIdx<groups.size(); ++giIdx) {
        for(size_t wi=0; wi<groups[giIdx].words.size(); ++wi) {
            if(IsEffectivelyUsed(groups[giIdx].group, groups[giIdx].words[wi].c_str()))
                effectiveUsed++;
        }
    }
    for(VOCAB_PLAN_ENTRY *p = workingPlan; p; p = p->next) {
        if(p->forceKeep) keep++;
        if(p->forceRemove) remove++;
        if(p->hidden) hide++;
        if(p->pickerVisibility == 0 || p->pickerVisibility == 1) show++;
        if(p->addAlias) added++;
        if(p->replaceGroup) replaced++;
        if(p->columnOverride == 0 || p->columnOverride == 1) col++;
    }
    char buf[256];
    sprintf(buf, "Auto %d | Effective %d | keep %d remove %d add %d hide %d show %d col %d replace %d", autoUsed, effectiveUsed, keep, remove, added, hide, show, col, replaced);
    lblStatus->Caption = buf;
    lblStatus->Update();
}

void TFormVocabEdit::ApplySearchFocus()
{
    PopulateGroups();
}

void TFormVocabEdit::UpdateActionButtons()
{
    int group = -1;
    int groupIndex = GetSelectedGroupVectorIndex();
    if(groupIndex >= 0 && groupIndex < (int)groups.size())
        group = groups[groupIndex].group;
    bool hasWord = listWords->ItemIndex >= 0 && listWords->ItemIndex < (int)visibleWords.size();
    if(btnClearGroup && btnClearGroup->hWnd)
        ::EnableWindow(btnClearGroup->hWnd, group >= 0);
    if(btnKeep && btnKeep->hWnd)
        ::EnableWindow(btnKeep->hWnd, hasWord);
    if(btnRemoveWord && btnRemoveWord->hWnd)
        ::EnableWindow(btnRemoveWord->hWnd, hasWord);
    if(btnHide && btnHide->hWnd)
        ::EnableWindow(btnHide->hWnd, hasWord);
    if(btnClearWord && btnClearWord->hWnd)
        ::EnableWindow(btnClearWord->hWnd, hasWord);
    if(btnColLeft && btnColLeft->hWnd)
        ::EnableWindow(btnColLeft->hWnd, hasWord);
    if(btnColRight && btnColRight->hWnd)
        ::EnableWindow(btnColRight->hWnd, hasWord);
    if(btnColAuto && btnColAuto->hWnd)
        ::EnableWindow(btnColAuto->hWnd, hasWord);
    if(btnShowAuto && btnShowAuto->hWnd)
        ::EnableWindow(btnShowAuto->hWnd, hasWord);
    if(btnShowNormal && btnShowNormal->hWnd)
        ::EnableWindow(btnShowNormal->hWnd, hasWord);
    if(btnShowMore && btnShowMore->hWnd)
        ::EnableWindow(btnShowMore->hWnd, hasWord);
    UpdateModeButtonCaptions();
}

void TFormVocabEdit::UpdateModeButtonCaptions()
{
    int groupIndex = GetSelectedGroupVectorIndex();
    bool hasWord = listWords && listWords->ItemIndex >= 0 && listWords->ItemIndex < (int)visibleWords.size();
    const char *word = hasWord ? visibleWords[listWords->ItemIndex].c_str() : NULL;
    int group = (groupIndex >= 0 && groupIndex < (int)groups.size()) ? groups[groupIndex].group : -1;
    int effectiveColumn = hasWord ? GetEffectiveColumn(group, word) : -1;
    int effectivePicker = hasWord ? GetEffectivePickerVisibility(group, word) : -2;
    bool hidden = false;
    bool removed = false;
    bool kept = false;
    const VOCAB_PLAN_ENTRY *e = (hasWord ? FindPlanEntry(group, word) : NULL);
    if(e) {
        hidden = e->hidden ? true : false;
        removed = e->forceRemove ? true : false;
        kept = e->forceKeep ? true : false;
    }

    if(btnKeep && btnKeep->hWnd)
        ::SetWindowText(btnKeep->hWnd, _T("Keep"));
    if(btnRemoveWord && btnRemoveWord->hWnd)
        ::SetWindowText(btnRemoveWord->hWnd, _T("Remove"));
    if(btnHide && btnHide->hWnd)
        ::SetWindowText(btnHide->hWnd, hidden ? _T("Hidden") : _T("Hide"));

    {
        bool autoCol = !e || (e->columnOverride != 0 && e->columnOverride != 1);
        bool autoShow = !e || (e->pickerVisibility != 0 && e->pickerVisibility != 1);
        if(btnColLeft && btnColLeft->hWnd)
            ::SetWindowText(btnColLeft->hWnd, (hasWord && !autoCol && effectiveColumn == 0) ? _T(">> Column Left <<") : _T("Column Left"));
        if(btnColRight && btnColRight->hWnd)
            ::SetWindowText(btnColRight->hWnd, (hasWord && !autoCol && effectiveColumn == 1) ? _T(">> Column Right <<") : _T("Column Right"));
        if(btnColAuto && btnColAuto->hWnd)
            ::SetWindowText(btnColAuto->hWnd, (hasWord && autoCol) ? _T(">> Column Auto <<") : _T("Column Auto"));
        if(btnShowNormal && btnShowNormal->hWnd)
            ::SetWindowText(btnShowNormal->hWnd, (hasWord && !autoShow && effectivePicker == 0) ? _T(">> Show Normal <<") : _T("Show Normal"));
        if(btnShowMore && btnShowMore->hWnd)
            ::SetWindowText(btnShowMore->hWnd, (hasWord && !autoShow && effectivePicker == 1) ? _T(">> Show More <<") : _T("Show More"));
        if(btnShowAuto && btnShowAuto->hWnd)
            ::SetWindowText(btnShowAuto->hWnd, (hasWord && autoShow) ? _T(">> Show Auto <<") : _T("Show Auto"));
        if(btnColLeft && btnColLeft->hWnd) ::InvalidateRect(btnColLeft->hWnd, NULL, TRUE);
        if(btnColRight && btnColRight->hWnd) ::InvalidateRect(btnColRight->hWnd, NULL, TRUE);
        if(btnColAuto && btnColAuto->hWnd) ::InvalidateRect(btnColAuto->hWnd, NULL, TRUE);
        if(btnShowNormal && btnShowNormal->hWnd) ::InvalidateRect(btnShowNormal->hWnd, NULL, TRUE);
        if(btnShowMore && btnShowMore->hWnd) ::InvalidateRect(btnShowMore->hWnd, NULL, TRUE);
        if(btnShowAuto && btnShowAuto->hWnd) ::InvalidateRect(btnShowAuto->hWnd, NULL, TRUE);
    }
}

VOCAB_PLAN_ENTRY *TFormVocabEdit::ClonePlan(VOCAB_PLAN_ENTRY *src)
{
    VOCAB_PLAN_ENTRY *head = NULL, *tail = NULL;
    while(src) {
        VOCAB_PLAN_ENTRY *p = (VOCAB_PLAN_ENTRY*)calloc(1, sizeof(VOCAB_PLAN_ENTRY));
        *p = *src;
        p->word = src->word ? strdup(src->word) : NULL;
        p->next = NULL;
        if(!head) head = p; else tail->next = p;
        tail = p;
        src = src->next;
    }
    return head;
}

void TFormVocabEdit::RemovePlanEntry(VOCAB_PLAN_ENTRY *entry)
{
    VOCAB_PLAN_ENTRY **pp = &workingPlan;
    while(*pp) {
        if(*pp == entry) {
            *pp = entry->next;
            if(entry->word) free(entry->word);
            free(entry);
            return;
        }
        pp = &((*pp)->next);
    }
}

void TFormVocabEdit::RemovePlanEntryIfEmpty(VOCAB_PLAN_ENTRY *entry)
{
    if(!entry) return;
    if(entry->forceKeep || entry->forceRemove || entry->hidden || entry->addAlias || entry->replaceGroup)
        return;
    if(entry->pickerVisibility == 0 || entry->pickerVisibility == 1)
        return;
    if(entry->columnOverride == 0 || entry->columnOverride == 1)
        return;
    RemovePlanEntry(entry);
}

void TFormVocabEdit::FreePlan(VOCAB_PLAN_ENTRY *plan)
{
    while(plan) {
        VOCAB_PLAN_ENTRY *next = plan->next;
        if(plan->word) free(plan->word);
        free(plan);
        plan = next;
    }
}

bool TFormVocabEdit::SavePreset(const TCHAR *filename)
{
    FILE *f = _tfopen(filename, _T("wb"));
    if(!f) return false;
    fprintf(f, "GBAGI_VOCAB_PRESET_V4\n");
    for(VOCAB_PLAN_ENTRY *p = workingPlan; p; p = p->next) {
        int keep = p->forceKeep ? 1 : 0;
        int remove = p->forceRemove ? 1 : 0;
        int hidden = p->hidden ? 1 : 0;
        int addAlias = p->addAlias ? 1 : 0;
        int replaceGroup = p->replaceGroup ? 1 : 0;
        int pickerVisibility = p->pickerVisibility;
        int columnOverride = p->columnOverride;

        if(p->word && p->word[0] && !p->addAlias) {
            if((columnOverride == 0 || columnOverride == 1) &&
               columnOverride == GetAutoColumn(p->group, p->word))
            {
                columnOverride = -1;
            }
        }

        if(!keep && !remove && !hidden && !addAlias && !replaceGroup &&
           pickerVisibility == -1 && columnOverride == -1)
        {
            continue;
        }

        fprintf(f, "%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%s\n",
            p->group,
            keep,
            remove,
            hidden,
            addAlias,
            replaceGroup,
            pickerVisibility,
            columnOverride,
            p->word ? p->word : "");
    }
    fclose(f);
    return true;
}

bool TFormVocabEdit::LoadPreset(const TCHAR *filename)
{
    FILE *f = _tfopen(filename, _T("rb"));
    char line[2048];
    bool isV3 = false;
    bool isV4 = false;
    if(!f) return false;
    if(!fgets(line, sizeof(line), f)) { fclose(f); return false; }
    if(strncmp(line, "GBAGI_VOCAB_PRESET_V4", 21) == 0) {
        isV3 = true;
        isV4 = true;
    } else if(strncmp(line, "GBAGI_VOCAB_PRESET_V3", 21) == 0)
        isV3 = true;
    else if(strncmp(line, "GBAGI_VOCAB_PRESET_V2", 21) != 0) { fclose(f); return false; }
    FreePlan(workingPlan);
    workingPlan = NULL;
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
        if(!groupS || !keepS || !removeS || !hideS || !colS) continue;
        if(isV3 && (!addS || !replaceS)) continue;
        if(!wordS) wordS = (char*)"";
        int group = atoi(groupS);
        VOCAB_PLAN_ENTRY *p = (wordS[0] == 0) ? GetOrCreateGroupMetaEntry(group) : GetOrCreatePlanEntry(group, wordS);
        p->forceKeep = atoi(keepS) ? TRUE : FALSE;
        p->forceRemove = atoi(removeS) ? TRUE : FALSE;
        p->hidden = atoi(hideS) ? TRUE : FALSE;
        p->addAlias = isV3 && atoi(addS) ? TRUE : FALSE;
        p->replaceGroup = isV3 && atoi(replaceS) ? TRUE : FALSE;
        p->pickerVisibility = (isV4 && showS) ? atoi(showS) : -1;
        p->columnOverride = atoi(colS);
        RemovePlanEntryIfEmpty(p);
    }
    fclose(f);
    PopulateWords();
    return true;
}

void TFormVocabEdit::AutoSavePresetIfPossible()
{
    TCHAR path[1024];
    size_t len;
    if(!gameInfo || !gameInfo->path)
        return;
    _tcsncpy(path, gameInfo->path, _countof(path) - 1);
    path[_countof(path) - 1] = 0;
    len = _tcslen(path);
    if(len && path[len - 1] != '\\' && path[len - 1] != '/')
        _tcscat(path, _T("\\"));
    _tcscat(path, _T("gbagi_vocab_preset.tsv"));
    SavePreset(path);
}

static bool PromptOpenPreset(HWND owner, TCHAR *path, DWORD pathLen)
{
    OPENFILENAME ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    path[0] = 0;
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = owner;
    ofn.lpstrFilter = _T("GBAGI vocab preset (*.tsv)\0*.tsv\0All Files\0*.*\0");
    ofn.lpstrFile = path;
    ofn.nMaxFile = pathLen;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
    ofn.lpstrDefExt = _T("tsv");
    return GetOpenFileName(&ofn) == TRUE;
}

static bool PromptSavePreset(HWND owner, TCHAR *path, DWORD pathLen, const TCHAR *initial)
{
    OPENFILENAME ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    _tcsncpy(path, initial, pathLen - 1);
    path[pathLen - 1] = 0;
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = owner;
    ofn.lpstrFilter = _T("GBAGI vocab preset (*.tsv)\0*.tsv\0All Files\0*.*\0");
    ofn.lpstrFile = path;
    ofn.nMaxFile = pathLen;
    ofn.Flags = OFN_OVERWRITEPROMPT;
    ofn.lpstrDefExt = _T("tsv");
    return GetSaveFileName(&ofn) == TRUE;
}

void __fastcall TFormVocabEdit::listGroupsChange(TObject *Sender)
{
    PopulateWords();
}

void __fastcall TFormVocabEdit::listWordsChange(TObject *Sender)
{
    UpdateActionButtons();
}

void __fastcall TFormVocabEdit::tbSearchChange(TObject *Sender)
{
    char *searchRaw = strdup_tchar_to_char(tbSearch ? tbSearch->Text.c_str() : _T(""));
    searchFilter = LowerAscii(TrimAscii(searchRaw ? searchRaw : ""));
    if(searchRaw) free(searchRaw);
    ApplySearchFocus();
    PopulateWords();
}

void __fastcall TFormVocabEdit::btnKeepClick(TObject *Sender)
{
    int groupIndex = GetSelectedGroupVectorIndex();
    if(groupIndex < 0 || groupIndex >= (int)groups.size()) return;
    TokGroup &g = groups[groupIndex];
    if(listWords->ItemIndex < 0 || listWords->ItemIndex >= (int)visibleWords.size()) return;
    SetWordMode(g.group, visibleWords[listWords->ItemIndex].c_str(), TRUE, FALSE);
}

void __fastcall TFormVocabEdit::btnRemoveWordClick(TObject *Sender)
{
    int groupIndex = GetSelectedGroupVectorIndex();
    if(groupIndex < 0 || groupIndex >= (int)groups.size()) return;
    TokGroup &g = groups[groupIndex];
    if(listWords->ItemIndex < 0 || listWords->ItemIndex >= (int)visibleWords.size()) return;
    SetWordMode(g.group, visibleWords[listWords->ItemIndex].c_str(), FALSE, TRUE);
}

void __fastcall TFormVocabEdit::btnHideClick(TObject *Sender)
{
    int groupIndex = GetSelectedGroupVectorIndex();
    if(groupIndex < 0 || groupIndex >= (int)groups.size()) return;
    TokGroup &g = groups[groupIndex];
    if(listWords->ItemIndex < 0 || listWords->ItemIndex >= (int)visibleWords.size()) return;
    ToggleHidden(g.group, visibleWords[listWords->ItemIndex].c_str());
}

void __fastcall TFormVocabEdit::btnClearWordClick(TObject *Sender)
{
    int groupIndex = GetSelectedGroupVectorIndex();
    if(groupIndex < 0 || groupIndex >= (int)groups.size()) return;
    TokGroup &g = groups[groupIndex];
    if(listWords->ItemIndex < 0 || listWords->ItemIndex >= (int)visibleWords.size()) return;
    ClearWordState(g.group, visibleWords[listWords->ItemIndex].c_str());
}

void __fastcall TFormVocabEdit::btnClearGroupClick(TObject *Sender)
{
    int groupIndex = GetSelectedGroupVectorIndex();
    if(groupIndex < 0 || groupIndex >= (int)groups.size()) return;
    ClearGroupState(groups[groupIndex].group);
}

void __fastcall TFormVocabEdit::btnColLeftClick(TObject *Sender)
{
    int groupIndex = GetSelectedGroupVectorIndex();
    if(groupIndex < 0 || groupIndex >= (int)groups.size()) return;
    TokGroup &g = groups[groupIndex];
    if(listWords->ItemIndex < 0 || listWords->ItemIndex >= (int)visibleWords.size()) return;
    SetColumnOverride(g.group, visibleWords[listWords->ItemIndex].c_str(), 0);
}

void __fastcall TFormVocabEdit::btnColRightClick(TObject *Sender)
{
    int groupIndex = GetSelectedGroupVectorIndex();
    if(groupIndex < 0 || groupIndex >= (int)groups.size()) return;
    TokGroup &g = groups[groupIndex];
    if(listWords->ItemIndex < 0 || listWords->ItemIndex >= (int)visibleWords.size()) return;
    SetColumnOverride(g.group, visibleWords[listWords->ItemIndex].c_str(), 1);
}

void __fastcall TFormVocabEdit::btnColAutoClick(TObject *Sender)
{
    int groupIndex = GetSelectedGroupVectorIndex();
    if(groupIndex < 0 || groupIndex >= (int)groups.size()) return;
    TokGroup &g = groups[groupIndex];
    if(listWords->ItemIndex < 0 || listWords->ItemIndex >= (int)visibleWords.size()) return;
    SetColumnOverride(g.group, visibleWords[listWords->ItemIndex].c_str(), -1);
}

void __fastcall TFormVocabEdit::btnShowAutoClick(TObject *Sender)
{
    int groupIndex = GetSelectedGroupVectorIndex();
    if(groupIndex < 0 || groupIndex >= (int)groups.size()) return;
    TokGroup &g = groups[groupIndex];
    if(listWords->ItemIndex < 0 || listWords->ItemIndex >= (int)visibleWords.size()) return;
    VOCAB_PLAN_ENTRY *p = GetOrCreatePlanEntry(g.group, visibleWords[listWords->ItemIndex].c_str());
    SetPickerVisibilityInternal(p, -1);
    RemovePlanEntryIfEmpty(p);
    UpdateStatus();
    PopulateWords();
}

void __fastcall TFormVocabEdit::btnShowNormalClick(TObject *Sender)
{
    int groupIndex = GetSelectedGroupVectorIndex();
    if(groupIndex < 0 || groupIndex >= (int)groups.size()) return;
    TokGroup &g = groups[groupIndex];
    if(listWords->ItemIndex < 0 || listWords->ItemIndex >= (int)visibleWords.size()) return;
    VOCAB_PLAN_ENTRY *p = GetOrCreatePlanEntry(g.group, visibleWords[listWords->ItemIndex].c_str());
    SetPickerVisibilityInternal(p, 0);
    RemovePlanEntryIfEmpty(p);
    UpdateStatus();
    PopulateWords();
}

void __fastcall TFormVocabEdit::btnShowMoreClick(TObject *Sender)
{
    int groupIndex = GetSelectedGroupVectorIndex();
    if(groupIndex < 0 || groupIndex >= (int)groups.size()) return;
    TokGroup &g = groups[groupIndex];
    if(listWords->ItemIndex < 0 || listWords->ItemIndex >= (int)visibleWords.size()) return;
    VOCAB_PLAN_ENTRY *p = GetOrCreatePlanEntry(g.group, visibleWords[listWords->ItemIndex].c_str());
    SetPickerVisibilityInternal(p, 1);
    RemovePlanEntryIfEmpty(p);
    UpdateStatus();
    PopulateWords();
}

void __fastcall TFormVocabEdit::btnAddAliasClick(TObject *Sender)
{
    int groupIndex = GetSelectedGroupVectorIndex();
    if(groupIndex < 0 || groupIndex >= (int)groups.size()) return;
    char *aliasRaw = strdup_tchar_to_char(tbAlias ? tbAlias->Text.c_str() : _T(""));
    std::string alias = TrimAscii(aliasRaw ? aliasRaw : "");
    if(aliasRaw) free(aliasRaw);
    if(alias.empty()) {
        ShowMessage(_T("Type an alias to add for the selected group."));
        return;
    }
    AddAliasWord(groups[groupIndex].group, alias.c_str());
    if(tbAlias) {
        tbAlias->Text = _T("");
        tbAlias->Update();
    }
}

void __fastcall TFormVocabEdit::btnLoadClick(TObject *Sender)
{
    TCHAR path[1024];
    if(PromptOpenPreset(this->hWnd, path, _countof(path))) {
        if(!LoadPreset(path))
            ShowMessage(_T("Could not load preset."));
    }
}

void __fastcall TFormVocabEdit::btnSaveClick(TObject *Sender)
{
    TCHAR path[1024];
    TCHAR initial[1024] = _T("");
    if(gameInfo && gameInfo->path) {
        _tcsncpy(initial, gameInfo->path, _countof(initial)-1);
        initial[_countof(initial)-1] = 0;
        if(_tcslen(initial) && initial[_tcslen(initial)-1] != '\\' && initial[_tcslen(initial)-1] != '/')
            _tcscat(initial, _T("\\"));
        _tcscat(initial, _T("gbagi_vocab_preset.tsv"));
    }
    if(PromptSavePreset(this->hWnd, path, _countof(path), initial)) {
        if(!SavePreset(path))
            ShowMessage(_T("Could not save preset."));
    }
}

void __fastcall TFormVocabEdit::btnResetAutoClick(TObject *Sender)
{
    ResetToAuto();
}

void __fastcall TFormVocabEdit::btnPreviewToggleClick(TObject *Sender)
{
    previewMoreMode = !previewMoreMode;
    PopulatePreview();
}

void __fastcall TFormVocabEdit::btnOKClick(TObject *Sender)
{
    AutoSavePresetIfPossible();
    okClose = TRUE;
    Close();
}

void __fastcall TFormVocabEdit::btnCancelClick(TObject *Sender)
{
    Close();
}
