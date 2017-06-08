cmake_minimum_required(VERSION 3.1)

project(blackbird)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
include(cotire)

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

find_package(Jansson)

set_target_properties(${PROJECT_NAME} PROPERTIES
                                      CXX_STANDARD 11
                                      CXX_STANDARD_REQUIRED YES
                                      CXX_EXTENSIONS NO)

target_include_directories (${PROJECT_NAME} PRIVATE
                            ${PROJECT_SOURCE_DIR}/src
                            ${SQLITE3_INCLUDE_DIR}
                            ${CURL_INCLUDE_DIR}
                            ${OPENSSL_INCLUDE_DIR}
                            ${JANSSON_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} ${JANSSON_LIBRARIES})
cotire(${PROJECT_NAME})
