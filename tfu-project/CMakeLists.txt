cmake_minimum_required(VERSION 3.10)
project(tfu-project LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OSX_ARCHITECTURES "${CMAKE_SYSTEM_PROCESSOR}")

include_directories(${CMAKE_SOURCE_DIR}/include)

set(LIB_SOURCES
    src/socket/isocket.cpp
    src/crypto/integrity.cpp
    src/io/file_handler.cpp
)

add_library(tfu_core STATIC ${LIB_SOURCES})

if(WIN32)
    target_link_libraries(tfu_core ws2_32)
endif()

add_executable(tfu_sender   src/sender.cpp)
add_executable(tfu_receiver src/receiver.cpp)

target_link_libraries(tfu_sender   tfu_core)
target_link_libraries(tfu_receiver tfu_core)

# Tests
enable_testing()

add_executable(test_integrity    tests/test_integrity.cpp)
add_executable(test_file_handler tests/test_file_handler.cpp)
add_executable(test_protocol     tests/test_protocol.cpp)

target_link_libraries(test_integrity    tfu_core)
target_link_libraries(test_file_handler tfu_core)
target_link_libraries(test_protocol     tfu_core)

add_test(NAME integrity    COMMAND test_integrity)
add_test(NAME file_handler COMMAND test_file_handler)
add_test(NAME protocol     COMMAND test_protocol)
