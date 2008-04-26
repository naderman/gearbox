SET( lib_name GbxGarminAcfr )
GBX_ADD_LICENSE( LGPL )

SET( build TRUE )
GBX_REQUIRE_OPTION( build LIB ${lib_name} ON )
GBX_REQUIRE_VAR( build LIB ${lib_name} GBX_OS_LINUX "only Linux OS is supported" )

SET( int_libs GbxUtilAcfr GbxSerialAcfr ) 
# SET( int_libs GbxUtilAcfr GbxSerialAcfr GbxGpsUtilAcfr ) 
GBX_REQUIRE_TARGETS( build LIB ${lib_name} ${int_libs} )

IF( build )

    ADD_SUBDIRECTORY( gbxgpsutilacfr )

    INCLUDE( ${GBX_CMAKE_DIR}/UseBasicRules.cmake )
    
    FILE( GLOB hdrs *.h )
    FILE( GLOB srcs *.cpp )
    SET( dep_libs ${int_libs} )
    
    GBX_ADD_LIBRARY( ${lib_name} SHARED ${srcs} )
    TARGET_LINK_LIBRARIES( ${lib_name} ${dep_libs} )
    GBX_ADD_PKGCONFIG( ${lib_name} "Garmin GPS driver" "" dep_libs "" "" )

    GBX_ADD_HEADERS( gbxgarminacfr ${hdrs} )

    IF( GBX_BUILD_TESTS )
        ADD_SUBDIRECTORY( test )
    ENDIF( GBX_BUILD_TESTS )

ENDIF( build )
