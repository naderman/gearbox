# A place for libraries that have been retired from the source tree.

OPTION ( GBX_BUILD_RETIRED "Build retired libraries" OFF )
MARK_AS_ADVANCED ( FORCE GBX_BUILD_RETIRED )

IF( GBX_BUILD_RETIRED )

    MESSAGE( STATUS "== RETIRED ==" )

    # When adding new directories, please maintain order of inter-dependencies.
    # Otherwise, maintain alphabetical order.

    ADD_SUBDIRECTORY( urg_nz )

ENDIF( GBX_BUILD_RETIRED )
