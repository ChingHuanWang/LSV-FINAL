
set(SOURCE_FILES
    sat.cpp
    File.cpp
    Proof.cpp
    Solver.cpp
)

add_library(sat STATIC ${SOURCE_FILES})
target_include_directories(sat PUBLIC ${PROJECT_SOURCE_DIR}/include/sat)
target_include_directories(sat PUBLIC ${PROJECT_SOURCE_DIR}/include/cir)
target_include_directories(sat PUBLIC ${PROJECT_SOURCE_DIR}/include/util)
