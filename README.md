# CppCurses - FTXUI Application

A modern terminal UI application built with [FTXUI](https://github.com/ArthurSonzogni/FTXUI), featuring a clean menu-driven interface with extensible content areas.

## Features

- **Menu Bar** - Blue menu bar with File, Edit, View, and Help menus
- **File Dropdown** - Functional dropdown menu with common file operations
- **Bordered Content Area** - Large central workspace for custom content
- **Keyboard Shortcuts** - Ctrl+C to exit, Escape to close menus
- **Extensible Design** - Modular components for easy customization

## Building

```bash
mkdir -p build
cd build
cmake ..
ninja
```

## Running

```bash
./build/TerminalCPP
```

## Architecture

### Components

- **`CreateContentArea()`** - Returns a component for the main content area. Replace this function to customize what's displayed in the center of the window.

- **`CreateApplication()`** - Builds the complete UI with menu bar, dropdowns, and content area. Returns a composable FTXUI component.

### Extending the Application

#### Custom Content Area

Replace the `CreateContentArea()` function to add your own content:

```cpp
Component CreateContentArea() {
  return Renderer([] {
    return vbox({
      text("My Custom Content") | bold,
      separator(),
      text("Add your UI elements here"),
    }) | center | flex;
  });
}
```

#### Adding Menu Items

Modify the `file_menu_entries` vector in `CreateApplication()`:

```cpp
auto file_menu_entries = std::make_shared<std::vector<std::string>>(
  std::vector<std::string>{"New", "Open", "Save", "Export", "Exit"}
);
```

#### Adding More Menus

Add additional menu buttons to the menu bar:

```cpp
auto edit_button = Button("Edit", [show_edit_menu] { 
  *show_edit_menu = !*show_edit_menu; 
}, ButtonOption::Ascii());
```

Then include it in the menu bar rendering.

## Keyboard Shortcuts

- **Ctrl+C** - Exit application
- **Escape** - Close open dropdown menus
- **Mouse Click** - Navigate menus and interact with UI

## License

MIT License (inherited from FTXUI)

## Dependencies

- FTXUI v6.1.8+ (automatically fetched by CMake)
- C++17 or later
- CMake 3.28+
