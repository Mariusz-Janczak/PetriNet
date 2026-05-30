{ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿}
{³                Wyzsza Szkola Inzynierska w Zielonej Gorze                ³}
{³                    Instytut Informatyki i Elektroniki                    ³}
{³                                                                          ³}
{³                            Praca dyplomowa                               ³}
{³                                                                          ³}
{³                                              napisal: Mariusz Janczak    ³}
{ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ}
{ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿}
{³ Biblioteka pomocnicza do programu PETRI.PAS , zawierajaca procedury :    ³}
{³ - kolorowania sieci oraz makrosieci ;                                    ³}
{ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ}

unit Colored;
interface
  uses
       Crt, Dos, Printer, Objects, Typy, Tools, Automat;

  {--------------------------------------------------------------------------}

  Procedure Coloring (    Branches : PAddress;
                      var Color    : TPalete;
                      var ColNr    : integer);

  Procedure WhichColor (    MColor    : TPalete;
                            ColNr     : integer;
                            Contents  : TContents;
                            LastMacro : integer;
                        var Color     : TPalete);

  Procedure WriteColored (var FileName   : FNameStr;
                              Net        : PTransition;
                              Color      : TPalete;
                              ColNr,What : integer;
                              Reg        : TRegisters;
                              Macro      : boolean);

implementation

{ננננננננננננננננננננננננ Kolorowanie (makro)sieci ננננננננננננננננננננננננננ}

