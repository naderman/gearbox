#
# utility macro
#
MACRO( GBX_MAKE_OPTION_NAME option_name module_type module_name )

    STRING( COMPARE EQUAL ${module_type} "EXE" is_exe )
    STRING( COMPARE EQUAL ${module_type} "LIB" is_lib )
    IF( NOT is_exe AND NOT is_lib )
        MESSAGE( FATAL_ERROR "In macro GBX_MAKE_OPTION_NAME, module_type must be either 'EXE' or 'LIB'" )
    ENDIF( NOT is_exe AND NOT is_lib )
    IF( is_exe AND is_lib )
        MESSAGE( FATAL_ERROR "In macro GBX_MAKE_OPTION_NAME, module_type must be either 'EXE' or 'LIB'" )
    ENDIF( is_exe AND is_lib )

    STRING( TOUPPER ${module_name} module_name_upper )
    IF( is_exe )
        SET( option_name "ENABLE_${module_name_upper}" )
    ELSE ( is_exe )
        SET( option_name "ENABLE_LIB_${module_name_upper}" )
    ENDIF( is_exe )

ENDMACRO( GBX_MAKE_OPTION_NAME option_name module_name )

#
# GBX_REQUIRE_OPTION( cumulative_var [EXE | LIB] module_name default_option_value [option_name] [OPTION DESCRIPTION] )
#
# E.g.
# Initialize a variable first
#   SET( BUILD TRUE )
# Now set up and test option value
#   GBX_REQUIRE_OPTION ( BUILD EXE localiser ON )
# This does the same thing
#   GBX_REQUIRE_OPTION ( BUILD EXE localiser ON BUILD_LOCALISER )
#
MACRO( GBX_REQUIRE_OPTION cumulative_var module_type module_name default_option_value )

    STRING( COMPARE EQUAL ${module_type} "EXE" is_exe )
    STRING( COMPARE EQUAL ${module_type} "LIB" is_lib )
    IF( NOT is_exe AND NOT is_lib )
        MESSAGE( FATAL_ERROR "In macro GBX_REQUIRE_OPTION, module_type must be either 'EXE' or 'LIB'" )
    ENDIF( NOT is_exe AND NOT is_lib )
    IF( is_exe AND is_lib )
        MESSAGE( FATAL_ERROR "In macro GBX_REQUIRE_OPTION, module_type must be either 'EXE' or 'LIB'" )
    ENDIF( is_exe AND is_lib )

    IF( ${ARGC} GREATER 5 )
        SET( option_name ${ARGV6} )
    ELSE ( ${ARGC} GREATER 5 )
        STRING( TOUPPER ${module_name} module_name_upper )
        IF( is_exe )
            SET( option_name "ENABLE_${module_name_upper}" )
        ELSE ( is_exe )
            SET( option_name "ENABLE_LIB_${module_name_upper}" )
        ENDIF( is_exe )
    ENDIF( ${ARGC} GREATER 5 )

    IF( ${ARGC} GREATER 6 )
        SET( option_descr ${ARGV7} )
    ELSE ( ${ARGC} GREATER 6 )
        SET( option_descr "disabled by user, use ccmake to enable" )
    ENDIF( ${ARGC} GREATER 6 )

    # debug
#     MESSAGE( STATUS
#         "GBX_REQUIRE_OPTION (CUM_VAR=${cumulative_var}, MOD_TYPE=${module_type}, MOD_NAME=${module_name}, default_option_value=${default_option_value}, OPT_NAME=${option_name}, OPT_DESC=${option_descr})" )

    # set up the option
    IF( is_exe )
        OPTION( ${option_name} "Try to build ${module_name}" ${default_option_value} )
    ELSE ( is_exe )
        OPTION( ${option_name} "Try to build lib${module_name} library" ${default_option_value} )
    ENDIF( is_exe )

    # add option to the list: this has nothing to do with the build system.
    # it is useful to have a text list of all options if you want to build a particular
    # configuration from the command line.
    SET( templist ${OPTION_LIST} )
    # (escaping \)
    LIST( APPEND templist "${option_name}=${default_option_value}" )
    SET( OPTION_LIST ${templist} CACHE INTERNAL "Global list of cmake options" FORCE )

    # must dereference both var and option names once (!) and IF will evaluate their values
    IF( ${cumulative_var} AND NOT ${option_name}  )
        SET( ${cumulative_var} FALSE )
        IF( is_exe )
            GBX_NOT_ADD_EXECUTABLE( ${module_name} ${option_descr} )
        ELSE ( is_exe )
            GBX_NOT_ADD_LIBRARY( ${module_name} ${option_descr} )
        ENDIF( is_exe )
    ENDIF( ${cumulative_var} AND NOT ${option_name} )

