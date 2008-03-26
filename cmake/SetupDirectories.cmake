#
# Installation directory is determined by looking at 3 sources of information in the following order,
# later sources overwrite earlier ones:
# 1. OS-dependent defaults (effective only the first time CMake runs or after CMakeCache is deleted)
# 2. Enviroment variable whose name is held in variable project_install_var
# 3. CMake variable whose name is held in project_install_var.
#
# E.g.
# - rm CMakeCache.txt; cmake .
#       /opt/orca-1.2.3
# - export HYDRO_INSTALL=/home/myname; cmake .
#       /home/myname
# - export HYDRO_INSTALL=/home/myname; cmake -DHYDRO_INSTALL=/home/myname/opt.
#       /home/myname/opt
#
# Afterwards, it's ok to just use "cmake .", the previously set installation dir is held in cache.
#
# A manually set installation dir (e.i. with ccmake) is not touched until an environment variable or
# a command line variable is introduced.

# 1. using custom defaults (effective only the very first time CMake runs, or after CMakeCache is deleted)
IF( CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    MESSAGE( STATUS "Setting default installation directory..." )
    IF( NOT GBX_OS_WIN )
        SET( CMAKE_INSTALL_PREFIX /usr/local CACHE PATH "Installation directory" FORCE )
    ELSE ( NOT GBX_OS_WIN )
        SET( CMAKE_INSTALL_PREFIX "C:\Program Files\${PROJECT_NAME}\Include" CACHE PATH "Installation directory" FORCE )
    ENDIF( NOT GBX_OS_WIN )
ENDIF( CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT )

# the name of the variable controlling install directory for this project
STRING( TOUPPER ${PROJECT_NAME} project_name_upper )
SET( project_install_var "${project_name_upper}_INSTALL" )

# 2. check if environment variable is set
SET( install_dir $ENV{${project_install_var}} )
STRING( LENGTH "A${install_dir}" is_env_var_defined_plus_one )
MATH( EXPR is_env_var_defined "${is_env_var_defined_plus_one}-1" )
IF( is_env_var_defined )
    # debug
    MESSAGE( STATUS "Overwriting install dir with enviroment variable ${project_install_var}=${install_dir}" )

    SET( CMAKE_INSTALL_PREFIX ${install_dir} CACHE PATH "Installation directory" FORCE )
ENDIF( is_env_var_defined )

# 3. check if CMake variable is set on the command line
IF( DEFINED ${project_install_var} )
    SET( install_dir ${${project_install_var}} )
    # debug
    MESSAGE( STATUS "Overwriting install dir with command line variable ${project_install_var}=${install_dir}" )

    # using user-supplied installation directory
    SET( CMAKE_INSTALL_PREFIX ${install_dir} CACHE PATH "Installation directory" FORCE )
ENDIF( DEFINED ${project_install_var} )
    
# final result
MESSAGE( STATUS "Setting installation directory to ${CMAKE_INSTALL_PREFIX}" )

# special installation directories
SET( GBX_BIN_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/bin )
SET( GBX_LIB_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/lib/${PROJECT_NAME} )
SET( GBX_INCLUDE_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/install/${PROJECT_NAME} )
SET( GBX_SHARE_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/share/${PROJECT_NAME} )

#
# It's sometimes useful to refer to the top level of the project.
# CMake does not make it very easy.
#
SET( GBX_PROJECT_SOURCE_DIR ${${PROJECT_NAME}_SOURCE_DIR} )
SET( GBX_PROJECT_BINARY_DIR ${${PROJECT_NAME}_BINARY_DIR} )
