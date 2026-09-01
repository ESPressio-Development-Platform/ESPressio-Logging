cmake_minimum_required(VERSION 3.16)
project(ESPressioLoggingTests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

enable_testing()
find_package(Threads REQUIRED)

add_executable(test_logging test_logging.cpp)
target_include_directories(test_logging PRIVATE
    ../src
    ../../ESPressio-System/src
    ../../ESPressio-Observable/src
    ../../ESPressio-Timing/src
    ../../ESPressio-Units/src
)
target_link_libraries(test_logging PRIVATE Threads::Threads)

add_test(NAME LoggingCore COMMAND test_logging)
