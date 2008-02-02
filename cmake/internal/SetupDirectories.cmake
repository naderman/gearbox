#
# Default installation directory is OS-dependent.
#
IF ( NOT GBX_OS_WIN )
    SET( CMAKE_INSTALL_PREFIX /usr/local CACHE PATH "Installation directory" )
ELSE ( NOT GBX_OS_WIN )
    SET( CMAKE_INSTALL_PREFIX "C:\Program Files\Gearbox\Include" CACHE PATH "Installation directory" )
ENDIF ( NOT GBX_OS_WIN )

MESSAGE( STATUS "Setting installation directory to ${CMAKE_INSTALL_PREFIX}" )

#
# It's sometimes useful to refer to the top level of the project.
# CMake does not make it very easy.
#
SET( GBX_PROJECT_SOURCE_DIR ${${PROJECT_NAME}_SOURCE_DIR} )
SET( GBX_PROJECT_BINARY_DIR ${${PROJECT_NAME}_BINARY_DIR} )
