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
#include "torrent_toggle.h"
#include "torrent_formatter.h"
#include "file_browser.h"
#include "help_page.h"
#include "multi_viewer.h"
using namespace ftxui;

using std::make_shared;

/// Functions Unimplemented, From, FromList, FromDict, FromString, etc are heavily borrowed from json tui.
Component Unimplemented()
{
  return Renderer([]
                  { return text("Unimplemented"); });
}
Component FakeHorizontal(Component a, Component b)
{
  auto c = Container::Vertical({a, b});
  c->SetActiveChild(b);

  return Renderer(c, [a, b]
                  { return hbox({
                        a->Render(),
                        b->Render(),
                    }); });
}
/**
 * TODO: This structure doesn't fit 1:1 with torrent data, need to adjust.  Works beautifully for JSON tho
 */
Component From(const TorrentValue &tval, bool is_last, int depth, TorrentExpander &expander)
{
  if (tval.isDict())
  {
    return FromDict(Empty(), tval, is_last, depth, expander);
  }
  else if (tval.isList())
  {
    return FromList(Empty(), tval.asList(), is_last, depth, expander);
  }
  else if (tval.isInt())
  {
    return FromNumber(tval, is_last);
  }
  else if (tval.isString())
  {
    return FromString(tval, is_last);
  }
  else
  {
    return Unimplemented();
  }
}

/**
 * @brief Create a list view component
 */
Component FromList(Component prefix, const TorrentList &list, bool is_last, int depth, TorrentExpander &expander)
{
  class Impl : public ComponentExpandable
  {
  public:
    Impl(Component prefix, const TorrentList &list, bool is_last, int depth, TorrentExpander &expander)
        : ComponentExpandable(expander), prefix_(prefix), tlist_(list), is_last_(is_last), depth_(depth)
    {
      Expanded() = (depth <= 0);
      auto children = Container::Vertical({});
      int size = static_cast<int>(list.size());
      for (auto &t : list)
      {
        bool is_children_last = --size == 0;
        auto child_expander = expander_->Child();
        children->Add(Indentation(From(t, is_children_last, depth + 1, child_expander)));
      }
      // TODO: These aren't needed since we're not pretending like it's JSON
      if (is_last)
      {
        children->Add(Renderer([]
                               { return text(" "); }));
      }
      else
      {
        children->Add(Renderer([]
                               { return text(" "); }));
      }
      //"▼ " : "▶ "
      auto toggle = TorrentToggle("▼", is_last ? "▶" : "▶,", &Expanded());
      auto upper = Container::Horizontal({FakeHorizontal(prefix_, toggle)});
      Add(Container::Vertical({upper, Maybe(children, &Expanded())}));
    };
    Component prefix_;
    const TorrentList &tlist_;
    bool is_last_;
    int depth_;
  };
  return Make<Impl>(prefix, list, is_last, depth, expander);
}
/**
 *
 */
Component Basic(std::string value, Color c, bool is_last)
{
  return Renderer([value, c, is_last](bool focused)
                  {
    auto element = paragraph(value) | color(c);
    if (focused)
      element = element | bgcolor(Color::Blue) | color(Color::White) | bold | focus;
    if (!is_last)
      element = hbox({element, text("")});
    return element; });
}
/**
 * @brief Create a dictionary view component
 */
Component FromDict(Component prefix, const TorrentValue &val, bool is_last, int depth, TorrentExpander &expander)
{

  class Impl : public ComponentExpandable
  {
  public:
    Impl(Component prefix, const TorrentValue &dict, bool is_last, int depth, TorrentExpander &expander)
        : ComponentExpandable(expander)
    {
      Expanded() = (depth < 2);
      // Auto-expand first 2 levels
      auto children = Container::Vertical({});
      int size = static_cast<int>(dict.asDict().size());

      for (const auto &[key, value] : dict.asDict())
      {
        bool is_children_last = --size == 0;
        auto prefix = Renderer([key, is_children_last]()
                               {
          auto element = text(key + ": ") | color(Color::Yellow);

          return element; });
        children->Add(prefix);
        auto child_expander = expander_->Child();
        children->Add(Indentation(From(value, is_children_last, depth + 1, child_expander)));
      }
      auto toggle = TorrentToggle("▼", is_last ? "▶" : "▶", &Expanded());
      Add(Container::Vertical({FakeHorizontal(prefix, toggle), Maybe(children, &Expanded())}));
    };
  };
  return Make<Impl>(prefix, val, is_last, depth, expander);
}
Component FromString(const TorrentValue &val, bool is_last)
{
  return Basic(formatValuePreview(val), Color::Green, is_last);
}
Component FromNumber(const TorrentValue &val, bool is_last)
{
  return Basic(formatValuePreview(val), Color::Cyan, is_last);
}

