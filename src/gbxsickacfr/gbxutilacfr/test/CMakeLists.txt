# INCLUDE( ${GBX_CMAKE_DIR}/UseBasicRules.cmake )

LINK_LIBRARIES( GbxUtilAcfr )

ADD_EXECUTABLE( mathtest mathtest.cpp )
GBX_ADD_TEST( GbxUtilAcfrMathTest mathtest )
