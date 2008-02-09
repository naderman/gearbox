#
# Make it easy to include headers from library directories.
# e.g. #include <somelib/somelib.h>
#
INCLUDE_DIRECTORIES( ${GBX_PROJECT_SOURCE_DIR}/src )

#
# Do the same for the submitted directory when its compilation
# is enabled.
#
IF ( GBX_BUILD_SUBMITTED )
    INCLUDE_DIRECTORIES( ${GBX_PROJECT_SOURCE_DIR}/submitted )
ENDIF ( GBX_BUILD_SUBMITTED )

#
# Platform-specific compiler and linker flags
#
IF ( NOT OS_WIN )
    ADD_DEFINITIONS( "-Wall" )
ELSE ( NOT OS_WIN )
    ADD_DEFINITIONS( "-Wall -D_CRT_SECURE_NO_DEPRECATE" )
ENDIF ( NOT OS_WIN )
