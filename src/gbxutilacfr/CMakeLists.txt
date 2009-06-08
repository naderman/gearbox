set( lib_name GbxUtilAcfr )
set( lib_version 1.0.0 )
GBX_ADD_LICENSE( LGPL )

set( build TRUE )
GBX_REQUIRE_OPTION( build LIB ${lib_name} ON )

if( build )
    
    include( ${GBX_CMAKE_DIR}/UseBasicRules.cmake )

    file( GLOB hdrs *.h )
    file( GLOB srcs *.cpp )

    # for config.h
    include_directories( ${PROJECT_BINARY_DIR} )

    GBX_ADD_LIBRARY( ${lib_name} DEFAULT ${lib_version} ${srcs} )

    GBX_ADD_HEADERS( gbxutilacfr ${hdrs} )

    if( GBX_BUILD_TESTS )
        add_subdirectory( test )
    endif( GBX_BUILD_TESTS )

endif( build )
