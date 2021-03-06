CMAKE_MINIMUM_REQUIRED ( VERSION 2.6 )

## ===========================================================================
##  Mozilla SDK
## ===========================================================================

## Ensure that the variable PKG_OS_ARCH exists! If undefined, define it with
## the value "x86_32"
IF ( NOT PKG_OS_ARCH )
  SET ( PKG_OS_ARCH x86_32 )
ENDIF ( NOT PKG_OS_ARCH )

IF ( ${PKG_OS_ARCH} STREQUAL "x86_64" )
    set( PKG_ARCH_DIR "win32-x86_64" )
ELSE ( ${PKG_OS_ARCH} STREQUAL "x86_64" )
    set( PKG_ARCH_DIR "win32-ix86" )
ENDIF ( ${PKG_OS_ARCH} STREQUAL "x86_64" )

## ===========================================================================
## Please verify that the following variables contain suitable values:
## ===========================================================================

IF ( WIN32 )
  SET ( UICC          uicc )
ENDIF ( WIN32 )

IF ( UNIX )
  SET ( PlatformDir ../unix )
ENDIF ( UNIX )

## ===========================================================================
##  No need to modify below this point!
## ===========================================================================


## ===========================================================================
##  Project Information
## ===========================================================================
PROJECT ( tkribbon )
## Package version information:
SET ( PKG_NAME          ${PROJECT_NAME} )
SET ( PKG_MAJOR_VERSION 1 )
SET ( PKG_MINOR_VERSION 1 )
SET ( PKG_BUILD_VERSION 0 )
## Author:
SET ( PKG_VENDOR        "Georgios Petasis" )

## Arange project version information...
SET ( PKG_VERSION
     "${PKG_MAJOR_VERSION}.${PKG_MINOR_VERSION}.${PKG_BUILD_VERSION}" )
SET ( PKG_VERSION
     "${PKG_MAJOR_VERSION}.${PKG_MINOR_VERSION}" )
SET ( PKG_NAME_VERSION  ${PKG_NAME}-${PKG_VERSION} )
SET ( PKG_HOME_DIR      ${PROJECT_SOURCE_DIR} )

## ===========================================================================
##  Project sources
## ===========================================================================
SET ( PKG_SOURCES
          generic/TkRibbon.cpp
    )

SET ( NO_MSVCRT ON )

IF ( WIN32 )
  SET ( PlatformDir ${PKG_HOME_DIR}/win )
  
  ##
  ## Custom command: compile the Ribbon XML specification into a resource.
  ##
  ADD_CUSTOM_COMMAND ( OUTPUT  ${PlatformDir}/SimpleRibbonUI.rc
                       DEPENDS ${PlatformDir}/SimpleRibbon.xml
      COMMAND ${UICC} SimpleRibbon.xml SimpleRibbon.bml
                      /header:SimpleRibbonUI.h /res:SimpleRibbonUI.rc
                      /name:APPLICATION
    WORKING_DIRECTORY ${PlatformDir} )

  SET ( PKG_SOURCES ${PKG_SOURCES}
            ${PlatformDir}/Application.cpp
            ${PlatformDir}/CommandHandler.cpp
            ${PlatformDir}/PropertySet.cpp
            ${PlatformDir}/RibbonFramework.cpp
            ${PlatformDir}/TkRibbon.cpp
            ${PlatformDir}/SimpleRibbonUI.rc
       )
ENDIF ( WIN32 )



## Greet the user...
MESSAGE ( STATUS "===========================================================" )
MESSAGE ( STATUS " Welcome to the ${PKG_NAME} ${PKG_VERSION} build system!" )
MESSAGE ( STATUS "  * Selected generator:  ${CMAKE_GENERATOR}" )
MESSAGE ( STATUS "  * Operating System ID: "
        ${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_VERSION}-${CMAKE_SYSTEM_PROCESSOR} )
MESSAGE ( STATUS "  * Installation Directory: " ${CMAKE_INSTALL_PREFIX} )
MESSAGE ( STATUS "===========================================================" )

## Allow additional cmake modules in the modules directory...
SET ( CMAKE_MODULE_PATH cmake/modules )

## ===========================================================================
##  Utilities
## ===========================================================================
SET ( RM           ${CMAKE_COMMAND} -E remove -f )
SET ( RM_FLAGS     )
SET ( RMDIR        ${CMAKE_COMMAND} -E remove_directory )
SET ( RMDIR_FLAGS  )
SET ( MKDIR        ${CMAKE_COMMAND} -E make_directory )
SET ( CP           ${CMAKE_COMMAND} -E copy )

