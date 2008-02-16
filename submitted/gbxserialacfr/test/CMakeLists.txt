INCLUDE( ${GBX_CMAKE_DIR}/UseBasicRules.cmake )

LINK_LIBRARIES( GbxSerialAcfr )

GBX_ADD_EXECUTABLE( serialechotest serialechotest.cpp )

GBX_ADD_EXECUTABLE( serialloopbacktest serialloopbacktest.cpp )
