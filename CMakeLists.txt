cmake_minimum_required(VERSION 3.20.1)
project(bmatch)

add_subdirectory(src/util)
add_subdirectory(src/cir)

add_executable(${PROJECT_NAME} src/main/bmatch.cpp)

# target_include_directories(${PROJECT_NAME} PUBLIC 
#     ${PROJECT_SOURCE_DIR}/include/abc
# )

target_compile_options(${PROJECT_NAME} PRIVATE
    -Wall
    -Wextra
)

target_link_libraries(${PROJECT_NAME} PRIVATE  
    cir
    util
    ${PROJECT_SOURCE_DIR}/lib/libcmd.a
    ${PROJECT_SOURCE_DIR}/lib/libgenaag.a
    ${PROJECT_SOURCE_DIR}/lib/libabc.a
    -lm
    -ldl
    -lreadline
    -lpthread
)
