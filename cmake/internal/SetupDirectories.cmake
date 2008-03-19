#
# This CMake variable may be provided by the user on the command line
# e.g. $ cmake -DGEARBOX_INSTALL=/home/user .
#
IF( CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT )

    IF( DEFINED GEARBOX_INSTALL )
    
        MESSAGE( STATUS GEARBOX_INSTALL=${GEARBOX_INSTALL} )
    
        # using user-supplied installation directory
    #     SET( CMAKE_INSTALL_PREFIX ${GEARBOX_INSTALL} )
        SET( CMAKE_INSTALL_PREFIX ${GEARBOX_INSTALL} CACHE PATH "Installation directory" FORCE )
    
        MESSAGE( STATUS CMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX} )
    
    ELSE( DEFINED GEARBOX_INSTALL )
        #
        # Default installation directory is OS-dependent.
        #
        IF ( NOT GBX_OS_WIN )
            SET( CMAKE_INSTALL_PREFIX /usr/local CACHE PATH "Installation directory" FORCE )
        ELSE ( NOT GBX_OS_WIN )
            SET( CMAKE_INSTALL_PREFIX "C:\Program Files\Gearbox\Include" CACHE PATH "Installation directory" FORCE )
        ENDIF ( NOT GBX_OS_WIN )
    
    ENDIF( DEFINED GEARBOX_INSTALL )

ENDIF( CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT )

MESSAGE( STATUS "Setting installation directory to ${CMAKE_INSTALL_PREFIX}" )

#
# It's sometimes useful to refer to the top level of the project.
# CMake does not make it very easy.
#
SET( GBX_PROJECT_SOURCE_DIR ${${PROJECT_NAME}_SOURCE_DIR} )
SET( GBX_PROJECT_BINARY_DIR ${${PROJECT_NAME}_BINARY_DIR} )
