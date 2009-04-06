#
# Check the OS type.
# Sets the 'GBX_OS_INCLUDES' variable, for system-wide Includes that must be set.
#


# CMake does not distinguish Linux from other Unices.
string( REGEX MATCH Linux GBX_OS_LINUX ${CMAKE_SYSTEM_NAME})

# Rename CMake's variable to something which makes more sense.
if( QNXNTO )
    set( GBX_OS_QNX TRUE BOOL INTERNAL )
endif( QNXNTO )

# In windows we just mirror CMake's own variable
if( WIN32 )
    set( GBX_OS_WIN TRUE BOOL INTERNAL )
endif( WIN32 )

# In MacOS X we just mirror CMake's own variable
if( APPLE )
    set( GBX_OS_MAC TRUE BOOL INTERNAL )
endif( APPLE )


# From now on, use our own OS flags

if( GBX_OS_LINUX )
    message( STATUS "Running on Linux" )
endif( GBX_OS_LINUX )

if( GBX_OS_QNX )
    message( STATUS "Running on QNX" )
    add_definitions( -shared -fexceptions )
endif( GBX_OS_QNX )

if( GBX_OS_WIN )
    # CMake seems not to set this property correctly for some reason
    set( GBX_EXE_EXTENSION ".exe" )
    message( STATUS "Running on Windows" )
endif( GBX_OS_WIN )

if( GBX_OS_MAC )
    message( STATUS "Running on OS-X" )
endif( GBX_OS_MAC )
