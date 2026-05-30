{⁄ƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒø}
{≥                Wyzsza Szkola Inzynierska w Zielonej Gorze                ≥}
{≥                    Instytut Informatyki i Elektroniki                    ≥}
{≥                                                                          ≥}
{≥                            Praca dyplomowa                               ≥}
{≥                                                                          ≥}
{≥                                              napisal: Mariusz Janczak    ≥}
{¿ƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒŸ}

Program Petri;

{$X+,S-}
{$M 65520,65520,655360}

uses
  Dos,            { Polecenia DOS-owskie.            }
  Printer,        { Polecenia drukarki.              }
  Crt,            { Podstawowe procedury i funkcje   }
                  {   ekranowe, matematyczne, itp.   }
  Objects,        { Zarzadzanie obiektami i grupami. }
  Drivers,        { Zarzadzanie mysza i klawiatura.  }
  Memory,         { Zarzadzanie pamiecia.            }
  Views,          { Obsluga widokow.                 }
  Menus,          { Obsluga menu i linii statusu.    }
  Dialogs,        { Obsluga okien dialogowych.       }
  StdDlgs,        { Standardowe okna dialogowe.      }
  MsgBox,         { Wyswietlanie komunikatow.        }
  App,            { Obsluga okien i widokow.         }
  ColorSel,       { Selekcja kolorow.                }
  Editors,        { Edytor ekranowy.                 }
  Tools,          { Odczyt danych z pliku.           }
  Encoding,       { Kodowanie miejsc sieci.          }
  Automat,        { Dekompozycja sieci na automaty.  }
  Colored,        { Kolorowanie sieci i makrosieci.  }
  Graf,           { Tworzenie grafow znakowan.       }
  Typy,           { Definicje typow programu.        }
  CmHc;           { Stale polecen i kontekstow.      }

const
  HeapSize = 65 * (1024 div 16);

{  Deklaracje typow zegara  }

