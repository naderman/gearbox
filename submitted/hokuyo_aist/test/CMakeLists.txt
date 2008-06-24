INCLUDE (${GBX_CMAKE_DIR}/UseBasicRules.cmake)

GBX_ADD_EXECUTABLE (urg_nz_example example.cpp)
TARGET_LINK_LIBRARIES (urg_nz_example urg_nz flexiport)

GBX_ADD_EXAMPLE (urg_nz example.cmake.in example.cmake
	example.cpp example.readme)