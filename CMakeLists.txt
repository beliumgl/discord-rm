cmake_minimum_required(VERSION 3.29)
project(discord-rm)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
  fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG 11.2.0
)
FetchContent_Declare(
  argparse
  GIT_REPOSITORY https://github.com/p-ranav/argparse
  GIT_TAG v3.2
)
FetchContent_Declare(
  json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.12.0
)

FetchContent_MakeAvailable(fmt argparse json)

add_executable(discord-rm "${CMAKE_SOURCE_DIR}/src/main.cpp"
                          "${CMAKE_SOURCE_DIR}/src/arguments.cpp"
                          "${CMAKE_SOURCE_DIR}/src/remover.cpp"
                          "${CMAKE_SOURCE_DIR}/src/config.cpp"
                          "${CMAKE_SOURCE_DIR}/src/helpers.cpp")
target_include_directories(discord-rm PRIVATE "${CMAKE_SOURCE_DIR}"
                                              "${argparse_SOURCE_DIR}/include")
target_link_libraries(discord-rm PRIVATE fmt::fmt nlohmann_json::nlohmann_json curl)
