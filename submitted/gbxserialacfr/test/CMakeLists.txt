INCLUDE( ${GBX_CMAKE_DIR}/UseBasicRules.cmake )

LINK_LIBRARIES( GbxSerialAcfr )

GBX_ADD_EXECUTABLE( serialechotest serialechotest.cpp )
GBX_ADD_EXECUTABLE( serialloopbacktest serialloopbacktest.cpp )

MESSAGE( "CMAKE_INSTALL_PREFIX: ${CMAKE_INSTALL_PREFIX}" )
GBX_ADD_EXAMPLE( gbxserialacfr example.cmake.in example.cmake serialechotest.cpp serialloopbacktest.cpp )