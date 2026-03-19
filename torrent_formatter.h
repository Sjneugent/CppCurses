#ifndef TORRENT_FORMATTER_H
#define TORRENT_FORMATTER_H
#include "ftxui/component/component.hpp"
#include "ftxui/component/event.hpp"
#include "torrent_expander.h"
#include "torrent_reader.h"
#include <string>
using namespace ftxui;
Component Empty();
Component Unimplemented();
Component From(const TorrentValue &val, bool is_last, int depth,
               TorrentExpander &expander);
Component FromList(const Component &prefix, const TorrentList &list,
                   bool is_last, int depth, TorrentExpander &expander);
Component FromString(const TorrentValue &val, bool is_last);
Component FromNumber(const TorrentValue &val, bool is_last);
Component FromDict(const Component &prefix, const TorrentValue &val,
                   bool is_last, int depth, TorrentExpander &expander);
Component FromKeyValue(Component prefix, const TorrentValue &val,
                       const std::string &key, bool is_last, int depth,
                       TorrentExpander &expander);
/// @brief  Component that can be expanded/collapsed
class ComponentExpandable : public ComponentBase {
public:
  explicit ComponentExpandable(TorrentExpander &expander)
      : expander_(expander) {}

  [[nodiscard]] bool &Expanded() const { return expander_->expanded; }
  bool OnEvent(const Event event) override {
    if (ComponentBase::OnEvent(event)) {
      return true;
    }

    return false;
  }

protected:
  TorrentExpander &expander_;
};

inline Component Indentation(const Component &child) {
  return Renderer(child, [child] {
    return hbox({
        text("  "),
        child->Render(),
    });
  });
}

static std::string formatValuePreview(const TorrentValue &val) {
  if (val.isInt()) {
    return std::to_string(val.asInt());
  } else if (val.isString()) {
    const auto &str = val.asString();
    // Check if printable
    bool printable = true;
    for (const char c : str) {
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

#endif
