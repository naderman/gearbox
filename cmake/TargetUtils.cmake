#
# Default library type (shared or static).
#
set( GBX_DEFAULT_LIB_TYPE "SHARED" CACHE STRING "Default library type (SHARED or STATIC)" )
mark_as_advanced( GBX_DEFAULT_LIB_TYPE )

#
# Default preference for installing header files.
#
set( GBX_INSTALL_HEADERS TRUE CACHE BOOLEAN "Do you want to install header files?" )
mark_as_advanced( GBX_INSTALL_HEADERS )

#
# Default preference for installing shared files.
#
set( GBX_INSTALL_SHARED_FILES TRUE CACHE BOOLEAN "Do you want to install shared files?" )
mark_as_advanced( GBX_INSTALL_SHARED_FILES )

#
# Default preference for installing examples.
#
set( GBX_INSTALL_EXAMPLES TRUE CACHE BOOLEAN "Do you want to install example files?" )
mark_as_advanced( GBX_INSTALL_EXAMPLES )

#
# Default preference for installing PKGCONFIG's.
#
set( GBX_INSTALL_PKGCONFIGS TRUE CACHE BOOLEAN "Do you want to install PKGCONFIG's files?" )
mark_as_advanced( GBX_INSTALL_PKGCONFIGS )


#
# Executables should add themselves by calling 'GBX_ADD_EXECUTABLE'
# instead of 'ADD_EXECUTABLE' in CMakeLists.txt.
# Usage is the same as ADD_EXECUTABLE, all parameters are passed to ADD_EXECUTABLE.
#
macro( GBX_ADD_EXECUTABLE name )
    if( COMMAND cmake_policy )
        cmake_policy( SET CMP0003 NEW )
    endif( COMMAND cmake_policy )

    add_executable( ${name} ${ARGN} )
#     set_target_properties( ${name} PROPERTIES
#         INSTALL_RPATH "${INSTALL_RPATH};${CMAKE_INSTALL_PREFIX}/lib/${PROJECT_NAME}"
#         BUILD_WITH_INSTALL_RPATH TRUE )
    install( TARGETS ${name} RUNTIME DESTINATION bin )
    set( templist ${EXE_LIST} )
    list( APPEND templist ${name} )
#   message( STATUS "DEBUG: ${templist}" )
    set( EXE_LIST ${templist} CACHE INTERNAL "Global list of executables to build" FORCE )
    message( STATUS "Planning to build executable:      ${name}" )
endmacro( GBX_ADD_EXECUTABLE name )

#
# Libraries should add themselves by calling 'GBX_ADD_LIBRARY'
# instead of 'ADD_LIBRARY' in CMakeLists.txt.
# The second parameter specifies the library type to build: SHARED, STATIC, or DEFAULT.
# If the default is used, the library type will depend on the gearbox-wide value as set
# by the user (or SHARED if the user hasn't changed it from the original setting).
# All extra parameters are passed to ADD_LIBRARY as source files.
#
macro( GBX_ADD_LIBRARY name type )
    if( COMMAND cmake_policy )
        cmake_policy( SET CMP0003 NEW )
    endif( COMMAND cmake_policy )

    if( ${type} STREQUAL SHARED )
        set( libType SHARED )
    ELSEIF( ${type} STREQUAL STATIC )
        set( libType STATIC )
    else( ${type} STREQUAL SHARED )
        set( libType ${GBX_DEFAULT_LIB_TYPE} )
    endif( ${type} STREQUAL SHARED )

    add_library( ${name} ${libType} ${ARGN} )
#     SET_TARGET_PROPERTIES( ${name} PROPERTIES
#         INSTALL_RPATH "${INSTALL_RPATH};${CMAKE_INSTALL_PREFIX}/lib/${PROJECT_NAME}"
#         BUILD_WITH_INSTALL_RPATH TRUE )
    install( TARGETS ${name}
             DESTINATION lib/${PROJECT_NAME}
             EXPORT ${PROJECT_NAME}-targets )

    set( templist ${LIB_LIST} )
    list( APPEND templist ${name} )
    set( LIB_LIST ${templist} CACHE INTERNAL "Global list of libraries to build" FORCE )

    if( libType STREQUAL SHARED )
        set( libTypeDesc "shared" )
    else( libType STREQUAL SHARED )
        set( libTypeDesc "static" )
    endif( libType STREQUAL SHARED )
    message( STATUS "Planning to build ${libTypeDesc} library:  ${name}" )
endmacro( GBX_ADD_LIBRARY name )

