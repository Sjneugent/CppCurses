#include "help_page.h"

HelpPage::HelpPage() {
  sections_ = {
      {"Navigation",
       {
           {"j / ↓", "Move down"},
           {"k / ↑", "Move up"},
           {"gg", "Jump to top"},
           {"G", "Jump to bottom"},
           {"Space / Enter", "Toggle expand/collapse"},
       }},
      {"Torrent Tabs",
       {
           {"Tab / l", "Next torrent tab"},
           {"Shift-Tab / h", "Previous torrent tab"},
           {"1-9", "Jump to torrent tab N"},
       }},
      {"File Browser",
       {
           {"o", "Open file browser"},
           {"a", "Select all files in browser"},
           {"d", "Deselect all files in browser"},
           {"Enter", "Open directory / confirm selection"},
           {"Backspace", "Go to parent directory"},
       }},
      {"General",
       {
           {"?", "Toggle this help page"},
           {"q / Esc", "Quit application"},
           {"Ctrl-C", "Force quit"},
       }},
  };
}

const std::vector<HelpSection> &HelpPage::Sections() const {
  return sections_;
}

std::vector<HelpEntry> HelpPage::AllEntries() const {
  std::vector<HelpEntry> all;
  for (const auto &sec : sections_) {
    for (const auto &e : sec.entries) {
      all.push_back(e);
    }
  }
  return all;
}

std::string HelpPage::DescriptionFor(const std::string &key) const {
  for (const auto &sec : sections_) {
    for (const auto &e : sec.entries) {
      if (e.key == key)
        return e.description;
    }
  }
  return "";
}
