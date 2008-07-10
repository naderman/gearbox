INCLUDE (${GBX_CMAKE_DIR}/UseBasicRules.cmake)

IF (NOT WIN32)
	GBX_ADD_EXECUTABLE (porttoport porttoport.cpp)
	TARGET_LINK_LIBRARIES (porttoport flexiport)

GBX_ADD_EXAMPLE (flexiport/utils utils.cmake.in utils.cmake
	porttoport.cpp utils.readme)
ENDIF (NOT WIN32)
