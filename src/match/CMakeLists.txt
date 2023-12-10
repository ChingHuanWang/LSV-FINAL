set(SOURCE_FILES
    match.cpp
)

add_library(match STATIC ${SOURCE_FILES})
target_include_directories(match PUBLIC ${PROJECT_SOURCE_DIR}/include/match)
target_include_directories(match PUBLIC ${PROJECT_SOURCE_DIR}/include/sat)
target_include_directories(match PUBLIC ${PROJECT_SOURCE_DIR}/include/util)
target_include_directories(match PUBLIC ${PROJECT_SOURCE_DIR}/include/cir)
