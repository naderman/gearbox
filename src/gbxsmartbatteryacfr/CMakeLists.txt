set( lib_name GbxSmartBatteryAcfr )
GBX_ADD_LICENSE( LGPL )

set( build TRUE )
GBX_REQUIRE_OPTION( build LIB ${lib_name} ON )
GBX_REQUIRE_VAR( build LIB ${lib_name} GBX_OS_LINUX "only Linux OS is supported" )

set( dep_libs GbxUtilAcfr GbxSerialAcfr )
GBX_REQUIRE_LIBS( build LIB ${lib_name} ${dep_libs} )

if( build )

    include( ${GBX_CMAKE_DIR}/UseBasicRules.cmake )
    
    file( GLOB hdrs *.h )
    file( GLOB srcs *.cpp )
    
    GBX_ADD_LIBRARY( ${lib_name} DEFAULT ${srcs} )
    target_link_libraries( ${lib_name} ${dep_libs} )
    
    GBX_ADD_HEADERS( gbxsmartbatteryacfr ${hdrs} )
    
    if( GBX_BUILD_TESTS )
        add_subdirectory( test )
    endif( GBX_BUILD_TESTS )

endif( build )
