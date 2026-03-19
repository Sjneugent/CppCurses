#include <gtest/gtest.h>
#include "file_browser.h"
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

class FileBrowserTest : public ::testing::Test {
protected:
  void SetUp() override {
    test_dir_ = fs::temp_directory_path() / "file_browser_test";
    fs::create_directories(test_dir_);
    fs::create_directories(test_dir_ / "subdir");
    fs::create_directories(test_dir_ / "another_dir");

    // Create torrent files
    CreateFile(test_dir_ / "alpha.torrent", "d4:infod4:name5:alphaee");
    CreateFile(test_dir_ / "beta.torrent", "d4:infod4:name4:betaee");
    CreateFile(test_dir_ / "gamma.torrent", "d4:infod4:name5:gammaee");

    // Create non-torrent files (should be hidden from the browser)
    CreateFile(test_dir_ / "readme.txt", "hello");
    CreateFile(test_dir_ / "image.png", "png");

    // Create a torrent file inside the subdirectory
    CreateFile(test_dir_ / "subdir" / "nested.torrent",
               "d4:infod4:name6:nestedee");
  }

  void TearDown() override { fs::remove_all(test_dir_); }

  void CreateFile(const fs::path &p, const std::string &content) {
    std::ofstream out(p, std::ios::binary);
    out << content;
  }

  fs::path test_dir_;
};

// --- Basic listing tests ---

TEST_F(FileBrowserTest, ListsEntriesInDirectory) {
  FileBrowser fb(test_dir_.string());
  // Should see 2 directories + 3 torrent files
  EXPECT_EQ(fb.Entries().size(), 5u);
}

TEST_F(FileBrowserTest, DirectoriesAppearFirst) {
  FileBrowser fb(test_dir_.string());
  const auto &entries = fb.Entries();
  // First entries should be directories
  ASSERT_GE(entries.size(), 2u);
  EXPECT_TRUE(entries[0].is_directory);
  EXPECT_TRUE(entries[1].is_directory);
}

TEST_F(FileBrowserTest, EntriesSortedAlphabetically) {
  FileBrowser fb(test_dir_.string());
  const auto &entries = fb.Entries();
  // Directories sorted
  EXPECT_EQ(entries[0].name, "another_dir");
  EXPECT_EQ(entries[1].name, "subdir");
  // Files sorted
  EXPECT_EQ(entries[2].name, "alpha.torrent");
  EXPECT_EQ(entries[3].name, "beta.torrent");
  EXPECT_EQ(entries[4].name, "gamma.torrent");
}

TEST_F(FileBrowserTest, FiltersNonTorrentFiles) {
  FileBrowser fb(test_dir_.string());
  for (const auto &e : fb.Entries()) {
    if (!e.is_directory) {
      EXPECT_TRUE(e.name.find(".torrent") != std::string::npos)
          << "Non-torrent file shown: " << e.name;
    }
  }
}

TEST_F(FileBrowserTest, CurrentDirReturnsAbsolutePath) {
  FileBrowser fb(test_dir_.string());
  EXPECT_TRUE(fs::path(fb.CurrentDir()).is_absolute());
}

// --- Navigation tests ---

TEST_F(FileBrowserTest, EnterDirectory) {
  FileBrowser fb(test_dir_.string());
  // Find "subdir" index
  int subdir_idx = -1;
  for (int i = 0; i < static_cast<int>(fb.Entries().size()); ++i) {
    if (fb.Entries()[i].name == "subdir") {
      subdir_idx = i;
      break;
    }
  }
  ASSERT_NE(subdir_idx, -1);
  EXPECT_TRUE(fb.Enter(subdir_idx));
  // Now should see 1 torrent file
  bool found_nested = false;
  for (const auto &e : fb.Entries()) {
    if (e.name == "nested.torrent")
      found_nested = true;
  }
  EXPECT_TRUE(found_nested);
}

TEST_F(FileBrowserTest, EnterFileReturnsFalse) {
  FileBrowser fb(test_dir_.string());
  // Find a file entry
  int file_idx = -1;
  for (int i = 0; i < static_cast<int>(fb.Entries().size()); ++i) {
    if (!fb.Entries()[i].is_directory) {
      file_idx = i;
      break;
    }
  }
  ASSERT_NE(file_idx, -1);
  EXPECT_FALSE(fb.Enter(file_idx));
}

TEST_F(FileBrowserTest, EnterOutOfBoundsReturnsFalse) {
  FileBrowser fb(test_dir_.string());
  EXPECT_FALSE(fb.Enter(-1));
  EXPECT_FALSE(fb.Enter(999));
}

