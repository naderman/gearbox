SET( lib_name GbxNovatelAcfr )
GBX_ADD_LICENSE( LGPL )

SET( build TRUE )
GBX_REQUIRE_OPTION( build LIB ${lib_name} ON )

#INCLUDE( ${GBX_CMAKE_DIR}/FindIceUtil.cmake )
#GBX_REQUIRE_VAR( build LIB ${lib_name} ICEUTIL_FOUND "libIceUtil not found" )

SET( int_libs GbxUtilAcfr GbxSerialAcfr )
GBX_REQUIRE_TARGETS( build LIB ${lib_name} ${int_libs} )

# built internally
SET( private_libs GbxNovatelUtilAcfr )

IF( build )

    ADD_SUBDIRECTORY( gbxnovatelutilacfr)

    INCLUDE( ${GBX_CMAKE_DIR}/UseBasicRules.cmake )
    INCLUDE( ${GBX_CMAKE_DIR}/UseIceUtil.cmake )

    FILE( GLOB hdrs *.h )
    FILE( GLOB srcs *.cpp )
    SET( dep_libs ${int_libs} ${private_libs} )

    GBX_ADD_LIBRARY( ${lib_name} DEFAULT ${srcs} )
    TARGET_LINK_LIBRARIES( ${lib_name} ${dep_libs} )
    GBX_ADD_PKGCONFIG( ${lib_name} "Drives NOVATEL hardware, directly connected to the computer." proj_libs int_libs "" "" )

    GBX_ADD_HEADERS( gbxnovatelacfr ${hdrs} )

    IF( GBX_BUILD_TESTS )
        ADD_SUBDIRECTORY( test )
    ENDIF( GBX_BUILD_TESTS )


ENDIF( build )
