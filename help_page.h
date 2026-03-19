#ifndef HELP_PAGE_H
#define HELP_PAGE_H

#include <string>
#include <utility>
#include <vector>

/// @brief Represents a single keybinding entry for the help page
struct HelpEntry {
  std::string key;
  std::string description;
};

/// @brief Groups of keybindings shown on the help page
struct HelpSection {
  std::string title;
  std::vector<HelpEntry> entries;
};

/// @brief Provides help page content in a testable, UI-agnostic way.
class HelpPage {
public:
  HelpPage();

  const std::vector<HelpSection> &Sections() const;

  /// Flat list of all entries across all sections
  std::vector<HelpEntry> AllEntries() const;

  /// Find the description for a given key (empty string if not found)
  std::string DescriptionFor(const std::string &key) const;

private:
  std::vector<HelpSection> sections_;
};

#endif