ENDMACRO( GBX_REQUIRE_OPTION cumulative_var module_type module_name default_option_value )

#
# GBX_REQUIRE_VAR ( cumulative_var [EXE | LIB] module_name test_var reason )
#
# E.g.
# Initialize a variable first
#   SET( BUILD TRUE )
# Now test the variable value
#   GBX_REQUIRE_VAR ( BUILD LIB HydroStuff GOOD_TO_GO "good-to-go is no good" )
#
MACRO( GBX_REQUIRE_VAR cumulative_var module_type module_name test_var reason )

    # debug
#     MESSAGE( STATUS "GBX_REQUIRE_VAR [ CUM_VAR=${cumulative_var}, MOD_TYPE=${module_type}, MOD_NAME=${module_name}, test_var=${${test_var}}, reason=${reason} ]" )

    STRING( COMPARE EQUAL ${module_type} "EXE" is_exe )
    STRING( COMPARE EQUAL ${module_type} "LIB" is_lib )
    IF( NOT is_exe AND NOT is_lib )
        MESSAGE( FATAL_ERROR "In macro GBX_REQUIRE_VAR, module_type must be either 'EXE' or 'LIB'" )
    ENDIF( NOT is_exe AND NOT is_lib )
    IF( is_exe AND is_lib )
        MESSAGE( FATAL_ERROR "In macro GBX_REQUIRE_VAR, module_type must be either 'EXE' or 'LIB'" )
    ENDIF( is_exe AND is_lib )

    # must dereference both var names once (!) and IF will evaluate their values
    IF( ${cumulative_var} AND NOT ${test_var}  )
        SET( ${cumulative_var} FALSE )
        IF( is_exe )
            GBX_NOT_ADD_EXECUTABLE( ${module_name} ${reason} )
        ELSE ( is_exe )
            GBX_NOT_ADD_LIBRARY( ${module_name} ${reason} )
        ENDIF( is_exe )
    ENDIF( ${cumulative_var} AND NOT ${test_var} )

ENDMACRO( GBX_REQUIRE_VAR cumulative_var module_type module_name test_var reason )

#
# GBX_REQUIRE_INSTALL ( cumulative_var [EXE | LIB] module_name installed_module [reason] )
#
# A special case of REQUIRE_VAR. Checks whether a manifest variable is defined 
# for the module with a name "installed_module".
# E.g.
# Initialize a variable first
#   SET( BUILD TRUE )
# Now test the variable value
#   REQUIRE_INSTALL ( build LIB HydroStuff GbxStuff )
#           will check if GBXSTUFF_INSTALLED is defined.
#   This example is equivalent to
#   REQUIRE_VAR( build LIB HydroStuff GBXSTUFF_INSTALLED "GbxStuff was not installed" )
#
MACRO( GBX_REQUIRE_INSTALL cumulative_var module_type module_name installed_module )

    # debug
#     MESSAGE( STATUS "REQUIRE_INSTALL [ CUM_VAR=${cumulative_var}, MOD_TYPE=${module_type}, MOD_NAME=${module_name}, installed_module=${installed_module} ]" )

    STRING( TOUPPER ${installed_module} upper_installed_module )
    SET( test_var ${upper_installed_module}_INSTALLED )

    IF( ${ARGC} GREATER 5 )
        SET( reason ${ARGV6} )
    ELSE ( ${ARGC} GREATER 5 )
        SET( reason "${installed_module} was not installed" )
    ENDIF( ${ARGC} GREATER 5 )

    # must dereference both var names once (!)
    GBX_REQUIRE_VAR( ${cumulative_var} ${module_type} ${module_name} ${test_var} ${reason} )

ENDMACRO( GBX_REQUIRE_INSTALL cumulative_var module_type module_name installed_module )

#
# GBX_REQUIRE_INSTALLS( cumulative_var [EXE | LIB] module_name target0 [targe1 target2 ...] )
#
MACRO( GBX_REQUIRE_INSTALLS cumulative_var module_type module_name )
    
    IF( ${ARGC} LESS 4 )
        MESSAGE( FATAL_ERROR "GBX_REQUIRE_INSTALLS macro needs to at least one target name (${ARGC} params were given)." ) 
    ENDIF( ${ARGC} LESS 4 )

    FOREACH( TRGT ${ARGN} )
        GBX_REQUIRE_INSTALL( ${cumulative_var} ${module_type} ${module_name} ${TRGT} )
    ENDFOREACH( TRGT ${ARGN} )

ENDMACRO( GBX_REQUIRE_INSTALLS cumulative_var module_type module_name )