Component Empty()
{
  return Renderer([]
                  { return text(""); });
}

// ---------------------------------------------------------------------------
// Help page modal component (VIM-like keybinding reference)
// ---------------------------------------------------------------------------
Component MakeHelpModal(bool *shown)
{
  HelpPage help;
  auto sections = help.Sections();

  return Renderer([sections, shown](bool /*focused*/)
                  {
    Elements rows;
    rows.push_back(text("  Help — Press ? to close") | bold | center | color(Color::Cyan));
    rows.push_back(separator());
    for (const auto &sec : sections) {
      rows.push_back(text(" " + sec.title) | bold | color(Color::Yellow));
      for (const auto &e : sec.entries) {
        rows.push_back(hbox({
            text("   " + e.key) | color(Color::Green) | size(WIDTH, EQUAL, 26),
            text(e.description) | color(Color::White),
        }));
      }
      rows.push_back(text(""));
    }
    return vbox(rows) | border | bgcolor(Color::Grey15) | center | size(WIDTH, LESS_THAN, 60); });
}

// ---------------------------------------------------------------------------
// File browser modal component
// ---------------------------------------------------------------------------
Component MakeFileBrowserModal(FileBrowser &fb, int *cursor, bool *shown,
                               std::function<void(std::vector<std::string>)> on_open)
{
  auto component = Renderer([&fb, cursor, shown, on_open](bool /*focused*/)
                            {
    Elements rows;
    rows.push_back(text("  File Browser") | bold | center | color(Color::Cyan));
    rows.push_back(text("  " + fb.CurrentDir()) | color(Color::GrayLight));
    rows.push_back(separator());
    rows.push_back(hbox({
      text("  [Backspace] Up  ") | color(Color::Yellow),
      text("[Space] Select  ") | color(Color::Yellow),
      text("[Enter] Open/Confirm  ") | color(Color::Yellow),
      text("[a] All  ") | color(Color::Yellow),
      text("[d] Deselect  ") | color(Color::Yellow),
      text("[q] Close") | color(Color::Yellow),
    }));
    rows.push_back(separator());

    const auto &entries = fb.Entries();
    if (entries.empty()) {
      rows.push_back(text("  (empty directory)") | dim);
    }
    for (int i = 0; i < static_cast<int>(entries.size()); ++i) {
      const auto &e = entries[i];
      std::string icon = e.is_directory ? "📁 " : "📄 ";
      std::string sel_mark = (!e.is_directory && fb.IsSelected(e.full_path)) ? "[x] " : "[ ] ";
      if (e.is_directory) sel_mark = "    ";
      bool is_cursor = (i == *cursor);
      auto line = text(sel_mark + icon + e.name);
      if (is_cursor) line = line | bgcolor(Color::Blue) | color(Color::White) | bold;
      else if (e.is_directory) line = line | color(Color::Cyan);
      else if (fb.IsSelected(e.full_path)) line = line | color(Color::Green);
      rows.push_back(line);
    }

    auto selected_count = fb.Selected().size();
    rows.push_back(separator());
    rows.push_back(text("  " + std::to_string(selected_count) + " file(s) selected") | color(Color::GrayLight));
    return vbox(rows) | border | bgcolor(Color::Grey15) | center | size(WIDTH, LESS_THAN, 80) | yframe; });

  return CatchEvent(component, [&fb, cursor, shown, on_open](Event event)
                    {
    const auto &entries = fb.Entries();
    int count = static_cast<int>(entries.size());
    if (event == Event::ArrowDown || event == Event::Character('j')) {
      if (count > 0) *cursor = (*cursor + 1) % count;
      return true;
    }
    if (event == Event::ArrowUp || event == Event::Character('k')) {
      if (count > 0) *cursor = (*cursor - 1 + count) % count;
      return true;
    }
    if (event == Event::Character(' ')) {
      fb.ToggleSelect(*cursor);
      return true;
    }
    if (event == Event::Return) {
      if (*cursor >= 0 && *cursor < count && entries[*cursor].is_directory) {
        fb.Enter(*cursor);
        *cursor = 0;
      } else {
        // Confirm selection and open selected files
        auto paths = fb.SelectedPaths();
        if (!paths.empty()) {
          *shown = false;
          on_open(paths);
        }
      }
      return true;
    }
    if (event == Event::Backspace) {
      fb.GoUp();
      *cursor = 0;
      return true;
    }
    if (event == Event::Character('a')) {
      fb.SelectAll();
      return true;
    }
    if (event == Event::Character('d')) {
      fb.DeselectAll();
      return true;
    }
    if (event == Event::Character('q') || event == Event::Escape) {
      *shown = false;
      return true;
    }
    return false; });
}