## ===========================================================================
##  Locate Tcl/Tk
## ===========================================================================
MESSAGE ( STATUS "Searching for Tcl/Tk..." )
FIND_PACKAGE ( TCL REQUIRED )
FIND_PACKAGE ( TclStub REQUIRED )

## Tcl/Tk info (useful for debug purposes)...
  MESSAGE ( STATUS "  TCL_TCLSH:               " ${TCL_TCLSH} )
# MESSAGE ( STATUS "  TCL_INCLUDE_PATH:        " ${TCL_INCLUDE_PATH} )
  MESSAGE ( STATUS "  TCL_STUB_LIBRARY:        " ${TCL_STUB_LIBRARY} )
# MESSAGE ( STATUS "  TCL_LIBRARY:             " ${TCL_LIBRARY} )
# MESSAGE ( STATUS "  TK_WISH:                 " ${TK_WISH} )
# MESSAGE ( STATUS "  TK_INCLUDE_PATH:         " ${TK_INCLUDE_PATH} )
# MESSAGE ( STATUS "  TK_STUB_LIBRARY:         " ${TK_STUB_LIBRARY} )
# MESSAGE ( STATUS "  TK_LIBRARY:              " ${TK_LIBRARY} )

## Enable Tcl/Tk stubs globally...
ADD_DEFINITIONS     ( -DUSE_TCL_STUBS )
ADD_DEFINITIONS     ( -DUSE_TK_STUBS )
ADD_DEFINITIONS     ( -DTCL_THREADS )
INCLUDE_DIRECTORIES ( ${TCL_INCLUDE_PATH} )
INCLUDE_DIRECTORIES ( ${TK_INCLUDE_PATH} )
LINK_LIBRARIES      ( ${TCL_STUB_LIBRARY} )
LINK_LIBRARIES      ( ${TK_STUB_LIBRARY} )

## Various useful definitions...
IF ( WIN32 )
  INCLUDE_DIRECTORIES ( ${PKG_HOME_DIR}/../win )
  ADD_DEFINITIONS     ( -DWIN32 )
  ADD_DEFINITIONS     ( -D_WIN32 )
  ADD_DEFINITIONS     ( -D_WINDOWS )
  ADD_DEFINITIONS     ( -D__WIN32__ )
  SET ( CMAKE_SHARED_LIBRARY_PREFIX lib )
  LINK_LIBRARIES      ( propsys shlwapi )
ENDIF ( WIN32 )

IF ( UNIX )
  INCLUDE_DIRECTORIES ( ${PKG_HOME_DIR}/../unix )
  ADD_DEFINITIONS     ( -DUNIX )
ENDIF ( UNIX )

