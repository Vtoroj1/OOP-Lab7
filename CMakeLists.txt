cmake_minimum_required(VERSION 3.10)
project(Lab6)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

add_executable(main
    main.cpp
    npc.cpp
    knight.cpp
    bear.cpp
    orc.cpp
    visitor.cpp
    observer.cpp
    factory.cpp
    game_manager.cpp
)

include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.15.0
  TLS_VERIFY OFF
)

FetchContent_MakeAvailable(googletest)

add_executable(test
    tests.cpp
    npc.cpp
    knight.cpp
    bear.cpp
    orc.cpp
    visitor.cpp
    observer.cpp
    factory.cpp
    game_manager.cpp
)

target_link_libraries(test gtest_main)