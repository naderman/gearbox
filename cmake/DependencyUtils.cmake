#
# This is a utility macro for internal use.
# If module_type is not equal "EXE" or "LIB", prints error message and quits.
#
macro( GBX_UTIL_CHECK_MODULE_TYPE type )
#     message( STATUS "GBX_UTIL_CHECK_MODULE_TYPE type=${type}" )

    string( COMPARE EQUAL ${type} "EXE" is_exe )
    string( COMPARE EQUAL ${type} "LIB" is_lib )
    if( NOT is_exe AND NOT is_lib )
        message( FATAL_ERROR "In GBX_REQUIRE_* macros, module_type must be either 'EXE' or 'LIB'" )
    endif( NOT is_exe AND NOT is_lib )
    if( is_exe AND is_lib )
        message( FATAL_ERROR "In GBX_REQUIRE_* macros, module_type must be either 'EXE' or 'LIB'" )
    endif( is_exe AND is_lib )

endmacro( GBX_UTIL_CHECK_MODULE_TYPE type )

#
# This is a utility macro for internal use.
#
macro( GBX_UTIL_MAKE_OPTION_NAME option_name module_type module_name )
#     message( STATUS "GBX_UTIL_MAKE_OPTION_NAME [ option_name=${option_name}, MOT_TYPE=${module_type}, MOD_NAME=${module_name} ]" )

    GBX_UTIL_CHECK_MODULE_TYPE( ${module_type} )

    string( COMPARE EQUAL ${module_type} "EXE" is_exe )
    string( COMPARE EQUAL ${module_type} "LIB" is_lib )
    string( TOUPPER ${module_name} module_name_upper )
    # dereference the variable name once, so that we are setting the variable in the calling context!
    if( is_exe )
        set( ${option_name} "ENABLE_${module_name_upper}" )
    else( is_exe )
        set( ${option_name} "ENABLE_LIB_${module_name_upper}" )
    endif( is_exe )

#     message( STATUS "GBX_UTIL_MAKE_OPTION_NAME output: option_name=${${option_name}}" )
endmacro( GBX_UTIL_MAKE_OPTION_NAME option_name module_name )

#
# This is a utility macro for internal use.
# (there's another copy of this simple macro in TargetUtils.cmake)
#
# macro( GBX_UTIL_MAKE_MANIFEST_NAME manifest_name module_name )
# #     message( STATUS "GBX_UTIL_MAKE_MANIFEST_NAME [ manifest_name=${manifest_name}, MOD_NAME=${module_name} ]" )
# 
#     string( TOUPPER ${module_name} module_name_upper )
#     # dereference the variable name once, so that we are setting the variable in the calling context!
#     set( ${manifest_name} ${module_name_upper}_INSTALLED )
# 
# #     message( STATUS "GBX_UTIL_MAKE_MANIFEST_NAME output: manifest_name=${${manifest_name}}" )
# endmacro( GBX_UTIL_MAKE_MANIFEST_NAME manifest_name module_name )


#
# GBX_REQUIRE_OPTION( cumulative_var [EXE | LIB] module_name default_option_value [option_name] [OPTION DESCRIPTION] )
#
# E.g.
# Initialize a variable first
#   set( build TRUE )
# Now set up and test option value
#   GBX_REQUIRE_OPTION ( build EXE localiser ON )
# This does the same thing
#   GBX_REQUIRE_OPTION ( build EXE localiser ON BUILD_LOCALISER )
#
macro( GBX_REQUIRE_OPTION cumulative_var module_type module_name default_option_value )

    if( ${ARGC} GREATER 5 )
        set( option_name ${ARGV6} )
    else( ${ARGC} GREATER 5 )
        GBX_UTIL_MAKE_OPTION_NAME( option_name ${module_type} ${module_name} )
    endif( ${ARGC} GREATER 5 )

    if( ${ARGC} GREATER 6 )
        set( option_descr ${ARGV7} )
    else( ${ARGC} GREATER 6 )
        set( option_descr "disabled by user, use ccmake to enable" )
    endif( ${ARGC} GREATER 6 )

    # debug
