#
# Default library type (shared or static).
#
SET( GBX_DEFAULT_LIB_TYPE "SHARED" CACHE STRING "Default library type (SHARED or STATIC)" )
MARK_AS_ADVANCED( GBX_DEFAULT_LIB_TYPE )

#
# Executables should add themselves by calling 'GBX_ADD_EXECUTABLE'
# instead of 'ADD_EXECUTABLE' in CMakeLists.txt.
# Usage is the same as ADD_EXECUTABLE, all parameters are passed to ADD_EXECUTABLE.
#
MACRO( GBX_ADD_EXECUTABLE name )
    IF( COMMAND cmake_policy )
        CMAKE_POLICY( SET CMP0003 NEW )
    ENDIF( COMMAND cmake_policy )

    ADD_EXECUTABLE( ${name} ${ARGN} )
#     SET_TARGET_PROPERTIES( ${name} PROPERTIES
#         INSTALL_RPATH "${INSTALL_RPATH};${CMAKE_INSTALL_PREFIX}/lib/${PROJECT_NAME}"
#         BUILD_WITH_INSTALL_RPATH TRUE )
    INSTALL( TARGETS ${name} RUNTIME DESTINATION bin )
    SET( templist ${EXE_LIST} )
    LIST( APPEND templist ${name} )
#   MESSAGE( STATUS "DEBUG: ${templist}" )
    SET( EXE_LIST ${templist} CACHE INTERNAL "Global list of executables to build" FORCE )
    MESSAGE( STATUS "Planning to build executable:      ${name}" )
ENDMACRO( GBX_ADD_EXECUTABLE name )

#
# Libraries should add themselves by calling 'GBX_ADD_LIBRARY'
# instead of 'ADD_LIBRARY' in CMakeLists.txt.
# The second parameter specifies the library type to build: SHARED, STATIC, or DEFAULT.
# If the default is used, the library type will depend on the gearbox-wide value as set
# by the user (or SHARED if the user hasn't changed it from the original setting).
# All extra parameters are passed to ADD_LIBRARY as source files.
#
MACRO( GBX_ADD_LIBRARY name type )
    IF( COMMAND cmake_policy )
        CMAKE_POLICY( SET CMP0003 NEW )
    ENDIF( COMMAND cmake_policy )

    IF( ${type} STREQUAL SHARED )
        SET( libType SHARED )
    ELSEIF( ${type} STREQUAL STATIC )
        SET( libType STATIC )
    ELSE( ${type} STREQUAL SHARED )
        SET( libType ${GBX_DEFAULT_LIB_TYPE} )
    ENDIF( ${type} STREQUAL SHARED )

    ADD_LIBRARY( ${name} ${libType} ${ARGN} )
#     SET_TARGET_PROPERTIES( ${name} PROPERTIES
#         INSTALL_RPATH "${INSTALL_RPATH};${CMAKE_INSTALL_PREFIX}/lib/${PROJECT_NAME}"
#         BUILD_WITH_INSTALL_RPATH TRUE )
    INSTALL( TARGETS ${name} DESTINATION lib/${PROJECT_NAME} )

    SET( templist ${LIB_LIST} )
    LIST( APPEND templist ${name} )
    SET( LIB_LIST ${templist} CACHE INTERNAL "Global list of libraries to build" FORCE )

    IF( libType STREQUAL SHARED )
        SET( libTypeDesc "shared" )
    ELSE( libType STREQUAL SHARED )
        SET( libTypeDesc "static" )
    ENDIF( libType STREQUAL SHARED )
    MESSAGE( STATUS "Planning to build ${libTypeDesc} library:  ${name}" )
ENDMACRO( GBX_ADD_LIBRARY name )

#
# GBX_ADD_HEADERS( install_subdir FILE0 [FILE1 FILE2 ...] )
#
# Specialization of INSTALL(FILES ...) to install header files.
# All files are installed into PREFIX/include/${PROJECT_NAME}/${install_subdir}
#
MACRO( GBX_ADD_HEADERS install_subdir )
    INSTALL( FILES ${ARGN} DESTINATION include/${PROJECT_NAME}/${install_subdir} )
