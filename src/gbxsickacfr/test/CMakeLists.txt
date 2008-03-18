INCLUDE( ${GBX_CMAKE_DIR}/UseBasicRules.cmake )

GBX_ADD_EXECUTABLE( gbxsickacfrtest test.cpp )
TARGET_LINK_LIBRARIES( gbxsickacfrtest GbxSickAcfr )

GBX_ADD_EXAMPLE( gbxsickacfr example.cmake.in example.cmake test.cpp example.readme )