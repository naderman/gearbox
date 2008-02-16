SET ( lib_name GbxLockFileAcfr )
GBX_ADD_LICENSE( LGPL )

SET ( build TRUE )
# don't give user an option
# GBX_REQUIRE_OPTION( build LIB ${lib_name} ON )
# this was already tested in the dir above
# GBX_REQUIRE_VAR( build LIB ${lib_name} GBX_OS_LINUX "only Linux OS is supported" )

IF ( build )
    
    INCLUDE( ${GBX_CMAKE_DIR}/UseBasicRules.cmake )

    FILE( GLOB hdrs *.h )
    FILE( GLOB srcs *.cpp )

    GBX_ADD_LIBRARY( ${lib_name} SHARED ${srcs} )
    TARGET_LINK_LIBRARIES( ${lib_name} ${dep_libs} )

    GBX_ADD_HEADERS( gbxserialacfr/lockfile ${hdrs} )

    IF ( GBX_BUILD_TESTS )
        ADD_SUBDIRECTORY ( test )
    ENDIF ( GBX_BUILD_TESTS )

ENDIF ( build )