#     message( STATUS
#         "GBX_REQUIRE_OPTION (CUM_VAR=${cumulative_var}, MOD_TYPE=${module_type}, MOD_NAME=${module_name}, default_option_value=${default_option_value}, OPT_NAME=${option_name}, OPT_DESC=${option_descr})" )

    # set up the option
    if( is_exe )
        option( ${option_name} "Try to build ${module_name}" ${default_option_value} )
    else( is_exe )
        option( ${option_name} "Try to build lib${module_name} library" ${default_option_value} )
    endif( is_exe )

    # add option to the list: this has nothing to do with the build system.
    # it is useful to have a text list of all options if you want to build a particular
    # configuration from the command line.
    set( templist ${OPTION_LIST} )
    # (escaping \)
    list( APPEND templist "${option_name}=${default_option_value}" )
    set( OPTION_LIST ${templist} CACHE INTERNAL "Global list of cmake options" FORCE )

    # must dereference both var and option names once (!) and IF will evaluate their values
    if( ${cumulative_var} AND NOT ${option_name}  )
        set( ${cumulative_var} FALSE )
        if( is_exe )
            GBX_NOT_ADD_EXECUTABLE( ${module_name} ${option_descr} )
        else( is_exe )
            GBX_NOT_add_library( ${module_name} ${option_descr} )
        endif( is_exe )
    endif( ${cumulative_var} AND NOT ${option_name} )

endmacro( GBX_REQUIRE_OPTION cumulative_var module_type module_name default_option_value )

#
# GBX_REQUIRE_VAR ( cumulative_var [EXE | LIB] module_name test_var reason )
#
# E.g.
# Initialize a variable first
#   set( build TRUE )
# Now test the variable value
#   GBX_REQUIRE_VAR ( build LIB HydroStuff GOOD_TO_GO "good-to-go is no good" )
#
macro( GBX_REQUIRE_VAR cumulative_var module_type module_name test_var reason )

    # debug
#     message( STATUS "GBX_REQUIRE_VAR [ CUM_VAR=${cumulative_var}, MOD_TYPE=${module_type}, MOD_NAME=${module_name}, test_var=${${test_var}}, reason=${reason} ]" )

    GBX_UTIL_CHECK_MODULE_TYPE( ${module_type} )

    # must dereference both var names once (!) and IF will evaluate their values
    if( ${cumulative_var} AND NOT ${test_var}  )
        set( ${cumulative_var} FALSE )
        if( is_exe )
            GBX_NOT_ADD_EXECUTABLE( ${module_name} ${reason} )
        else( is_exe )
            GBX_NOT_add_library( ${module_name} ${reason} )
        endif( is_exe )
    endif( ${cumulative_var} AND NOT ${test_var} )

endmacro( GBX_REQUIRE_VAR cumulative_var module_type module_name test_var reason )

# OBSOLETE! CONVERTED FROM HOME-MADE MANIFESTS TO FIND_PACKAGE()
#
# GBX_REQUIRE_install( cumulative_var [EXE | LIB] module_name installed_module [reason] )
#
# A special case of REQUIRE_VAR. Checks whether a manifest variable is defined 
# for the module with a name "installed_module".
# E.g.
# Initialize a variable first
#   set( build TRUE )
# Now test the variable value
#   REQUIRE_install( build LIB HydroStuff GbxStuff )
#           will check if GBXSTUFF_INSTALLED is defined.
#   This example is equivalent to
#   REQUIRE_VAR( build LIB HydroStuff GBXSTUFF_INSTALLED "GbxStuff was not installed" )
#
# macro( GBX_REQUIRE_INSTALL cumulative_var module_type module_name installed_module )
# 
#     # debug
# #     message( STATUS "GBX_REQUIRE_INSTALL [ CUM_VAR=${cumulative_var}, MOD_TYPE=${module_type}, MOD_NAME=${module_name}, installed_module=${installed_module} ]" )
# 
#     GBX_UTIL_MAKE_MANIFEST_NAME( test_var ${installed_module} )
# 
#     if( ${ARGC} GREATER 5 )
#         set( reason ${ARGV6} )
#     else( ${ARGC} GREATER 5 )
#         set( reason "${installed_module} was not installed" )
#     endif( ${ARGC} GREATER 5 )
# 
#     # must dereference both var names once (!)
#     GBX_REQUIRE_VAR( ${cumulative_var} ${module_type} ${module_name} ${test_var} ${reason} )
# 
# endmacro( GBX_REQUIRE_INSTALL cumulative_var module_type module_name installed_module )

