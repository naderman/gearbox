#
# Check the OS type.
# Sets the 'GBX_OS_INCLUDES' variable, for system-wide Includes that must be set.
#


# CMake does not distinguish Linux from other Unices.
STRING( REGEX MATCH Linux GBX_OS_LINUX ${CMAKE_SYSTEM_NAME})

# Rename CMake's variable to something which makes more sense.
IF ( QNXNTO )
    SET( GBX_OS_QNX TRUE BOOL INTERNAL )
ENDIF ( QNXNTO )

# In windows we just mirror CMake's own variable
IF ( WIN32 )
    SET( GBX_OS_WIN TRUE BOOL INTERNAL )
ENDIF ( WIN32 )

# In MacOS X we just mirror CMake's own variable
IF ( APPLE )
    SET( GBX_OS_MAC TRUE BOOL INTERNAL )
ENDIF ( APPLE )


# From now on, use our own OS flags

IF ( GBX_OS_LINUX )
    MESSAGE ( STATUS "Running on Linux" )
ENDIF ( GBX_OS_LINUX )

IF ( GBX_OS_QNX )
    MESSAGE ( STATUS "Running on QNX" )
    ADD_DEFINITIONS( -shared -fexceptions )
ENDIF ( GBX_OS_QNX )

IF ( GBX_OS_WIN )
    # CMake seems not to set this property correctly for some reason
    SET( GBX_EXE_EXTENSION ".exe" )
    MESSAGE ( STATUS "Running on Windows" )
ENDIF ( GBX_OS_WIN )
