unit typy;

interface

  type

       TPlace         = set of 1..255;

       PTransition    = ^TTransition;
       TTransition    = record
                          Nr                              : integer;
                          InpPlc,OutPlc,Inp,Out,InpN,OutR : TPlace;
                          NextTrn                         : PTransition
                        end;

       TInputData     = record
                          Plc,Inp,Reg,Com : TPlace;
                        end;

       RRegisters     = record
                          Plc,Out : byte;
                        end;
       TRegisters     = array [0..255] of RRegisters;

       PTree          = ^TTree;

       PBranch        = ^TBranch;
       TBranch        = record
                          MrkPlc : TPlace;
                          RdyTrn : PTree;
                        end;

       TTree          = record
                          NextTree : PTree;
                          TranNr   : PTransition;
                          Branch   : PBranch;
                        end;

       PAddress       = ^TAddress;
       TAddress       = record
                          Addr : PBranch;
                          Next : PAddress;
                        end;

       REssencialPlc  = record
                          Nr  : byte;
                          Rep : boolean;
                        end;
       TEssencialPlc  = array [1..100] of REssencialPlc;

       TContents      = array [101..199] of TPlace;

       TPalete        = array [1..50] of TPlace;

       TCiag          = string [127];

       TName          = string [79];

       TExtension     = string [3];

       RComp          = record
                          Plc    : byte;
                          EssPlc : boolean;
                        end;
       TComp          = array [0..20] of RComp;

       TPlc           = record
                          Plc : byte;
                          D   : TPlace;
                        end;

       PCompAut       = ^TCompAut;
       TCompAut       = record
                          Plc          : TComp;
                          D            : TPlc;
                          CondNr,ColNr : byte;
                          MinFF,MaxFF  : integer;
                          Roz          : boolean;
                          Inp,Reg,Comb : TPlace;
                          TrnNr        : TPlace;
                          Trn          : PTransition;
                          NextAut      : PCompAut;
                        end;

      PContMc         = ^TContMc;
      TContMc         = record
                          McNr   : byte;
                          Price  : byte;
                          ContMc : TPlace;
                          Next   : PContMc;
                        end;

      PPeak           = ^TPeak;
      TPeak           = record
                          PeakCont : TPlace;
                          Next     : PPeak;
                        end;

      PConcurrency    = ^TConcurrency;
      TConcurrency    = record
                          McNr   : byte;
                          ConcMc : TPlace;
                          Next   : PConcurrency
                        end;

      PNonconcurrency = ^TNonconcurrency;
      TNonconcurrency = record
                          McNr   : byte;
                          NoncMc : TPlace;
                          Next   : PNonconcurrency
                        end;

      PVerified       = ^TVerified;
      TVerified       = record
                          IncLev  : byte;
                          UniteNr : byte;
                          UniteMc : TPlace;
                          NoncMc  : TPlace;
                          Next    : PVerified;
                        end;

      PFlipFlop       = ^TFlipFlop;
      TFlipFlop       = record
                          FFNr : byte;
                          Ch   : char;
                          Next : PFlipFlop
                        end;

      PCodes          = ^TCodes;
      TCodes          = record
                          PlcNr   : byte;
                          McNr    : byte;
                          PlcCode : PFlipFlop;
                          Next    : PCodes
                        end;

      PListCodes      = ^TListCodes;
      TListCodes      = record
                          PlcNr  : byte;
                          StrCod : string;
                          Next   : PListCodes;
                        end;

      FNameStr        = string [79];

implementation

end.

