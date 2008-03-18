SET ( lib_name GbxSerialAcfr )
GBX_ADD_LICENSE( LGPL )

SET ( build TRUE )
GBX_REQUIRE_OPTION( build LIB ${lib_name} ON )
GBX_REQUIRE_VAR( build LIB ${lib_name} GBX_OS_LINUX "only Linux OS is supported" )

SET( dep_libs GbxLockFileAcfr )
# this is currently internal, so we don't have to check it
# GBX_REQUIRE_TARGETS( build LIB ${lib_name} ${dep_libs} )

IF ( build )

    ADD_SUBDIRECTORY( lockfile )
    
    INCLUDE( ${GBX_CMAKE_DIR}/UseBasicRules.cmake )

    # for config.h
    INCLUDE_DIRECTORIES( ${PROJECT_BINARY_DIR} )

    FILE( GLOB hdrs *.h )
    FILE( GLOB srcs *.cpp )

    GBX_ADD_LIBRARY( ${lib_name} SHARED ${srcs} )
    TARGET_LINK_LIBRARIES( ${lib_name} ${dep_libs} )
    GBX_ADD_PKGCONFIG( ${lib_name} "C++ class wrapping a serial port." "" dep_libs "" "" )

    GBX_ADD_HEADERS( gbxserialacfr ${hdrs} )

    IF ( GBX_BUILD_TESTS )
        ADD_SUBDIRECTORY ( test )
    ENDIF ( GBX_BUILD_TESTS )

ENDIF ( build )

