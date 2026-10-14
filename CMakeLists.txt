cmake_minimum_required(VERSION 3.16)
project(MiniCache CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

add_compile_options(-Wall -Wextra -Wpedantic)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_compile_options(-g -fsanitize=address,undefined)
  add_link_options(-fsanitize=address,undefined)
endif()

# The cache code as a library, used by both programs.
add_library(minicache_core STATIC src/CacheConfig.cpp src/Cache.cpp)
target_include_directories(minicache_core PUBLIC include)

# Program 1: the demo
add_executable(minicache src/main.cpp)
target_link_libraries(minicache PRIVATE minicache_core)

# Program 2: the tests
add_executable(tests tests/test_cache.cpp)
target_link_libraries(tests PRIVATE minicache_core)

enable_testing()
add_test(NAME cache_tests COMMAND tests)