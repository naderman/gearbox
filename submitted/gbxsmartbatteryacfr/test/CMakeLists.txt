INCLUDE( ${GBX_CMAKE_DIR}/UseBasicRules.cmake )

GBX_ADD_EXECUTABLE( gbxsmartbatterysimpletest simpletest.cpp )
TARGET_LINK_LIBRARIES( gbxsmartbatterysimpletest GbxSmartBatteryAcfr )

GBX_ADD_EXECUTABLE( gbxsmartbatteryfancytest fancytest.cpp )
TARGET_LINK_LIBRARIES( gbxsmartbatteryfancytest GbxSmartBatteryAcfr )

GBX_ADD_EXAMPLE( gbxsmartbatteryacfr example.cmake.in example.cmake simpletest.cpp fancytest.cpp example.readme )

IF( GBX_BUILD_TESTS )
    ADD_SUBDIRECTORY( darttest )
ENDIF( GBX_BUILD_TESTS )


