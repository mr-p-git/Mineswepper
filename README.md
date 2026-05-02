# GeeseSpotter

GeeseSpotter is a C++ game inspired by Minesweeper. The player chooses a board size, chooses how many geese to hide, and then reveals cells while avoiding hidden geese. Numbered cells show how many geese are nearby, marked cells help track suspected goose locations, and the game is won by revealing all safe cells or correctly marking every goose.

The core game logic is shared between both versions of the project. The terminal version uses typed commands, while the Windows GUI version adds a clickable interface on top of the same board operations.

The project includes two ways to play:

- **GUI version:** a clickable Windows interface with input boxes, mouse controls, scrollbars for large boards, and visual win/loss feedback
- **Terminal version:** the original console version with typed commands and coordinates

## Quick Start

If the executables are already included in the project folder, run one of these from Windows PowerShell or Command Prompt.

For the clickable GUI version:

```powershell
.\geesespotter_gui.exe
```

For the terminal version:

```powershell
.\geesespotter.exe
```

In VS Code, open the Run and Debug panel and choose either `GeeseSpotter GUI` or `GeeseSpotter Console`.

The GUI executable is intended for Windows because it uses the Win32 API.

## Build Instructions

If the executables are missing or the source code has changed, rebuild them with `g++`.

Build the Windows GUI version:

```powershell
g++ geesespotter_gui.cpp geesespotter.cpp -lgdi32 -mwindows -o geesespotter_gui.exe
```

Build the terminal version:

```powershell
g++ geesespotter.cpp geesespotter_lib.cpp -o geesespotter.exe
```

## How To Play

Game setup rules:

- Board width must be between `2` and `60`
- Board height must be between `2` and `60`
- Number of geese must be at least `1`
- Number of geese cannot be greater than `width * height - 1`

Win conditions:

- Reveal every safe cell, or
- Correctly mark every goose

## GUI Controls

Use the input boxes at the top of the window to set:

- `X`: board width
- `Y`: board height
- `Geese`: number of hidden geese

Then click `Start Game`.

Mouse controls:

- Left click reveals a cell
- Right click marks or unmarks a cell
- A blank revealed cell means `0` nearby geese
- Large boards can be navigated with the window scrollbars
- Close the window to quit

## Terminal Controls

The terminal version asks for the board width, board height, and number of geese. After setup, the board is printed in the terminal.

Available actions:

| Action | Description |
| --- | --- |
| `S` | Show/reveal a cell |
| `M` | Mark or unmark a suspected goose cell |
| `R` | Restart the game |
| `Q` | Quit |

Coordinates are zero-based. On a `5 x 4` board, valid `x` values are `0` through `4`, and valid `y` values are `0` through `3`.

Terminal board symbols:

| Symbol | Meaning |
| --- | --- |
| `*` | Hidden cell |
| `M` | Marked cell |
| `0`-`8` | Revealed safe cell showing adjacent geese |
| `9` | Goose cell, shown after a goose is disturbed or the board is revealed |

## File Structure

| File | Purpose |
| --- | --- |
| `geesespotter.cpp` | Core board operations: allocation, neighbor computation, display, reveal, mark, win checking, and cleanup |
| `geesespotter.h` | Function declarations for the core board operations |
| `geesespotter_lib.cpp` | Terminal game loop, user prompts, random goose placement, and typed action handlers |
| `geesespotter_lib.h` | Shared declarations for the terminal version |
| `geesespotter_gui.cpp` | Windows GUI version with clickable cells and visual feedback |
| `geesespotter.exe` | Built terminal executable |
| `geesespotter_gui.exe` | Built GUI executable |
| `.vscode/` | VS Code build and debug configuration for both play modes |

## Technical Highlights

- Uses dynamic memory allocation for boards selected at runtime
- Stores each board cell as a single `char`
- Uses bit masks to track cell value and state:
  - `0x0F` stores the cell value
  - `0x10` stores whether the cell is marked
  - `0x20` stores whether the cell is hidden
- Reuses the same core board logic in both the terminal version and GUI version
- Handles invalid setup values before starting a game
- Prevents marked cells from being revealed and revealed cells from being marked
- Adds a Win32 GUI layer without replacing the original terminal version

## Skills Demonstrated

- C++ fundamentals
- Pointer-based dynamic arrays
- Bit masking and flag management
- Console input/output
- Basic Windows GUI programming
- Game loop design
- Boundary checking in a 2D grid
- Procedural decomposition
- Manual resource cleanup
