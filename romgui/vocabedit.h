#ifndef vocabeditH
#define vocabeditH

#include "vcl-shim/vcl-shim.h"
#include "makerom.h"
#include <vector>
#include <string>
#include <map>

class TFormVocabEdit : public TForm
{
__published:
    TGroupBox *GroupGroups;
    TGroupBox *GroupWords;
    TListBox *listGroups;
    TListBox *listWords;
    TButton *btnKeep;
    TButton *btnRemoveWord;
    TButton *btnHide;
    TButton *btnClearWord;
    TButton *btnClearGroup;
    TButton *btnColLeft;
    TButton *btnColRight;
    TButton *btnColAuto;
    TButton *btnShowAuto;
    TButton *btnShowNormal;
    TButton *btnShowMore;
    TButton *btnLoad;
    TButton *btnSave;
    TButton *btnResetAuto;
    TGroupBox *GroupPreview;
    TListBox *listPreviewLeft;
    TListBox *listPreviewRight;
    TButton *btnPreviewToggle;
    TEdit *tbSearch;
    TEdit *tbAlias;
    TButton *btnAddAlias;
    TButton *btnOK;
    TButton *btnCancel;
    TLabel *lblGame;
    TLabel *lblStatus;
    TLabel *lblHelp;

    void __fastcall btnKeepClick(TObject *Sender);
    void __fastcall btnRemoveWordClick(TObject *Sender);
    void __fastcall btnHideClick(TObject *Sender);
    void __fastcall btnClearWordClick(TObject *Sender);
    void __fastcall btnClearGroupClick(TObject *Sender);
    void __fastcall btnColLeftClick(TObject *Sender);
    void __fastcall btnColRightClick(TObject *Sender);
    void __fastcall btnColAutoClick(TObject *Sender);
    void __fastcall btnShowAutoClick(TObject *Sender);
    void __fastcall btnShowNormalClick(TObject *Sender);
    void __fastcall btnShowMoreClick(TObject *Sender);
    void __fastcall btnLoadClick(TObject *Sender);
    void __fastcall btnSaveClick(TObject *Sender);
    void __fastcall btnResetAutoClick(TObject *Sender);
    void __fastcall btnPreviewToggleClick(TObject *Sender);
    void __fastcall btnAddAliasClick(TObject *Sender);
    void __fastcall btnOKClick(TObject *Sender);
    void __fastcall btnCancelClick(TObject *Sender);
    void __fastcall listGroupsChange(TObject *Sender);
    void __fastcall listWordsChange(TObject *Sender);
    void __fastcall tbSearchChange(TObject *Sender);
private:
    virtual void CreateControls();
    virtual void OnInitDialog(HWND hWnd);
    virtual void OnCommand(WPARAM wParam, LPARAM lParam);
    virtual LRESULT OnDrawItem(WPARAM wParam, LPARAM lParam);

    struct TokGroup { int group; std::vector<std::string> words; };
    struct AutoWordState { bool used; int column; };
    struct PreviewEntry { int group; std::string word; };
    std::vector<TokGroup> groups;
    std::vector<int> visibleGroupIndices;
    std::vector<std::string> visibleWords;
    std::vector<PreviewEntry> autoPreviewOrder;
    std::string searchFilter;
    bool previewMoreMode;
    std::map<std::string, AutoWordState> autoStates;
    GAMEINFO *gameInfo;
    VOCAB_PLAN_ENTRY *workingPlan;

    bool LoadWordsTok(const TCHAR *gamePath);
    bool AnalyzeCurrentWordset();
    void PopulateGroups();
    void PopulateWords();
    void PopulatePreview();
    void ApplySearchFocus();
    int GetSelectedGroupVectorIndex() const;
    void UpdateStatus();
    void UpdateActionButtons();
    void SyncPreviewSelection();
    void UpdateModeButtonCaptions();
    VOCAB_PLAN_ENTRY *FindPlanEntry(int group, const char *word);
    const VOCAB_PLAN_ENTRY *FindPlanEntry(int group, const char *word) const;
    VOCAB_PLAN_ENTRY *FindGroupMetaEntry(int group);
    const VOCAB_PLAN_ENTRY *FindGroupMetaEntry(int group) const;
    VOCAB_PLAN_ENTRY *GetOrCreatePlanEntry(int group, const char *word);
    VOCAB_PLAN_ENTRY *GetOrCreateGroupMetaEntry(int group);
    void SetWordMode(int group, const char *word, BOOL forceKeep, BOOL forceRemove);
    void ToggleHidden(int group, const char *word);
    void SetColumnOverride(int group, const char *word, int columnOverride);
    void AddAliasWord(int group, const char *word);
    void ClearWordState(int group, const char *word);
    void ClearGroupState(int group);
    void ResetToAuto();
    void FreePlan(VOCAB_PLAN_ENTRY *plan);
    void RemovePlanEntry(VOCAB_PLAN_ENTRY *entry);
    void RemovePlanEntryIfEmpty(VOCAB_PLAN_ENTRY *entry);
    VOCAB_PLAN_ENTRY *ClonePlan(VOCAB_PLAN_ENTRY *src);
    bool SavePreset(const TCHAR *filename);
    bool LoadPreset(const TCHAR *filename);
    void AutoSavePresetIfPossible();
    std::string MakeWordKey(int group, const char *word) const;
    bool IsAutoUsed(int group, const char *word) const;
    int GetAutoColumn(int group, const char *word) const;
    bool IsEffectivelyUsed(int group, const char *word) const;
    int GetEffectiveColumn(int group, const char *word) const;
    int GetEffectivePickerVisibility(int group, const char *word) const;
public:
    __fastcall TFormVocabEdit(TComponent* Owner);
    void SetUp(GAMEINFO *game);
    VOCAB_PLAN_ENTRY *DetachPlan();
    BOOL okClose;
};

#endif
