SET( lib_name GbxNovatelUtilAcfr )
GBX_ADD_LICENSE( LGPL )

SET( build TRUE )
# don't give user an option
# GBX_REQUIRE_OPTION( build LIB ${lib_name} ON )

# this was already tested in the dir above
# GBX_REQUIRE_VAR( build LIB ${lib_name} GBX_OS_LINUX "only Linux OS is supported" )

# this was already tested in the dir above
# INCLUDE( ${GBX_CMAKE_DIR}/FindIceUtil.cmake )
# GBX_REQUIRE_VAR( build LIB ${lib_name} ICEUTIL_FOUND "libIceUtil not found" )

SET( dep_libs GbxUtilAcfr GbxSerialAcfr )
GBX_REQUIRE_TARGETS( build LIB ${lib_name} ${int_libs} )

IF( build )
    
    INCLUDE( ${GBX_CMAKE_DIR}/UseBasicRules.cmake )

    FILE( GLOB hdrs *.h )
    FILE( GLOB srcs *.cpp )

    # for config.h
    INCLUDE_DIRECTORIES( ${PROJECT_BINARY_DIR} )

    GBX_ADD_LIBRARY( ${lib_name} DEFAULT ${srcs} )
    TARGET_LINK_LIBRARIES( ${lib_name} ${dep_libs} )

    GBX_ADD_HEADERS( gbxnovatelacfr/gbxnovatelutilacfr ${hdrs} )

    IF( GBX_BUILD_TESTS )
        ADD_SUBDIRECTORY( test )
    ENDIF( GBX_BUILD_TESTS )

ENDIF( build )
