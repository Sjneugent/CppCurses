// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.
#include "torrent_reader.h"
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <sstream>

#include <memory>
#include <string>
using namespace ftxui;

using std::make_shared;
using std::string;
using std::vector;

// Helper to format TorrentValue as a string preview
string formatValuePreview(const TorrentValue& val) {
  if (val.isInt()) {
    return std::to_string(val.asInt());
  } else if (val.isString()) {
    const auto& str = val.asString();
    // Check if printable
    bool printable = true;
    for (char c : str) {
      if (!isprint(static_cast<unsigned char>(c)) && c != '\n' && c != '\t') {
        printable = false;  
        break;
      }
    }
    if (printable && str.size() < 60) {
      return "\"" + str + "\"";
    } else if (printable) {
      return "\"" + str.substr(0, 57) + "...\"";
    } else {
      return "<binary: " + std::to_string(str.size()) + " bytes>";
    }
  } else if (val.isList()) {
    return "[" + std::to_string(val.asList().size()) + " items]";
  } else if (val.isDict()) {
    return "{" + std::to_string(val.asDict().size()) + " keys}";
  }
  return "";
}

// Recursive function to build tree nodes from TorrentValue
Component CreateTreeNode(const string& key, const TorrentValue& val, int depth = 0);

Component CreateTreeFromDict(const TorrentDict& dict, int depth = 0) {
  if (dict.empty()) {
    return Renderer([] { return text("  (empty)") | dim; });
  }
  
  vector<Component> children;
  for (const auto& [key, value] : dict) {
    children.push_back(CreateTreeNode(key, value, depth + 1));
  }
  
  return Container::Vertical(std::move(children));
}

Component CreateTreeFromList(const TorrentList& list, int depth = 0) {
  if (list.empty()) {
    return Renderer([] { return text("  (empty)") | dim; });
  }
  
  vector<Component> children;
  for (size_t i = 0; i < list.size(); ++i) {
    children.push_back(CreateTreeNode("[" + std::to_string(i) + "]", list[i], depth + 1));
  }
  
  return Container::Vertical(std::move(children));
}

Component CreateTreeNode(const string& key, const TorrentValue& val, int depth) {
  auto expanded = make_shared<bool>(depth < 2); // Auto-expand first 2 levels
  
  if (val.isDict()) {
    auto child_tree = CreateTreeFromDict(val.asDict(), depth);
    
    return Renderer(child_tree, [key, val, expanded, child_tree, depth] {
      string indent(depth * 2, ' ');
      string toggle = *expanded ? "▼ " : "▶ ";
      auto header = text(indent + toggle + key + ": " + formatValuePreview(val)) | bold;
      
      if (*expanded) {
        return vbox({
          header,
          child_tree->Render()
        });
      }
      return header;
    }) | CatchEvent([expanded](Event event) {
      if (event == Event::Return || event == Event::Character(' ')) {
        *expanded = !*expanded;
        return true;
      }
      return false;
    });
  } else if (val.isList()) {
    auto child_tree = CreateTreeFromList(val.asList(), depth);
    
    return Renderer(child_tree, [key, val, expanded, child_tree, depth] {
      string indent(depth * 2, ' ');
      string toggle = *expanded ? "▼ " : "▶ ";
      auto header = text(indent + toggle + key + ": " + formatValuePreview(val)) | bold;
      
      if (*expanded) {
        return vbox({
          header,
          child_tree->Render()
        });
      }
      return header;
    }) | CatchEvent([expanded](Event event) {
      if (event == Event::Return || event == Event::Character(' ')) {
        *expanded = !*expanded;
        return true;
      }
      return false;
    });
  } else {
    // Leaf node (int or string)
    return Renderer([key, val, depth] {
      string indent(depth * 2, ' ');
      return text(indent + "• " + key + ": " + formatValuePreview(val));
    });
  }
}

// Create tree view from torrent data
Component CreateContentArea(const TorrentValue& root_value) {
  if (!root_value.isDict()) {
    return Renderer([] {
      return text("Error: Root is not a dictionary") | color(Color::Red) | center;
    });
  }
  
  auto tree = CreateTreeFromDict(root_value.asDict(), 0);
  
  int selected = 0;
  auto tree_with_selection = Container::Vertical({tree});
  
  return Renderer(tree_with_selection, [tree_with_selection] {
    return vbox({
      text("Torrent Metadata (Space/Enter to expand/collapse)") | bold | underlined,
      separator(),
      tree_with_selection->Render() | vscroll_indicator | frame | flex,
    });
  });
}

// Create the main application with menu bar and content area
Component CreateApplication(const TorrentValue& root_value) {
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
  auto content_area = CreateContentArea(root_value);

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
  TorrentReader tr("/home/backltrack/Tulsa.torrent");
  
  if (!tr.isValidTorrent()) {
    std::cerr << "Error: Invalid torrent file\n";
    return EXIT_FAILURE;
  }
  
  auto root = tr.getRoot();
  auto app = CreateApplication(root);
  
  auto screen = ScreenInteractive::Fullscreen();
  screen.Loop(app);

  return EXIT_SUCCESS;
}
