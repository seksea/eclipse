cmake_minimum_required(VERSION 3.11.0)
project(eclipse)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_FLAGS_DEBUG "-W -Wall -Wextra -O3 -march=native -mtune=native -fvisibility=hidden -flto -ggdb -DDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -std=c++2a -Wall -Wextra")

string(TOLOWER $ENV{USE_CLANG} USE_CLANG)
if(USE_CLANG STREQUAL "true")
  set(CMAKE_C_COMPILER "/usr/bin/clang")
  set(CMAKE_CXX_COMPILER "/usr/bin/clang++")

  set(CMAKE_C_FLAGS_RELEASE "-O3 -fvisibility=hidden -flto")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -std=c++2a")
else()
  set(CMAKE_C_COMPILER "/usr/bin/gcc")
  set(CMAKE_CXX_COMPILER "/usr/bin/g++")

  set(CMAKE_C_FLAGS_RELEASE "-O3 -s -fvisibility=hidden -flto -fwhole-program -Wl,--strip-all")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} --no-gnu-unique -std=c++2a")
endif()

set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-undefined")

# These are for funchook
add_compile_definitions(DISASM_DISTORM)
add_compile_definitions(_GNU_SOURCE)

file(GLOB_RECURSE SOURCES "${CMAKE_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE CSOURCES "${CMAKE_SOURCE_DIR}/src/*.c")
file(GLOB_RECURSE FUNCHOOK_SOURCES "${CMAKE_SOURCE_DIR}/dependencies/funchook/*.c")
list(APPEND SOURCES ${CSOURCES} ${FUNCHOOK_SOURCES})

include_directories(/usr/local/include /usr/include/freetype2)
link_directories(/usr/local/lib src)

add_library(eclipse SHARED ${SOURCES})
target_link_libraries(eclipse dl pthread SDL2 GL freetype luajit-5.1 gamesdk ssl crypto vulkan)
target_include_directories("${CMAKE_PROJECT_NAME}" PRIVATE
    "${CMAKE_SOURCE_DIR}/dependencies/funchook/include")
