#pragma once

#include <iostream>

class TorrentLogger {
public:
  TorrentLogger();
  ~TorrentLogger();
  void operator()(std::string const &message, char const *function,
                  char const *file, int line);
  TorrentLogger &Debug();
};
