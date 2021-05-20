project(zhbox)

add_definitions(-DPROG=\"${PROJECT_NAME}\")
add_definitions(-D_VERSION_=\"0.0.3\")
add_definitions(-D_POSIX_C_SOURCE=200809L)
add_definitions(-D_DEFAULT_SOURCE)
add_definitions(-std=c99)

FILE(GLOB SRC_FILES "src/*.c")

find_package(PkgConfig REQUIRED)

pkg_check_modules(LIBCFU REQUIRED IMPORTED_TARGET libcfu)
pkg_check_modules(LIBCONFIG REQUIRED IMPORTED_TARGET libconfig)
pkg_check_modules(LIBEVENT_CORE REQUIRED IMPORTED_TARGET libevent_core)
pkg_check_modules(LIBMOSQUITTO REQUIRED IMPORTED_TARGET libmosquitto)
#pkg_check_modules(LIBOPEN62541 REQUIRED IMPORTED_TARGET open62541)
#pkg_check_modules(LIBOPENSSL REQUIRED IMPORTED_TARGET openssl)

SET(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin/lib)

add_library(zhboxobj SHARED
    src/obj.c
    src/class.c
    src/mqtt.c
    src/task.c
    )
target_include_directories(zhboxobj PUBLIC
    ${PROJECT_SOURCE_DIR}/include
    ${LIBCFU_INCLUDE_DIRS}
    ${LIBCONFIG_INCLUDE_DIRS}
    ${LIBMOSQUITTO_INCLUDE_DIRS}
    )
target_link_libraries(zhboxobj PUBLIC
    dl
    ${LIBCFU_LIBRARIES}
    ${LIBCONFIG_LIBRARIES}
    ${LIBMOSQUITTO_LIBRARIES}
    )


SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)
add_executable(${PROJECT_NAME} ${SRC_FILES})
target_include_directories(${PROJECT_NAME} PRIVATE
    #${LIBMOSQUITTO_INCLUDE_DIRS}
    ${CMAKE_BINARY_DIR}
    )
target_link_libraries(${PROJECT_NAME} PRIVATE
    ${LIBCONFIG_LIBRARIES}
    ${LIBEVENT_CORE_LIBRARIES}
    ${LIBMOSQUITTO_LIBRARIES}
    #${LIBOPEN62541_LIBRARIES}
    #${LIBOPENSSL_LIBRARIES}
    zhboxobj
    )

INSTALL(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION /usr/bin/)
