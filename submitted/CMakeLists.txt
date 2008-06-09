# Libraries submitted for consideration for inclusion in Gearbox.

OPTION ( GBX_BUILD_SUBMITTED "Build submitted libraries" OFF )
MARK_AS_ADVANCED ( FORCE GBX_BUILD_SUBMITTED )

IF( GBX_BUILD_SUBMITTED )

    MESSAGE( STATUS "== SUBMITTED ==" )

    # When adding new directories, please maintain order of inter-dependencies.
    # Otherwise, maintain alphabetical order.
    # E.g. ADD_SUBDIRECTORY( mydir )


ENDIF( GBX_BUILD_SUBMITTED )