#
# GBX_ADD_HEADERS( install_subdir FILE0 [FILE1 FILE2 ...] )
#
# Specialization of install(FILES ...) to install header files.
# All files are installed into PREFIX/include/${PROJECT_NAME}/${install_subdir}
#
macro( GBX_ADD_HEADERS install_subdir )
    if( GBX_INSTALL_HEADERS )
        install( FILES ${ARGN} DESTINATION include/${PROJECT_NAME}/${install_subdir} )
    endif()
endmacro( GBX_ADD_HEADERS install_subdir )
#
# GBX_ADD_SHARED_FILES( install_subdir FILE0 [FILE1 FILE2 ...] )
#
# Specialization of install(FILES ...) to install shared files.
# All files are installed into PREFIX/share/${PROJECT_NAME}/${install_subdir} directory.
#
macro( GBX_ADD_SHARED_FILES install_subdir )
    if( GBX_INSTALL_SHARED_FILES )
        install( FILES ${ARGN} DESTINATION share/${PROJECT_NAME}/${install_subdir} )
    endif()
endmacro( GBX_ADD_SHARED_FILES install_subdir )

#
# GBX_ADD_EXAMPLE( install_subdir makefile.in makefile.out [FILE0 FILE1 FILE2 ...] )
#
# Specialisation of install(FILES ...) to install examples.
# All files are installed into PREFIX/share/${PROJECT_NAME}/${install_subdir}.
# makefile is passed through CONFIGURE_FILE to add in correct include and library
# paths based on the install prefix.
#
macro( GBX_ADD_EXAMPLE install_subdir makefile.in makefile.out )
    configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/${makefile.in} ${CMAKE_CURRENT_BINARY_DIR}/${makefile.out} @ONLY)
    if( GBX_INSTALL_EXAMPLES )
        install( FILES ${CMAKE_CURRENT_BINARY_DIR}/${makefile.out} DESTINATION share/${PROJECT_NAME}/${install_subdir} RENAME CMakeLists.txt )
        install( FILES ${ARGN} DESTINATION share/${PROJECT_NAME}/${install_subdir} )
    endif()
endmacro( GBX_ADD_EXAMPLE install_subdir makefile )

#
# GBX_ADD_PKGCONFIG( name cflags libflags [DEPENDENCY0 DEPENDENCY1 ...] )
#
# Creates a pkg-config file for library "name".
# desc is a description of the library.
# ext_deps is a list containing all the external libraries this one requires (pass by reference).
# int_deps is a list containing all the internal libraries this library depends on (pass by reference).
# cflags is appended to the "Cflags" value.
# libflags is appended to the "Libs" value.
# that should be linked with at the same time as linking to this library.
#
macro( GBX_ADD_PKGCONFIG name desc ext_deps int_deps cflags libflags )
    set( PKG_NAME ${name} )
    set( PKG_DESC ${desc} )
    set( PKG_CFLAGS ${cflags} )
    set( PKG_LIBFLAGS ${libflags} )
    set( PKG_EXTERNAL_DEPS ${${ext_deps}} )
    set( PKG_INTERNAL_DEPS "" )
    if( ${int_deps} )
        foreach( item ${${int_deps}} )
            set( PKG_INTERNAL_DEPS "${PKG_INTERNAL_DEPS} -l${item}" )
        endforeach( item ${${int_deps}} )
    endif( ${int_deps} )

    configure_file( ${GBX_CMAKE_DIR}/pkgconfig.in ${CMAKE_CURRENT_BINARY_DIR}/${name}.pc @ONLY)

    if( GBX_INSTALL_PKGCONFIGS )
        install( FILES ${CMAKE_CURRENT_BINARY_DIR}/${name}.pc DESTINATION lib/pkgconfig/ )
    endif()
endmacro( GBX_ADD_PKGCONFIG name desc cflags deps libflags libs )

#
# This is a mechanism to register special items which are not
# executables or libraries. This function only records the name of
# the item to display it at the end of the cmake run and to submit
# to the Dashboard.
# Usage: GBX_ADD_ITEM( name )
#
macro( GBX_ADD_ITEM name )
    set( templist ${ITEM_LIST} )
    list( APPEND templist ${name} )
    set( ITEM_LIST ${templist} CACHE INTERNAL "Global list of special items to build" FORCE )
    message( STATUS "Planning to build item:            ${name}" )
endmacro( GBX_ADD_ITEM name )

