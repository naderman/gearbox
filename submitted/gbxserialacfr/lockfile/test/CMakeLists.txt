INCLUDE( ${GBX_CMAKE_DIR}/UseBasicRules.cmake )

ADD_EXECUTABLE( locktest locktest.cpp )
TARGET_LINK_LIBRARIES( locktest GbxLockFileAcfr )
GBX_ADD_TEST( GbxLockFileAcfr locktest )