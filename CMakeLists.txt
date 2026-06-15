cmake_minimum_required(VERSION 3.20)
project(offender CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG        5.5
    GIT_SHALLOW    TRUE
)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(raylib)

file(GLOB SOURCES CONFIGURE_DEPENDS "src/*.cpp")
add_executable(offender WIN32 ${SOURCES})
target_include_directories(offender PRIVATE src)
target_link_libraries(offender PRIVATE raylib)

include(CTest)
if(BUILD_TESTING)
    add_executable(game_logic_tests tests/test_game_logic.cpp)
    target_include_directories(game_logic_tests PRIVATE src)
    add_test(NAME game_logic_tests COMMAND game_logic_tests)
endif()