#
# This is a mechanism to specify a license for the current source directory.
# Usage: GBX_ADD_LICENSE( license )
#
macro( GBX_ADD_LICENSE license )
    set( templist ${LICENSE_LIST} )

    # get relative path to the current source dir
    string( LENGTH ${GBX_PROJECT_SOURCE_DIR} proj_src_dir_length )
    string( LENGTH ${CMAKE_CURRENT_SOURCE_DIR} current_src_dir_length )
    math( EXPR relative_path_length "${current_src_dir_length} - ${proj_src_dir_length} - 1" )
    math( EXPR relative_path_start "${proj_src_dir_length} + 1" )
    string( SUBSTRING ${CMAKE_CURRENT_SOURCE_DIR}
        ${relative_path_start} ${relative_path_length} current_src_dir_relative )

    # format the string to line up properly
    set( spaces "A                                                                                       Z" )
    string( LENGTH ${current_src_dir_relative} current_src_dir_relative_length )
    math( EXPR white_space_length "60 - ${current_src_dir_relative_length}" )
    string( SUBSTRING ${spaces} 1 ${white_space_length} white_space )

    set( line_item "${current_src_dir_relative}${white_space}${license}" )
    list( APPEND templist ${line_item} )
    set( LICENSE_LIST ${templist} CACHE INTERNAL "Global list of directories and their licenses" FORCE )
#     message( STATUS ${line_item} )
endmacro( GBX_ADD_LICENSE license )

#
# Usage: GBX_ADD_TEST( testname exename [arg1 arg2 ...] )
# Example:  GBX_ADD_TEST( IntegerTest inttest --verbose )
#
macro( GBX_ADD_TEST name executable )
    add_test( ${name} ${executable} ${ARGN} )
    set( templist ${TEST_LIST} )
    list( APPEND templist ${name} )
    set( TEST_LIST ${templist} CACHE INTERNAL "Global list of (CTest) tests to build" FORCE )
#     message( STATUS "Planning to build test:          ${name}" )
endmacro( GBX_ADD_TEST name executable )

#
# Usage: GBX_NOT_add_executable( name reason )
#
macro( GBX_NOT_ADD_EXECUTABLE name reason )
  set( templist ${EXE_NOT_LIST} )
  list( APPEND templist ${name} )
#  message( STATUS "DEBUG: ${templist}" )
  set( EXE_NOT_LIST ${templist} CACHE INTERNAL "Global list of executables NOT to build" FORCE )
  message( STATUS "Not planning to build executable:  ${name} because ${reason}" )
endmacro( GBX_NOT_ADD_EXECUTABLE name reason )

#
# Usage: GBX_NOT_add_library( name reason )
#
macro( GBX_NOT_ADD_LIBRARY name reason )
  set( templist ${LIB_NOT_LIST} )
  list( APPEND templist ${name} )
#  message( STATUS "DEBUG: ${templist}" )
  set( LIB_NOT_LIST ${templist} CACHE INTERNAL "Global list of libraries NOT to build" FORCE )
  message( STATUS "Not planning to build library:     ${name} because ${reason}" )
endmacro( GBX_NOT_ADD_LIBRARY name reason )

#
# This is a utility macro for internal use.
# Prints out list information: size, and items.
# Prints nothing if list is empty.
# Example: LIST_REPORT( EXE_LIST "executable(s)" )
#
# Tricky list stuff.
# see http://www.cmake.org/Wiki/CMakeMacroMerge for an example
#
macro( LIST_REPORT action item_name note L )
    set( templist ${L} )
    list( LENGTH templist templist_length )
    set( report_file ${GBX_PROJECT_BINARY_DIR}/cmake_config_report.txt )

    if( templist_length GREATER 0 )
        list( SORT templist )

        message( STATUS "${action} ${templist_length} ${item_name} ${note}:" )
        message( STATUS "    ${templist}" )

        write_file( ${report_file} "${action} ${templist_length} ${item_name}:" APPEND )
        write_file( ${report_file} "    ${templist}" APPEND )
    endif( templist_length GREATER 0 )
endmacro( LIST_REPORT action item_name note L )

#
# This is a utility macro for internal use.
# Puts messages on the screen.
# Writes to a text file.
#
macro( GBX_CONFIG_REPORT )

    message( STATUS "== SUMMARY ==" )

    # write configuration results to file (this line clears existing contents)
    set( report_file ${GBX_PROJECT_BINARY_DIR}/cmake_config_report.txt )
    write_file( ${report_file} "Autogenerated by CMake for ${PROJECT_NAME} project" )
