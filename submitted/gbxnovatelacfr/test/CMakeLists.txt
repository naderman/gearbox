INCLUDE( ${GBX_CMAKE_DIR}/UseBasicRules.cmake )

GBX_ADD_EXECUTABLE( gbxnovatelacfrtest test.cpp )
TARGET_LINK_LIBRARIES( gbxnovatelacfrtest GbxNovatelAcfr )

GBX_ADD_EXAMPLE( gbxnovatelacfr example.cmake.in example.cmake test.cpp example.readme )
