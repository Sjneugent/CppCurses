// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.
#include "torrent_reader.h"
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

#include <memory>
#include <string>
using namespace ftxui;

using std::make_shared;
using std::string;
using std::vector;

// Extensible content area component - can be replaced with any custom content
Component CreateContentArea() {
  return Renderer([] {
    // Blank content area - replace this with custom content as needed
    return vbox({
               text("Announce Value") | center,
               text("==============") | dim | center,
               text("Misc Values") | dim | center,
           }) |
           center | flex;
  });
}

// Create the main application with menu bar and content area
Component CreateApplication() {
  // Menu state (shared via captured variables)
  auto show_file_menu = make_shared<bool>(false);
  auto file_menu_selected = make_shared<int>(0);
  auto show_view_menu = make_shared<bool>(false);
  auto view_menu_selected = make_shared<int>(0);
  // File menu options
  auto file_menu_entries = make_shared<vector<string>>(
      vector<string>{ /*"New",*/ "Open", /*"Save", "Save As...",*/ "Exit"});

  // File menu button
  auto file_button = Button(
      "File", [show_file_menu] { *show_file_menu = !*show_file_menu; },
      ButtonOption::Ascii());

  auto view_button = Button("View", [show_view_menu] { *show_view_menu = !*show_view_menu; },
                           ButtonOption::Ascii());

  // Create the content area (can be extended/replaced)
  auto content_area = CreateContentArea();

  // Main container
  auto container = Container::Vertical({
      file_button,
      view_button,
      content_area,
  });

  // Add renderer with custom event handling for both menus
  auto component = CatchEvent(container, [show_file_menu, show_view_menu](Event event) {
    // Close menus on Escape
    if ((*show_file_menu || *show_view_menu) && event == Event::Escape) {
      *show_file_menu = false;
      *show_view_menu = false;
      return true;
    }

    // Handle Ctrl+C to quit
    if (event == Event::CtrlC) {
      return false;
    }

    // Close menus on clicks outside
    if ((*show_file_menu || *show_view_menu) && event.is_mouse()) {
      *show_file_menu = false;
      *show_view_menu = false;
    }

    return false;
  });

  // Wrap with renderer for custom display
  return Renderer(component, [file_button, view_button, content_area, show_file_menu, show_view_menu] {
    // Render menu bar
    auto menu_bar = hbox({
                        text(" "),
                        file_button->Render(),
                        text(" | "),
                        text("Edit") | dim,
                        text(" | "),
                        text("")  ,       
                        view_button->Render(),
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
                                                   text("Open      Ctrl+O"),
                                                   text("Save      Ctrl+S"),
                                                   separator(),
                                                   text("Exit      Ctrl+C"),
                                               })) |
                              size(WIDTH, EQUAL, 25),
                      }) |
                      clear_under;
    }

    // Render view dropdown if visible
    Element view_dropdown = emptyElement();
    if (*show_view_menu) {
      view_dropdown = vbox({
                          text("") | size(HEIGHT, EQUAL, 1),
                          window(text("View"), vbox({
                                                   //text("Zoom In"),
                                                   //text("Zoom Out"),
                                                   separator(),
                                                   //text("Reset Zoom"),
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
            view_dropdown
        }) | flex,
    });
  });
}
int main() {
  auto app = CreateApplication();
  TorrentReader tr("/home/backltrack/Tulsa.torrent");
  if(tr.isValidTorrent()) {
      auto root = tr.getRoot();
      std::cout << "Is valid\n";  
      for( const auto&  [key, value] : root.asDict()){
        std::cout << "Key: " << key << ", Value: " << value << "\n";
      }
      // Further processing can be done here
  }
  auto screen = ScreenInteractive::Fullscreen();
 screen.Loop(app);

  return EXIT_SUCCESS;
}