type

  PClockView = ^TClockView;
  TClockView = object (TView)
                 Refresh: Byte;
                 LastTime: DateTime;
                 TimeStr: string [10];
                 Constructor Init (var Bounds: TRect);
                 Procedure Draw; virtual;
                 Procedure Update; virtual;
                 Function FormatTimeStr (H,M,S: Word): String; virtual;
               end;

  Function LeadingZero (w: Word): String;
    var
        s : String;
    begin
      Str(w:0,s);
      LeadingZero:=Copy('00',1,2-Length(s))+s;
    end;

  Constructor TClockView.Init (var Bounds: TRect);
    begin
      TView.Init(Bounds);
      FillChar(LastTime,SizeOf(LastTime),#$FF);
      TimeStr:='';
      Refresh:=1;
    end;

  Procedure TClockView.Draw;
    var
        B : TDrawBuffer;
        C : Byte;
    begin
      C:=GetColor(2);
      MoveChar(B,' ',C,Size.X);
      MoveStr(B,TimeStr,C);
      WriteLine(0,0,Size.X,1,B);
    end;

  Procedure TClockView.Update;
    var
        h,m,s,hund : word;
    begin
      GetTime(h,m,s,hund);
      if Abs(s-LastTime.sec)>=Refresh then
        begin
          with LastTime do
            begin
              Hour:=h;
              Min :=m;
              Sec :=s;
            end;
          TimeStr:=FormatTimeStr(h,m,s);
          DrawView;
        end;
    end;

  Function TClockView.FormatTimeStr (H,M,S: Word): String;
    begin
      FormatTimeStr:=LeadingZero(h)+':'+LeadingZero(m)+':'+LeadingZero(s);
    end;

{  Deklaracje typow pamieci  }

type

  PHeapView = ^THeapView;
  THeapView = object (TView)
    OldMem: LongInt;
    constructor Init (var Bounds: TRect);
    procedure Draw; virtual;
    procedure Update;
  end;

  Constructor THeapView.Init (var Bounds: TRect);
    begin
      {inherited Init(Bounds);}
      OldMem:=0;
    end;

  Procedure THeapView.Draw;
    var
      S: String;
      B: TDrawBuffer;
      C: Byte;
    begin
      OldMem:=MemAvail;
      Str(OldMem:Size.X,S);
      C:=GetColor(2);
      MoveChar(B,' ',C,Size.X);
      MoveStr(B,S,C);
      WriteLine(0,0,Size.X,1,B);
    end;

  Procedure THeapView.Update;
    begin
      if (OldMem <> MemAvail) then DrawView;
    end;

{  Deklaracje typow edytora  }

type

  PDialogWindow = ^TDialogWindow;
  TDialogWindow = object (TDialog)
    x: int;
  end;

  PMyStatusLine = ^TMyStatusLine;
  TMyStatusLine = object (TStatusLine)
    Function Hint (AHelpCtx: word): string; virtual;
  end;

  PEdytor = ^TEdytor;
  TEdytor = object (TApplication)
    Clock: PClockView;
    Heap: PHeapView;
    FileName: FNameStr;
    Constructor Init;
    Destructor Done; virtual;
    Procedure FileOpen (WildCard: PathStr; var FNameNew: FNameStr);
    Procedure PrintDocument (FNameNew: FNameStr);
    Procedure HandleEvent (var Event: TEvent); virtual;
    Procedure Idle; virtual;
    Procedure InitMenuBar; virtual;
    Procedure InitStatusLine; virtual;
    Procedure OutOfMemory; virtual;
    Procedure About; virtual;
    Function GetPalette: PPalette; virtual;
    Function OpenEditor (var FNameNew: FNameStr; Visible: Boolean): PEditWindow;
  end;

var
    FNameNew                  : FNameStr;
   {--------------------------------------------}
    ClipWindow                : PEditWindow;
    Peak                      : PPeak;
    Net,McNet,Transfer        : PTransition;
    Branches,McBranches       : PAddress;
    Concurrency               : PConcurrency;
    NonConcurrency            : PNonConcurrency;
    Verified                  : PVerified;
    McCodes,PlcCodes          : PCodes;
    ListCodes                 : PListCodes;
   {--------------------------------------------}
    InData                    : TInputData;
    Marking,McMarking,PlcOfMc : TPlace;
    Color,McColor             : TPalete;
    Contents                  : TContents;
    FName1,FName2             : TName;
    Reg                       : TRegisters;
   {--------------------------------------------}
    Death,Finish,Danger,Rst   : boolean;
  {--------------------------------------------}
    LastMacro,ColorsNr,What   : integer;
    HlpL                      : integer;

{===================== Okno dialogowe wyszukiwania tekstu ===================}

Function CreateFindDialog: PDialog;
  var
      D       : PDialog;
      Control : PView;
      Rect    : TRect;
  begin
    Rect.Assign(0,0,38,12);
    D:=New(PDialog,Init(Rect,'Find'));
    with D^ do
      begin
        Options:=Options or ofCentered;

        Rect.Assign(3,3,32,4);
        Control:=New(PInputLine,Init(Rect,80));
        Insert(Control);
        Rect.Assign(2,2,15,3);
        Insert(New(PLabel,Init(Rect,'~T~ext to find',Control)));
        Rect.Assign(32,3,35,4);
        Insert(New(PHistory,Init(Rect,PInputLine(Control),10)));

        Rect.Assign(3,5,35,7);
        Insert(New(PCheckBoxes,Init(Rect,
          NewSItem('~C~ase sensitive',
          NewSItem('~W~hole words only',nil)))));

        Rect.Assign(7,9,17,11);
        Insert(New(PButton,Init(Rect,'O~K~',cmOk,bfDefault)));
        Rect.Assign(21,9,31,11);
        Insert(New(PButton,Init(Rect,'Cancel',cmCancel,bfNormal)));

        SelectNext(False);
      end;
    CreateFindDialog:=D;
  end;

{===================== Okno dialogowe podmiany tekstu =======================}

Function CreateReplaceDialog: PDialog;
  var
      D       : PDialog;
      Control : PView;
      Rect    : TRect;
  begin
    Rect.Assign(0,0,40,16);
    D:=New(PDialog,Init(Rect,'Replace'));
    with D^ do
      begin
        Options:=Options or ofCentered;

        Rect.Assign(3,3,34,4);
        Control:=New(PInputLine,Init(Rect,80));
        Insert(Control);
        Rect.Assign(2,2,15,3);
        Insert(New(PLabel,Init(Rect,'~T~ext to find',Control)));
        Rect.Assign(34,3,37,4);
        Insert(New(PHistory,Init(Rect,PInputLine(Control),10)));

        Rect.Assign(3,6,34,7);
        Control:=New(PInputLine,Init(Rect,80));
        Insert(Control);
        Rect.Assign(2,5,11,6);
        Insert(New(PLabel,Init(Rect,'~N~ew text',Control)));
        Rect.Assign(34,6,37,7);
        Insert(New(PHistory,Init(Rect,PInputLine(Control),11)));

        Rect.Assign(3,8,37,12);
        Insert(New(PCheckBoxes,Init(Rect,
          NewSItem('~C~ase sensitive',
          NewSItem('~W~hole words only',
          NewSItem('~P~rompt on replace',
          NewSItem('~R~eplace all',
          nil)))))));

        Rect.Assign(8,13,18,15);
        Insert(New(PButton,Init(Rect,'O~K~',cmOk,bfDefault)));
        Rect.Assign(22,13,32,15);
        Insert(New(PButton,Init(Rect,'Cancel',cmCancel,bfNormal)));

        SelectNext(False);
      end;
    CreateReplaceDialog:=D;
  end;

{============================================================================}

Function GetMessage (MsgNr: word): string;
  begin
    MsgNr:=MsgNr+HlpL;
    case MsgNr of
      1  : GetMessage:='Brak pamiëci dla wykonania operacji.';
      2  : GetMessage:='Not enough memory for this operation.';
      3  : GetMessage:='BíÜd odczytu pliku %s.';
      4  : GetMessage:='Error reading file %s.';
      5  : GetMessage:='BíÜd zapisu pliku %s.';
      6  : GetMessage:='Error writing file %s.';
      7  : GetMessage:='BíÜd tworzenia pliku %s.';
      8  : GetMessage:='Error creating file %s.';
      9  : GetMessage:='Plik %s zostaí zmodyfikowany. Czy zapisaç zmiany?';
      10 : GetMessage:='%s has been modified. Save changes?';
      11 : GetMessage:='Czy zapisaç plik Untitled?';
      12 : GetMessage:='Save Untitled file?';
      13 : GetMessage:='Zapis pliku jako';
      14 : GetMessage:='Save file as';
      15 : GetMessage:='Nie znalazíem szukanego tekstu.';
      16 : GetMessage:='Search string not found.';
      17 : GetMessage:='Czy podmieniç?';
      18 : GetMessage:='Replace this occurence?';
      19 : GetMessage:='Brak pamiëci dla wykonania tej operacji.';
      20 : GetMessage:='Not enough memory for this operation.';
      21 : GetMessage:='Nie wczytano pliku danych do edytora.';
      22 : GetMessage:='Data file not read to editor.';
      23 : GetMessage:='BíÜd skíadni.';
      24 : GetMessage:='Syntax error.';
      25 : GetMessage:='Sieç nie jest bezpieczna.';
      26 : GetMessage:='The net is not save.';
      27 : GetMessage:='Sieç jest martwa.';
      28 : GetMessage:='The net is not live.';
    end;
  end;

{======================== Informacje pomocnicze menu ========================}

Function TMyStatusLine.Hint (AHelpCtx: word): string;
  begin
    if HlpL=0 then
      case AHelpCtx of
        hcSystem       : hint:='Zmiana konfiguracji programu';
         hcAbout       : hint:='Informacja o programie';
         hcColors      : hint:='Ustawienie kolor¢w dla programu';
         hcLanguage    : hint:='Wyb¢r jëzyka dla podpowiedzi helpa';
        hcFile         : hint:='Operacje na plikach';
         hcNew         : hint:='Otwarcie nowego pliku do edycji';
         hcOpen        : hint:='Otwarcie istniejacego pliku do edycji';
         hcSave        : hint:='Zapisanie pliku w bießÜcym katalogu i pod aktualnÜ nazwÜ';
         hcSaveAs      : hint:='Zapisanie pliku do wybranego katalogu i pod wybranÜ nazwë';
         hcPrintDoc    : hint:='Wydruk pliku z edytora';
         hcChangeDir   : hint:='Zmiana katalogu';
         hcDosShell    : hint:='Wyjûcie do DOS-u';
         hcExit        : hint:='Zako§czenie programu';
        hcEdit         : hint:='Operacje edytora';
         hcUndo        : hint:='Odwoíanie ostatniej operacji';
         hcCut         : hint:='Przeniesienie zaznaczonego bloku do bufora';
         hcCopy        : hint:='Skopiowanie zaznaczonego bloku do bufora';
         hcPaste       : hint:='Wstawienie zaznaczonego bloku z bufora';
         hcClear       : hint:='Skasowanie zaznaczonego bloku';
         hcShowClip    : hint:='Wyûwietlenie zawartoûci bufora';
        hcSearch       : hint:='Przeszukiwanie pliku';
         hcFind        : hint:='Odszukanie zadanego ciÜgu';
         hcReplace     : hint:='Odszukanie zadanego ciÜgu i jego zamiana na zadany';
         hcSearchAgain : hint:='Powt¢rzenie ostatniego poszukiwania';
        hcNets         : hint:='Przeksztaícenia sieci Petriego';
         hcRechGrph    : hint:='Tworzenie grafu znakowa§ sieci Petriego';
         hcColoredNet  : hint:='Kolorowanie sieci Petriego';
         hcCreateMcNet : hint:='Tworzenie makrosieci sieci Petriego';
         hcMcRechGrph  : hint:='Tworzenie grafu znakowa§ makrosieci sieci Petriego';
         hcColoredMc   : hint:='Kolorowanie makrosieci Petriego';
         hcDecomp      : hint:='Dekompozycja sieci na skíadowe automatowe';
         hcEncoding    : hint:='Kodowanie sieci metodÜ heurystycznÜ';
        hcWindow       : hint:='Operacje na oknach';
         hcTile        : hint:='Rozmieszczenie wszystkich okien obok siebie';
         hcCascade     : hint:='Rozmieszczenie wszystkich okien kaskadowo';
         hcSizeMove    : hint:='Zmiana rozmiaru aktywnego okna';
         hcZoom        : hint:='Okreûlenie strefy wyûwietlania aktywnego okna';
         hcNext        : hint:='Przejûcie do nastepnego okna';
         hcPrevious    : hint:='Przejûcie do poprzedniego okna';
         hcClose       : hint:='Zamkniëcie aktywnego okna';
      else
         hint:='';
      end
    else
      case AHelpCtx of
        hcSystem       : hint:='Options commands';
         hcAbout       : hint:='About Petri';
         hcColors      : hint:='Customize IDE colors for windows, menus, etc.';
         hcLanguage    : hint:='Language for help';
        hcFile         : hint:='File management commands';
         hcNew         : hint:='Create a new file in a new Edit window';
         hcOpen        : hint:='Locate and open a file in an Edit window';
         hcSave        : hint:='Save the file in the active Edit window';
         hcSaveAs      : hint:='Save the current file under a different name, directory, or drive';
         hcPrintDoc    : hint:='Print the contents of the active Edit window';
         hcChangeDir   : hint:='Choose a new default directory';
         hcDosShell    : hint:='Temporarily exit to DOS';
         hcExit        : hint:='Exit Petri';
        hcEdit         : hint:='Edit commands';
         hcUndo        : hint:='Undo the previous editor operation';
         hcCut         : hint:='Remove the selected text and put it in the Clipboard';
         hcCopy        : hint:='Copy the selected text into the clipboard';
         hcPaste       : hint:='Insert selected text from Clipboard at the cursor position';
         hcClear       : hint:='Delete the selected text';
         hcShowClip    : hint:='Open the Clipboard window';
        hcSearch       : hint:='Text search commands';
         hcFind        : hint:='Search for text';
         hcReplace     : hint:='Search for text and replace it with new text';
         hcSearchAgain : hint:='Repeat the last Find or Replace command';
        hcNets         : hint:='Petri nets commands';
         hcRechGrph    : hint:='Create reachability graph';
         hcColoredNet  : hint:='Colouring Petri net';
         hcCreateMcNet : hint:='Create a macronet';
         hcMcRechGrph  : hint:='Create a macronet reachability graph';
         hcColoredMc   : hint:='Colouring Petri macronet';
         hcDecomp      : hint:='Decomposition Petri net';
         hcEncoding    : hint:='Encoding Petri net by heuristic method';
        hcWindow       : hint:='Window commands';
         hcTile        : hint:='Arrange windows on desktop by tiling';
         hcCascade     : hint:='Arrange windows on desktop by cascading';
         hcSizeMove    : hint:='Change the size or position of the active window';
         hcZoom        : hint:='Enlarge or restore the size of the active window';
         hcNext        : hint:='Make the next window active';
         hcPrevious    : hint:='Make the previous window active';
         hcClose       : hint:='Close the active window';
      else
        hint:='';
    end;
end;

{========================== Wyswietlanie komunikatow ========================}

Function DoEditDialog (Dialog: Integer; Info: Pointer): Word; far;
  var
      Rect : TRect;
      T    : TPoint;
  begin
    case Dialog of
      edOutOfMemory:
        DoEditDialog:=MessageBox(GetMessage(1),nil,mfError+mfOkButton);
      edReadError:
        DoEditDialog:=MessageBox(GetMessage(3),@Info,mfError+mfOkButton);
      edWriteError:
        DoEditDialog:=MessageBox(GetMessage(5),@Info,mfError+mfOkButton);
      edCreateError:
        DoEditDialog:=MessageBox(GetMessage(7),@Info,mfError+mfOkButton);
      edSaveModify:
        DoEditDialog:=MessageBox(GetMessage(9),@Info,mfInformation+mfYesNoCancel);
      edSaveUntitled:
        DoEditDialog:=MessageBox(GetMessage(11),nil,mfInformation+mfYesNoCancel);
      edSaveAs:
        DoEditDialog:=Application^.ExecuteDialog(New(PFileDialog,Init('*.*',
                                 GetMessage(13),'~N~ame',fdOkButton,101)),Info);
      edFind:
        DoEditDialog:=Application^.ExecuteDialog(CreateFindDialog,Info);
      edSearchFailed:
        DoEditDialog:=MessageBox(GetMessage(15),nil,mfError+mfOkButton);
      edReplace:
        DoEditDialog:=Application^.ExecuteDialog(CreateReplaceDialog,Info);
      edReplacePrompt:
        begin
          Rect.Assign(0,1,40,8);
          Rect.Move((Desktop^.Size.X-Rect.B.X) div 2,0);
          Desktop^.MakeGlobal(Rect.B,T);
          Inc(T.Y);
          if TPoint(Info).Y<=T.Y then Rect.Move(0,Desktop^.Size.Y-Rect.B.Y-2);
          DoEditDialog:=MessageBoxRect(Rect,GetMessage(17),nil,
                                       mfYesNoCancel+mfInformation);
        end;
    end;
  end;

{============================ Inicjacja programu ============================}

Constructor TEdytor.Init;
  var
      Rect : TRect;
      I    : integer;
      St   : PBufStream;
  begin
    MaxHeapSize:=HeapSize;
    {inherited Init;}
    RegisterObjects;
    RegisterViews;
    RegisterMenus;
    RegisterDialogs;
    RegisterApp;
    RegisterEditors;

    GetExtent(Rect);
    Rect.A.X:=Rect.B.X-9;
    Rect.B.Y:=Rect.A.Y+1;
    Clock:=New(PClockView,Init(Rect));
    Insert(Clock);

    GetExtent(Rect);
    Rect.A.X:=Rect.B.X-9;
    Rect.A.Y:=Rect.B.Y-1;
    Heap:=New(PHeapView,Init(Rect));
    Insert(Heap);

    EditorDialog:=DoEditDialog;
    FileName:='';
    ClipWindow:=OpenEditor(FileName,False);
    if ClipWindow<>nil then
      begin
        Clipboard:=ClipWindow^.Editor;
        Clipboard^.CanUndo:=False;
      end;

    DisableCommands([cmSave,cmSaveAs,cmPrintDoc,cmUndo,cmCut,cmCopy,cmPaste,
      cmClear,cmFind,cmReplace,cmSearchAgain,cmTile,cmCascade,cmRechGrph,
      cmColoredNet,cmCreateMcNet,cmMcRechGrph,cmColoredMc,cmDecomp,
      cmEncoding]);

    FileName:='*.*';
    HlpL:=1;
    delay(500);
    About;
  end;

{============================================================================}

Destructor TEdytor.Done;
  begin
    TApplication.Done;
  end;

{========================== Informacja o programie ==========================}

Procedure TEdytor.About;
  var
      DialogWindow : PDialogWindow;
      Rect         : TRect;
  begin
    Rect.Assign(14,5,66,19);
    DialogWindow:=New(PDialogWindow,Init(Rect,'Petri'));
    with DialogWindow^ do
      begin
        Options:=ofCentered;
        Rect.Assign(1,1,51,10);
        Insert(New(PStaticText,Init(Rect,
                 #13+
                 ^C'Department of Computer Engineering'#13+
                 ^C'Higher College of Engineering'#13+
                 ^C'in Zielona G¢ra'#13+
                 #13+
                 ^C'Decomposition and Place Encoding of Petri Net.'#13+
                 ^C'Msc Thesis, Supervisor Prof. M. Adamski.'#13+
                 #13+
                 ^C'(c) Mariusz Ja§czak (1993)')));
        Rect.Assign(21,11,31,13);
        Insert(New(PButton,Init(Rect,'O~K',cmOk,bfDefault)));
      end;
    if ValidView(DialogWindow)<>nil then
      begin
        Desktop^.ExecView(DialogWindow);
        Dispose(DialogWindow,Done);
      end;
  end;

{============================= Otwarcie edytora =============================}

Function TEdytor.OpenEditor (var FNameNew: FNameStr; Visible: Boolean): PEditWindow;
  var
      P    : PView;
      Rect : TRect;
  begin
    DeskTop^.GetExtent(Rect);
    P:=Application^.ValidView(New(PEditWindow,Init(Rect,FNameNew,wnNoNumber)));
    if not Visible then P^.Hide;
    DeskTop^.Insert(P);
    OpenEditor:=PEditWindow(P);
    EnableCommands([cmRechGrph,cmColoredNet,cmCreateMcNet,cmMcRechGrph,
                    cmColoredMc,cmDecomp,cmEncoding]);
  end;

{============================== Otwarcie pliku ==============================}

Procedure TEdytor.FileOpen (WildCard: PathStr; var FNameNew: FNameStr);
  begin
    FileName:=WildCard;
    if ExecuteDialog(New(PFileDialog,Init(WildCard,'Open editor','N~ame',
                     fdOpenButton,100)),@FNameNew) <> cmCancel then
      OpenEditor(FNameNew,True);
  end;

{========================== Drukowanie dokumentu ============================}

Procedure TEdytor.PrintDocument (FNameNew: FNameStr);

    begin
    end;

{============================================================================}

Procedure TEdytor.Idle;

  Function IsTileable (P: PView): Boolean; far;
    begin
      IsTileable:=P^.Options and ofTileable<>0;
    end;

  begin
    {inherited Idle;}
    Clock^.Update;
    Heap^.Update;
    if Desktop^.FirstThat(@IsTileable) <> nil then
      EnableCommands([cmTile,cmCascade])
    else
      DisableCommands([cmTile,cmCascade,cmRechGrph,cmColoredNet,cmCreateMcNet,
                       cmMcRechGrph,cmColoredMc,cmDecomp,cmEncoding]);
  end;

{=================== Wczytanie palety kolorow dla programu ==================}

Function TEdytor.GetPalette: PPalette;
  const
        CNewColor      = CColor      + CHelpColor;
        CNewBlackWhite = CBlackWhite + CHelpBlackWhite;
        CNewMonochrome = CMonochrome + CHelpMonochrome;
        P : array [apColor..apMonochrome] of string [Length(CNewColor)] =
            (CNewColor,CNewBlackWhite,CNewMonochrome);
  begin
    GetPalette:=@P[AppPalette];
  end;

{======================== Realizacja zdarzen menu ===========================}

Procedure TEdytor.HandleEvent (var Event: TEvent);

  {------------------- Otwarcie nowego pliku do edycji ----------------------}

  Procedure FileNew (var FileName: FNameStr);
    begin
      FileName:='';
      OpenEditor(FileName,True);
    end;

  {--------------------------- Zmiana katalogu ------------------------------}

  Procedure ChangeDir;
    var
        D: PChDirDialog;
    begin
      D:=New(PChDirDialog,Init(cdNormal,101));
      ExecuteDialog(D,nil);
    end;

  {--------------------------- Ustawianie kolorow ---------------------------}

  Procedure Colors;
    var
        D : PColorDialog;
    begin
      D:=New(PColorDialog,Init('',
           ColorGroup('Desktop',DesktopColorItems(nil),
           ColorGroup('Menus',MenuColorItems(nil),
           ColorGroup('Dialogs',DialogColorItems(dpGrayDialog,nil),
           ColorGroup('Editor',WindowColorItems(wpBlueWindow,nil),
           nil))))));
      if ExecuteDialog(D,Application^.GetPalette)<>cmCancel then
        begin
          DoneMemory;
          ReDraw;
        end;
    end;

  {------------------------ Wybor wersji jezykowej --------------------------}

  Procedure Language (var HlpL: integer);
    var
        D            : PDialog;
        Control      : PView;
        Rect         : TRect;
        Data         : word;
        St           : PBufStream;
        FCName,CName : string;
    begin
      Rect.Assign(0,0,40,10);
      D:=New(PDialog,Init(Rect,'Language'));
      with D^ do
        begin
          Options:=Options or ofCentered;

          Rect.Assign(3,3,37,5);
          Control:=New(PRadioButtons,Init(Rect,
            NewSItem('~P~olish',
            NewSItem('~E~nglish',
            nil))));
          Insert(Control);

          Rect.Assign(2,2,11,3);
          Insert(New(PLabel,Init(Rect,'~L~anguage',Control)));

          Rect.Assign(8,7,18,9);
          Insert(New(PButton,Init(Rect,'O~K~',cmOk,bfDefault)));
          Rect.Assign(22,7,32,9);
          Insert(New(PButton,Init(Rect,'Cancel',cmCancel,bfNormal)));

          SelectNext(False);
        end;
      if ValidView(D) <> nil then
        begin
          D^.SetData(HlpL);
          Desktop^.ExecView(D);
          D^.GetData(HlpL);
          Dispose(D,Done);
        end;
    end;

  {--------------------- Wyswietlanie zawartosci bufora ---------------------}

  Procedure ShowClip;
    begin
      ClipWindow^.Select;
      ClipWindow^.Show;
    end;

  {------------------------ Wyswietlanie komunikatow ------------------------}

  Procedure ShowMessage (Message: string);
    var
        D       : PDialog;
        Rect    : TRect;
    begin
      Rect.Assign(0,0,40,9);
      D:=New(PDialog,Init(Rect,'Worning'));
      with D^ do
        begin
          Options:=Options or ofCentered;

          Rect.Grow(-1,-1);
          Inc(Rect.A.X,2);
          Inc(Rect.A.Y,1);
          Dec(Rect.B.X,2);
          Dec(Rect.B.Y,3);
          Insert(New(PStaticText,Init(Rect,Message)));

          Rect.Assign(15,6,25,8);
          Insert(New(PButton,Init(Rect,'O~K',cmOk,bfDefault)));
        end;
      if ValidView(D)<>nil then
        begin
          Desktop^.ExecView(D);
          Dispose(D,Done);
        end;
    end;

{-------------- Tworzenie grafu znakowan sieci lub makrosieci ---------------}

Procedure RechGrph (FName: FNameStr; What: integer);
  var
      Err,ReadErr : boolean;
  begin
    Mark(Net);
    ReadData(FName,InData,Net,Reg,Marking,ReadErr,Err);
    if ReadErr then ShowMessage(GetMessage(21))
    else
      if Err then ShowMessage(GetMessage(23))
      else
        begin
          if What=1 then   {- Graf znakowan sieci -}
            begin
              Mark(Branches);
              Mark(Peak);
              CreateGraf(FName,true,1,Net,Marking,Branches,Peak,Death);
              Release(Peak);
              Release(Branches);
              Release(Net);
            end;
          if What=2 then   {- Graf znakowan makrosieci -}
            begin
              Mark(McNet);
              Mark(Transfer);
              CreateMacro(FName,false,Net,Marking,McNet,Transfer,McMarking,
                          PlcOfMc,Contents,LastMacro,Death,Danger);
              Release(Transfer);
              if (not Death) and (not Danger) then
                begin
                  Mark(Branches);
                  Mark(Peak);
                  CreateGraf(FName,true,2,McNet,McMarking,Branches,Peak,Death);
                  Release(Peak);
                  Release(Branches);
                  Release(McNet);
                  Release(Net);
                end
              else
                if Danger then ShowMessage(GetMessage(25));
            end;
          if Death then ShowMessage(GetMessage(27))
                   else OpenEditor(FName,true);
        end;
  end;

{------------------------- Tworzenie makrosieci -----------------------------}

Procedure CreateMc (FName: FNameStr);
  var
      Err,ReadErr : boolean;
  begin
    Mark(Net);
    ReadData(FName,InData,Net,Reg,Marking,ReadErr,Err);
    if ReadErr then ShowMessage(GetMessage(21))
    else
      if Err then ShowMessage(GetMessage(23))
      else
        begin
          Mark(McNet);
          Mark(Transfer);
          CreateMacro(FName,true,Net,Marking,McNet,Transfer,McMarking,
                      PlcOfMc,Contents,LastMacro,Death,Danger);
          Release(Transfer);
          Release(McNet);
          Release(Net);
          if (not Death) and (not Danger) then OpenEditor(FName,True)
          else
            if Death then ShowMessage(GetMessage(27))
                     else ShowMessage(GetMessage(25));
        end;
  end;

{--------------------- Kolorowanie sieci lub makrosieci ---------------------}

Procedure Colored (FName: FNameStr; What: integer);
  var
      Err,ReadErr : boolean;
  begin
    Mark(Net);
    ReadData(FName,InData,Net,Reg,Marking,ReadErr,Err);
    if ReadErr then ShowMessage(GetMessage(21))
    else
      if Err then ShowMessage(GetMessage(23))
      else
        begin
          Death:=false;
          Mark(McNet);
          Mark(Transfer);
          CreateMacro(FName,false,Net,Marking,McNet,Transfer,McMarking,
                      PlcOfMc,Contents,LastMacro,Death,Danger);
          Release(Transfer);
          if (not Death) and (not Danger) then
            begin
              Mark(McBranches);
              Mark(Peak);
              CreateGraf(FName,false,2,McNet,McMarking,McBranches,Peak,Death);
              Release(Peak);
              if Death then ShowMessage(GetMessage(27))
              else
                begin
                  Coloring(McBranches,McColor,ColorsNr);
                  Release(McBranches);
                  if What=1 then   {- Kolorowanie sieci -}
                    begin
                      Release(McNet);
                      WhichColor(McColor,ColorsNr,Contents,LastMacro,Color);
                      WriteColored(FName,Net,Color,ColorsNr,What,Reg,false);
                      Release(Net);
                    end
                  else   {- Kolorowanie makrosieci -}
                    begin
                      WriteColored(FName,McNet,McColor,ColorsNr,What,Reg,true);
                      Release(McNet);
                      Release(Net);
                    end;
                  OpenEditor(FName,true);
                end;
            end
          else
            if Death then ShowMessage(GetMessage(27))
                     else ShowMessage(GetMessage(25));
        end;
  end;

{------------------- Dekompozycja na skladowe automatowe --------------------}

Procedure Decomposition (FName: FNameStr);
  var
      Err,ReadErr : boolean;
      Place       : TPlace;

  Procedure CreateChoseDecompositionDialog (var What: integer);
    var
        D       : PDialog;
        Control : PView;
        Rect    : TRect;
        Data    : word;
    begin
      Rect.Assign(0,0,40,10);
      D:=New(PDialog,Init(Rect,'Decomposition'));
      with D^ do
        begin
          Options:=Options or ofCentered;

          Rect.Assign(3,3,37,5);
          Control:=New(PRadioButtons,Init(Rect,
                     NewSItem('~D~ flip-flop',
                     NewSItem('~J~K flip-flop',
                     nil))));
          Insert(Control);

          Rect.Assign(2,2,20,3);
          Insert(New(PLabel,Init(Rect,'~K~ind of flip-flop',Control)));

          Rect.Assign(8,7,18,9);
          Insert(New(PButton,Init(Rect,'O~K~',cmOk,bfDefault)));
          Rect.Assign(22,7,32,9);
          Insert(New(PButton,Init(Rect,'Cancel',cmCancel,bfNormal)));

          SelectNext(False);
        end;
      if ValidView(D) <> nil then
        begin
          Desktop^.ExecView(D);
          D^.GetData(Data);
          if Data=0 then What:=1
                    else What:=2;
          Dispose(D,Done);
        end;
    end;

  begin
    CreateChoseDecompositionDialog(What);
    Mark(Net);
    ReadData(FName,InData,Net,Reg,Marking,ReadErr,Err);
    if ReadErr then ShowMessage(GetMessage(21))
    else
      if Err then ShowMessage(GetMessage(23))
      else
        begin
          Mark(McNet);
          Mark(Transfer);
          CreateMacro(FName,false,Net,Marking,McNet,Transfer,McMarking,
                      PlcOfMc,Contents,LastMacro,Death,Danger);
          Release(Transfer);
          if (not Death) and (not Danger) then
            begin
              Mark(McBranches);
              Mark(Peak);
              CreateGraf(FName,false,2,McNet,McMarking,McBranches,Peak,Death);
              Release(Peak);
              if not Death then
                begin
                  Coloring(McBranches,McColor,ColorsNr);
                  Release(McBranches);
                  Release(McNet);
                  WhichColor(McColor,ColorsNr,Contents,LastMacro,Color);
                  Automats(FName,InData,Net,Reg,Marking,Color,ColorsNr,What);
                  Release(Net);
                  OpenEditor(FName,true);
                end
              else
                ShowMessage(GetMessage(27));
            end
          else
            if Death then ShowMessage(GetMessage(27))
                     else ShowMessage(GetMessage(25));
        end;
  end;

{-------------------- Kodowanie miejsc sieci Petriego -----------------------}

Procedure Encoding (FName: FNameStr);
  var
      ReadErr,Err,WrResF : boolean;
      ByteStr            : string;

  Procedure CreateChoseEncodingDialog (var What: integer);
    var
        D       : PDialog;
        Control : PView;
        Rect    : TRect;
    begin
      What:=64;
      Rect.Assign(0,0,45,14);
      D:=New(PDialog,Init(Rect,'Encoding'));
      with D^ do
        begin
          Options:=Options or ofCentered;

          Rect.Assign(3,3,42,10);
          Control:=New(PCheckBoxes,Init(Rect,
                     NewSItem('List of macroplace ~c~oncurrency',
                     NewSItem('List of macroplace ~n~onconcurrency',
                     NewSItem('~V~erified list of conconcurrency',
                     NewSItem('Codes of ~m~acroplaces',
                     NewSItem('Codes of ~p~laces',
                     NewSItem('~F~ull table of codes',
                     NewSItem('~E~ncoded sequents',
                     nil)))))))));
          Insert(Control);

          Rect.Assign(2,2,20,3);
          Insert(New(PLabel,Init(Rect,'~C~hoose results',Control)));

          Rect.Assign(11,11,21,13);
          Insert(New(PButton,Init(Rect,'O~K~',cmOk,bfDefault)));
          Rect.Assign(25,11,35,13);
          Insert(New(PButton,Init(Rect,'Cancel',cmCancel,bfNormal)));

          SelectNext(False);
        end;
      if ValidView(D) <> nil then
        begin
          D^.SetData(What);
          Desktop^.ExecView(D);
          D^.GetData(What);
          Dispose(D,Done);
        end;
    end;

  Procedure CreateChangeInitialDialog (var Rst: boolean);
    var
        D       : PDialog;
        Control : PView;
        Rect    : TRect;
        What    : integer;
    begin
      Rst:=true;
      What:=1;
      Rect.Assign(0,0,30,9);
      D:=New(PDialog,Init(Rect,'Encoding'));
      with D^ do
        begin
          Options:=Options or ofCentered;

          Rect.Assign(3,3,27,5);
          Control:=New(PRadioButtons,Init(Rect,
                     NewSItem('~S~et',
                     NewSItem('~R~eset',
                     nil))));
          Insert(Control);

          Rect.Assign(2,2,22,3);
          Insert(New(PLabel,Init(Rect,'~C~hange initial code',Control)));

          Rect.Assign(4,6,14,8);
          Insert(New(PButton,Init(Rect,'O~K~',cmOk,bfDefault)));
          Rect.Assign(16,6,26,8);
          Insert(New(PButton,Init(Rect,'Cancel',cmCancel,bfNormal)));

          SelectNext(False);
        end;
      if ValidView(D) <> nil then
        begin
          D^.SetData(What);
          Desktop^.ExecView(D);
          D^.GetData(What);
          if What=0 then Rst:=false
                    else Rst:=true;
          Dispose(D,Done);
        end;
    end;

  Procedure WriteResultFile (ByteStr: string; Bit: byte; var WrResF: boolean);
    begin
      if ByteStr[Bit]='1' then WrResF:=true
                          else WrResF:=false;
    end;

  begin
    CreateChoseEncodingDialog(What);
    GetByteStr(What,ByteStr);
    if ByteStr<>'0000000' then
      begin
        CreateChangeInitialDialog(Rst);
        Mark(Net);
        ReadData(FName,InData,Net,Reg,Marking,ReadErr,Err);
        if ReadErr then ShowMessage(GetMessage(21))
        else
          if Err then ShowMessage(GetMessage(23))
          else
            begin
              Mark(McNet);
              Mark(Transfer);
              CreateMacro(FName,false,Net,Marking,McNet,Transfer,McMarking,
                          PlcOfMc,Contents,LastMacro,Death,Danger);
              Release(Transfer);
              if (not Death) and (not Danger) then
                begin
                  Mark(Peak);
                  Mark(McBranches);
                  CreateGraf(FName,false,2,McNet,McMarking,McBranches,Peak,Death);
                  Release(McBranches);
                  if not Death then
                    begin
                      Mark(Concurrency);
                      WriteResultFile(ByteStr,1,WrResF);
{poprawiç}            ListOfConcurrency(FName,WrResF,PlcOfMc,Peak,Concurrency);
                      if WrResF then OpenEditor(FName,true);
                   {---------------------------------------------------------}
                      Mark(NonConcurrency);
                      WriteResultFile(ByteStr,2,WrResF);
                      ListOfNonConcurrency(FName,WrResF,Concurrency,PlcOfMc,NonConcurrency);
                      if WrResF then OpenEditor(FName,true);
                   {---------------------------------------------------------}
                      Mark(Verified);
                      WriteResultFile(ByteStr,3,WrResF);
                      VerifiedList(FName,WrResF,NonConcurrency,Verified);
                      if WrResF then OpenEditor(FName,true);
                   {---------------------------------------------------------}
                      Mark(McCodes);
                      WriteResultFile(ByteStr,4,WrResF);
                      CodesOfMcPlaces(FName,WrResF,PlcOfMc,Concurrency,Verified,McCodes);
                      if WrResF then OpenEditor(FName,true);
                   {---------------------------------------------------------}
                      Mark(PlcCodes);
                      Mark(ListCodes);
                      WriteResultFile(ByteStr,5,WrResF);
                      CodesOfPlaces(FName,WrResF,Rst,Concurrency,Peak,Marking,
                                    PlcOfMc,LastMacro,Contents,McCodes,PlcCodes,ListCodes);
                      if WrResF then OpenEditor(FName,true);
                   {---------------------------------------------------------}
                      WriteResultFile(ByteStr,6,WrResF);
                      if WrResF then
                        begin
                          FullTable(FName,PlcCodes);
                          OpenEditor(FName,true);
                        end;
                   {---------------------------------------------------------}
                      WriteResultFile(ByteStr,7,WrResF);
                      if WrResF then
                        begin
                          WriteSeq(FName,Net,Reg,InData,Marking,ListCodes);
                          OpenEditor(FName,true);
                        end;
                      Release(ListCodes);
                      Release(PlcCodes);
                      Release(McCodes);
                      Release(Verified);
                      Release(NonConcurrency);
                      Release(Concurrency);
                      Release(Peak);
                      Release(McNet);
                      Release(Net);
                    end
                  else
                    ShowMessage(GetMessage(27));
                end
              else
                if Death then ShowMessage(GetMessage(27))
                         else ShowMessage(GetMessage(25));
            end;
      end;
  end;

{----------------------------------------------------------------------------}

  begin
    {inherited HandleEvent(Event);}
    case Event.What of
      evCommand : begin
                    case Event.Command of
                      cmAbout        : About;
                      cmColors       : Colors;
                      cmLanguage     : Language(HlpL);
                      cmOpen         : FileOpen('*.DAT',FileName);
                      cmNew          : FileNew(FileName);
                      cmPrintDoc     : PrintDocument(FileName);
                      cmChangeDir    : ChangeDir;
                      cmShowClip     : ShowClip;
                      cmRechGrph     : RechGrph(FileName,1);
                      cmColoredNet   : Colored(FileName,1);
                      cmCreateMcNet  : CreateMc(FileName);
                      cmMcRechGrph   : RechGrph(FileName,2);
                      cmColoredMc    : Colored(FileName,2);
                      cmDecomp       : Decomposition(FileName);
                      cmEncoding     : Encoding(FileName);
                    else
                      Exit;
                    end;
                    ClearEvent(Event);
                  end;
    else
      Exit;
    end;
  end;

{======================== Deklaracja menu glownego ==========================}

Procedure TEdytor.InitMenuBar;
  var
      Rect: TRect;
  begin
    GetExtent(Rect);
    Rect.B.Y:=Rect.A.Y+1;
    MenuBar:=New(PMenuBar,Init(Rect,
      NewMenu(
        NewSubMenu('~'#240'~',hcSystem,NewMenu(
          NewItem('~A~bout','Alt+F10',kbAltF10,cmAbout,hcAbout,
          NewLine(
          NewItem('~C~olours ...','',kbNoKey,cmColors,hcColors,
          NewItem('~L~anguage ...','Alt+F1',kbAltF1,cmLanguage,hcLanguage,
          nil))))),
        NewSubMenu('~F~ile',hcFile,NewMenu(
          NewItem('~N~ew','',kbNoKey,cmNew,hcNew,
          NewItem('~O~pen ...','F3',kbF3,cmOpen,hcOpen,
          NewItem('~S~ave','F2',kbF2,cmSave,hcSave,
          NewItem('Save ~a~s ...','',kbNoKey,cmSaveAs,hcSaveAs,
          NewLine(
          NewItem('~P~rint','',kbNoKey,cmPrintDoc,hcPrintDoc,
          NewItem('~C~hange dir ...','',kbNoKey,cmChangeDir,hcChangeDir,
          NewItem('~D~OS shell','',kbNoKey,cmDosShell,hcDosShell,
          NewItem('E~x~it','Alt+X',kbAltX,cmQuit,hcExit,
          nil)))))))))),
        NewSubMenu('~E~dit',hcEdit,NewMenu(
          NewItem('~U~ndo','Ctrl+BkSp',kbCtrlBack,cmUndo,hcUndo,
          NewLine(
          NewItem('Cu~t~','Shift+Del',kbShiftDel,cmCut,hcCut,
          NewItem('~C~opy','Ctrl+Ins',kbCtrlIns,cmCopy,hcCopy,
          NewItem('~P~aste','Shift+Ins',kbShiftIns,cmPaste,hcPaste,
          NewItem('C~l~ear','Ctrl+Del',kbCtrlDel,cmClear,hcClear,
          NewLine(
          NewItem('~S~how clipboard','',kbNoKey,cmShowClip,hcShowClip,
          nil))))))))),
        NewSubMenu('~S~earch',hcSearch,NewMenu(
          NewItem('~F~ind ...','',kbNoKey,cmFind,hcFind,
          NewItem('~R~eplace ...','',kbNoKey,cmReplace,hcReplace,
          NewItem('~S~earch again','',kbNoKey,cmSearchAgain,hcSearchAgain,
          nil)))),
        NewSubMenu('~P~etri',hcNets,NewMenu(
          NewItem('~R~eachability graph','',kbNoKey,cmRechGrph,hcRechGrph,
          NewItem('~C~oloured net','',kbNoKey,cmColoredNet,hcColoredNet,
          NewLine(
          NewItem('~M~acronet','',kbNoKey,cmCreateMcNet,hcCreateMcNet,
          NewItem('Macronet reachability ~g~raph','',kbNoKey,cmMcRechGrph,hcMcRechGrph,
          NewItem('Co~l~oured macronet','',kbNoKey,cmColoredMc,hcColoredMc,
          NewLine(
          NewItem('~D~ecomposition ...','',kbNoKey,cmDecomp,hcDecomp,
          NewItem('~E~ncoding','',kbNoKey,cmEncoding,hcEncoding,
          nil)))))))))),
        NewSubMenu('~W~indow',hcWindow,NewMenu(
          NewItem('~T~ile','',kbNoKey,cmTile,hcTile,
          NewItem('C~a~scade','',kbNoKey,cmCascade,hcCascade,
          NewLine(
          NewItem('~S~ize/Move','Ctrl+F5',kbCtrlF5,cmResize,hcSizeMove,
          NewItem('~Z~oom','F5',kbF5,cmZoom,hcZoom,
          NewItem('~N~ext','F6',kbF6,cmNext,hcNext,
          NewItem('~P~revious','Shift+F6',kbShiftF6,cmPrev,hcPrevious,
          NewItem('~C~lose','Alt+F3',kbAltF3,cmClose,hcClose,
          nil))))))))),
        nil)))))))));
  end;

