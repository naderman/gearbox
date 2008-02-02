#
# Make it easy to include headers from library directories.
# e.g. #include <somelib/somelib.h>
#
INCLUDE_DIRECTORIES( ${GBX_PROJECT_SOURCE_DIR}/src )

#
# Platform-specific compiler and linker flags
#
IF ( NOT OS_WIN )
    ADD_DEFINITIONS( "-Wall" )
ELSE ( NOT OS_WIN )
    ADD_DEFINITIONS( "-Wall -D_CRT_SECURE_NO_DEPRECATE" )
ENDIF ( NOT OS_WIN )
