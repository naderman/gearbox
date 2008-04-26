INCLUDE( ${GBX_CMAKE_DIR}/UseBasicRules.cmake )

LINK_LIBRARIES( GbxGpsUtilAcfr )

ADD_EXECUTABLE( testmga testmga.cpp )
GBX_ADD_TEST( GbxGpsUtilAcfrTestMga testmga )
