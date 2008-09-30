INCLUDE (${GBX_CMAKE_DIR}/UseBasicRules.cmake)

GBX_ADD_EXECUTABLE (hokuyo_aist_example example.cpp)
IF (NOT GBX_DEFAULT_LIB_TYPE STREQUAL SHARED)
	ADD_DEFINITIONS (-D
ENDIF (NOT GBX_DEFAULT_LIB_TYPE STREQUAL SHARED)
TARGET_LINK_LIBRARIES (hokuyo_aist_example hokuyo_aist flexiport)

GBX_ADD_EXAMPLE (hokuyo_aist example.cmake.in example.cmake
	example.cpp example.readme example.logr example.logw)