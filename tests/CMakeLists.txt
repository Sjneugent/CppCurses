# Test executable
add_executable(
  torrent_tests
  torrent_reader_test.cpp
  torrent_expander_test.cpp
)

target_link_libraries(
  torrent_tests
  GTest::gtest_main
)

# Include parent directories for accessing source files
target_include_directories(torrent_tests PRIVATE ${CMAKE_SOURCE_DIR})

# Add source files to compile with tests
target_sources(torrent_tests PRIVATE
  ${CMAKE_SOURCE_DIR}/torrent_reader.cpp
  ${CMAKE_SOURCE_DIR}/torrent_expander.cpp
)

include(GoogleTest)
gtest_discover_tests(torrent_tests)
