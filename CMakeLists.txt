## Copied from williamkellers CMake code on my repository: https://github.com/aenemenate/Aztheria-and-Elendor-Worlds-of-Dragons

cmake_minimum_required(VERSION 3.10.0)
project("tw_text_edit")

# Compiler settings. These might need to be platform specific?
set(CMAKE_CXX_STANDARD 17)

# Platform specific settings
if(WIN32)
  include(cmake/windows.cmake)
elseif(APPLE)
  include(cmake/macos.cmake)
endif()

# Setup bearlibterminal
include_directories("3rdparty/bearlibterminal/include")
set(BearLibTerminal_LIBRARIES "BearLibTerminal")

# Specify all the code to compile
file(GLOB_RECURSE SOURCES "src/*.cpp")

# Build it all
add_executable(tw-text-edit ${SOURCES})
target_link_libraries(tw-text-edit ${BearLibTerminal_LIBRARIES})