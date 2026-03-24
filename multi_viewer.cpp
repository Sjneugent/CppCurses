#include "multi_viewer.h"
#include <filesystem>

int MultiViewer::Add(const std::string &path) {
  paths_.push_back(path);
  int idx = static_cast<int>(paths_.size()) - 1;
  if (active_ < 0)
    active_ = idx;
  return idx;
}

bool MultiViewer::Remove(int index) {
  if (index < 0 || index >= static_cast<int>(paths_.size()))
    return false;
  paths_.erase(paths_.begin() + index);
  if (paths_.empty()) {
    active_ = -1;
  } else if (active_ >= static_cast<int>(paths_.size())) {
    active_ = static_cast<int>(paths_.size()) - 1;
  }
  return true;
}

int MultiViewer::Count() const { return static_cast<int>(paths_.size()); }

int MultiViewer::ActiveIndex() const { return active_; }

bool MultiViewer::SetActive(int index) {
  if (index < 0 || index >= static_cast<int>(paths_.size()))
    return false;
  active_ = index;
  return true;
}

int MultiViewer::NextTab() {
  if (paths_.empty())
    return -1;
  active_ = (active_ + 1) % static_cast<int>(paths_.size());
  return active_;
}

int MultiViewer::PrevTab() {
  if (paths_.empty())
    return -1;
  active_ = (active_ - 1 + static_cast<int>(paths_.size())) %
            static_cast<int>(paths_.size());
  return active_;
}

const std::string &MultiViewer::PathAt(int index) const {
  return paths_.at(index);
}

const std::vector<std::string> &MultiViewer::Paths() const { return paths_; }

std::string MultiViewer::TabLabel(const std::string &path) {
  return std::filesystem::path(path).filename().string();
}
