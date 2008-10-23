#
# Make it easy to include headers from library directories.
# e.g. #include <somelib/somelib.h>
#
INCLUDE_DIRECTORIES( ${GBX_PROJECT_SOURCE_DIR}/src )

#
# Do the same for the submitted directory when its compilation
# is enabled.
#
IF( GBX_BUILD_SUBMITTED )
    INCLUDE_DIRECTORIES( ${GBX_PROJECT_SOURCE_DIR}/submitted )
ENDIF( GBX_BUILD_SUBMITTED )

#
# Platform-specific compiler and linker flags
#
IF( NOT GBX_OS_WIN )
    ADD_DEFINITIONS( "-Wall" )
#
# AlexB: Using -Wconversion finds some real bugs, but turns up lots of 
#        false positives, especially in gcc4.3.
#        (see: http://gcc.gnu.org/ml/gcc/2008-05/msg00363.html)
#
#    ADD_DEFINITIONS( "-Wall -Wconversion" )
ELSE ( NOT GBX_OS_WIN )
    ADD_DEFINITIONS( "-Wall -D_CRT_SECURE_NO_DEPRECATE" )
ENDIF( NOT GBX_OS_WIN )