TEST_F(FileBrowserTest, GoUpNavigatesToParent) {
  // Start in subdir
  FileBrowser fb((test_dir_ / "subdir").string());
  std::string subdir_path = fb.CurrentDir();
  EXPECT_TRUE(fb.GoUp());
  EXPECT_NE(fb.CurrentDir(), subdir_path);
}

// --- Selection tests ---

TEST_F(FileBrowserTest, ToggleSelectFile) {
  FileBrowser fb(test_dir_.string());
  int file_idx = -1;
  for (int i = 0; i < static_cast<int>(fb.Entries().size()); ++i) {
    if (!fb.Entries()[i].is_directory) {
      file_idx = i;
      break;
    }
  }
  ASSERT_NE(file_idx, -1);

  EXPECT_TRUE(fb.ToggleSelect(file_idx));
  EXPECT_TRUE(fb.IsSelected(fb.Entries()[file_idx].full_path));

  // Toggle off
  EXPECT_TRUE(fb.ToggleSelect(file_idx));
  EXPECT_FALSE(fb.IsSelected(fb.Entries()[file_idx].full_path));
}

TEST_F(FileBrowserTest, CannotSelectDirectory) {
  FileBrowser fb(test_dir_.string());
  int dir_idx = -1;
  for (int i = 0; i < static_cast<int>(fb.Entries().size()); ++i) {
    if (fb.Entries()[i].is_directory) {
      dir_idx = i;
      break;
    }
  }
  ASSERT_NE(dir_idx, -1);
  EXPECT_FALSE(fb.ToggleSelect(dir_idx));
}

TEST_F(FileBrowserTest, SelectAll) {
  FileBrowser fb(test_dir_.string());
  fb.SelectAll();
  // Should have 3 torrent files selected
  EXPECT_EQ(fb.Selected().size(), 3u);
}

TEST_F(FileBrowserTest, DeselectAll) {
  FileBrowser fb(test_dir_.string());
  fb.SelectAll();
  EXPECT_EQ(fb.Selected().size(), 3u);
  fb.DeselectAll();
  EXPECT_EQ(fb.Selected().size(), 0u);
}

TEST_F(FileBrowserTest, SelectedPathsReturnsExistingFiles) {
  FileBrowser fb(test_dir_.string());
  fb.SelectAll();
  auto paths = fb.SelectedPaths();
  EXPECT_EQ(paths.size(), 3u);
  for (const auto &p : paths) {
    EXPECT_TRUE(fs::exists(p));
  }
}

TEST_F(FileBrowserTest, SelectionSurvivesDirectoryChange) {
  FileBrowser fb(test_dir_.string());
  // Select a file
  int file_idx = -1;
  for (int i = 0; i < static_cast<int>(fb.Entries().size()); ++i) {
    if (!fb.Entries()[i].is_directory) {
      file_idx = i;
      break;
    }
  }
  ASSERT_NE(file_idx, -1);
  fb.ToggleSelect(file_idx);
  std::string selected_path = fb.Entries()[file_idx].full_path;

  // Navigate to subdir and back
  int subdir_idx = -1;
  for (int i = 0; i < static_cast<int>(fb.Entries().size()); ++i) {
    if (fb.Entries()[i].name == "subdir") {
      subdir_idx = i;
      break;
    }
  }
  fb.Enter(subdir_idx);
  fb.GoUp();

  // Selection should persist
  EXPECT_TRUE(fb.IsSelected(selected_path));
}

TEST_F(FileBrowserTest, RefreshReloadsListing) {
  FileBrowser fb(test_dir_.string());
  size_t initial = fb.Entries().size();

  // Add a new file
  CreateFile(test_dir_ / "delta.torrent", "d4:infod4:name5:deltaee");
  fb.Refresh();
  EXPECT_EQ(fb.Entries().size(), initial + 1);

  // Cleanup
  fs::remove(test_dir_ / "delta.torrent");
}

TEST_F(FileBrowserTest, ToggleSelectOutOfBoundsReturnsFalse) {
  FileBrowser fb(test_dir_.string());
  EXPECT_FALSE(fb.ToggleSelect(-1));
  EXPECT_FALSE(fb.ToggleSelect(999));
}

TEST_F(FileBrowserTest, EmptyDirectoryShowsNoEntries) {
  fs::path empty_dir = test_dir_ / "empty_dir";
  fs::create_directories(empty_dir);
  FileBrowser fb(empty_dir.string());
  EXPECT_EQ(fb.Entries().size(), 0u);
}
