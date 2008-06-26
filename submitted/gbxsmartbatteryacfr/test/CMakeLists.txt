INCLUDE( ${GBX_CMAKE_DIR}/UseBasicRules.cmake )

GBX_ADD_EXECUTABLE( gbxsmartbatterychecksumtest checksumtest.cpp )
GBX_ADD_TEST( GbxSmartBatteryChecksumTest gbxsmartbatterychecksumtest )

GBX_ADD_EXECUTABLE( gbxsmartbatterysimpletest simpletest.cpp )
TARGET_LINK_LIBRARIES( gbxsmartbatterysimpletest GbxSmartBatteryAcfr )

GBX_ADD_EXECUTABLE( gbxsmartbatteryfancytest fancytest.cpp )
TARGET_LINK_LIBRARIES( gbxsmartbatteryfancytest GbxSmartBatteryAcfr )

GBX_ADD_EXAMPLE( gbxsmartbatteryacfr example.cmake.in example.cmake checksumtest.cpp simpletest.cpp fancytest.cpp example.readme )