ENDMACRO( GBX_ADD_HEADERS install_subdir )
#
# GBX_ADD_SHARED_FILES( install_subdir FILE0 [FILE1 FILE2 ...] )
#
# Specialization of INSTALL(FILES ...) to install shared files.
# All files are installed into PREFIX/share/${PROJECT_NAME}/${install_subdir} directory.
#
MACRO( GBX_ADD_SHARED_FILES install_subdir )
    INSTALL( FILES ${ARGN} DESTINATION share/${PROJECT_NAME}/${install_subdir} )
ENDMACRO( GBX_ADD_SHARED_FILES install_subdir )

#
# GBX_ADD_EXAMPLE( install_subdir makefile.in makefile.out [FILE0 FILE1 FILE2 ...] )
#
# Specialisation of INSTALL(FILES ...) to install examples.
# All files are installed into PREFIX/share/${PROJECT_NAME}/${install_subdir}.
# makefile is passed through CONFIGURE_FILE to add in correct include and library
# paths based on the install prefix.
#
MACRO( GBX_ADD_EXAMPLE install_subdir makefile.in makefile.out )
    CONFIGURE_FILE( ${CMAKE_CURRENT_SOURCE_DIR}/${makefile.in} ${CMAKE_CURRENT_BINARY_DIR}/${makefile.out} @ONLY)
    INSTALL( FILES ${CMAKE_CURRENT_BINARY_DIR}/${makefile.out} DESTINATION share/${PROJECT_NAME}/${install_subdir} RENAME CMakeLists.txt )
    INSTALL( FILES ${ARGN} DESTINATION share/${PROJECT_NAME}/${install_subdir} )
ENDMACRO( GBX_ADD_EXAMPLE install_subdir makefile )

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
MACRO( GBX_ADD_PKGCONFIG name desc ext_deps int_deps cflags libflags )
    SET( PKG_NAME ${name} )
    SET( PKG_DESC ${desc} )
    SET( PKG_CFLAGS ${cflags} )
    SET( PKG_LIBFLAGS ${libflags} )
    SET( PKG_EXTERNAL_DEPS ${${ext_deps}} )
    SET( PKG_INTERNAL_DEPS "" )
    IF( ${int_deps} )
        FOREACH( A ${${int_deps}} )
            SET( PKG_INTERNAL_DEPS "${PKG_INTERNAL_DEPS} -l${A}" )
        ENDFOREACH( A ${${int_deps}} )
    ENDIF( ${int_deps} )

    CONFIGURE_FILE( ${GBX_CMAKE_DIR}/pkgconfig.in ${CMAKE_CURRENT_BINARY_DIR}/${name}.pc @ONLY)
    INSTALL( FILES ${CMAKE_CURRENT_BINARY_DIR}/${name}.pc DESTINATION lib/pkgconfig/ )
ENDMACRO( GBX_ADD_PKGCONFIG name desc cflags deps libflags libs )

#
# This is a mechanism to register special items which are not
# executables or libraries. This function only records the name of
# the item to display it at the end of the cmake run and to submit
# to the Dashboard.
# Usage: GBX_ADD_ITEM( name )
#
MACRO( GBX_ADD_ITEM name )
    SET( templist ${ITEM_LIST} )
    LIST( APPEND templist ${name} )
    SET( ITEM_LIST ${templist} CACHE INTERNAL "Global list of special items to build" FORCE )
    MESSAGE( STATUS "Planning to build item:            ${name}" )
ENDMACRO( GBX_ADD_ITEM name )

#
# This is a mechanism to specify a license for the current source directory.
# Usage: GBX_ADD_LICENSE( license )
#
MACRO( GBX_ADD_LICENSE license )
    SET( templist ${LICENSE_LIST} )

    # get relative path to the current source dir
    STRING( LENGTH ${GBX_PROJECT_SOURCE_DIR} proj_src_dir_length )
    STRING( LENGTH ${CMAKE_CURRENT_SOURCE_DIR} current_src_dir_length )
    MATH( EXPR relative_path_length "${current_src_dir_length} - ${proj_src_dir_length} - 1" )
    MATH( EXPR relative_path_start "${proj_src_dir_length} + 1" )
    STRING( SUBSTRING ${CMAKE_CURRENT_SOURCE_DIR}
        ${relative_path_start} ${relative_path_length} current_src_dir_relative )

    # format the string to line up properly
    SET( spaces "A                                                                                       Z" )
    STRING( LENGTH ${current_src_dir_relative} current_src_dir_relative_length )
    MATH( EXPR white_space_length "60 - ${current_src_dir_relative_length}" )
    STRING( SUBSTRING ${spaces} 1 ${white_space_length} white_space )

    SET( line_item "${current_src_dir_relative}${white_space}${license}" )
    LIST( APPEND templist ${line_item} )
    SET( LICENSE_LIST ${templist} CACHE INTERNAL "Global list of directories and their licenses" FORCE )