#
# GBX_REQUIRE_TARGET( cumulative_var [EXE | LIB] module_name target_name [reason] )
#DEPS
# E.g.
# Initialize a variable first
#   SET( BUILD TRUE )
# Now set up and test option value
#   GBX_REQUIRE_TARGET ( BUILD EXE localiser HydroStuff )
#
MACRO( GBX_REQUIRE_TARGET cumulative_var module_type module_name target_name )

    # debug
#     MESSAGE( STATUS "GBX_REQUIRE_TARGET [ CUM_VAR=${${cumulative_var}}, MOD_TYPE=${module_type}, MOD_NAME=${module_name}, target_name=${target_name} ]" )

    STRING( COMPARE EQUAL ${module_type} "EXE" is_exe )
    STRING( COMPARE EQUAL ${module_type} "LIB" is_lib )
    IF( NOT is_exe AND NOT is_lib )
        MESSAGE( FATAL_ERROR "In macro GBX_REQUIRE_TARGET, module_type must be either 'EXE' or 'LIB'" )
    ENDIF( NOT is_exe AND NOT is_lib )
    IF( is_exe AND is_lib )
        MESSAGE( FATAL_ERROR "In macro GBX_REQUIRE_TARGET, module_type must be either 'EXE' or 'LIB'" )
    ENDIF( is_exe AND is_lib )

    IF( ${ARGC} GREATER 5 )
        SET( reason ${ARGV6} )
    ELSE ( ${ARGC} GREATER 5 )
        SET( reason "lib${target_name} is not being built" )
    ENDIF( ${ARGC} GREATER 5 )

    GET_TARGET_PROPERTY( target_location ${target_name} LOCATION )

    # must dereference both var and option names once (!) and IF will evaluate their values
    IF( ${cumulative_var} AND NOT target_location  )
        SET( ${cumulative_var} FALSE )
        GBX_MAKE_OPTION_NAME( option_name ${module_type} ${module_name} )
        IF( is_exe )
            GBX_NOT_ADD_EXECUTABLE( ${module_name} ${reason} )
            SET( ${option_name} OFF CACHE BOOL "Try to build ${module_name}" FORCE )
        ELSE ( is_exe )
            GBX_NOT_ADD_LIBRARY( ${module_name} ${reason} )
            SET( ${option_name} OFF CACHE BOOL "Try to build lib${module_name} library" FORCE )
        ENDIF( is_exe )
    ENDIF( ${cumulative_var} AND NOT target_location )

ENDMACRO( GBX_REQUIRE_TARGET cumulative_var module_type module_name target_name )


#
# GBX_REQUIRE_TARGETS( cumulative_var [EXE | LIB] module_name TARGET0 [TARGET1 TARGET2 ...] )
#
MACRO( GBX_REQUIRE_TARGETS cumulative_var module_type module_name )

    # debug
#     MESSAGE( STATUS "GBX_REQUIRE_TARGETS [ CUM_VAR=${${cumulative_var}}, MOD_TYPE=${module_type}, MOD_NAME=${module_name}, target_names=${ARGN} ]" )

    IF( ${ARGC} LESS 4 )
        MESSAGE( FATAL_ERROR "GBX_REQUIRE_TARGETS macro needs to at least one target name (${ARGC} params were given)." )
    ENDIF( ${ARGC} LESS 4 )

    FOREACH( trgt ${ARGN} )
        GBX_REQUIRE_TARGET( ${cumulative_var} ${module_type} ${module_name} ${trgt} )
    ENDFOREACH( trgt ${ARGN} )

ENDMACRO( GBX_REQUIRE_TARGETS cumulative_var module_type module_name )

#
# This is a utility macro for internal use.
#
MACRO( GBX_WRITE_OPTIONS )
    SET( output_file ${GBX_PROJECT_BINARY_DIR}/${PROJECT_NAME}_options.cmake )
    WRITE_FILE( ${output_file} "\# Autogenerated by CMake for ${PROJECT_NAME} project" )

    FOREACH( a ${OPTION_LIST} )
        WRITE_FILE( ${output_file} "-D${a} \\" APPEND )
    ENDFOREACH( a ${LIB_LIST} )
ENDMACRO( GBX_WRITE_OPTIONS )

#
# This is a utility macro for internal use.
# Reset global lists of components, libraries, etc.
#
MACRO( GBX_RESET_ALL_DEPENDENCY_LISTS )
    # MESSAGE( STATUS "DEBUG: Resetting global dependency lists" )
    SET( OPTION_LIST    "" CACHE INTERNAL "Global list of cmake options" FORCE )
ENDMACRO( GBX_RESET_ALL_DEPENDENCY_LISTS )
