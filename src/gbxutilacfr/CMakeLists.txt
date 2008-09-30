SET( lib_name GbxUtilAcfr )
GBX_ADD_LICENSE( LGPL )

SET( build TRUE )
GBX_REQUIRE_OPTION( build LIB ${lib_name} ON )

IF( build )
    
    INCLUDE( ${GBX_CMAKE_DIR}/UseBasicRules.cmake )

    FILE( GLOB hdrs *.h )
    FILE( GLOB srcs *.cpp )

    # for config.h
    INCLUDE_DIRECTORIES( ${PROJECT_BINARY_DIR} )

    GBX_ADD_LIBRARY( ${lib_name} DEFAULT ${srcs} )

    GBX_ADD_HEADERS( gbxutilacfr ${hdrs} )

    IF( GBX_BUILD_TESTS )
        ADD_SUBDIRECTORY( test )
    ENDIF( GBX_BUILD_TESTS )

ENDIF( build )
