# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

"Decomposition and Place Encoding of Petri Net" — a 1993 master's thesis tool (Turbo Pascal, DOS) being ported to C++17 for Linux. The original Pascal sources live in [org/](org/) and are the reference for behavior; [src/](src/) holds the C++ port, which is a work in progress. Comments and identifiers in both are partly Polish (e.g. `Kolorowanie` = coloring, `Dekompozycja` = decomposition, `Kodowanie` = encoding, `Plik` = file, `Ciag` = sequence).

All documentation, code comments and commit messages are written in English. When you touch C++ code that still has Polish comments carried over from the Pascal sources, translate them to English.

## Build and run

```sh
cmake -S . -B build
cmake --build build                 # builds build/petriNet
./build/petriNet -c <command> -i <input.dat>
```

Commands (dispatched in `ptnetCommand` in [src/petri.cpp](src/petri.cpp)): `help`, `about`, `reachability-graph`, `coloring`, `macronet`, `reachability-graph-macronet`, `coloring-macronet`, `decomposition`, `encoding`.

There is no test suite. Verify changes by running a command against [examples/](examples/) input `.dat` files and comparing with the reference outputs from the original DOS program (`PRZ*.GRF`, `.MCN`, `.MCG`, `.COL`, `.MCL`, `.CON`, `.NON`, `.VER`, `.CMC`, `.DCD`, `.DKJ`). Those reference files are CP437 with CRLF line endings, while the port writes UTF-8 with LF, so convert before comparing: `iconv -f cp437 -t utf-8 examples/PRZ1.GRF | tr -d '\r' | diff - examples/net01.grf`. `net01`/`net02` correspond to `PRZ1`/`PRZ2` and `net03` to `PRZ3`; the reachability graphs (`.grf`) currently match exactly. Output files are written next to the input, with the extension swapped via `PTNetFileNameNew` (e.g. `net01.dat` → `net01.grf`).

## Porting state

- Only `tools.cpp`, `graph.cpp` and `petri.cpp` are compiled; the other modules are commented out in `SOURCES`/`HEADERS` in [CMakeLists.txt](CMakeLists.txt). `automat.cpp`, `colored.cpp`, `colored1.cpp` and `encoding.cpp` are still near-literal Pascal translations (`byte`, `boolean`, `nil`, linked lists with `->Next`, `FNameStr`) and do not compile yet. To bring a module online, convert it to the new types and uncomment it there.
- Most of the pipeline in `PetriNet::Coloring/Decomposition/Encoding/CreateMacronet` is disabled with `#if 0` blocks that still use the old free-function signatures; they show the intended call order.
- The order of list elements matters for matching the reference outputs: many Pascal lists were built by prepending (`New^.Next := List; List := New`), so a straight `push_back` port reverses them (see `ReadyTrn` in `graph.cpp`).

## Architecture

- `PetriNet` class ([src/petri.h](src/petri.h)) owns all net state: parsed input sections (`input_data`), transition lists (`net`, `macronet`, `transfers`), registers, reachability branches, markings and macroplace `contents`. Module functions take `PetriNet&` and read or write these members. The older Pascal-style globals (`Peak`, `Concurrency`, `McCodes`, `Color`, `LastMacro`, …) are still defined at the top of `petri.cpp`.
- Each module is a namespace with a `using namespace` at the end of its header: `tools` (input parsing `PTNetReadData`, file naming), `graph` (reachability graph and macronet construction, plus writers), and later coloring, automata decomposition (`automat`) and place encoding (`encoding`).
- [src/types.h](src/types.h) holds both the new types and the legacy Pascal records (`TPeak`, `TConcurrency`, `TCodes`, … as singly linked lists). Porting means migrating code from the second set to the first:
  - `ptnet_places_t` is the Pascal `set of 1..255`, a `std::bitset<256>` wrapper with Pascal operators (`+` union, `-` difference, `*` intersection, `<=` subset, `has()` for `in`).
  - Lists become `std::vector`. The reachability graph (`ptnet_branch_t` nodes with `ptnet_arc_t` arcs) refers to nodes and transitions by index, never by pointer or reference, because the vectors grow while it is built.
  - Pascal `byte` fields are `int`. Arrays with a non-zero lower bound (`TContents [101..199]`, `TPalete [1..50]`, …) are oversized so the original indices work unchanged.
- [src/defs.h](src/defs.h) provides Pascal compatibility shims (`boolean`, `integer`, `nil`).
- Processing pipeline: parse net → build macronet (merging sequential places into macroplaces, which are numbered from 101 and appear as `MPxxx`) → reachability graph of the macronet → coloring → either decomposition into state-machine components (D or JK flip-flop variant) or place encoding (concurrency/nonconcurrency lists → codes of macroplaces → codes of places → encoded sequents).

## Input format (`.dat`)

Sections are `PLACES`, `INPUTS`, `REG_OUTS`, `COMB_OUTS`, `MARKINGS`, `TRANSITIONS`, `OUTPUTS`. A transition reads like `T1: P1 * X0 |- @P2 * @P3 * SY1`: input places and input conditions (`/X` means negated) on the left; output places (`@P`) and set/reset of registered outputs (`SY`/`RY`) on the right. The `OUTPUTS` lines map places to combinational outputs (`P3 |- Y2`).
