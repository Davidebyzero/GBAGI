object FormAddGame: TFormAddGame
  Left = 191
  Top = 160
  BorderStyle = bsDialog
  Caption = 'FormAddGame'
  ClientHeight = 185
  ClientWidth = 425
  Color = 16775408
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Verdana'
  Font.Style = []
  OldCreateOrder = False
  Position = poMainFormCenter
  PixelsPerInch = 96
  TextHeight = 13
  object GroupBox1: TGroupBox
    Left = 0
    Top = 0
    Width = 425
    Height = 66
    Align = alTop
    Caption = 'Select Game'
    TabOrder = 0
    object Label1: TLabel
      Left = 16
      Top = 44
      Width = 33
      Height = 13
      Caption = 'Path:'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Verdana'
      Font.Style = [fsBold]
      ParentFont = False
    end
    object dropGames: TComboBox
      Left = 8
      Top = 16
      Width = 409
      Height = 21
      Style = csDropDownList
      ItemHeight = 13
      TabOrder = 0
      OnChange = dropGamesChange
    end
    object tbPath: TEdit
      Left = 64
      Top = 40
      Width = 353
      Height = 21
      ReadOnly = True
      TabOrder = 1
    end
  end
  object GroupBox2: TGroupBox
    Left = 0
    Top = 66
    Width = 425
    Height = 94
    Align = alClient
    Caption = 'Specify Attributes'
    TabOrder = 1
    object Label2: TLabel
      Left = 8
      Top = 20
      Width = 41
      Height = 13
      Caption = 'Name:'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Verdana'
      Font.Style = [fsBold]
      ParentFont = False
    end
    object Label3: TLabel
      Left = 8
      Top = 44
      Width = 19
      Height = 13
      Caption = 'ID:'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Verdana'
      Font.Style = [fsBold]
      ParentFont = False
    end
    object Label4: TLabel
      Left = 8
      Top = 68
      Width = 53
      Height = 13
      Caption = 'Version:'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Verdana'
      Font.Style = [fsBold]
      ParentFont = False
    end
    object Label5: TLabel
      Left = 112
      Top = 43
      Width = 91
      Height = 13
      Caption = '(version 3 only)'
    end
    object dropVersion: TComboBox
      Left = 64
      Top = 64
      Width = 249
      Height = 21
      Style = csDropDownList
      ItemHeight = 13
      TabOrder = 0
      OnChange = dropVersionChange
    end
    object tbName: TEdit
      Left = 64
      Top = 16
      Width = 353
      Height = 21
      MaxLength = 25
      TabOrder = 1
    end
    object tbId: TEdit
      Left = 64
      Top = 40
      Width = 41
      Height = 21
      MaxLength = 3
      TabOrder = 2
    end
    object btnAutodetect: TButton
      Left = 312
      Top = 62
      Width = 105
      Height = 25
      Caption = 'Auto Detect'
      TabOrder = 3
      OnClick = btnAutodetectClick
    end
  end
  object Panel5: TPanel
    Left = 0
    Top = 160
    Width = 425
    Height = 25
    Align = alBottom
    BevelOuter = bvNone
    ParentColor = True
    TabOrder = 2
    object Panel11: TPanel
      Left = 281
      Top = 0
      Width = 144
      Height = 25
      Align = alRight
      AutoSize = True
      BevelOuter = bvNone
      ParentColor = True
      TabOrder = 0
      object btnOK: TButton
        Left = 72
        Top = 0
        Width = 72
        Height = 25
        Caption = 'OK'
        TabOrder = 0
        OnClick = btnOKClick
      end
      object btnCancel: TButton
        Left = 0
        Top = 0
        Width = 72
        Height = 25
        Caption = 'Cancel'
        TabOrder = 1
        OnClick = btnCancelClick
      end
    end
  end
end