Procedure Coloring (    Branches : PAddress;
                    var Color    : TPalete;
                    var ColNr    : integer);
  var
      I,J       : integer;
      MaxBranch : PBranch;
      PlcAct    : TPlace;
      Plik      : text;
      ForbPlc   : TPalete;

  {==========================================================================}

  Procedure MrkMax (    Branches  : PAddress;
                    var MaxBranch : PBranch;
                    var PlcsNbr   : integer);
    var
        I : byte;

    {------------------------------------------------------------------------}

    Function PlcNr (Coll: TPlace): integer;
      var
          I,DPlcNr : integer;
      begin
        DPlcNr:=0;
        for I:=1 to 255 do
          if I in Coll then inc(DPlcNr);
        PlcNr:=DPlcNr;
      end;

    {------------------------------------------------------------------------}

    begin
      MaxBranch:=Branches^.Addr;
      PlcsNbr:=PlcNr(Branches^.Addr^.MrkPlc);
      Branches:=Branches^.Next;
      while Branches<>nil do
        begin
          if PlcsNbr<PlcNr(Branches^.Addr^.MrkPlc) then
            begin
              PlcsNbr:=PlcNr(Branches^.Addr^.MrkPlc);
              MaxBranch:=Branches^.Addr;
            end;
          Branches:=Branches^.Next;
        end;
    end;

  {==========================================================================}

  Procedure WriteColor (    Branches      : PAddress;
                            J             : integer;
                        var Color,ForbPlc : TPlace);
    var
        DBranches : PAddress;
    begin
      Color:=Color+[J];
      DBranches:=Branches;
      while DBranches<>nil do
        begin
          if J in DBranches^.Addr^.MrkPlc then
            ForbPlc:=ForbPlc+DBranches^.Addr^.MrkPlc-[J];
          DBranches:=DBranches^.Next;
        end;
    end;

  {==========================================================================}

  Procedure Dye (    Branches            : PAddress;
                     Trees               : PTree;
                     ColMin,PlcMin,ColNr : integer;
                 var Color               : TPalete;
                 var aktBranch           : PBranch);
    type
         Rainbow = set of 1..50;
    var
        PlcAct,Colored : TPlace;
        I,J,K,L,M      : integer;
        ColFree        : Rainbow;
        HowMany        : byte;

    {------------------------------------------------------------------------}

    Procedure FreeColors (    Places  : TPlace;
                              ColNr   : integer;
                          var Color   : TPalete;
                          var ColFree : Rainbow);
      var
          I,J : integer;
      begin
        ColFree:=[1..ColNr];
        I:=0;
        repeat
          repeat
            inc(I);
          until (I in Places) or (i=256);
          if I<256 then
            for J:=1 to ColNr do
              if I in Color[J] then ColFree:=ColFree-[J];
        until I=256;
      end;

    {------------------------------------------------------------------------}

    Procedure GetHowMany (    Branches      : PAddress;
                              Color,ForbPlc : TPalete;
                              Place,ColNr   : integer;
                          var HowMany       : byte);
      var
          I,J,DHowMany,NrCol : integer;
      begin
        HowMany:=ColNr;
        DHowMany:=ColNr;
        while (Branches<>nil) and (DHowMany>0) do
          begin
            if Place in Branches^.Addr^.MrkPlc then
              begin
                I:=0;
                DHowMany:=ColNr;
                repeat
                  repeat
                    inc(I);
                  until (I in Branches^.Addr^.MrkPlc) or (I=256);
                  if I<256 then
                    begin
                      NrCol:=0;
                      for J:=1 to ColNr do
                        if I in Color[J] then inc(NrCol);
                      if NrCol=0 then NrCol:=1;
                      DHowMany:=DHowMany-NrCol;
                    end;
                until I=256;
                if HowMany>DHowMany then HowMany:=DHowMany;
              end;
            Branches:=Branches^.Next;
          end;
      end;

    {------------------------------------------------------------------------}

    begin
      while Trees<>nil do
        begin
          Colored:=[];
          for L:=1 to ColNr do Colored:=Colored+Color[L];
          if not (Trees^.Branch^.MrkPlc<=Colored) then
            begin
              FreeColors(Trees^.Branch^.MrkPlc,ColNr,Color,ColFree);
              PlcAct:=Trees^.TranNr^.OutPlc-Colored;
              J:=PlcMin-1;
              for I:=ColMin to ColNr do
                if I in ColFree then
                  begin
                    repeat
                      repeat
                        inc(J);
                      until (J in PlcAct) or (J=256);
                      if (not (J in ForbPlc[I])) and (J<256) then
                        begin
                          ColFree:=ColFree-[I];
                          WriteColor(Branches,J,Color[I],ForbPlc[I]);
                        end;
                      if j in ForbPlc[I] then
                        begin
                          K:=ColMin-1;
                          repeat
                            repeat
                              inc(K);
                            until (K in ColFree) or  (K>ColNr);
                          until (not (J in ForbPlc[K])) or (K>ColNr);
                          if K<=ColNr then
                            begin
                              WriteColor(Branches,J,Color[K],ForbPlc[K]);
                              ColFree:=ColFree-[K];
                            end;
                        end;
                    until (not (J in ForbPlc[I])) or (J=256);
                    if (ColFree<>[]) and (J=256) then
                      begin
                        PlcAct:=Trees^.TranNr^.OutPlc-Colored;
                        J:=0;
                        repeat
                          repeat
                            inc(J);
                          until (J in PlcAct) or (J=256);
                          GetHowMany(Branches,Color,ForbPlc,J,ColNr,HowMany);
                          while (HowMany>0) and (ColFree<>[]) do
                            begin
                              K:=0;
                              repeat
                                repeat
                                  inc(K);
                                until (K in ColFree) or (K>ColNr);
                              until (not (J in ForbPlc[J])) or (J>ColNr);
                              if K<=ColNr then
                                begin
                                  WriteColor(Branches,J,Color[K],ForbPlc[K]);
                                  ColFree:=ColFree-[K];
                                  HowMany:=HowMany-1;
                                end;
                            end;
                          PlcAct:=PlcAct-[J];
                        until ColFree=[];
                      end;
                  end;
              Dye(Branches,Trees^.Branch^.RdyTrn,1,1,ColNr,Color,Trees^.Branch);
            end;
          Trees:=Trees^.NextTree;
        end;
    end;

  {==========================================================================}

  begin
    MrkMax(Branches,MaxBranch,ColNr);
    J:=0;
    PlcAct:=MaxBranch^.MrkPlc;
    for I:=1 to ColNr do
      begin
        repeat
          inc(J);
        until J in PlcAct;
        Color[I]:=[];
        ForbPlc[I]:=[];
        WriteColor(Branches,J,Color[I],ForbPlc[I]);
      end;
    Dye(Branches,MaxBranch^.RdyTrn,1,1,ColNr,Color,MaxBranch);
  end;

{ננננננננננננננננננננננננננננננננננננננננננננננננננננננננננננננננננננננננננננ}

Procedure WhichColor (    MColor    : TPalete;
                          ColNr     : integer;
                          Contents  : TContents;
                          LastMacro : integer;
                      var Color     : TPalete);
  var
      I,J : byte;
  begin
    for I:=1 to ColNr do
      begin
        Color[I]:=MColor[I];
        for J:=101 to LastMacro do
          if J in Color[I] then
            Color[I]:=Color[I]+Contents[J]-[J];
      end;
  end;

{ננננננננננננננננננננננננננננננננננננננננננננננננננננננננננננננננננננננננננננ}

