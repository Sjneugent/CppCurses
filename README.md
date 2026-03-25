# CppCurses - Terminal Torrent Viewer

A modern terminal-based torrent metadata viewer built with [FTXUI](https://github.com/ArthurSonzogni/FTXUI), featuring an interactive tree navigation interface with vim-like keybindings.

## Features

- **Interactive Tree View**: Navigate torrent file metadata with expandable/collapsible nodes
- **Smart Rendering**: Compact inline display for simple values, expanded view for complex structures
- **Vim-like Navigation**: 
  - `j`/`k` or arrow keys for navigation
  - `gg` to jump to top, `G` to jump to bottom
  - Space/Enter to toggle expansion
- **Styled Interface**: 
  - Bordered, centered layout with gray background
  - Blue highlight for selected items
  - Color-coded values (cyan for numbers, green for strings, yellow for keys)
- **Bencode Parser**: Full support for torrent file format (.torrent)
- **Independent Node Expansion**: Each tree node maintains its own expansion state

## Building

```bash
mkdir -p build
cd build
cmake ..
cmake --build .
```

## Running

```bash
./build/TerminalCPP [path/to/file.torrent]
```

## Testing

The project includes a comprehensive test suite with 40 unit tests covering the core components:

```bash
# Build and run tests
cd build
cmake --build . --target torrent_tests
./tests/torrent_tests

# Or use CTest
ctest --output-on-failure
```

See [tests/README.md](tests/README.md) for detailed information about the test suite.


## Architecture

### Core Components

- **TorrentReader**: Bencode parser for .torrent files, validates structure
- **TorrentExpander**: Per-node expansion state management (based on json-tui)
- **TorrentToggle**: Custom FTXUI component for expandable tree nodes
- **Tree Rendering**: Recursive component generation (`FromDict`, `FromList`, `From`)

### File Structure

- `curses.cpp` - Main application and tree rendering logic
- `torrent_reader.{h,cpp}` - Bencode parser and torrent validation
- `torrent_expander.{h,cpp}` - Expansion state management
- `torrent_toggle.{h,cpp}` - Custom toggle component
- `torrent_formatter.h` - Value formatting utilities

## Keyboard Shortcuts

- **Arrow Keys / j/k** - Navigate up/down through tree
- **Space / Enter** - Toggle expand/collapse on current node
- **gg** - Jump to top of tree
- **G** - Jump to bottom of tree
- **q / Escape** - Quit application
- **Mouse Wheel** - Scroll up/down

## Display Features

- Simple key-value pairs displayed inline: `"key": value`
- Small lists (≤2 primitive items) shown inline: `[item1, item2]`
- Complex nested structures displayed in expandable tree format
- Automatic indentation based on nesting depth
- Color-coded by type for easy readability

## License

MIT License (inherited from FTXUI)

## Dependencies

- FTXUI v6.1.8+ (automatically fetched by CMake)
- C++17 or later
- CMake 3.28+

## Images 
<img width="1076" height="562" alt="Screenshot 2026-03-24 172030" src="https://github.com/user-attachments/assets/08cfeb10-11d4-4323-9c22-5dcff21a82cc" />
<img width="1066" height="555" alt="Screenshot 2026-03-24 172058" src="https://github.com/user-attachments/assets/cb2a2896-b5d4-4f07-99f8-a841859d4df0" />
<img width="1087" height="569" alt="Screenshot 2026-03-24 172115" src="https://github.com/user-attachments/assets/2c070c2d-fda0-4752-90d7-bae2f04d2a42" />
