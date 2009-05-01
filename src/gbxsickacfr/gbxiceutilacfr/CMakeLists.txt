set( lib_name GbxIceUtilAcfr )
GBX_ADD_LICENSE( LGPL )

set( build TRUE )

GBX_REQUIRE_OPTION( build LIB ${lib_name} ON )

include( ${GBX_CMAKE_DIR}/FindIceUtil.cmake )
GBX_REQUIRE_VAR( build LIB ${lib_name} ICEUTIL_FOUND "libIceUtil not found" )

set( proj_libs GbxUtilAcfr )
GBX_REQUIRE_LIBS( build LIB ${lib_name} ${proj_libs} )

if( build )
    
    include( ${GBX_CMAKE_DIR}/UseBasicRules.cmake )
    include( ${GBX_CMAKE_DIR}/UseIceUtil.cmake )

    file( GLOB hdrs *.h )
    file( GLOB srcs *.cpp )
    set( dep_libs ${proj_libs} )

    # for config.h
    include_directories( ${PROJECT_BINARY_DIR} )

    GBX_ADD_LIBRARY( ${lib_name} DEFAULT ${srcs} )
    target_link_libraries( ${lib_name} ${dep_libs} )

    GBX_ADD_HEADERS( gbxsickacfr/gbxiceutilacfr ${hdrs} )

    if( GBX_BUILD_TESTS )
        add_subdirectory( test )
    endif( GBX_BUILD_TESTS )

endif( build )
