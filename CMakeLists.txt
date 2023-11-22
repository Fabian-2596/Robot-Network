cmake_minimum_required(VERSION 3.0.0)
project(VS-Praktikum VERSION 0.1.0 LANGUAGES C CXX)

include(CTest)
enable_testing()

add_executable(VS-Praktikum app/Server.cpp)
target_include_directories(VS-Praktikum PRIVATE app/DB/DB.cpp)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

add_executable(testP1 test/testP1.cpp)
target_include_directories(testP1 PRIVATE cmake/include)
add_test(NAME healthCheck COMMAND testP1 test/testP1.cpp)
