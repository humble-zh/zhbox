cmake_minimum_required(VERSION 3.5)
project(zhbox)

set (${PROJECT_NAME}_VERSION_MAJOR 0)
set (${PROJECT_NAME}_VERSION_MINOR 0)
set (${PROJECT_NAME}_VERSION_PATCH 1)
set (${PROJECT_NAME}_VERSION
    "${${PROJECT_NAME}_VERSION_MAJOR}.${${PROJECT_NAME}_VERSION_MINOR}.${${PROJECT_NAME}_VERSION_PATCH}")

add_definitions(-DPROG=\"${PROJECT_NAME}\")
add_definitions(-D_POSIX_C_SOURCE)
add_definitions(-std=c99)

configure_file(ver.h.in ${PROJECT_BINARY_DIR}/ver.h)

FILE(GLOB HELLO_SRC_FILES "src/*.c")

find_package(PkgConfig REQUIRED)

pkg_check_modules(LIBEVENT_CORE REQUIRED IMPORTED_TARGET libevent_core)

add_executable(${PROJECT_NAME} ${HELLO_SRC_FILES})
#target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_BINARY_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE
    ${LIBEVENT_CORE_LIBRARIES}
)

INSTALL(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION /usr/bin/)
