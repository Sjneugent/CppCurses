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
using namespace ftxui;

using std::make_shared;
Component Unimplemented() {
  return Renderer([] { return text("Unimplemented"); });
}
Component FakeHorizontal(Component a, Component b) {
  auto c = Container::Vertical({a, b});
  c->SetActiveChild(b);

  return Renderer(c, [a, b] {
    return hbox({
        a->Render(),
        b->Render(),
    });
  });
}

Component From(const TorrentValue & tval, bool is_last, int depth, TorrentExpander & expander)
{
  std::cout << "From called: isDict=" << tval.isDict() << ", isList=" << tval.isList() <<  ", isString=" << tval.isString() << "\n";
  if(tval.isDict())
  {
    
    return FromDict(Empty(), tval, is_last, depth, expander);
  }
  else if(tval.isList())
  {
    return FromList(Empty(), tval.asList(), is_last, depth, expander);
  }else if(tval.isInt())
  {
    std::cout << "From number called\n";
    return FromNumber(tval, is_last);
  }else if(tval.isString())
  {
    std::cout << "From string called\n";
    return FromString(tval, is_last);
  } 
  else
  {
    std::cout << "Not list or dict\n";
    return Unimplemented();
  }
}


Component FromList(Component prefix, const TorrentList & list, bool is_last, int depth, TorrentExpander & expander)
{
  class Impl : public ComponentExpandable
  {
  public:
    Impl(Component prefix, const TorrentList & list, bool is_last, int depth, TorrentExpander & expander)
        : ComponentExpandable(expander),prefix_(prefix),tlist_(list),is_last_(is_last),depth_(depth)
    {
      Expanded() = (depth <= 0);
       // Auto-expand first 2 levels
      auto children = Container::Vertical({});
      int size = static_cast<int>(list.size());
      for(auto & t: list){
        bool is_children_last = --size == 0;
        children->Add(Indentation(From(t, is_children_last, depth + 1, expander_)));
      }

      if(is_last)
      {
        children->Add(Renderer([] { return text(" ]"); })); 
      }else {
        children->Add(Renderer([] { return text("], "); })); 
      }
      auto toggle = TorrentToggle("[", is_last ? "[...]" : "[...],", &Expanded());
      auto upper = Container::Horizontal({FakeHorizontal(prefix_, toggle)});
      Add(Container::Vertical({upper, Maybe(children, &Expanded())}));
      // for (size_t i = 0; i < list.size(); ++i)
      // {
      //   bool is_children_last = --size == 0;
      //   children->Add(Indentation(From(list[i], is_children_last, depth + 1, expander_)));
      // }

  };
    Component prefix_;
    const TorrentList& tlist_;
    bool is_last_;
    int depth_;
};
        return Make<Impl>(prefix, list, is_last, depth, expander);

}
Component Basic(std::string value, Color c, bool is_last) {
  return Renderer([value, c, is_last](bool focused) {
    auto element = paragraph(value) | color(c);
    if (focused)
      element = element | inverted | focus;
    if (!is_last)
      element = hbox({element, text(",")});
    return element;
  });
}

Component FromDict(Component prefix, const TorrentValue & val, bool is_last, int depth, TorrentExpander & expander)
{
  
  class Impl : public ComponentExpandable
  {
  public:
    Impl(Component prefix, const TorrentValue & dict, bool is_last, int depth, TorrentExpander & expander)
        : ComponentExpandable(expander)
    {
      Expanded() = (depth < 2);
       // Auto-expand first 2 levels
      auto children = Container::Vertical({});
      int size = static_cast<int>(dict.asDict().size());
      for (const auto &[key, value] : dict.asDict())
      {
        bool is_children_last = --size == 0;
        auto prefix = Renderer([key, is_children_last]() {
          auto element = text("\"" + key + "\": ") | color(Color::Yellow);
          if (!is_children_last)
            element = hbox({element, text(" ")});
          return element;
        });
        // children->Add(Indentation(From(key, is_children_last, depth, expander_)));
        children->Add(prefix);
        children->Add(Indentation(From(value, is_children_last, depth + 1, expander_)));
      }

      if(is_last){
        children->Add(Renderer([] { return text(" {"); }));

      }else {
        children->Add(Renderer([] { return text("} "); }));
      }

      auto toggle = TorrentToggle("{", is_last ? "{...}": "{...},", &Expanded());
      Add(Container::Vertical({   FakeHorizontal(prefix, toggle),Maybe(children, &Expanded())}));
  };

}; 
        return Make<Impl>(prefix, val, is_last, depth, expander);

}
Component FromString(const TorrentValue & val, bool is_last)
{
  return Basic("\"" + val.asString() + "\"", Color::Green, is_last);
}
Component FromNumber(const TorrentValue & val, bool is_last)
{
  return Basic( std::to_string(val.asInt()), Color::Cyan, is_last);
}

Component Empty() {
  return Renderer([] { return text(""); });
}

int main()
{
  TorrentReader tr("/home/backltrack/Tulsa.torrent");
  TorrentExpander expander = TorrentExpanderImpl::Root();
  if (!tr.isValidTorrent())
  {
    std::cerr << "Error: Invalid torrent file\n";
    return EXIT_FAILURE;
  }

  const TorrentValue & root = tr.getRoot();
  //starts off as a dict,
  // 
  auto app = From(root, true, 0, expander);
  app = Renderer(app, [app] { return app->Render() | yframe;});
  auto screen = ScreenInteractive::Fullscreen();
  Event previous_event, next_event;
  auto wrapped_component = CatchEvent(app, [&](Event event)
                                                {
    previous_event = next_event;
    next_event = event;
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

    // Allow the user to quit using 'q' or ESC ---------------------------------
    if (event == Event::Character('q') || event == Event::Escape) {
      screen.ExitLoopClosure()();
      return true;
    }

    // Convert mouse whell into their corresponding Down/Up events.-------------
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

  screen.Loop(wrapped_component);

  return EXIT_SUCCESS;
}
