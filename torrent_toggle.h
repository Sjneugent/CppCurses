#ifndef    TORRENT_TOGGLE_H
#define    TORRENT_TOGGLE_H
#include <ftxui/component/component.hpp>

ftxui::Component TorrentToggle(const char * label_on, const char * label_off, bool * state);
#endif