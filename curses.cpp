// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <memory>
#include <string>

using namespace ftxui;

// Extensible content area component - can be replaced with any custom content
Component CreateContentArea() {
  return Renderer([] {
    // Blank content area - replace this with custom content as needed
    return vbox({
               text("Content Area") | center,
               text("(Empty for now)") | dim | center,
               text("(Oooh im also here)") | dim | center,
           }) |
           center | flex;
  });
}

// Create the main application with menu bar and content area
Component CreateApplication() {
  // Menu state (shared via captured variables)
  auto show_file_menu = std::make_shared<bool>(false);
  auto file_menu_selected = std::make_shared<int>(0);

  // File menu options
  auto file_menu_entries = std::make_shared<std::vector<std::string>>(
      std::vector<std::string>{"New", "Open", "Save", "Save As...", "Exit"});

  // File menu button
  auto file_button = Button(
      "File", [show_file_menu] { *show_file_menu = !*show_file_menu; },
      ButtonOption::Ascii());

  // Create the content area (can be extended/replaced)
  auto content_area = CreateContentArea();

  // Main container
  auto container = Container::Vertical({
      file_button,
      content_area,
  });

  // Add renderer with custom event handling
  auto component = CatchEvent(container, [show_file_menu](Event event) {
    // Close menu on Escape
    if (*show_file_menu && event == Event::Escape) {
      *show_file_menu = false;
      return true;
    }

    // Handle Ctrl+C to quit
    if (event == Event::CtrlC) {
      return false;
    }

    // Close menu on clicks outside
    if (*show_file_menu && event.is_mouse()) {
      *show_file_menu = false;
    }

    return false;
  });

  // Wrap with renderer for custom display
  return Renderer(component, [file_button, content_area, show_file_menu] {
    // Render menu bar
    auto menu_bar = hbox({
                        text(" "),
                        file_button->Render(),
                        text(" | "),
                        text("Edit") | dim,
                        text(" | "),
                        text("View") | dim,
                        text(" | "),
                        text("Help") | dim,
                        filler(),
                    }) |
                    bgcolor(Color::Blue);

    // Render file dropdown if visible
    Element file_dropdown = emptyElement();
    if (*show_file_menu) {
      file_dropdown = vbox({
                          text("") | size(HEIGHT, EQUAL, 1),
                          window(text("File"), vbox({
                                                   text("New       Ctrl+N"),
                                                   text("Open      Ctrl+O"),
                                                   text("Save      Ctrl+S"),
                                                   text("Save As..."),
                                                   separator(),
                                                   text("Exit      Ctrl+Q"),
                                               })) |
                              size(WIDTH, EQUAL, 25),
                      }) |
                      clear_under;
    }

    // Main content with border
    auto main_content = content_area->Render() | border | flex;

    // Combine everything
    return vbox({
        menu_bar,
        dbox({
            main_content,
            file_dropdown,
        }) | flex,
    });
  });
}
#include "torrent_reader.h"
int main() {
  auto app = CreateApplication();

  auto screen = ScreenInteractive::Fullscreen();
  screen.Loop(app);

  return EXIT_SUCCESS;
}
