#include "torrent_reader.h"
#include <sstream>

// --- TorrentValue Implementation ---

// Helper to print variants recursively
struct ValuePrinter {
  std::ostream &os;

  void operator()(const TorrentInt &val) { os << val; }
  void operator()(const TorrentString &val) {
    // Print strings generally; for torrents, some are binary, so be careful.
    // For display purposes, we might want to quote them or print hex if
    // non-printable.
    bool printable = true;
    for (char c : val) {
      if (!isprint(static_cast<unsigned char>(c))) {
        printable = false;
        break;
      }
    }
    if (printable)
      os << "\"" << val << "\"";
    else
      os << "<binary data: " << val.size() << " bytes>";
  }
  void operator()(const TorrentList &val) {
    os << "[";
    for (size_t i = 0; i < val.size(); ++i) {
      os << val[i];
      if (i < val.size() - 1)
        os << ", ";
    }
    os << "]";
  }
  void operator()(const TorrentDict &val) {
    os << "{";
    auto it = val.begin();
    while (it != val.end()) {
      os << "\"" << it->first << "\": " << it->second;
      if (++it != val.end())
        os << ", ";
    }
    os << "}";
  }
};

std::ostream &operator<<(std::ostream &os, const TorrentValue &val) {
  std::visit(ValuePrinter{os}, val.data);
  return os;
}

// --- TorrentReader Implementation ---

TorrentReader::TorrentReader(const std::string &filepath) {
  // 1. Basic extension check
  if (filepath.size() < 9 ||
      filepath.substr(filepath.size() - 8) != ".torrent") {
    // This is a soft check, we will attempt to parse anyway but warn or could
    // throw. For strictness: throw std::runtime_error("File does not have
    // .torrent extension");
  }

  // 2. Read File
  std::ifstream file(filepath, std::ios::binary);
  if (!file) {
    throw std::runtime_error("Cannot open file: " + filepath);
  }

  // Read entire file into string
  file.seekg(0, std::ios::end);
  size_t size = file.tellg();
  source_data.resize(size);
  file.seekg(0, std::ios::beg);
  file.read(&source_data[0], size);

  // 3. Parse
  if (source_data.empty()) {
    throw std::runtime_error("File is empty");
  }

  // Check if it starts with 'd' (Torrents are dictionaries)
  if (peek() != 'd') {
    throw std::runtime_error(
        "Invalid torrent file: Must start with a dictionary 'd'");
  }

  try {
    root = parseElement();
  } catch (const std::exception &e) {
    throw std::runtime_error(std::string("Parsing error: ") + e.what());
  }
}

DictView TorrentReader::field() const {
  if (!root.isDict()) {
    throw std::runtime_error(
        "Root is not a dictionary (Invalid torrent structure)");
  }
  return DictView(root.asDict());
}

const TorrentValue &TorrentReader::getRoot() const { return root; }

// --- Parser Logic ---

char TorrentReader::peek() const {
  if (pos >= source_data.size())
    return 0; // EOF
  return source_data[pos];
}

char TorrentReader::consume() {
  if (pos >= source_data.size())
    throw std::out_of_range("Unexpected End Of File");
  return source_data[pos++];
}

bool TorrentReader::match(char expected) {
  if (peek() == expected) {
    consume();
    return true;
  }
  return false;
}

void TorrentReader::expect(char expected) {
  if (consume() != expected) {
    throw std::runtime_error(std::string("Expected '") + expected +
                             "' at position " + std::to_string(pos));
  }
}

TorrentValue TorrentReader::parseElement() {
  char c = peek();
  if (isdigit(c))
    return {parseString()};
  if (c == 'i')
    return {parseInt()};
  if (c == 'l')
    return {parseList()};
  if (c == 'd')
    return {parseDict()};

  throw std::runtime_error(std::string("Unknown type indicator '") + c +
                           "' at " + std::to_string(pos));
}

TorrentInt TorrentReader::parseInt() {
  expect('i');
  size_t end = source_data.find('e', pos);
  if (end == std::string::npos)
    throw std::runtime_error("Unterminated integer");

  std::string numStr = source_data.substr(pos, end - pos);
  pos = end + 1; // Skip 'e'

  if (numStr == "-0")
    throw std::runtime_error("Invalid integer -0");
  if (numStr.size() > 1 && numStr[0] == '0')
    throw std::runtime_error("Invalid leading zero");

  try {
    return std::stoll(numStr);
  } catch (...) {
    throw std::runtime_error("Integer parse error: " + numStr);
  }
}

TorrentString TorrentReader::parseString() {
  size_t colon = source_data.find(':', pos);
  if (colon == std::string::npos)
    throw std::runtime_error("Invalid string length format");

  std::string lenStr = source_data.substr(pos, colon - pos);
  long long len = std::stoll(lenStr);

  pos = colon + 1; // Skip ':'

  if (pos + len > source_data.size())
    throw std::runtime_error("String content out of bounds");

  std::string str = source_data.substr(pos, len);
  pos += len;

  return str;
}

TorrentList TorrentReader::parseList() {
  expect('l');
  TorrentList list;
  while (peek() != 'e' && peek() != 0) {
    list.push_back(parseElement());
  }
  expect('e');
  return list;
}

TorrentDict TorrentReader::parseDict() {
  expect('d');
  TorrentDict dict;
  while (peek() != 'e' && peek() != 0) {
    // Keys must be strings
    TorrentString key = parseString();
    TorrentValue value = parseElement();
    dict[key] = value;
  }
  expect('e');
  return dict;
}

// Simple validator: root must be a dictionary and contain at least an "info" key.
// Additional checks (announce, piece length, etc.) can be added later.
bool TorrentReader::isValidTorrent() const {
  if (!root.isDict())
    return false;
  const auto &dict = root.asDict();
  return dict.find("info") != dict.end();
}