// ---------------------------------------------------------------------------
// Tab bar rendering
// ---------------------------------------------------------------------------
Element RenderTabBar(const MultiViewer &mv)
{
  if (mv.Count() <= 1)
    return text("");
  Elements tabs;
  for (int i = 0; i < mv.Count(); ++i)
  {
    std::string label = " " + std::to_string(i + 1) + ":" + MultiViewer::TabLabel(mv.PathAt(i)) + " ";
    if (i == mv.ActiveIndex())
      tabs.push_back(text(label) | bold | bgcolor(Color::Blue) | color(Color::White));
    else
      tabs.push_back(text(label) | color(Color::GrayLight));
    if (i < mv.Count() - 1)
      tabs.push_back(text("│") | color(Color::Grey50));
  }
  return hbox(tabs) | center;
}

// ---------------------------------------------------------------------------
// Main
// ---------------------------------------------------------------------------
struct TorrentTab
{
  std::unique_ptr<TorrentReader> reader;
  TorrentExpander expander;
  Component component;
};

int main(int argc, const char **argv)
{
  // -- State --
  MultiViewer multi;
  std::vector<std::unique_ptr<TorrentTab>> tabs;
  bool help_shown = false;
  bool browser_shown = false;
  int browser_cursor = 0;
  std::string start_dir = ".";

  // Helper: load a torrent into a new tab
  auto load_torrent = [&](const std::string &path) -> bool
  {
    try
    {
      auto tab = std::make_unique<TorrentTab>();
      tab->reader = std::make_unique<TorrentReader>(path);
      if (!tab->reader->isValidTorrent())
        return false;
      tab->expander = TorrentExpanderImpl::Root();
      tab->component = From(tab->reader->getRoot(), true, 0, tab->expander);
      tabs.push_back(std::move(tab));
      multi.Add(path);
      return true;
    }
    catch (...)
    {
      return false;
    }
  };

  // Load files from command-line arguments
  for (int i = 1; i < argc; ++i)
  {
    load_torrent(argv[i]);
  }

  // If nothing was loaded, show the browser immediately
  if (tabs.empty())
  {
    browser_shown = true;
  }

  FileBrowser fb(start_dir);

  // Build a container that dynamically picks the active tab's component
  auto tab_container = Container::Tab({}, nullptr);
  // We'll populate it below
  // Since FTXUI containers are static, we build a component that
  // delegates to the active tab's component.
  auto active_viewer = Renderer([&](bool /*focused*/)
                                {
    if (tabs.empty())
      return vbox({
        text("No torrent files loaded") | bold | center | color(Color::Yellow),
        text("Press 'o' to open the file browser") | center | color(Color::GrayLight),
      });
    int idx = multi.ActiveIndex();
    if (idx < 0 || idx >= static_cast<int>(tabs.size()))
      return text("Invalid tab") | center;
    return tabs[idx]->component->Render() | yframe | xflex | bgcolor(Color::Grey19); });

  // Wrap so active tab can receive events
  auto active_viewer_interactive = CatchEvent(active_viewer, [&](Event event)
                                              {
    if (tabs.empty()) return false;
    int idx = multi.ActiveIndex();
    if (idx < 0 || idx >= static_cast<int>(tabs.size())) return false;
    return tabs[idx]->component->OnEvent(event); });

  auto app = Renderer(active_viewer_interactive, [&]
                      {
    Elements layout;
    // Tab bar
    layout.push_back(RenderTabBar(multi));
    if (multi.Count() > 0) {
      std::string title = "Torrent Viewer: " + multi.PathAt(multi.ActiveIndex());
      layout.push_back(text(title) | bold | center | color(Color::Cyan));
    } else {
      layout.push_back(text("Torrent Viewer") | bold | center | color(Color::Cyan));
    }
    layout.push_back(separator());
    layout.push_back(text("Press ? for help") | italic | center | color(Color::GrayLight));
    layout.push_back(active_viewer_interactive->Render());
    return vbox(layout) | border | center | bgcolor(Color::Grey23); });

  auto screen = ScreenInteractive::Fullscreen();
  Event previous_event, next_event;

  auto help_component = MakeHelpModal(&help_shown);
  auto file_browser_component = MakeFileBrowserModal(fb, &browser_cursor, &browser_shown,
                                                     [&](std::vector<std::string> paths)
                                                     {
                                                       for (const auto &p : paths)
                                                         load_torrent(p);
                                                       fb.DeselectAll();
                                                     });

  auto wrapped_component = CatchEvent(app, [&](Event event)
                                      {
    previous_event = next_event;
    next_event = event;

    // '?' toggles help
    if (event == Event::Character('?')) {
      help_shown = !help_shown;
      return true;
    }

    // 'o' opens file browser (when help and browser aren't shown)
    if (event == Event::Character('o') && !help_shown && !browser_shown) {
      browser_shown = true;
      browser_cursor = 0;
      return true;
    }

    // Tab navigation: Tab / l = next, Shift-Tab / h = prev
    if (!help_shown && !browser_shown && multi.Count() > 1) {
      if (event == Event::Tab || event == Event::Character('l')) {
        multi.NextTab();
        return true;
      }
      if (event == Event::TabReverse || event == Event::Character('h')) {
        multi.PrevTab();
        return true;
      }
      // Number keys 1-9 to jump to tab
      for (char c = '1'; c <= '9'; ++c) {
        if (event == Event::Character(c)) {
          int idx = c - '1';
          if (idx < multi.Count()) {
            multi.SetActive(idx);
            return true;
          }
        }
      }
    }

    // Vim motions: G = bottom, gg = top
    if (!help_shown && !browser_shown) {
      if (event == Event::Character('G')) {
        while (app->OnEvent(Event::ArrowUp))
          ;
        return true;
      }
      if (previous_event == Event::Character('g') &&
          next_event == Event::Character('g')) {
        while (app->OnEvent(Event::ArrowDown))
          ;
        return true;
      }
    }

    // Allow the user to quit using 'q' or ESC
    if (event == Event::Character('q') || event == Event::Escape) {
      if (help_shown) { help_shown = false; return true; }
      if (browser_shown) { browser_shown = false; return true; }
      screen.ExitLoopClosure()();
      return true;
    }

    // Convert mouse wheel into their corresponding Down/Up events.
    if (!event.is_mouse())
      return false;
    if (event.mouse().button == Mouse::WheelDown) {
      screen.PostEvent(Event::ArrowDown);
      return true;
    }
    if (event.mouse().button == Mouse::WheelUp) {
      screen.PostEvent(Event::ArrowUp);
      return true;
    }
    return false; });

  wrapped_component |= Modal(file_browser_component, &browser_shown);
  wrapped_component |= Modal(help_component, &help_shown);
  screen.Loop(wrapped_component);

  return EXIT_SUCCESS;
}