{======================== Deklaracja linii statusu ==========================}

Procedure TEdytor.InitStatusLine;
  var
      Rect         : TRect;
      MyStatusLine : PMyStatusLine;
  begin
    GetExtent(Rect);
    Rect.A.Y:=Rect.B.Y-1;
    New(MyStatusLine,Init(Rect,
      NewStatusDef(0,hcSystem-1,
        NewStatusKey('~F2~ Save',kbF2,cmSave,
        NewStatusKey('~F3~ Open',kbF3,cmOpen,
        NewStatusKey('',kbAltF3,cmClose,
        NewStatusKey('~F5~ Zoom',kbF5,cmZoom,
        NewStatusKey('~F6~ Next',kbF6,cmNext,
        NewStatusKey('~F10~ Menu',kbF10,cmMenu,
        NewStatusKey('~Alt+X~ Quit',kbAltX,cmQuit,
        NewStatusKey('',kbCtrlF5,cmResize,
        nil)))))))),
      NewStatusDef(hcSystem,hcFile-1,
        NewStatusKey('',kbNoKey,0,
        nil),
      NewStatusDef(hcFile,hcEdit-1,
        NewStatusKey('File',kbNoKey,0,
        nil),
      NewStatusDef(hcEdit,hcSearch-1,
        NewStatusKey('Edit',kbNoKey,0,
        nil),
      NewStatusDef(hcSearch,hcNets-1,
        NewStatusKey('Search',kbNoKey,0,
        nil),
      NewStatusDef(hcNets,hcWindow-1,
        NewStatusKey('Petri',kbNoKey,0,
        nil),
      NewStatusDef(hcWindow,hcClose,
        NewStatusKey('Window',kbNoKey,0,
        nil),
      nil)))))))));
    StatusLine:=MyStatusLine;
  end;

{========================= Komunikat braku pamieci ==========================}

Procedure TEdytor.OutOfMemory;
  begin
    MessageBox(GetMessage(19),nil,mfError+mfOkButton);
  end;

{ Program glowny }

var
    EditorApp : TEdytor;

begin
  EditorApp.Init;
  EditorApp.Run;
  EditorApp.Done;
end.