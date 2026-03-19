#include "torrent_logger.h"

TorrentLogger::TorrentLogger() {}

TorrentLogger::~TorrentLogger() {}

TorrentLogger TorrentLogger::Debug() {
  static TorrentLogger logger("Debug", Console);
}