# OBSOLETE! CONVERTED FROM HOME-MADE MANIFESTS TO FIND_PACKAGE()
#
# GBX_REQUIRE_INSTALLS( cumulative_var [EXE | LIB] module_name target0 [targe1 target2 ...] )
#
# macro( GBX_REQUIRE_INSTALLS cumulative_var module_type module_name )
#     
#     if( ${ARGC} LESS 4 )
#         message( FATAL_ERROR "GBX_REQUIRE_INSTALLS macro needs to at least one target name (${ARGC} params were given)." ) 
#     endif( ${ARGC} LESS 4 )
# 
#     foreach( trgt ${ARGN} )
#         GBX_REQUIRE_install( ${cumulative_var} ${module_type} ${module_name} ${trgt} )
#     endforeach( trgt ${ARGN} )
# 
# endmacro( GBX_REQUIRE_INSTALLS cumulative_var module_type module_name )

#
# GBX_REQUIRE_LIB( cumulative_var [EXE | LIB] module_name target_name [reason] )
#
# Despite the name, this macro can enforce dependence on any target, not just a library.
#
# E.g.
# Initialize a variable first
#   set( build TRUE )
# Now set up and test option value
#   GBX_REQUIRE_LIB ( build EXE localiser HydroStuff )
#
macro( GBX_REQUIRE_LIB cumulative_var module_type module_name target_name )
    # debug
#     message( STATUS "GBX_REQUIRE_LIB [ CUM_VAR=${${cumulative_var}}, MOD_TYPE=${module_type}, MOD_NAME=${module_name}, target_name=${target_name} ]" )

    if( ${cumulative_var} )

        GBX_UTIL_CHECK_MODULE_TYPE( ${module_type} )
    
        if( ${ARGC} GREATER 5 )
            set( reason ${ARGV6} )
        else( ${ARGC} GREATER 5 )
            set( reason "${target_name} is not being built" )
        endif( ${ARGC} GREATER 5 )
    
        GET_TARGET_PROPERTY( target_location ${target_name} LOCATION )
    
        # must dereference both var and option names once (!) and IF will evaluate their values
        if( NOT target_location  )
            set( ${cumulative_var} FALSE )
            # force ENABLE_* variables to off in ccmake UI if dependencies aren't met.
            GBX_UTIL_MAKE_OPTION_NAME( option_name ${module_type} ${module_name} )
            if( is_exe )
                GBX_NOT_ADD_EXECUTABLE( ${module_name} ${reason} )
                set( ${option_name} OFF CACHE BOOL "Try to build ${module_name}" FORCE )
            else( is_exe )
                GBX_NOT_add_library( ${module_name} ${reason} )
                set( ${option_name} OFF CACHE BOOL "Try to build lib${module_name} library" FORCE )
            endif( is_exe )
        endif( NOT target_location )

    endif( ${cumulative_var} )

endmacro( GBX_REQUIRE_LIB cumulative_var module_type module_name target_name )

#
# GBX_REQUIRE_LIBS( cumulative_var [EXE | LIB] module_name TARGET0 [TARGET1 TARGET2 ...] )
#
# Despite the name, this macro can enforce dependence on a set of target, not just a set of libraries.
#
macro( GBX_REQUIRE_LIBS cumulative_var module_type module_name )
    # debug
#     message( STATUS "GBX_REQUIRE_LIBS [ CUM_VAR=${${cumulative_var}}, MOD_TYPE=${module_type}, MOD_NAME=${module_name}, target_names=${ARGN} ]" )

    if( ${ARGC} LESS 4 )
        message( FATAL_ERROR "GBX_REQUIRE_LIBS macro needs to at least one target name (${ARGC} params were given)." )
    endif( ${ARGC} LESS 4 )

    foreach( trgt ${ARGN} )
        GBX_REQUIRE_LIB( ${cumulative_var} ${module_type} ${module_name} ${trgt} )
    endforeach( trgt ${ARGN} )

endmacro( GBX_REQUIRE_LIBS cumulative_var module_type module_name )

