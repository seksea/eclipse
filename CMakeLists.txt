cmake_minimum_required(VERSION 3.11.0)
project(csgo-cheat)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_FLAGS_DEBUG "-Wall -Wextra -march=core2 -mtune=native -fvisibility=hidden -flto -ggdb -DDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -std=c++2a -Wall -Wextra")

string(TOLOWER $ENV{USE_CLANG} USE_CLANG)
if(USE_CLANG STREQUAL "true")
  set(CMAKE_C_COMPILER "/usr/bin/clang")
  set(CMAKE_CXX_COMPILER "/usr/bin/clang++")

  set(CMAKE_C_FLAGS_RELEASE "-W -Wall -Wextra -O3 -march=native -mtune=native -fvisibility=hidden -flto")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -fno-rtti -std=c++2a -Wall -Wextra")
else()
  set(CMAKE_C_COMPILER "/usr/bin/gcc")
  set(CMAKE_CXX_COMPILER "/usr/bin/g++")

  set(CMAKE_C_FLAGS_RELEASE "-W -Wall -Wextra -O3 -march=native -mtune=native -s -fvisibility=hidden -flto -fwhole-program -Wl,--strip-all")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} --no-gnu-unique -fno-rtti -std=c++2a")
endif()

set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-undefined")

file(GLOB_RECURSE SOURCES "${CMAKE_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE CSOURCES "${CMAKE_SOURCE_DIR}/src/*.c")
list(APPEND SOURCES ${CSOURCES})

include_directories(/usr/local/include /usr/include/freetype2)
link_directories(/usr/local/lib)

add_library(csgo-cheat SHARED ${SOURCES})
target_link_libraries(csgo-cheat dl pthread SDL2 GL freetype luajit-5.1)
