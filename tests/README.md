# CppCurses Test Suite

This directory contains the unit tests and test data for the CppCurses torrent viewer project.

## Structure

```
tests/
├── CMakeLists.txt              # Test build configuration
├── README.md                   # This file
├── torrent_reader_test.cpp     # Unit tests for TorrentReader class
├── torrent_expander_test.cpp   # Unit tests for TorrentExpander class
└── data/                       # Test data files
    ├── simple_int.torrent      # Simple integer bencode
    ├── simple_string.torrent   # Simple string bencode
    ├── simple_list.torrent     # Simple list bencode
    ├── simple_dict.torrent     # Simple dictionary bencode
    ├── valid_torrent.torrent   # Valid minimal torrent structure
    ├── nested_struct.torrent   # Nested dictionary/list structure
    ├── empty.torrent           # Empty file (for error testing)
    └── invalid.torrent         # Invalid bencode (for error testing)
```

## Running Tests

### Build and Run All Tests

```bash
cd build
cmake ..
cmake --build .
./tests/torrent_tests
```

### Run Tests with CTest

```bash
cd build
ctest --output-on-failure
```

### Run Specific Tests

```bash
cd build
./tests/torrent_tests --gtest_filter="TorrentReaderTest.*"
./tests/torrent_tests --gtest_filter="TorrentExpanderTest.*"
```

## Test Coverage

### TorrentReader Tests (20 tests)

The TorrentReader tests validate the bencode parser and torrent file reading functionality:

- **Parsing Tests**: Verify correct parsing of integers, strings, lists, and dictionaries
- **Nested Structures**: Test deeply nested bencode structures
- **Edge Cases**: Empty strings, empty lists, empty dictionaries
- **Data Types**: Negative integers, large integers, binary strings, mixed-type lists
- **Validation**: Torrent file structure validation (checking for required "info" key)
- **Error Handling**: Invalid files, empty files, non-existent files, non-dictionary root

### TorrentExpander Tests (20 tests)

The TorrentExpander tests validate the tree expansion state management:

- **Basic Operations**: Initialize, set expanded, toggle state
- **Tree Structure**: Create children, parent-child relationships
- **Level Calculations**: MinLevel and MaxLevel for various tree structures
- **Expand/Collapse**: Recursive expansion and collapse operations
- **Complex Trees**: Multi-level trees with mixed expansion states
- **Edge Cases**: Child destruction, independent expansion states

## Test Data Files

### Bencode Format

All test data files use the bencode format, which is the encoding used by torrent files:

- **Integers**: `i<number>e` (e.g., `i42e` for 42)
- **Strings**: `<length>:<string>` (e.g., `5:hello`)
- **Lists**: `l<elements>e` (e.g., `li1ei2ee` for [1, 2])
- **Dictionaries**: `d<key><value>...e` (e.g., `d3:key5:valuee`)

### Test Data Descriptions

- `simple_int.torrent`: Contains just `i42e` (integer 42)
- `simple_string.torrent`: Contains `5:hello` (string "hello")
- `simple_list.torrent`: Contains `li1ei2ei3ee` (list [1, 2, 3])
- `simple_dict.torrent`: Contains `d3:key5:valuee` (dict {"key": "value"})
- `valid_torrent.torrent`: Minimal valid torrent with "info" dictionary
- `nested_struct.torrent`: Complex nested structure with lists and dictionaries
- `empty.torrent`: Empty file (0 bytes)
- `invalid.torrent`: Contains invalid bencode data

## Adding New Tests

To add new tests:

1. Create test data files in `tests/data/` if needed
2. Add test cases to appropriate `*_test.cpp` file
3. Follow the existing test structure and naming conventions
4. Rebuild and verify all tests pass:
   ```bash
   cd build
   cmake --build . --target torrent_tests
   ./tests/torrent_tests
   ```

## Test Framework

The test suite uses [Google Test](https://github.com/google/googletest) (GTest) framework, which is automatically downloaded and configured via CMake's FetchContent.

### Key Features

- Automatic test discovery via `gtest_discover_tests()`
- Rich assertion macros (EXPECT_*, ASSERT_*)
- Test fixtures for shared setup/teardown
- Parameterized tests support
- Death tests for error conditions

## Continuous Integration

These tests are designed to run in CI/CD pipelines. The test executable returns:
- Exit code 0 if all tests pass
- Non-zero exit code if any tests fail

CTest integration provides additional reporting and filtering capabilities.
