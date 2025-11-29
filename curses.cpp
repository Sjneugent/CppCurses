// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.
#include "torrent_reader.h"
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <sstream>
#include <memory>
#include <string>
#include "torrent_expander.h"
using namespace ftxui;

using std::make_shared;

// Helper to format TorrentValue as a std::string preview
std::string formatValuePreview(const TorrentValue &val)
{
  if (val.isInt())
  {
    return std::to_string(val.asInt());
  }
  else if (val.isString())
  {
    const auto &str = val.asString();
    // Check if printable
    bool printable = true;
    for (char c : str)
    {
      if (!isprint(static_cast<unsigned char>(c)) && c != '\n' && c != '\t')
      {
        printable = false;
        break;
      }
    }
    if (printable && str.size() < 60)
    {
      return "\"" + str + "\"";
    }
    else if (printable)
    {
      return "\"" + str.substr(0, 57) + "...\"";
    }
    else
    {
      return "<binary: " + std::to_string(str.size()) + " bytes>";
    }
  }
  else if (val.isList())
  {
    return "[" + std::to_string(val.asList().size()) + " items]";
  }
  else if (val.isDict())
  {
    return "{" + std::to_string(val.asDict().size()) + " keys}";
  }
  return "";
}

// Recursive function to build tree nodes from TorrentValue
Component CreateTreeNode(const std::string &key, const TorrentValue &val,
                         int depth, TorrentExpander& expander);

Component CreateTreeFromDict(const TorrentDict &dict, int depth , TorrentExpander& expander)
{
  // class Impl: public TorrentExpander {
  //   public: 
  //   Impl(Component prefix, const TorrentDict & dict, bool is_last, int depth, TorrentExpander& expander): TorrentExpander(expander) {
  //       Expanded() = (depth < 2); // Auto-expand first 2 levels
  //   }
  // };
   if (dict.empty())
  {
    return Renderer([]
                    { return text("  (empty)") | dim; });
  }

  std::vector<Component> children;
  for (const auto &[key, value] : dict)
  {
    children.push_back(CreateTreeNode(key, value, depth + 1, expander));
  }

  // Wrap children in a vertical container for proper navigation
  return Container::Vertical(std::move(children));
}

Component CreateTreeFromList(const TorrentList &list, int depth , TorrentExpander& expander)
{
  if (list.empty())
  {
    return Renderer([]
                    { return text("  (empty)") | dim; });
  }

  std::vector<Component> children;
  for (size_t i = 0; i < list.size(); ++i)
  {
    children.push_back(
        CreateTreeNode("[" + std::to_string(i) + "]", list[i], depth + 1, expander));
  }

  // Wrap children in a vertical container for proper navigation
  return Container::Vertical(std::move(children));
}

Component CreateTreeNode(const std::string &key, const TorrentValue &val,
                         int depth, TorrentExpander& expander)
{
  auto expanded = make_shared<bool>(depth < 2); // Auto-expand first 2 levels

  if (val.isDict())
  {
    auto child_tree = CreateTreeFromDict(val.asDict(), depth, expander);

    // Create header button that toggles expansion
    auto toggle_button = Button(
        "", [expanded]
        { *expanded = !*expanded; }, ButtonOption::Border());

    // Combine header and children in a vertical container
    auto node_with_children = Container::Vertical({toggle_button, child_tree});

    // Render with conditional display of children
    return Renderer(node_with_children, [key, val, expanded, child_tree, depth,
                                         toggle_button]
                    {
      std::string indent(depth * 2, ' ');
      std::string toggle = *expanded ? "▼ " : "▶ ";
      std::string label = indent + toggle + key + ": " + formatValuePreview(val);

      // Check if toggle button is focused
      bool is_focused = toggle_button->Focused();

      // Create header with cursor indicator and styling
      auto header_element = text((is_focused ? "→ " : "  ") + label) | bold;
      if (is_focused) {
        header_element =
            header_element | bgcolor(Color::Grey23) | color(Color::White);
      }

      if (*expanded) {
        // When expanded, show header and children
        return vbox({header_element, child_tree->Render()});
      } else {
        // When collapsed, only show header
        return header_element;
      } });
  }
  else if (val.isList())
  {
    auto child_tree = CreateTreeFromList(val.asList(), depth, expander);

    auto toggle_button = Button(
        "", [expanded]
        { *expanded = !*expanded; }, ButtonOption::Border());

    auto node_with_children = Container::Vertical({toggle_button, child_tree});

    return Renderer(node_with_children, [key, val, expanded, child_tree, depth,
                                         toggle_button]
                    {
      std::string indent(depth * 2, ' ');
      std::string toggle = *expanded ? "▼ " : "▶ ";
      std::string label = indent + toggle + key + ": " + formatValuePreview(val);

      bool is_focused = toggle_button->Focused();

      auto header_element = text((is_focused ? "→ " : "  ") + label) | bold;
      if (is_focused) {
        header_element =
            header_element | bgcolor(Color::Grey23) | color(Color::White);
      }

      if (*expanded) {
        return vbox({header_element, child_tree->Render()});
      } else {
        return header_element;
      } });
  }
  else
  {
    // Leaf node - simple button with no children
    auto leaf_button = Button(
        "", []() {}, // No action on leaf
        ButtonOption::Border());

    return Renderer(leaf_button, [key, val, depth, leaf_button]
                    {
      std::string indent(depth * 2, ' ');
      std::string label = indent + "• " + key + ": " + formatValuePreview(val);

      bool is_focused = leaf_button->Focused();

      auto content = text((is_focused ? "→ " : "  ") + label) | bold;
      if (is_focused) {
        content = content | bgcolor(Color::Grey23) | color(Color::White);
      }

      return content; });
  }
}

