object FormMain: TFormMain
  Left = 192
  Top = 116
  Width = 544
  Height = 371
  Caption = 'GBAGI Injection Utility'
  Color = clWhite
  Font.Charset = ANSI_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Verdana'
  Font.Style = []
  OldCreateOrder = False
  Position = poDesktopCenter
  OnDestroy = FormDestroy
  OnResize = FormResize
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 13
  object Label1: TLabel
    Left = 0
    Top = 0
    Width = 536
    Height = 18
    Align = alTop
    Alignment = taCenter
    Caption = 'GBAGI Injection Utility 2.0'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -16
    Font.Name = 'Verdana'
    Font.Style = [fsBold]
    ParentFont = False
    Layout = tlCenter
  end
  object Label2: TLabel
    Left = 0
    Top = 18
    Width = 536
    Height = 16
    Align = alTop
    Alignment = taCenter
    Caption = 'By Brian Provinciano'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -13
    Font.Name = 'Verdana'
    Font.Style = []
    ParentFont = False
    Layout = tlCenter
  end
  object Label3: TLabel
    Left = 0
    Top = 34
    Width = 536
    Height = 13
    Cursor = crHandPoint
    Align = alTop
    Alignment = taCenter
    Caption = 'http://www.bripro.com'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clBlue
    Font.Height = -11
    Font.Name = 'Verdana'
    Font.Style = [fsUnderline]
    ParentFont = False
    Layout = tlCenter
    OnClick = Label3Click
  end
  object Panel1: TPanel
    Left = 0
    Top = 47
    Width = 536
    Height = 4
    Align = alTop
    BevelOuter = bvNone
    Color = 16775408
    TabOrder = 0
  end
  object Panel2: TPanel
    Left = 0
    Top = 51
    Width = 536
    Height = 42
    Align = alTop
    BevelOuter = bvNone
    ParentColor = True
    TabOrder = 1
    object Panel6: TPanel
      Left = 0
      Top = 0
      Width = 536
      Height = 21
      Align = alTop
      BevelOuter = bvNone
      ParentColor = True
      TabOrder = 0
      object Label6: TLabel
        Left = 0
        Top = 0
        Width = 153
        Height = 21
        Align = alLeft
        AutoSize = False
        Caption = ' ROM Input Filename:'
        Font.Charset = ANSI_CHARSET
        Font.Color = clWindowText
        Font.Height = -11
        Font.Name = 'Verdana'
        Font.Style = [fsBold]
        ParentFont = False
        Layout = tlCenter
      end
      object tbInput: TEdit
        Left = 152
        Top = 0
        Width = 305
        Height = 21
        TabOrder = 0
      end
      object Panel12: TPanel
        Left = 464
        Top = 0
        Width = 72
        Height = 21
        Align = alRight
        AutoSize = True
        BevelOuter = bvNone
        ParentColor = True
        TabOrder = 1
        object btnBrowseInp: TButton
          Left = 0
          Top = 0
          Width = 72
          Height = 21
          Caption = 'Browse...'
          TabOrder = 0
          OnClick = btnBrowseInpClick
        end
      end
    end
    object Panel7: TPanel
      Left = 0
      Top = 21
      Width = 536
      Height = 21
      Align = alClient
      BevelOuter = bvNone
      ParentColor = True
      TabOrder = 1
      object Label5: TLabel
        Left = 0
        Top = 0
        Width = 153
        Height = 21
        Align = alLeft
        AutoSize = False
        Caption = ' Vocab Filename:'
        Font.Charset = ANSI_CHARSET
        Font.Color = clWindowText
        Font.Height = -11
        Font.Name = 'Verdana'
        Font.Style = [fsBold]
        ParentFont = False
        Layout = tlCenter
      end
      object tbVocab: TEdit
        Left = 152
        Top = 0
        Width = 305
        Height = 21
        TabOrder = 0
      end
      object Panel13: TPanel
        Left = 464
        Top = 0
        Width = 72
        Height = 21
        Align = alRight
        AutoSize = True
        BevelOuter = bvNone
        ParentColor = True
        TabOrder = 1
        object btnBrowseVoc: TButton
          Left = 0
          Top = 0
          Width = 72
          Height = 21
          Caption = 'Browse...'
          TabOrder = 0
          OnClick = btnBrowseVocClick
        end
      end
    end
  end
  object Panel3: TPanel
    Left = 0
    Top = 93
    Width = 536
    Height = 4
    Align = alTop
    BevelOuter = bvNone
    Color = 16775408
    TabOrder = 2
  end
  object Panel4: TPanel
    Left = 0
    Top = 97
    Width = 536
    Height = 21
    Align = alTop
    BevelOuter = bvNone
    ParentColor = True
    TabOrder = 3
    object Label4: TLabel
      Left = 0
      Top = 0
      Width = 153
      Height = 21
      Align = alLeft
      AutoSize = False
      Caption = ' ROM Output Filename:'
      Font.Charset = ANSI_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Verdana'
      Font.Style = [fsBold]
      ParentFont = False
      Layout = tlCenter
    end
    object tbOutput: TEdit
      Left = 152
      Top = 0
      Width = 305
      Height = 21
      TabOrder = 0
      OnChange = tbOutputChange
    end
    object Panel14: TPanel
      Left = 464
      Top = 0
      Width = 72
      Height = 21
      Align = alRight
      AutoSize = True
      BevelOuter = bvNone
      ParentColor = True
      TabOrder = 1
      object btnBrowseOut: TButton
        Left = 0
        Top = 0
        Width = 72
        Height = 21
        Caption = 'Browse...'
        TabOrder = 0
        OnClick = btnBrowseOutClick
      end
    end
  end
  object listbox: TListBox
    Left = 0
    Top = 144
    Width = 536
    Height = 175
    Align = alClient
    ItemHeight = 13
    TabOrder = 4
  end
  object Panel8: TPanel
    Left = 0
    Top = 122
    Width = 536
    Height = 22
    Align = alTop
    BevelOuter = bvNone
    Color = 11829830
    TabOrder = 5
    object Label7: TLabel
      Left = 0
      Top = 0
      Width = 150
      Height = 22
      Align = alLeft
      Caption = ' Games To Inject'
      Font.Charset = ANSI_CHARSET
      Font.Color = clWhite
      Font.Height = -16
      Font.Name = 'Verdana'
      Font.Style = [fsBold]
      ParentFont = False
      Layout = tlCenter
    end
    object Panel10: TPanel
      Left = 345
      Top = 0
      Width = 191
      Height = 22
      Align = alRight
      AutoSize = True
      BevelOuter = bvNone
      ParentColor = True
      TabOrder = 0
      object btnAdd: TButton
        Left = 97
        Top = 0
        Width = 94
        Height = 23
        Caption = 'Add Game'
        TabOrder = 0
        OnClick = btnAddClick
      end
      object btnRemove: TButton
        Left = 0
        Top = 0
        Width = 97
        Height = 23
        Caption = 'Remove Game'
        TabOrder = 1
        OnClick = btnRemoveClick
      end
    end
  end
  object Panel9: TPanel
    Left = 0
    Top = 118
    Width = 536
    Height = 4
    Align = alTop
    BevelOuter = bvNone
    Color = 16775408
    TabOrder = 6
  end
  object FormMain: TPanel
    Left = 0
    Top = 319
    Width = 536
    Height = 25
    Align = alBottom
    BevelOuter = bvNone
    Color = 16775408
    TabOrder = 7
    object txStatus: TLabel
      Left = 0
      Top = 0
      Width = 342
      Height = 25
      Align = alClient
      Alignment = taCenter
      Layout = tlCenter
    end
    object Panel11: TPanel
      Left = 342
      Top = 0
      Width = 194
      Height = 25
      Align = alRight
      AutoSize = True
      BevelOuter = bvNone
      ParentColor = True
      TabOrder = 0
      object btnExit: TButton
        Left = 122
        Top = 0
        Width = 72
        Height = 25
        Caption = 'Exit'
        TabOrder = 0
        OnClick = btnExitClick
      end
      object btnBuild: TButton
        Left = 0
        Top = 0
        Width = 121
        Height = 25
        Caption = 'Build ROM'
        Font.Charset = ANSI_CHARSET
        Font.Color = clWindowText
        Font.Height = -16
        Font.Name = 'Verdana'
        Font.Style = [fsBold]
        ParentFont = False
        TabOrder = 1
        OnClick = btnBuildClick
      end
    end
  end
  object dlgOpenInp: TOpenDialog
    Filter = 'GBAGI.BIN|gbagi.bin'
    Options = [ofHideReadOnly, ofPathMustExist, ofFileMustExist, ofEnableSizing]
    Left = 8
    Top = 8
  end
  object dlgOpenVoc: TOpenDialog
    Filter = 'VOCAB.BIN|vocab.bin'
    Options = [ofHideReadOnly, ofPathMustExist, ofFileMustExist, ofEnableSizing]
    Left = 40
    Top = 8
  end
  object dlgSaveOut: TSaveDialog
    DefaultExt = 'gba'
    Filter = 'GBA ROM (*.gba)|*.gba'
    Options = [ofOverwritePrompt, ofHideReadOnly, ofPathMustExist, ofEnableSizing]
    Left = 72
    Top = 8
  end
end