Procedure GetColored (var Plik       : text;
                          Net        : PTransition;
                          Color      : TPalete;
                          ColNr,What : integer;
                      var Place      : TPlace;
                          Macro,aut  : boolean);
  var
      I,J,K   : integer;
      Consent : boolean;
      Ch      : char;
      Start   : boolean;

  {--------------------------------------------------------------------------}

  Procedure WritePlcCl (Places,ster,stersig : TPlace;
                        Color               : TPalete;
                        ColNr               : integer;
                        Inp,prim,Macro      : boolean);
    var
        I,J   : byte;
        Start : boolean;
    begin
      Start:=true;
      for I:=0 to 199 do
        if I in Places then
          begin
            if not Start then write(Plik,' * ')
                         else Start:=false;
            if I>100 then
              if not prim then write(Plik,'@M')
                          else write(Plik,'M');
            if I<=100 then
              if not prim then write(Plik,'@');
            write(Plik,'P',I,'[');
            Start:=true;
            for J:=1 to ColNr do
              if I in Color[J] then
                begin
                  if not Start then write(Plik,',')
                               else Start:=false;
                  write(Plik,J);
                end;
            write(Plik,']');
          end;
      if not (Macro) then
        for I:=0 to 99 do
          if I in ster then
            begin
              if (I in stersig) and (Inp) then write(Plik,' * /X',I)
              else
                if (I in stersig) and not(Inp) then write(Plik,' * RY',I)
                else
                  if (Inp) then write(Plik,' * X',I)
                           else write(Plik,' * SY',I);
            end;
    end;

  {--------------------------------------------------------------------------}

  begin
    if What=1 then
                begin
                  writeln(Plik,' Coloured net.');
                  writeln(Plik,'---------------');
                end
              else
                begin
                  writeln(Plik,' Coloured macronet.');
                  writeln(Plik,'--------------------');
                end;
    writeln(Plik);
    for I:=1 to ColNr do
      begin
        write(Plik,'COLOR: ',I:2,',   PLACE: ');
        J:=1;
        Start:=true;
        while J<200 do
          begin
            if J in Color[I] then
              begin
                if J<=100 then
                  if Start then write(Plik,'P')
                           else write(Plik,',P')
                else
                  if Start then write(Plik,'MP')
                           else write(Plik,',MP');
                Start:=false;
                write(Plik,J);
              end;
            inc(J);
          end;
        writeln(Plik);
      end;
    writeln(Plik);
   {-------------------------------------------------------------------------}
    EssencialPlc(Place,Color,ColNr);
    write(Plik,'ESSENTIAL PLACES: ');
    Start:=true;
    for K:=1 to 200 do
      if K in Place then
        begin
          if K<100 then
            if Start then write(Plik,'P')
                     else write(Plik,',P')
          else
            if Start then write(Plik,'MP')
                     else write(Plik,',MP');
          Start:=false;
          write(Plik,K);
        end;
    writeln(Plik);
    writeln(Plik);
   {-------------------------------------------------------------------------}
    writeln(Plik,'TRANSITIONS');
    while Net<>nil do
      begin
        write(Plik,'T',Net^.Nr,': ');
        WritePlcCl(Net^.InpPlc,Net^.Inp,Net^.InpN,Color,ColNr,true,true,Macro);
        write(Plik,' |- ');
        WritePlcCl(Net^.OutPlc,Net^.Out,Net^.OutR,Color,ColNr,false,false,Macro);
        writeln(Plik,';');
        Net:=Net^.NextTrn;
      end;
    writeln(Plik);
  end;

{============================================================================}

Procedure WriteColored (var FileName   : FNameStr;
                            Net        : PTransition;
                            Color      : TPalete;
                            ColNr,What : integer;
                            Reg        : TRegisters;
                            Macro      : boolean);
  var
      Plik  : text;
      I,J   : byte;
      Start : boolean;
      Place : TPlace;
      Ext   : string;
  begin
    if What=1 then Ext:='COL'
              else Ext:='MCL';
    ChangeName(FileName,Ext);
    assign(Plik,FileName);
    rewrite(Plik);
    GetColored(Plik,Net,Color,ColNr,What,Place,Macro,false);
    if not Macro then
      begin
        writeln(Plik,'OUTPUTS');
        for I:=1 to Reg[0].Plc-1 do
          begin
            write(Plik,'P',Reg[I].Plc,'[');
            Start:=false;
            for J:=1 to ColNr do
              if Reg[I].Plc in Color[J] then
                begin
                  if Start then write(Plik,',');
                  write(Plik,J);
                  Start:=true;
                end;
            writeln(Plik,'] |- Y',Reg[I].Out,';');
          end;
        write(Plik,'P',Reg[I+1].Plc,'[');
        Start:=false;
        for J:=1 to ColNr do
          if Reg[I+1].Plc in Color[J] then
            begin
              if Start then write(Plik,',');
              write(Plik,J);
              Start:=true;
            end;
        writeln(Plik,'] |- Y',Reg[I+1].Out,';');
      end;
    close(Plik);
  end;

{ננננננננננננננננננננננננננננננננננננננננננננננננננננננננננננננננננננננננננננ}

end.