// Create tree view from torrent data
Component CreateContentArea(const TorrentValue &root_value, TorrentExpander& expander)
{
  if (!root_value.isDict())
  {
    return Renderer([]
                    { return text("Error: Root is not a dictionary") | color(Color::Red) |
                             center; });
  }

  auto tree = CreateTreeFromDict(root_value.asDict(), 0, expander);

  // Wrap tree in Container::Vertical to support arrow key navigation
  // The tree should start with the first item focused
  auto tree_container = Container::Vertical({tree});

  // Add hotkey handling for level jumping
  auto tree_with_hotkeys =
      CatchEvent(tree_container, [tree_container](Event event)
                 {
        // Ctrl+U: Jump up one level (multiple up arrow presses)
        if (event == Event::ArrowUpCtrl) {
          // Press up arrow 10 times to jump up a level
          for (int i = 0; i < 10; ++i) {
            tree_container->OnEvent(Event::ArrowUp);
          }
          return true;
        }
        // Ctrl+D: Jump down one level (multiple down arrow presses)
        if (event == Event::ArrowDownCtrl) {
          // Press down arrow 10 times to jump down a level
          for (int i = 0; i < 10; ++i) {
            tree_container->OnEvent(Event::ArrowDown);
          }
          return true;
        }
      if(event == Event::J || event == Event::j || event == Event::ArrowDown ){
         tree_container->OnEvent(Event::ArrowDown);
     //     tree_container->Index();
          auto idx = tree_container->Index();
          ftxui::Component child = tree_container->ChildAt(idx);

          return true;
      }else if(event == Event::K || event == Event::k || event == Event::ArrowUp ){
          tree_container->OnEvent(Event::ArrowUp);
          std::cout << "K hit\n";
          return true;
      }
        std::cout << "Event Hit on tree_with_hotkeys: " << event.DebugString()
                  << std::endl;
        return false; });

  return Renderer(tree_with_hotkeys, [tree_with_hotkeys]
                  { return vbox({
                        text("Torrent Metadata (↑↓ arrows, Ctrl+U/D to jump levels, "
                             "Space/Enter to expand) [ACTIVE]") |
                            bold | underlined | color(Color::Green),
                        separator(),
                        tree_with_hotkeys->Render() | vscroll_indicator | frame | flex,
                    }); });
}

// Create the main application with menu bar and content area
Component CreateApplication(const TorrentValue &root_value, TorrentExpander& expander)
{

  // Create the content area (can be extended/replaced)
  auto content_area = CreateContentArea(root_value, expander);
  // Main container - vertical with menu bar and content
  auto container = Container::Vertical({
      //  menu_bar_container,
      content_area,
  });

  // Add renderer with custom event handling for both menus
  auto component =
      CatchEvent(container, [content_area](Event event)
                 {
        // Close menus on Escape
        if (event == Event::ArrowDownCtrl) {
          return true;
        }
        if (  event == Event::Escape) {
          content_area.get()->TakeFocus();
          return true;
        }

        // Handle Ctrl+C to quit
        if (event == Event::CtrlC) {
          return false;
        } // Close menus on clicks outside
        if ( event.is_mouse()) {
  
        }

        return false; });

  // Wrap with renderer for custom display
  return Renderer(
      component, [content_area,container]
      {
        auto main_content = content_area->Render() | border | flex;

        // Combine everything
        return vbox({
            main_content,
            //      file_dropdown,
        }); });
}

class TorrentComponentExpandable : public ComponentBase
{
public:
  explicit TorrentComponentExpandable(TorrentExpander &expander)
      : expander_(expander) {}

  bool &Expanded() { return expander_->expanded; }
  bool OnEvent(Event event) override
  {
    if(ComponentBase::OnEvent(event))
      return true;
    if(event == Event::Character('+')){
      expander_->Expand();
      return true;
    }
    if(event == Event::Character('-')){
      expander_->Collapse();
      return true;
    }
    return false;
  }

private:
  TorrentExpander &expander_;
};
int main()
{
  TorrentReader tr("/home/backltrack/Tulsa.torrent");
  TorrentExpander expander = TorrentExpanderImpl::Root();
  if (!tr.isValidTorrent())
  {
    std::cerr << "Error: Invalid torrent file\n";
    return EXIT_FAILURE;
  }

  auto root = tr.getRoot();
  auto app = CreateApplication(root, expander);

  auto screen = ScreenInteractive::Fullscreen();
  screen.Loop(app);

  return EXIT_SUCCESS;
}
