set(SOURCE_FILES
    cirCmd.cpp
    cirFraig.cpp
    cirGate.cpp
    cirMgr.cpp
    cirOpt.cpp
    cirSim.cpp
    fec.cpp
)

add_library(cir STATIC ${SOURCE_FILES})
target_include_directories(cir PUBLIC ${PROJECT_SOURCE_DIR}/include/cir)
target_include_directories(cir PUBLIC ${PROJECT_SOURCE_DIR}/include/util)
target_include_directories(cir PUBLIC ${PROJECT_SOURCE_DIR}/include/cmd)
target_include_directories(cir PUBLIC ${PROJECT_SOURCE_DIR}/include/sat)