# OBSOLETE! Use GBX_REQUIRE_LIB instead.
#
# GBX_REQUIRE_LIB( cumulative_var [EXE | LIB] module_name depend_name [reason] )
#
# This is a convenience macro which combines the functionality of GBX_REQUIRE_INSTALL and GBX_REQUIRE_LIB.
# It can be used to require executables despite the name.
#
# First checks if a target called ${depend_name} exists, i.e. a library or executable with this name
# is part of the same project and is enabled. If it exists, sets ${cumulative_var} to TRUE and exits.
#
# Otherwise, checks if a library or executable called ${depend_name} is installed, i.e. a manifest 
# variable is defined.
#
# E.g.
# Initialize a variable first
#   set( build TRUE )
# Now set up and test option value
#   GBX_REQUIRE_LIB ( build EXE localiser HydroStuff )
#   GBX_REQUIRE_LIB ( build LIB MyStuff HydroStuff )
#
# macro( GBX_REQUIRE_LIB cumulative_var module_type module_name depend_name )
# #     message( STATUS "GBX_REQUIRE_LIB [ CUM_VAR=${${cumulative_var}}, MOD_TYPE=${module_type}, MOD_NAME=${module_name}, depend_name=${depend_name} ]" )
# 
#     if( ${cumulative_var}  )
# 
#         GBX_UTIL_CHECK_MODULE_TYPE( ${module_type} )
#     
#         if( ${ARGC} GREATER 5 )
#             set( reason ${ARGV6} )
#         else( ${ARGC} GREATER 5 )
#             set( reason "${depend_name} is not being built and was not installed" )
#         endif( ${ARGC} GREATER 5 )
#     
#         # first, look for a target
#         GET_TARGET_PROPERTY( target_location ${depend_name} LOCATION )
#     
#         # must dereference both var and option names once (!) and IF will evaluate their values
#         if( NOT target_location  )
#             
#             # target not found, look for an install
#             GBX_UTIL_MAKE_MANIFEST_NAME( test_var ${depend_name} )
# 
#             # must dereference both var names once (!)
#             GBX_REQUIRE_VAR( ${cumulative_var} ${module_type} ${module_name} ${test_var} ${reason} )
# 
#         endif( NOT target_location )
# 
#     endif( ${cumulative_var}  )
# 
# endmacro( GBX_REQUIRE_LIB cumulative_var module_type module_name target_name )

# OBSOLETE! Use GBX_REQUIRE_LIBS instead.
#
# GBX_REQUIRE_LIBS( cumulative_var [EXE | LIB] module_name LIB0 [LIB1 LIB2 ...] )
#
# macro( GBX_REQUIRE_LIBS cumulative_var module_type module_name )
# #     message( STATUS "GBX_REQUIRE_LIBS [ CUM_VAR=${${cumulative_var}}, MOD_TYPE=${module_type}, MOD_NAME=${module_name}, target_names=${ARGN} ]" )
# 
#     if( ${ARGC} LESS 4 )
#         message( FATAL_ERROR "GBX_REQUIRE_LIBS macro needs to at least one library name (${ARGC} params were given)." )
#     endif( ${ARGC} LESS 4 )
# 
#     foreach( lib ${ARGN} )
#         GBX_REQUIRE_LIB( ${cumulative_var} ${module_type} ${module_name} ${lib} )
#     endforeach( lib ${ARGN} )
# 
# endmacro( GBX_REQUIRE_LIBS cumulative_var module_type module_name )

#
# This is a utility macro for internal use.
#
macro( GBX_WRITE_OPTIONS )
    set( output_file ${GBX_PROJECT_BINARY_DIR}/${PROJECT_NAME}_options.cmake )
    write_file( ${output_file} "\# Autogenerated by CMake for ${PROJECT_NAME} project" )

    foreach( a ${OPTION_LIST} )
        write_file( ${output_file} "-D${a} \\" APPEND )
    endforeach( a ${LIB_LIST} )
endmacro( GBX_WRITE_OPTIONS )

#
# This is a utility macro for internal use.
# Reset global lists of components, libraries, etc.
#
macro( GBX_RESET_ALL_DEPENDENCY_LISTS )
    # message( STATUS "DEBUG: Resetting global dependency lists" )
    set( OPTION_LIST    "" CACHE INTERNAL "Global list of cmake options" FORCE )
endmacro( GBX_RESET_ALL_DEPENDENCY_LISTS )
