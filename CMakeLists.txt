cmake_minimum_required(VERSION 3.16)

project(MatchingEngine)
set(CMAKE_BUILD_TYPE Debug) # Release

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

file(GLOB SOURCES "src/*.cpp")

add_executable(matching_engine ${SOURCES})

target_compile_options(matching_engine PRIVATE
    # -g
    # -O0
    # -Wall
    # -Wextra
    # -fno-omit-frame-pointer
    # -fsanitize=address
    
    -O3
    -march=native
    -flto
    # -O3 -g -fno-omit-frame-pointer -fno-optimize-sibling-calls
)

add_executable(event_reader src/event_reader.cc)

# target_link_options(matching_engine PRIVATE
#     -fsanitize=address
# )