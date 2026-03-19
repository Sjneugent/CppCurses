#ifndef FILE_BROWSER_H
#define FILE_BROWSER_H

#include <filesystem>
#include <set>
#include <string>
#include <vector>

namespace fs = std::filesystem;

/// @brief Represents a single entry in the file browser
struct FileEntry {
  std::string name;
  std::string full_path;
  bool is_directory;

  bool operator<(const FileEntry &other) const {
    // Directories first, then alphabetical
    if (is_directory != other.is_directory)
      return is_directory > other.is_directory;
    return name < other.name;
  }
};

/// @brief Logic for browsing the filesystem and selecting torrent files.
/// Separates file system logic from the UI so it can be unit-tested.
class FileBrowser {
public:
  explicit FileBrowser(const std::string &start_dir = ".");

  /// Refresh the listing for the current directory
  void Refresh();

  /// Navigate into a child directory or go up to parent
  bool Enter(int index);
  bool GoUp();

  /// Toggle selection of a file entry at the given index
  bool ToggleSelect(int index);

  /// Select / deselect all torrent files in the current listing
  void SelectAll();
  void DeselectAll();

  // --- Accessors ---
  const std::string &CurrentDir() const;
  const std::vector<FileEntry> &Entries() const;
  const std::set<std::string> &Selected() const;
  bool IsSelected(const std::string &path) const;

  /// Return only the selected paths that still exist
  std::vector<std::string> SelectedPaths() const;

private:
  std::string current_dir_;
  std::vector<FileEntry> entries_;
  std::set<std::string> selected_;

  void BuildListing();
};

#endif
