SET( lib_name GbxSerialDeviceAcfr )
GBX_ADD_LICENSE( LGPL )

SET( build TRUE )
# don't give user an option
# GBX_REQUIRE_OPTION( build LIB ${lib_name} ON )

# this was already tested in the dir above
# GBX_REQUIRE_VAR( build LIB ${lib_name} GBX_OS_LINUX "only Linux OS is supported" )

# this was already tested in the dir above
# INCLUDE( ${GBX_CMAKE_DIR}/FindIceUtil.cmake )
# GBX_REQUIRE_VAR( build LIB ${lib_name} ICEUTIL_FOUND "libIceUtil not found" )

# this was already tested in the dir above
SET( proj_libs GbxSerialAcfr )
# GBX_REQUIRE_TARGETS( build LIB ${lib_name} ${proj_libs} )

IF( build )
    
    INCLUDE( ${GBX_CMAKE_DIR}/UseBasicRules.cmake )
    INCLUDE( ${GBX_CMAKE_DIR}/UseIceUtil.cmake )

    FILE( GLOB hdrs *.h )
    FILE( GLOB srcs *.cpp )
    SET( dep_libs ${proj_libs} )

    # for config.h
    INCLUDE_DIRECTORIES( ${PROJECT_BINARY_DIR} )

    GBX_ADD_LIBRARY( ${lib_name} SHARED ${srcs} )
    TARGET_LINK_LIBRARIES( ${lib_name} ${dep_libs} )

    GBX_ADD_HEADERS( gbxsickacfr/gbxserialdeviceacfr ${hdrs} )

#     IF( GBX_BUILD_TESTS )
#         ADD_SUBDIRECTORY( test )
#     ENDIF( GBX_BUILD_TESTS )

ENDIF( build )