#     MESSAGE( STATUS ${line_item} )
ENDMACRO( GBX_ADD_LICENSE license )

#
# Usage: GBX_ADD_TEST( testname exename [arg1 arg2 ...] )
# Example:  GBX_ADD_TEST( IntegerTest inttest --verbose )
#
MACRO( GBX_ADD_TEST name executable )
    ADD_TEST( ${name} ${executable} ${ARGN} )
    SET( templist ${TEST_LIST} )
    LIST( APPEND templist ${name} )
    SET( TEST_LIST ${templist} CACHE INTERNAL "Global list of (CTest) tests to build" FORCE )
#     MESSAGE( STATUS "Planning to build test:          ${name}" )
ENDMACRO( GBX_ADD_TEST name executable )

#
# Usage: GBX_NOT_ADD_EXECUTABLE( name reason )
#
MACRO( GBX_NOT_ADD_EXECUTABLE name reason )
  SET( templist ${EXE_NOT_LIST} )
  LIST( APPEND templist ${name} )
#  MESSAGE( STATUS "DEBUG: ${templist}" )
  SET( EXE_NOT_LIST ${templist} CACHE INTERNAL "Global list of executables NOT to build" FORCE )
  MESSAGE( STATUS "Not planning to build executable:  ${name} because ${reason}" )
ENDMACRO( GBX_NOT_ADD_EXECUTABLE name reason )

#
# Usage: GBX_NOT_ADD_LIBRARY( name reason )
#
MACRO( GBX_NOT_ADD_LIBRARY name reason )
  SET( templist ${LIB_NOT_LIST} )
  LIST( APPEND templist ${name} )
#  MESSAGE( STATUS "DEBUG: ${templist}" )
  SET( LIB_NOT_LIST ${templist} CACHE INTERNAL "Global list of libraries NOT to build" FORCE )
  MESSAGE( STATUS "Not planning to build library:     ${name} because ${reason}" )
ENDMACRO( GBX_NOT_ADD_LIBRARY name reason )

#
# This is a utility macro for internal use.
# Prints out list information: size, and items.
# Prints nothing if list is empty.
# Example: LIST_REPORT( EXE_LIST "executable(s)" )
#
# Tricky list stuff.
# see http://www.cmake.org/Wiki/CMakeMacroMerge for an example
#
MACRO( LIST_REPORT action item_name note L )
    SET( templist ${L} )
    LIST( LENGTH templist templist_length )
    SET( report_file ${GBX_PROJECT_BINARY_DIR}/cmake_config_report.txt )

    IF( templist_length GREATER 0 )
        LIST( SORT templist )

        MESSAGE( STATUS "${action} ${templist_length} ${item_name} ${note}:" )
        MESSAGE( STATUS "    ${templist}" )

        WRITE_FILE( ${report_file} "${action} ${templist_length} ${item_name}:" APPEND )
        WRITE_FILE( ${report_file} "    ${templist}" APPEND )
    ENDIF( templist_length GREATER 0 )
ENDMACRO( LIST_REPORT action item_name note L )

#
# This is a utility macro for internal use.
# Puts messages on the screen.
# Writes to a text file.
#
MACRO( GBX_CONFIG_REPORT )

    MESSAGE( STATUS "== SUMMARY ==" )

    # write configuration results to file (this line clears existing contents)
    SET( report_file ${GBX_PROJECT_BINARY_DIR}/cmake_config_report.txt )
    WRITE_FILE( ${report_file} "Autogenerated by CMake for ${PROJECT_NAME} project" )
