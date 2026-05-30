{ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿}
{³                Wyzsza Szkola Inzynierska w Zielonej Gorze                ³}
{³                    Instytut Informatyki i Elektroniki                    ³}
{³                                                                          ³}
{³                            Praca dyplomowa                               ³}
{³                                                                          ³}
{³                                              napisal: Mariusz Janczak    ³}
{ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ}
{ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿}
{³ Biblioteka pomocnicza do programu PETRI.PAS, zawierajaca procedury:      ³}
{³ - odczytu danych o sieci z pliku;                                        ³}
{ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ}

unit Tools;
interface
  uses
       Crt, Dos, Printer, Objects, Typy;

  {--------------------------------------------------------------------------}

  Procedure ReadData (var FileName    : FNameStr;
                      var InData      : TInputData;
                      var Net         : PTransition;
                      var Reg         : TRegisters;
                      var Marking     : TPlace;
                      var ReadErr,Err : boolean);

  {--------------------------------------------------------------------------}

  Procedure ChangeName (var FileName : FNameStr;
                            Ext      : TExtension);

  {--------------------------------------------------------------------------}

  Procedure GetByteStr (What: integer; var ByteStr: string);

implementation

{ðððððððððððððððððððððð Zmiana rozszerzenia pliku ððððððððððððððððððððððððððð}

Procedure ChangeName (var FileName : FNameStr;
                          Ext      : TExtension);
  var
      i,j : byte;
  begin
    i:=pos('.',FileName);
    j:=length(FileName);
    if (i>0) and (i<j) then
      begin
        delete(FileName,i+1,j-i);
        insert(Ext,FileName,i+1)
      end
    else
      insert('.'+Ext,FileName,j)
  end;

{ðððððððððððððððððððððððððððððððððððððððððððððððððððððððððððððððððððððððððððð}

Procedure GetByteStr (What: integer; var ByteStr: string);
  var
      B   : string[1];
      X,I : integer;
  begin
    x:=64;
    ByteStr:='';
    for i:=1 to 7 do
      begin
        str((What div x),B);
        insert(B,ByteStr,1);
        what:=what mod x;
        x:=x div 2;
      end;
  end;

{ðððððððððððððððððððððð Odczyt danych z pliku FileName ðððððððððððððððððððððð}

