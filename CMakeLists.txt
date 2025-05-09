cmake_minimum_required(VERSION 3.29)
project(discord-rm)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(discord-rm "${CMAKE_SOURCE_DIR}/src/main.cpp"
                          "${CMAKE_SOURCE_DIR}/src/arguments.cpp"
                          "${CMAKE_SOURCE_DIR}/src/remover.cpp"
                          "${CMAKE_SOURCE_DIR}/src/config.cpp"
                          "${CMAKE_SOURCE_DIR}/src/helpers.cpp")
target_include_directories(discord-rm PRIVATE "${CMAKE_SOURCE_DIR}")
target_link_options(discord-rm PRIVATE "-lcurl")
