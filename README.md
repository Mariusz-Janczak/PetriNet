# PetriNet

Decomposition and Place Encoding of Petri Net.

PetriNet takes a Petri net that describes a digital control unit (places, input
conditions, registered and combinational outputs) and transforms it for
hardware implementation. It can:

- build the reachability graph of the net and check that the net is live and
  safe;
- build a macronet by merging sequential places into macroplaces;
- color the net, which identifies its state machine components;
- decompose the net into communicating automata, for D or JK flip-flops;
- encode the net places as flip-flop codes.

The program was written in 1993 in Turbo Pascal for DOS as a master's thesis at
the Department of Computer Engineering, Higher College of Engineering in Zielona
Gora (supervisor: Prof. Marcin Adamski). This repository ports it to C++17 for
Linux, with a command line interface in place of the original text-mode user
interface.

## Status

The port is in progress. The original Pascal sources in [org/](org/) are the
reference for its behavior.

| Command | Status |
| --- | --- |
| `help`, `about` | working |
| `reachability-graph` | working, output identical to the original program |
| `macronet`, `reachability-graph-macronet` | in progress |
| `coloring`, `coloring-macronet` | not ported yet |
| `decomposition` | not ported yet |
| `encoding` | not ported yet |

## Building

Requirements: CMake 3.10 or newer and a C++17 compiler (GCC or Clang).

```sh
cmake -S . -B build
cmake --build build
```

The executable is `build/petriNet`.

## Usage

```sh
./build/petriNet -c <command> -i <input file>
```

| Option | Description |
| --- | --- |
| `-i`, `--input <filename>` | net description to process |
| `-c`, `--command <name>` | command to execute |
| `--help` | display help and exit |

Commands:

| Command | Description |
| --- | --- |
| `about` | display information about the program |
| `reachability-graph` | create the reachability graph of the net |
| `coloring` | color the net |
| `macronet` | create the macronet |
| `reachability-graph-macronet` | create the reachability graph of the macronet |
| `coloring-macronet` | color the macronet |
| `decomposition` | decompose the net into automata |
| `encoding` | encode the net places |

Example:

```sh
./build/petriNet -c reachability-graph -i examples/net03.dat
```

This writes the reachability graph to `examples/net03.grf`:

```text
 Reachability graph.
---------------------

╒══════════════════════════════╤═════════════╤═══════════════════════════════╕
│        MARKED PLACES         │ TRANSITIONS │       NEW MARKED PLACES       │
╞══════════════════════════════╪═════════════╪═══════════════════════════════╡
│ 1,4                          │      1      │ 2,5                           │
├──────────────────────────────┼─────────────┼───────────────────────────────┤
│ 2,5                          │      4      │ 2,6                           │
│                              │      2      │ 3,5                           │
├──────────────────────────────┼─────────────┼───────────────────────────────┤
...
```

## Input format

The net is described in a text file:

```text
PLACES P1,P2,P3,P4,P5,P6
INPUTS X1,X2,X3,X4,X5
REG_OUTS Y1,Y2,Y3,Y4
COMB_OUTS Y5
MARKINGS P1,P4

TRANSITIONS
T1: P1 * P4 * X1 |- @P2 * @P5 * SY1 * SY2
T2: P2 * X2 |- @P3 * RY1 * SY3
T3: P3 * X4 |- @P1 * RY3
T4: P5 * X3 |- @P6 * RY2 * SY4
T5: P6 * X5 |- @P4 * RY4

OUTPUTS
P1 * P4 |- Y5
```

| Section | Content |
| --- | --- |
| `PLACES` | places of the net (`P1`…`P100`) |
| `INPUTS` | input signals (`X0`…) |
| `REG_OUTS` | registered outputs, set and reset by transitions |
| `COMB_OUTS` | combinational outputs, active while places are marked |
| `MARKINGS` | initially marked places |
| `TRANSITIONS` | one transition per line: `T<n>: <inputs> \|- <outputs>` |
| `OUTPUTS` | combinational outputs of places: `<places> \|- <outputs>` |

Transition inputs are places (`P1`) and input conditions (`X1`, or `/X1` for a
negated condition), joined with `*`. Transition outputs are the places that
become marked (`@P2`) and the registered outputs that are set (`SY1`) or reset
(`RY1`). The declarations in the header sections must match what the
transitions and outputs use. The `TRANSITIONS` and `OUTPUTS` sections end at
an empty line, and lines starting with `#` are comments.

More examples are in [examples/](examples/) (`net01.dat`…`net03.dat`).

## Output files

Results are written next to the input file, with the extension changed:

| Extension | Content |
| --- | --- |
| `.grf` | reachability graph of the net |
| `.mcn` | macronet structure |
| `.mcg` | reachability graph of the macronet |
| `.col`, `.mcl` | colored net, colored macronet |
| `.dcd`, `.dkj` | net decomposed into automata for D and JK flip-flops |
| `.con`, `.non`, `.ver` | macroplace concurrency, nonconcurrency and verified nonconcurrency lists |
| `.cmc`, `.plc`, `.ftb` | codes of macroplaces, codes of places, full table of codes |
| `.cod` | encoded sequents |

The `PRZ1`…`PRZ3` files in [examples/](examples/) are outputs of the original
DOS program for `net01`…`net03` and serve as the reference for the port. They
use the CP437 code page with CRLF line endings, while the port writes UTF-8.

## License

GNU General Public License v3.0, see [LICENSE](LICENSE).
