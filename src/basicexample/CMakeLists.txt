SET( lib_name basicexample )
GBX_ADD_LICENSE( LGPL )

SET( build TRUE )
GBX_REQUIRE_OPTION( build LIB ${lib_name} OFF )

IF( build )
    
    INCLUDE( ${GBX_CMAKE_DIR}/UseBasicRules.cmake )

    FILE( GLOB hdrs *.h )
    FILE( GLOB srcs *.cpp )

    GBX_ADD_LIBRARY( ${lib_name} SHARED ${srcs} )

    GBX_ADD_HEADERS( basicexample ${hdrs} )
    # the custom command above is equivalent to this
#     INSTALL( FILES ${hdrs} DESTINATION include/gearbox/basicexample )

#     IF( GBX_BUILD_TESTS )
#         ADD_SUBDIRECTORY( test )
#     ENDIF( GBX_BUILD_TESTS )

ENDIF( build )
