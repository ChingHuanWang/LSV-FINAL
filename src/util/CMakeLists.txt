set(SOURCE_FILES
    aiger.c
    aigToAag.c
    cirParser.cpp
    genAag.cpp
    myGetChar.cpp
    myString.cpp
    util.cpp
)

add_library(util STATIC ${SOURCE_FILES})
target_include_directories(util PUBLIC ${PROJECT_SOURCE_DIR}/include/util)
