#include "file_browser.h"
#include <algorithm>

FileBrowser::FileBrowser(const std::string &start_dir)
    : current_dir_(fs::absolute(start_dir).string()) {
  BuildListing();
}

void FileBrowser::Refresh() { BuildListing(); }

void FileBrowser::BuildListing() {
  entries_.clear();
  if (!fs::exists(current_dir_) || !fs::is_directory(current_dir_))
    return;

  for (const auto &entry : fs::directory_iterator(current_dir_,
           fs::directory_options::skip_permission_denied)) {
    FileEntry fe;
    fe.full_path = entry.path().string();
    fe.name = entry.path().filename().string();
    // Skip hidden files/dirs
    if (!fe.name.empty() && fe.name[0] == '.')
      continue;

    if (entry.is_directory()) {
      fe.is_directory = true;
      entries_.push_back(fe);
    } else if (entry.is_regular_file()) {
      // Only show .torrent files
      auto ext = entry.path().extension().string();
      std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
      if (ext == ".torrent") {
        fe.is_directory = false;
        entries_.push_back(fe);
      }
    }
  }
  std::sort(entries_.begin(), entries_.end());
}

bool FileBrowser::Enter(int index) {
  if (index < 0 || index >= static_cast<int>(entries_.size()))
    return false;
  const auto &entry = entries_[index];
  if (!entry.is_directory)
    return false;
  current_dir_ = entry.full_path;
  BuildListing();
  return true;
}

bool FileBrowser::GoUp() {
  fs::path parent = fs::path(current_dir_).parent_path();
  if (parent == current_dir_)
    return false; // already at root
  current_dir_ = parent.string();
  BuildListing();
  return true;
}

bool FileBrowser::ToggleSelect(int index) {
  if (index < 0 || index >= static_cast<int>(entries_.size()))
    return false;
  const auto &entry = entries_[index];
  if (entry.is_directory)
    return false; // can only select files

  if (selected_.count(entry.full_path)) {
    selected_.erase(entry.full_path);
  } else {
    selected_.insert(entry.full_path);
  }
  return true;
}

void FileBrowser::SelectAll() {
  for (const auto &entry : entries_) {
    if (!entry.is_directory) {
      selected_.insert(entry.full_path);
    }
  }
}

void FileBrowser::DeselectAll() {
  // Only deselect items currently visible
  for (const auto &entry : entries_) {
    if (!entry.is_directory) {
      selected_.erase(entry.full_path);
    }
  }
}

const std::string &FileBrowser::CurrentDir() const { return current_dir_; }

const std::vector<FileEntry> &FileBrowser::Entries() const { return entries_; }

const std::set<std::string> &FileBrowser::Selected() const {
  return selected_;
}

bool FileBrowser::IsSelected(const std::string &path) const {
  return selected_.count(path) > 0;
}

std::vector<std::string> FileBrowser::SelectedPaths() const {
  std::vector<std::string> paths;
  for (const auto &p : selected_) {
    if (fs::exists(p)) {
      paths.push_back(p);
    }
  }
  return paths;
}