Procedure ReadData (var FileName    : FNameStr;
                    var InData      : TInputData;
                    var Net         : PTransition;
                    var Reg         : TRegisters;
                    var Marking     : TPlace;
                    var ReadErr,Err : boolean);
  type
       TChar = set of char;

  var
      Plik                         : text;
      Str1                         : TCiag;
      Number,Nr,I                  : integer;
      Ch                           : char;
      Prev,PNew                    : PTransition;
      AllPlc,AllInp,AllReg,AllCom  : TPlace;

  {==========================================================================}

  Procedure ReadStr1 (var Plik : text;
                      var Str1 : TCiag;
                      var Nr   : integer);
    var
        I : integer;
    begin
      repeat
        readln(Plik,Str1);
        for I:=1 to length(Str1) do Str1[I]:=UpCase(Str1[I]);
        Nr:=1;
      until Str1<>'';
    end;

  {==========================================================================}

  Procedure FindChr (    FndChr,AdmChr : TChar;
                     var Plik          : text;
                     var Str1          : TCiag;
                     var Nr            : integer;
                     var Ch            : char;
                     var Err           : boolean);
    begin
      repeat
        Ch:=UpCase(Str1[Nr]);
        inc(Nr);
        if not (Ch in (FndChr+AdmChr+[' ','{'])) then Err:=true;
        if Ch='{' then
          begin
            repeat
              inc(Nr);
            until UpCase(Str1[Nr])='}';
            inc(Nr);
          end;
      until (Ch in FndChr) or Err;
    end;

  {==========================================================================}

  Procedure ReadNum (var Plik      : text;
                     var Str1      : TCiag;
                     var Nr,Number : integer);
    var
        Nr1,ChangeErr : integer;
        Ch            : char;
    begin
      Nr1:=Nr;
      Ch:=Str1[Nr];
      while (Ch in ['0'..'9']) and (Nr1<length(Str1)) do
        begin
          inc(Nr1);
          Ch:=Str1[Nr1];
        end;
      if (Ch in ['0'..'9']) and (Nr1=length(Str1)) then inc(Nr1);
      val(copy(Str1,Nr,Nr1-Nr),Number,ChangeErr);
      Nr:=Nr1;
    end;

  {==========================================================================}

  Procedure CreateTran (    Number : integer;
                        var Prev   : PTransition);
    var
        PNew : PTransition;
    begin
      new(PNew);
      with PNew^ do
        begin
          Nr:=Number;
          InpPlc:=[]; OutPlc:=[]; Inp:=[]; Out:=[]; InpN:=[]; OutR:=[];
          NextTrn:=nil;
        end;
      Prev:=PNew;
    end;

  {==========================================================================}

  Procedure ReadSeq (    Transition : PTransition;
                     var Plik       : text;
                     var Str1       : TCiag;
                     var Nr         : integer;
                     var Ch         : char;
                     var Err        : boolean);

    {------------------------------------------------------------------------}

    Procedure ReadPlc (    EndChr     : TChar;
                       var Transition : PTransition;
                       var Places     : TPlace;
                       var Plik       : text;
                       var Str1       : TCiag;
                       var Nr         : integer;
                       var Ch         : char;
                       var Err        : boolean;
                       Which          : boolean);
      var
          Out : boolean;

      {----------------------------------------------------------------------}

      Function Check (Str1: TCiag; var Nr: integer): boolean;
        var
            N : integer;
        begin
          Check:=false;
          N:=Nr;
          while Str1[N]=' ' do inc(N);
          if ((Str1[N]='X') or
             ((Str1[N+1]='Y') and (Str1[N] in ['S','R'])) or
             ((Str1[N]='/') and (Str1[N+1]='X'))) then Check:=true;
          Nr:=N;
        end;

      {----------------------------------------------------------------------}

      Procedure ReadInOut (var Plik       : text;
                           var Transition : PTransition;
                           var Nr         : integer;
                           var Str1       : TCiag);
        begin
          if Str1[Nr]='X' then
            begin
              inc(Nr);
              ReadNum(Plik,Str1,Nr,Number);
              Transition^.Inp:=Transition^.Inp+[Number];
            end
          else
            if (Str1[Nr]='/') and (Str1[Nr+1]='X') then
              begin
                inc(Nr,2);
                ReadNum(Plik,Str1,Nr,Number);
                Transition^.Inp:=Transition^.Inp+[Number];
                Transition^.InpN:=Transition^.InpN+[Number];
              end
            else
              if (Str1[Nr]='S') and (Str1[Nr+1]='Y') then
                begin
                  inc(Nr,2);
                  ReadNum(Plik,Str1,Nr,Number);
                  Transition^.Out:=Transition^.Out+[Number];
                end
              else
                if (Str1[Nr]='R') and (Str1[Nr+1]='Y') then
                  begin
                    inc(Nr,2);
                    ReadNum(Plik,Str1,Nr,Number);
                    Transition^.Out:=Transition^.Out+[Number];
                    Transition^.OutR:=Transition^.OutR+[Number];
                  end;
        end;

      {----------------------------------------------------------------------}

      begin
        Out:=false;
        while (not (Ch in EndChr)) and not Err and not Out do
          begin
            if (Ch='*') and Check(Str1,Nr) then
              begin
                ReadInOut(Plik,Transition,Nr,Str1);
                Out:=true;
                if EndChr=['|'] then
                  repeat
                    inc(Nr);
                    if (Str1[Nr]='X') or (Str1[Nr]='/') then
                      ReadInOut(Plik,Transition,Nr,Str1);
                  until (Str1[Nr]='-') or (Nr=length(Str1));
                if EndChr=[';','.'] then
                  begin
                    dec(Nr);
                    repeat
                      inc(Nr);
                      if (Str1[Nr]='S') or (Str1[Nr]='R') then
                        ReadInOut(Plik,Transition,Nr,Str1);
                    until (Str1[Nr] in [';','.']) or (Nr=length(Str1));
                    if Str1[Nr]='.' then Ch:='.';
                    inc(Nr);
                  end;
              end
            else
              begin
                repeat
                  if Which then
                    FindChr(['P','A'..'Z'],[],Plik,Str1,Nr,Ch,Err)
                  else
                    FindChr(['@','P','A'..'Z'],[],Plik,Str1,Nr,Ch,Err);
                  if not (Ch in ['@','P']) then
                    FindChr(['*'],['A'..'Z'],Plik,Str1,Nr,Ch,Err);
                until Ch='P';
                while UpCase(Str1[Nr])=' ' do inc(Nr);
                if not (UpCase(Str1[Nr]) in ['0'..'9']) then Err:=true
                else
                  begin
                    ReadNum(Plik,Str1,Nr,Number);
                    Places:=Places+[Number];
                    FindChr(['*']+EndChr,[],Plik,Str1,Nr,Ch,Err);
                  end;
              end;
          end;
      end;

    {------------------------------------------------------------------------}

    begin
      ReadPlc(['|'],Transition,Transition^.InpPlc,Plik,Str1,Nr,Ch,Err,true);
      if UpCase(Str1[Nr])<>'-' then Err:=true
      else
        begin
          inc(Nr);
          Ch:=UpCase(Str1[Nr]);
          ReadPlc([';','.'],Transition,Transition^.OutPlc,Plik,Str1,Nr,Ch,Err,false);
        end;
    end;

  {==========================================================================}

  Procedure ReadDt (var Place  : TPlace;
                        AdmChr : TChar;
                    var Plik   : text;
                        Str2   : TCiag;
                    var Err    : boolean);
    var
        Ready : boolean;
    begin
      Place:=[];
      Reset(Plik);
      Err:=false;
      Ready:=false;
      while (not eof(Plik)) and (not Ready) do
        begin
          readln(Plik,Str1);
          if pos(Str2,Str1)<>0 then
            begin
              delete(Str1,1,length(Str2)+1);
              Place:=[];
              while (length(Str1)>0) and not Err do
                begin
                  if pos(',',Str1)<>0 then
                    begin
                      Str2:=copy(Str1,1,pos(',',Str1)-1);
                      delete(Str1,1,pos(',',Str1));
                    end
                  else
                    begin
                      Str2:=Str1;
                      Str1:='';
                    end;
                  if Str1[1] in AdmChr then
                    begin
                      delete(Str2,1,1);
                      val(Str2,Number,I);
                      if I<>0 then Err:=true
                              else Place:=Place+[Number];
                    end
                  else Err:=true;
                end;
              Ready:=true;
            end;
        end;
      close(Plik);
    end;

  {==========================================================================}

  begin
    ReadErr:=false;
    assign(Plik,FileName);
    {$I-}
      reset(Plik);
    {$I+}
    if IOresult<>0 then ReadErr:=true
    else
      begin
        Err:=false;
        close(plik);
        if not Err then ReadDt(InData.Plc, ['p','P'], Plik, 'PLACE',    Err);
        if not Err then ReadDt(InData.Inp, ['x','X'], Plik, 'INPUT',    Err);
        if not Err then ReadDt(InData.Reg, ['y','Y'], Plik, 'REG_OUT',  Err);
        if not Err then ReadDt(InData.Com, ['y','Y'], Plik, 'COMB_OUT', Err);
        if not Err then ReadDt(Marking,    ['p','P'], Plik, 'MARKING',  Err);
        if not Err then
          begin
  {--------------------------- Odczyt tranzycji -----------------------------}
            reset(Plik);
            repeat
              ReadStr1(Plik,Str1,Nr);
            until (pos('TRANSITION',Str1)<>0) or eof(Plik);
            if not eof(Plik) then
              begin
                ReadStr1(Plik,Str1,Nr);
                FindChr(['t','T'],[],Plik,Str1,Nr,Ch,Err);
                while UpCase(Str1[Nr])=' ' do inc(Nr);
                if UpCase(Str1[Nr]) in ['0'..'9'] then
                  begin
                    ReadNum(Plik,Str1,Nr,Number);
                    CreateTran(Number,Net);
                    FindChr([':'],[],Plik,Str1,Nr,Ch,Err);
                    ReadSeq(Net,Plik,Str1,Nr,Ch,Err);
                    ReadStr1(Plik,Str1,Nr);
                  end
                else
                  Err:=true;
                Prev:=Net;
                while (pos('OUTPUT',Str1)=0) and not Err do
                  begin
                    FindChr(['t','T'],[],Plik,Str1,Nr,Ch,Err);
                    if not Err then
                      begin
                        while UpCase(Str1[Nr])=' ' do inc(Nr);
                        if UpCase(Str1[Nr]) in ['0'..'9'] then
                          begin
                            ReadNum(Plik,Str1,Nr,Number);
                            CreateTran(Number,PNew);
                            FindChr([':'],[],Plik,Str1,Nr,Ch,Err);
                            ReadSeq(PNew,Plik,Str1,Nr,Ch,Err);
                            ReadStr1(Plik,Str1,Nr);
                          end
                        else
                          Err:=true;
                        Prev^.NextTrn:=PNew;
                        Prev:=PNew;
                      end;
                  end;
                Prev:=Net;
  {-------------------------- Odczyt wyjsc ----------------------------------}
                if pos('OUTPUT',Str1)<>0 then readln(Plik,Str1);
                i:=1;
                while (Ch<>'.') and (not(eof(Plik))) and (not Err) do
                  begin
                    Nr:=pos('P',Str1)+1;
                    ReadNum(Plik,Str1,Nr,Number);
                    Reg[i].Plc:=Number;
                    Nr:=pos('Y',Str1)+1;
                    ReadNum(Plik,Str1,Nr,Number);
                    Reg[i].Out:=Number;
                    inc(i);
                    readln(Plik,Str1);
                    Ch:=Str1[Nr];
                  end;
                Nr:=pos('P',Str1)+1;
                ReadNum(Plik,Str1,Nr,Number);
                Reg[i].Plc:=Number;
                Nr:=pos('Y',Str1)+1;
                ReadNum(Plik,Str1,Nr,Number);
                Reg[i].Out:=Number;
                Reg[0].Plc:=i;
  {--------------------------------------------------------------------------}
                AllPlc:=[];
                AllInp:=[];
                AllReg:=[];
                AllCom:=[];
                repeat
                  AllPlc:=AllPlc+Prev^.InpPlc+Prev^.OutPlc;
                  AllInp:=AllInp+Prev^.Inp;
                  AllReg:=AllReg+Prev^.Out;
                  Prev:=Prev^.NextTrn;
                until Prev=nil;
                for i:=1 to Reg[0].Plc do AllCom:=AllCom+[Reg[i].Out];
  {------------------------- Sprawdzenie skladni ----------------------------}
                if AllPlc<>InData.Plc then Err:=true;
                if AllInp<>InData.Inp then Err:=true;
                if AllReg<>InData.Reg then Err:=true;
                if AllCom<>InData.Com then Err:=true;
  {--------------------------------------------------------------------------}
              end
            else
              Err:=true;
            close(Plik);
          end;
      end;
  end;

{ðððððððððððððððððððððððððððððððððððððððððððððððððððððððððððððððððððððððððððð}

end.