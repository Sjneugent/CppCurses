#ifndef MULTI_VIEWER_H
#define MULTI_VIEWER_H

#include <string>
#include <vector>

/// @brief Manages multiple loaded torrent paths and provides tab-style
/// navigation.  The actual TorrentReader / UI components are owned externally;
/// this class only tracks the list and the active index.
class MultiViewer {
public:
  MultiViewer() = default;

  /// Add a torrent file path. Returns the index of the new tab.
  int Add(const std::string &path);

  /// Remove the torrent at the given index. Returns true on success.
  bool Remove(int index);

  /// Number of loaded torrents
  int Count() const;

  /// Currently active tab index (-1 if none)
  int ActiveIndex() const;

  /// Set the active tab by index. Returns true on success.
  bool SetActive(int index);

  /// Navigate to next / previous tab (wraps around). Returns new active index.
  int NextTab();
  int PrevTab();

  /// Get the path of the torrent at the given index
  const std::string &PathAt(int index) const;

  /// Get all loaded paths
  const std::vector<std::string> &Paths() const;

  /// Convenience: short label for a tab (filename without directory)
  static std::string TabLabel(const std::string &path);

private:
  std::vector<std::string> paths_;
  int active_ = -1;
};

#endif
