INCLUDE( ${GBX_CMAKE_DIR}/UseBasicRules.cmake )

GBX_ADD_EXECUTABLE( gbxsmartbatteryshorttest shorttest.cpp )
TARGET_LINK_LIBRARIES( gbxsmartbatteryshorttest GbxSmartBatteryAcfr )

GBX_ADD_EXECUTABLE( gbxsmartbatterylongtest longtest.cpp )
TARGET_LINK_LIBRARIES( gbxsmartbatterylongtest GbxSmartBatteryAcfr )

GBX_ADD_EXAMPLE( gbxsmartbatteryshorttest example.cmake.in example.cmake shorttest.cpp example.readme )

IF( GBX_BUILD_TESTS )
    ADD_SUBDIRECTORY( darttest )
ENDIF( GBX_BUILD_TESTS )


