INCLUDE( ${GBX_CMAKE_DIR}/UseBasicRules.cmake )

GBX_ADD_EXECUTABLE( checksumtest checksumtest.cpp )
TARGET_LINK_LIBRARIES( checksumtest )

GBX_ADD_EXECUTABLE( simpletest simpletest.cpp )
TARGET_LINK_LIBRARIES( simpletest GbxSmartBatteryAcfr )

GBX_ADD_EXECUTABLE( fancytest fancytest.cpp )
TARGET_LINK_LIBRARIES( fancytest GbxSmartBatteryAcfr )

GBX_ADD_EXAMPLE( gbxsmartbatteryacfr example.cmake.in example.cmake checksumtest.cpp simpletest.cpp fancytest.cpp example.readme )