#     write_file( ${report_file} "Using Ice version ${ICE_VERSION}" )

    #
    # Print some results
    #
    message( STATUS "Project name      ${PROJECT_NAME}")
    message( STATUS "Project version   ${GBX_PROJECT_VERSION}")
    # would be nice to print out Orca version for satellite projects
    # for this we need an executable which is guaranteed to be installed.
    # then we can run it with --version flag.
    # if( NOT ORCA_MOTHERSHIP )
    #     message( STATUS "Using Orca version ${ORCA_VERSION}")
    # endif( NOT ORCA_MOTHERSHIP )
    message( STATUS "Platform              ${CMAKE_SYSTEM}")
    message( STATUS "CMake version         ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}-patch ${CMAKE_PATCH_VERSION}")
    message( STATUS "Install dir           ${CMAKE_INSTALL_PREFIX}")
    message( STATUS "Default library type  ${GBX_DEFAULT_LIB_TYPE}")

    set( note " " )
    LIST_REPORT( "Will build" "executables" ${note} "${EXE_LIST}" )
    LIST_REPORT( "Will build" "libraries" ${note} "${LIB_LIST}" )
    LIST_REPORT( "Will build" "CTest tests" ${note} "${TEST_LIST}" )
    LIST_REPORT( "Will build" "special items" ${note} "${ITEM_LIST}" )

    set( note "(see above for reasons)" )
    LIST_REPORT( "Will NOT build" "executables" ${note} "${EXE_NOT_LIST}" )
    LIST_REPORT( "Will NOT build" "libraries" ${note} "${LIB_NOT_LIST}" )

endmacro( GBX_CONFIG_REPORT )

#
# This is a utility macro for internal use.
# (there's another copy of this simple macro in DependencyUtils.cmake)
#
macro( GBX_MAKE_MANIFEST_NAME manifest_name module_name )
    string( TOUPPER ${module_name} upper_module_name )
    # dereference the variable name once, so that we are setting the variable in the calling context!
    set( ${manifest_name} ${module_name_upper}_INSTALLED )
endmacro( GBX_MAKE_MANIFEST_NAME manifest_name module_name )

#
# This is a utility macro for internal use.
#
macro( GBX_WRITE_MANIFEST )
    set( output_file ${GBX_PROJECT_BINARY_DIR}/${PROJECT_NAME}_manifest.cmake )
    write_file( ${output_file} "\# Autogenerated by CMake for ${PROJECT_NAME} project" )

    foreach( item ${LIB_LIST} )
        GBX_MAKE_MANIFEST_NAME( manifest_name ${item} )
        write_file( ${output_file} "set( ${manifest_name} 1)" APPEND )
    endforeach( item ${LIB_LIST} )

    foreach( item ${LIB_NOT_LIST} )
        GBX_MAKE_MANIFEST_NAME( manifest_name ${item} )
        write_file( ${output_file} "set( ${manifest_name} 0)" APPEND )
    endforeach( item ${LIB_NOT_LIST} )

    write_file( ${output_file} " " APPEND )

    string( TOUPPER ${PROJECT_NAME} upper_project_name )
    write_file( ${output_file} "set( ${upper_project_name}_MANIFEST_LOADED 1)" APPEND )

    install( FILES ${output_file} DESTINATION . )
endmacro( GBX_WRITE_MANIFEST )

macro( GBX_WRITE_LICENSE )
    set( license_file ${GBX_PROJECT_SOURCE_DIR}/LICENSE )
    write_file( ${license_file} "Autogenerated by CMake for ${PROJECT_NAME} project" )
    write_file( ${license_file} "----------------------------------------------------------------------" APPEND )
    write_file( ${license_file} "DIRECTORY                                                   license" APPEND )
    write_file( ${license_file} "----------------------------------------------------------------------" APPEND )

    foreach( item ${LICENSE_LIST} )
        write_file( ${license_file} ${item} APPEND )
    endforeach( item ${LICENSE_LIST} )

endmacro( GBX_WRITE_LICENSE )

#
# This is a utility macro for internal use.
# Reset global lists of components, libraries, etc.
#
macro( GBX_RESET_ALL_TARGET_LISTS )
    # message( STATUS "DEBUG: Resetting global target lists" )
    set( EXE_LIST    "" CACHE INTERNAL "Global list of executables to build" FORCE )
    set( LIB_LIST    "" CACHE INTERNAL "Global list of libraries to build" FORCE )
    set( TEST_LIST   "" CACHE INTERNAL "Global list of CTest tests to build" FORCE )
    set( ITEM_LIST   "" CACHE INTERNAL "Global list of special items to build" FORCE )

    set( EXE_NOT_LIST  "" CACHE INTERNAL "Global list of executables NOT to build" FORCE )
    set( LIB_NOT_LIST  "" CACHE INTERNAL "Global list of libraries NOT to build" FORCE )

    set( LICENSE_LIST  "" CACHE INTERNAL "Global list of directories and their licenses" FORCE )
endmacro( GBX_RESET_ALL_TARGET_LISTS )
