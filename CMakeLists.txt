cmake_minimum_required(VERSION 3.14)
project(search_engine VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_TLS_VERIFY OFF)

add_library(search_engine_lib 
    src/converterJSON.cpp
    src/invertedIndex.cpp
)

target_include_directories(search_engine_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include 
    ${CMAKE_CURRENT_SOURCE_DIR}/nlohmann_json/include
)

add_executable(${PROJECT_NAME} src/main.cpp)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)

include_directories(${gtest_SOURCE_DIR}/include${gtest_SOURCE_DIR})

set(gtest_disable_pthreadson)
set(gtest_disable_pthreads OFF CACHE BOOL "" FORCE)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
enable_testing()
add_executable(search_engine_test tests/tests.cpp)

target_link_libraries(search_engine_test PRIVATE 
    search_engine_lib 
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(search_engine_test)