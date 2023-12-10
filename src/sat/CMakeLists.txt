
set(SOURCE_FILES
    File.cpp
    Proof.cpp
    Solver.cpp
)

add_library(sat STATIC ${SOURCE_FILES})
target_include_directories(sat PUBLIC ${PROJECT_SOURCE_DIR}/include/sat)
