#include <gtest/gtest.h>
#include "help_page.h"

class HelpPageTest : public ::testing::Test {
protected:
  HelpPage help;
};

// --- Section tests ---

TEST_F(HelpPageTest, HasMultipleSections) {
  EXPECT_GE(help.Sections().size(), 3u);
}

TEST_F(HelpPageTest, NavigationSectionExists) {
  bool found = false;
  for (const auto &sec : help.Sections()) {
    if (sec.title == "Navigation") {
      found = true;
      break;
    }
  }
  EXPECT_TRUE(found);
}

TEST_F(HelpPageTest, TorrentTabsSectionExists) {
  bool found = false;
  for (const auto &sec : help.Sections()) {
    if (sec.title == "Torrent Tabs") {
      found = true;
      break;
    }
  }
  EXPECT_TRUE(found);
}

TEST_F(HelpPageTest, FileBrowserSectionExists) {
  bool found = false;
  for (const auto &sec : help.Sections()) {
    if (sec.title == "File Browser") {
      found = true;
      break;
    }
  }
  EXPECT_TRUE(found);
}

TEST_F(HelpPageTest, GeneralSectionExists) {
  bool found = false;
  for (const auto &sec : help.Sections()) {
    if (sec.title == "General") {
      found = true;
      break;
    }
  }
  EXPECT_TRUE(found);
}

// --- Entry tests ---

TEST_F(HelpPageTest, AllEntriesNonEmpty) {
  auto all = help.AllEntries();
  EXPECT_GT(all.size(), 0u);
  for (const auto &e : all) {
    EXPECT_FALSE(e.key.empty());
    EXPECT_FALSE(e.description.empty());
  }
}

TEST_F(HelpPageTest, DescriptionForKnownKey) {
  std::string desc = help.DescriptionFor("q / Esc");
  EXPECT_EQ(desc, "Quit application");
}

TEST_F(HelpPageTest, DescriptionForHelpKey) {
  std::string desc = help.DescriptionFor("?");
  EXPECT_EQ(desc, "Toggle this help page");
}

TEST_F(HelpPageTest, DescriptionForUnknownKeyReturnsEmpty) {
  std::string desc = help.DescriptionFor("zzz_does_not_exist");
  EXPECT_TRUE(desc.empty());
}

TEST_F(HelpPageTest, NavigationSectionHasEntries) {
  for (const auto &sec : help.Sections()) {
    if (sec.title == "Navigation") {
      EXPECT_GE(sec.entries.size(), 4u);
      return;
    }
  }
  FAIL() << "Navigation section not found";
}

TEST_F(HelpPageTest, AllSectionsHaveEntries) {
  for (const auto &sec : help.Sections()) {
    EXPECT_GT(sec.entries.size(), 0u) << "Section '" << sec.title << "' is empty";
  }
}

TEST_F(HelpPageTest, VimNavigationKeysDocumented) {
  auto all = help.AllEntries();
  bool has_j = false, has_k = false, has_gg = false, has_G = false;
  for (const auto &e : all) {
    if (e.key.find("j") != std::string::npos)
      has_j = true;
    if (e.key.find("k") != std::string::npos)
      has_k = true;
    if (e.key.find("gg") != std::string::npos)
      has_gg = true;
    if (e.key.find("G") != std::string::npos)
      has_G = true;
  }
  EXPECT_TRUE(has_j);
  EXPECT_TRUE(has_k);
  EXPECT_TRUE(has_gg);
  EXPECT_TRUE(has_G);
}

TEST_F(HelpPageTest, TabNavigationDocumented) {
  std::string desc = help.DescriptionFor("Tab / l");
  EXPECT_FALSE(desc.empty());
  desc = help.DescriptionFor("Shift-Tab / h");
  EXPECT_FALSE(desc.empty());
}

TEST_F(HelpPageTest, FileBrowserKeysDocumented) {
  std::string desc = help.DescriptionFor("o");
  EXPECT_EQ(desc, "Open file browser");
}

TEST_F(HelpPageTest, SectionsHaveUniqueTitles) {
  std::set<std::string> titles;
  for (const auto &sec : help.Sections()) {
    EXPECT_TRUE(titles.insert(sec.title).second)
        << "Duplicate section title: " << sec.title;
  }
}

TEST_F(HelpPageTest, AllEntriesCount) {
  auto all = help.AllEntries();
  size_t expected = 0;
  for (const auto &sec : help.Sections()) {
    expected += sec.entries.size();
  }
  EXPECT_EQ(all.size(), expected);
}
