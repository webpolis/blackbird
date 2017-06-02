cmake_minimum_required(VERSION 2.8)

project(blackbird)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

macro(use_cxx11)
  if (CMAKE_VERSION VERSION_LESS "3.1")
    set (CMAKE_CXX_FLAGS "--std=c++11 ${CMAKE_CXX_FLAGS}")
  else ()
    set (CMAKE_CXX_STANDARD 11)
  endif ()
endmacro(use_cxx11)
use_cxx11()

include_directories(${PROJECT_SOURCE_DIR}/src)

file(GLOB SOURCES
    src/*.cpp
    src/exchanges/*.cpp
    src/utils/*.cpp)
add_executable(${PROJECT_NAME} ${SOURCES})

find_package(SQLite3)

target_link_libraries(${PROJECT_NAME} ${SQLITE3_LIBRARIES})

find_package(CURL)

target_link_libraries(${PROJECT_NAME} ${CURL_LIBRARIES})

find_package(OpenSSL)

target_link_libraries(${PROJECT_NAME} ${OPENSSL_LIBRARIES})

find_package(OpenSSL)

target_link_libraries(${PROJECT_NAME} ${OPENSSL_LIBRARIES})

find_package(Jansson)

target_link_libraries(${PROJECT_NAME} ${JANSSON_LIBRARIES})
