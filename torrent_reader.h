#pragma once

#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

// Forward declaration for recursive types
struct TorrentValue;

// Alias types for clarity
using TorrentInt = long long;
using TorrentString = std::string;
using TorrentList = std::vector<TorrentValue>;
// using std::map with incomplete type is allowed in some C++ versions,
// but wrapping in unique_ptr ensures standard compliance for recursive
// definitions.
using TorrentDict = std::map<std::string, TorrentValue>;

struct TorrentValue {
  // We use a variant to hold the specific Torrent type
  std::variant<TorrentInt, TorrentString, TorrentList, TorrentDict> data;

  // Helper to check types
  bool isInt() const { return std::holds_alternative<TorrentInt>(data); }
  bool isString() const { return std::holds_alternative<TorrentString>(data); }
  bool isList() const { return std::holds_alternative<TorrentList>(data); }
  bool isDict() const { return std::holds_alternative<TorrentDict>(data); }

  // Accessors (throw if type mismatch)
  const TorrentInt &asInt() const { return std::get<TorrentInt>(data); }
  const TorrentString &asString() const {
    return std::get<TorrentString>(data);
  }
  const TorrentList &asList() const { return std::get<TorrentList>(data); }
  const TorrentDict &asDict() const { return std::get<TorrentDict>(data); }

  // Friendly printer for std::cout
  friend std::ostream &operator<<(std::ostream &os, const TorrentValue &val);
};

// Helper class to provide the .key() / .value() syntax requested
class DictEntryProxy {
public:
  DictEntryProxy(const std::string &key, const TorrentValue &val)
      : k(key), v(val) {}

  const std::string &key() const { return k; }
  const TorrentValue &value() const { return v; }

private:
  const std::string &k;
  const TorrentValue &v;
};

// A view class to allow iteration over the dictionary
class DictView {
public:
  using iterator = TorrentDict::const_iterator;

  // Custom iterator wrapper to return DictEntryProxy
  struct ProxyIterator {
    iterator it;

    bool operator!=(const ProxyIterator &other) const { return it != other.it; }
    void operator++() { ++it; }
    DictEntryProxy operator*() const {
      return DictEntryProxy(it->first, it->second);
    }
  };

  DictView(const TorrentDict &dict) : dict_ref(dict) {}

  ProxyIterator begin() const { return {dict_ref.begin()}; }
  ProxyIterator end() const { return {dict_ref.end()}; }

private:
  const TorrentDict &dict_ref;
};

class TorrentReader {
public:
  explicit TorrentReader(const std::string &filepath);

  // Validates file extension and structure
  bool isValidTorrent() const;

  // Returns a view of the root dictionary fields
  DictView field() const;

  // Direct access to the root value
  const TorrentValue &getRoot() const;

private:
  std::string source_data;
  size_t pos = 0;
  TorrentValue root;

  // Parser methods
  TorrentValue parseElement();
  TorrentInt parseInt();
  TorrentString parseString();
  TorrentList parseList();
  TorrentDict parseDict();

  char peek() const;
  char consume();
  bool match(char expected);
  void expect(char expected);
};
