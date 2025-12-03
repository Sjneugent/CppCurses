
cmake_minimum_required(VERSION 3.28)

project(TerminalCPP)


set(TerminalCPP LANGUAGES C  CXX)
set(CMAKE_CXX_STANDARD 20)

include(FetchContent)
#set(CMAKE_GENERATOR Ninja)
set(FTXUI_BUILD_MODULES ON)
set(FTXUI_BUILD_EXAMPLES ON)
set(FTXUI_BUILD_DOCS ON)
set(FTXUI_DEV_WARNINGS ON)
FetchContent_Declare(ftxui 
    GIT_REPOSITORY https://github.com/ArthurSonzogni/FTXUI.git
    GIT_TAG        v6.1.8)

set(FTXUI_BUILD_MODULES ON)
FetchContent_MakeAvailable(ftxui)

if(NOT ftxui_POPULATED) 
  FetchCoontent_Populate(ftxui)
  add_subdirectory(${ftxui_SOURCE_DIR} ${ftxui_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()
add_executable(${PROJECT_NAME} curses.cpp torrent_reader.h torrent_reader.cpp torrent_expander.h torrent_expander.cpp torrent_toggle.h torrent_toggle.cpp torrent_formatter.h) 
target_link_libraries(${PROJECT_NAME}    PRIVATE ftxui::screen
  PRIVATE ftxui::dom
  PRIVATE ftxui::component)

# Enable testing
enable_testing()

# Fetch Google Test
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.14.0
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Add test subdirectory
add_subdirectory(tests)
