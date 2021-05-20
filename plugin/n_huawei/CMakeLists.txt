project(n_huawei)

add_definitions(-DPROG=\"${PROJECT_NAME}\")
add_definitions(-D_VERSION_=\"0.0.1\")

SET(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin/lib)

FILE(GLOB SRC_FILES "src/*.c")

add_library(${PROJECT_NAME} SHARED
    src/huawei.c
    )
target_include_directories(${PROJECT_NAME} PUBLIC
    ${PROJECT_SOURCE_DIR}/include
    )

INSTALL(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION /usr/bin/)