IF ( WIN32 AND NO_MSVCRT )
  STRING ( REPLACE /MD  /MT  CMAKE_C_FLAGS_RELEASE   ${CMAKE_C_FLAGS_RELEASE}  )
  STRING ( REPLACE /MDd /MTd CMAKE_C_FLAGS_DEBUG     ${CMAKE_C_FLAGS_DEBUG}    )
  STRING ( REPLACE /MD  /MT  CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE})
  STRING ( REPLACE /MDd /MTd CMAKE_CXX_FLAGS_DEBUG   ${CMAKE_CXX_FLAGS_DEBUG}  )
  SET (    CMAKE_EXE_LINKER_FLAGS
        "${CMAKE_EXE_LINKER_FLAGS} /nodefaultlib:MSVCRT.LIB" )
  SET (    CMAKE_EXE_LINKER_FLAGS_RELEASE
        "${CMAKE_EXE_LINKER_FLAGS_RELEASE} LIBCMT.LIB" )
  SET (    CMAKE_EXE_LINKER_FLAGS_DEBUG
        "${CMAKE_EXE_LINKER_FLAGS_DEBUG} LIBCMTD.LIB" )
  SET (    CMAKE_SHARED_LINKER_FLAGS
        "${CMAKE_SHARED_LINKER_FLAGS} /nodefaultlib:MSVCRT.LIB" )
  SET (    CMAKE_SHARED_LINKER_FLAGS_RELEASE
        "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} LIBCMT.LIB" )
  SET (    CMAKE_SHARED_LINKER_FLAGS_DEBUG
        "${CMAKE_SHARED_LINKER_FLAGS_DEBUG}
         /nodefaultlib:LIBCMT.LIB LIBCMTD.LIB" )
  ADD_DEFINITIONS ( -D_STATIC_CPPLIB )
  ADD_DEFINITIONS ( -D_CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES=1 )
  ADD_DEFINITIONS ( -D_CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES_COUNT=1 )
ENDIF ( WIN32 AND NO_MSVCRT )

## Package global definitions...
ADD_DEFINITIONS ( -DPACKAGE_NAME="${PKG_NAME}" )
ADD_DEFINITIONS ( -DPACKAGE_VERSION="${PKG_VERSION}" )
ADD_DEFINITIONS ( -DPACKAGE_NAME_VERSION="${PKG_NAME_VERSION}" )

## ===========================================================================
##  Declare the package shared library target...
## ===========================================================================
SET ( PKG_TARGET_LIB_NAME ${PKG_NAME}${PKG_VERSION} )
MESSAGE ( STATUS "      + Shared Library: ${PKG_NAME}" )
ADD_LIBRARY ( ${PKG_TARGET_LIB_NAME} SHARED ${PKG_SOURCES} )

## ===========================================================================
##  Generate the pkgIndex.tcl file...
## ===========================================================================
FILE ( WRITE library/pkgIndex.tcl 
"namespace eval ::tkribbon {
  if {[catch {package require platform}]} {
    ## Set platform variable to \"win32-x86_64\" if using 64-bit Tcl.
    variable platform win32-ix86
  } else {
    variable platform [::platform::generic]
  }
  variable version ${PKG_VERSION}
};# namespace tkribbon

package ifneeded tkribbon ${PKG_VERSION} \\
  [list load [file join $dir $::tkribbon::platform lib${PKG_TARGET_LIB_NAME}[info sharedlibextension]] tkribbon]" )

## ===========================================================================
##  Declare the package install targets...
## ===========================================================================

# SET ( CMAKE_INSTALL_PREFIX C:/TclApps/Tcl/lib )

INSTALL ( TARGETS ${PKG_TARGET_LIB_NAME}
          RUNTIME DESTINATION ${PKG_TARGET_LIB_NAME}/${PKG_ARCH_DIR}
          LIBRARY DESTINATION ${PKG_TARGET_LIB_NAME}/${PKG_ARCH_DIR}
          ARCHIVE DESTINATION ${PKG_TARGET_LIB_NAME}/${PKG_ARCH_DIR}
 )
INSTALL ( FILES library/pkgIndex.tcl library/tkribbon.tcl
          DESTINATION ${PKG_TARGET_LIB_NAME} )

## ===========================================================================
##  Project Package Generator
## ===========================================================================
MESSAGE ( STATUS "Generating Packaging Information..." )
SET ( CPACK_PACKAGE_NAME                          ${PKG_NAME} )
SET ( CPACK_PACKAGE_VENDOR                        ${PKG_VENDOR} )
SET ( CPACK_PACKAGE_VERSION_MAJOR                 ${PKG_MAJOR_VERSION} )
SET ( CPACK_PACKAGE_VERSION_MINOR                 ${PKG_MINOR_VERSION} )
SET ( CPACK_PACKAGE_VERSION_PATCH                 ${PKG_BUILD_VERSION} )
SET ( CPACK_SOURCE_PACKAGE_FILE_NAME              ${PKG_NAME_VERSION} )
SET ( CPACK_PACKAGE_INSTALL_DIRECTORY             ${PKG_NAME_VERSION} )
SET ( CPACK_PACKAGE_INSTALL_REGISTRY_KEY          ${PKG_NAME_VERSION} )
SET ( CPACK_RESOURCE_FILE_LICENSE
      ${PKG_HOME_DIR}/LICENSE )
SET ( CPACK_RESOURCE_FILE_README
      ${PKG_HOME_DIR}/README.md )
SET ( CPACK_PACKAGE_EXECUTABLES
     ${PKG_NAME} )
# SET ( CPACK_PACKAGE_DESCRIPTION_FILE     "" )
SET ( CPACK_PACKAGE_DESCRIPTION_SUMMARY
     "${PKG_NAME} is a natural language engineering platform/IDE" )

## ===========================================================================
##  Project Package Generator: NSIS Additional Information
## ===========================================================================
IF(WIN32 AND NOT UNIX)
  SET ( CPACK_NSIS_HELP_LINK      "http://www.ellogon.org" )
  SET ( CPACK_NSIS_URL_INFO_ABOUT "http://www.ellogon.org" )
  SET ( CPACK_NSIS_CONTACT        "petasis@iit.demokritos.gr" )
ELSE(WIN32 AND NOT UNIX)
ENDIF(WIN32 AND NOT UNIX)

## ===========================================================================
##  Project Package Generator: Load the package generator (CPack)
## ===========================================================================
INCLUDE ( CPack )
MESSAGE ( STATUS "  * Binary Package Generator: " ${CPACK_GENERATOR} )
MESSAGE ( STATUS "  * Source Package Generator: " ${CPACK_SOURCE_GENERATOR} )
MESSAGE ( STATUS "  * Installer package name:   " ${CPACK_PACKAGE_FILE_NAME} )
