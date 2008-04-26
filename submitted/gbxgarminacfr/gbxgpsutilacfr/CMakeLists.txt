SET( lib_name GbxGpsUtilAcfr )
GBX_ADD_LICENSE( LGPL )

SET( build TRUE )
# don't give user an option (while it's an internal library)
# GBX_REQUIRE_OPTION( build LIB ${lib_name} ON )

SET( int_libs GbxUtilAcfr )
GBX_REQUIRE_TARGETS( build LIB ${lib_name} ${int_libs} )

IF( build )

    INCLUDE( ${GBX_CMAKE_DIR}/UseBasicRules.cmake )

    FILE( GLOB hdrs *.h )
    FILE( GLOB srcs *.cpp )
    SET( dep_libs ${int_libs} ${ext_libs} )

    GBX_ADD_LIBRARY( ${lib_name} SHARED ${srcs} )

    TARGET_LINK_LIBRARIES( ${lib_name} ${dep_libs} )

    GBX_ADD_HEADERS( gbxgarminacfr/gbxgpsutilacfr ${hdrs} )
    
    IF( GBX_BUILD_TESTS )
        ADD_SUBDIRECTORY( test )
    ENDIF( GBX_BUILD_TESTS )

ENDIF( build )
