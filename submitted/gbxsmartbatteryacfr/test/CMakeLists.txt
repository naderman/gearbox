INCLUDE( ${GBX_CMAKE_DIR}/UseBasicRules.cmake )

GBX_ADD_EXECUTABLE( gbxsmartbatteryacfrtest test.cpp )
TARGET_LINK_LIBRARIES( gbxsmartbatteryacfrtest GbxSmartBatteryAcfr )

GBX_ADD_EXAMPLE( gbxsmartbatteryacfrtest example.cmake.in example.cmake test.cpp example.readme )

IF( GBX_BUILD_TESTS )
    ADD_SUBDIRECTORY( darttest )
ENDIF( GBX_BUILD_TESTS )