#     WRITE_FILE( ${report_file} "Using Ice version ${ICE_VERSION}" )

    #
    # Print some results
    #
    MESSAGE( STATUS "Project name      ${PROJECT_NAME}")
    MESSAGE( STATUS "Project version   ${GBX_PROJECT_VERSION}")
    # would be nice to print out Orca version for satellite projects
    # for this we need an executable which is guaranteed to be installed.
    # then we can run it with --version flag.
    # IF( NOT ORCA_MOTHERSHIP )
    #     MESSAGE( STATUS "Using Orca version ${ORCA_VERSION}")
    # ENDIF( NOT ORCA_MOTHERSHIP )
    MESSAGE( STATUS "Platform              ${CMAKE_SYSTEM}")
    MESSAGE( STATUS "CMake version         ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}-patch ${CMAKE_PATCH_VERSION}")
    MESSAGE( STATUS "Install dir           ${CMAKE_INSTALL_PREFIX}")
    MESSAGE( STATUS "Default library type  ${GBX_DEFAULT_LIB_TYPE}")

    SET( note " " )
    LIST_REPORT( "Will build" "executables" ${note} "${EXE_LIST}" )
    LIST_REPORT( "Will build" "libraries" ${note} "${LIB_LIST}" )
    LIST_REPORT( "Will build" "CTest tests" ${note} "${TEST_LIST}" )
    LIST_REPORT( "Will build" "special items" ${note} "${ITEM_LIST}" )

    SET( note "(see above for reasons)" )
    LIST_REPORT( "Will NOT build" "executables" ${note} "${EXE_NOT_LIST}" )
    LIST_REPORT( "Will NOT build" "libraries" ${note} "${LIB_NOT_LIST}" )

ENDMACRO( GBX_CONFIG_REPORT )

#
# This is a utility macro for internal use.
#
MACRO( GBX_WRITE_MANIFEST )
    SET( output_file ${GBX_PROJECT_BINARY_DIR}/${PROJECT_NAME}_manifest.cmake )
    WRITE_FILE( ${output_file} "\# Autogenerated by CMake for ${PROJECT_NAME} project" )

    FOREACH( A ${LIB_LIST} )
        STRING( TOUPPER ${A} UPPERA )
        WRITE_FILE( ${output_file} "SET( ${UPPERA}_INSTALLED 1)" APPEND )
    ENDFOREACH( A ${LIB_LIST} )

    FOREACH( A ${LIB_NOT_LIST} )
        STRING( TOUPPER ${A} UPPERA )
        WRITE_FILE( ${output_file} "SET( ${UPPERA}_INSTALLED 0)" APPEND )
    ENDFOREACH( A ${LIB_NOT_LIST} )

    WRITE_FILE( ${output_file} " " APPEND )

    STRING( TOUPPER ${PROJECT_NAME} upper_project_name )
    WRITE_FILE( ${output_file} "SET( ${upper_project_name}_MANIFEST_LOADED 1)" APPEND )

    INSTALL( FILES ${output_file} DESTINATION . )
ENDMACRO( GBX_WRITE_MANIFEST )

MACRO( GBX_WRITE_LICENSE )
    SET( license_file ${GBX_PROJECT_SOURCE_DIR}/LICENSE )
    WRITE_FILE( ${license_file} "Autogenerated by CMake for ${PROJECT_NAME} project" )
    WRITE_FILE( ${license_file} "----------------------------------------------------------------------" APPEND )
    WRITE_FILE( ${license_file} "DIRECTORY                                                   license" APPEND )
    WRITE_FILE( ${license_file} "----------------------------------------------------------------------" APPEND )

    FOREACH( A ${LICENSE_LIST} )
        WRITE_FILE( ${license_file} ${A} APPEND )
    ENDFOREACH( A ${LICENSE_LIST} )

ENDMACRO( GBX_WRITE_LICENSE )

#
# This is a utility macro for internal use.
# Reset global lists of components, libraries, etc.
#
MACRO( GBX_RESET_ALL_TARGET_LISTS )
    # MESSAGE( STATUS "DEBUG: Resetting global target lists" )
    SET( EXE_LIST    "" CACHE INTERNAL "Global list of executables to build" FORCE )
    SET( LIB_LIST    "" CACHE INTERNAL "Global list of libraries to build" FORCE )
    SET( TEST_LIST   "" CACHE INTERNAL "Global list of CTest tests to build" FORCE )
    SET( ITEM_LIST   "" CACHE INTERNAL "Global list of special items to build" FORCE )

    SET( EXE_NOT_LIST  "" CACHE INTERNAL "Global list of executables NOT to build" FORCE )
    SET( LIB_NOT_LIST  "" CACHE INTERNAL "Global list of libraries NOT to build" FORCE )

    SET( LICENSE_LIST  "" CACHE INTERNAL "Global list of directories and their licenses" FORCE )
ENDMACRO( GBX_RESET_ALL_TARGET_LISTS )
