SET( lib_name GbxSickAcfr )
GBX_ADD_LICENSE( LGPL )

SET( build TRUE )
GBX_REQUIRE_OPTION( build LIB ${lib_name} ON )
GBX_REQUIRE_VAR( build LIB ${lib_name} GBX_OS_LINUX "only Linux OS is supported" )

INCLUDE( ${GBX_CMAKE_DIR}/FindIceUtil.cmake )
GBX_REQUIRE_VAR( build LIB ${lib_name} ICEUTIL_FOUND "libIceUtil not found" )

SET( int_libs GbxUtilAcfr GbxSerialAcfr )
GBX_REQUIRE_TARGETS( build LIB ${lib_name} ${int_libs} )

# these are built internally
SET( private_libs GbxIceUtilAcfr GbxSerialDeviceAcfr )

IF( build )

    ADD_SUBDIRECTORY( gbxiceutilacfr )
    ADD_SUBDIRECTORY( gbxserialdeviceacfr )
    
    INCLUDE( ${GBX_CMAKE_DIR}/UseBasicRules.cmake )
    INCLUDE( ${GBX_CMAKE_DIR}/UseIceUtil.cmake )

    FILE( GLOB hdrs *.h )
    FILE( GLOB srcs *.cpp )
    SET( dep_libs ${int_libs} ${private_libs} )

    GBX_ADD_LIBRARY( ${lib_name} SHARED ${srcs} )
    TARGET_LINK_LIBRARIES( ${lib_name} ${dep_libs} )
    GBX_ADD_PKGCONFIG( ${lib_name} "Drives SICK hardware, directly connected to the computer." proj_libs int_libs "" "" )

    GBX_ADD_HEADERS( gbxsickacfr ${hdrs} )

    IF( GBX_BUILD_TESTS )
        ADD_SUBDIRECTORY( test )
    ENDIF( GBX_BUILD_TESTS )

ENDIF( build )
