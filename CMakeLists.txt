cmake_minimum_required(VERSION 3.0)

message ("-- Configuring Library for GVM ical functions...")

project(gvm-ical
        VERSION 1.0
        LANGUAGES C)
set(SRCS
src/manage_pg_server.c
src/manage_utils.c
)

set(SQL
  ical.in.sql
)

message ("-- Install prefix: ${CMAKE_INSTALL_PREFIX}")

configure_file (VERSION.in VERSION)

if (POLICY CMP0005)
  cmake_policy (SET CMP0005 NEW)
endif (POLICY CMP0005)

SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

include (FindPkgConfig)

pkg_check_modules (LIBICAL REQUIRED libical>=1.00)
PKG_CHECK_MODULES (GLIB REQUIRED glib-2.0>=2.40)
if (NOT CMAKE_BUILD_TYPE)
  set (CMAKE_BUILD_TYPE Debug)
endif (NOT CMAKE_BUILD_TYPE)

OPTION (ENABLE_COVERAGE "Enable support for coverage analysis" OFF)
OPTION (DEBUG_FUNCTION_NAMES "Print function names on entry and exit" OFF)

## Retrieve git revision (at configure time)
include (GetGit)
if (NOT CMAKE_BUILD_TYPE MATCHES "Release")
  if (EXISTS "${CMAKE_SOURCE_DIR}/.git/")
    if (GIT_FOUND)
      Git_GET_REVISION(${CMAKE_SOURCE_DIR} ProjectRevision)
      set (GIT_REVISION "~git-${ProjectRevision}")
    else (GIT_FOUND)
      set (GIT_REVISION "~git")
    endif (GIT_FOUND)
  endif (EXISTS "${CMAKE_SOURCE_DIR}/.git/")
endif (NOT CMAKE_BUILD_TYPE MATCHES "Release")

# Set dev version if this is a development version and not a full release,
# unset (put value 0 or delete line) before a full release and reset after.
set (PROJECT_DEV_VERSION 1)

# If PROJECT_DEV_VERSION is set, the version string will be set to:
#   "major.minor.patch~dev${PROJECT_DEV_VERSION}${GIT_REVISION}"
# If PROJECT_DEV_VERSION is NOT set, the version string will be set to:
#   "major.minor.patch${GIT_REVISION}"
# For CMAKE_BUILD_TYPE "Release" the git revision will be empty.
if (PROJECT_DEV_VERSION)
  set (PROJECT_VERSION_SUFFIX "~dev${PROJECT_DEV_VERSION}")
endif (PROJECT_DEV_VERSION)

set (PROJECT_VERSION_STRING 
"${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}${PROJECT_VERSION_SUFFIX}${GIT_REVISION}")

find_program(PGCONFIG pg_config)

if (NOT PGCONFIG)
	message(FATAL_ERROR "Could not find pg_config")
endif ()
execute_process(COMMAND ${PGCONFIG} --version OUTPUT_VARIABLE PGVERSION OUTPUT_STRIP_TRAILING_WHITESPACE)
if (NOT "${PGVERSION}" MATCHES "${PG_REQUIRED_VERSION}")
	message(FATAL_ERROR "Wrong PostgreSQL version: found ${PGVERSION}, required ${PG_REQUIRED_VERSION}")
endif ()

message("-- Found PostreSQL Verion ${PGVERSION}")

execute_process(COMMAND ${PGCONFIG} --includedir --includedir-server OUTPUT_VARIABLE PostgreSQL_ACTUAL_INCLUDE_DIR OUTPUT_STRIP_TRAILING_WHITESPACE)

include_directories(${PostgreSQL_ACTUAL_INCLUDE_DIR})
include_directories("include")

set(CONTROLIN "control.in")
set(CONTROLOUT "gvm_ical.control")

set(SQLOUT "gvm_ical--${PROJECT_VERSION}.sql")

include_directories(${PostgreSQL_ACTUAL_INCLUDE_DIR})
include_directories("include")
include_directories (${GLIB_INCLUDE_DIRS})

execute_process(COMMAND ${PGCONFIG} --sharedir OUTPUT_VARIABLE PostgreSQL_SHARE_DIR OUTPUT_STRIP_TRAILING_WHITESPACE) # FIXME: I don't think this work on Windows
execute_process(COMMAND ${PGCONFIG} --pkglibdir OUTPUT_VARIABLE PostgreSQL_EXTLIB_DIR OUTPUT_STRIP_TRAILING_WHITESPACE) # FIXME: I don't think this work on Windows

add_custom_target(sqlscript ALL DEPENDS ${CMAKE_BINARY_DIR}/${SQLOUT})
add_custom_target(control ALL DEPENDS ${CMAKE_BINARY_DIR}/${CONTROLOUT})

## CPack configuration

set (CPACK_CMAKE_GENERATOR "Unix Makefiles")
set (CPACK_GENERATOR "TGZ")
set (CPACK_INSTALL_CMAKE_PROJECTS ".;gvm;ALL;/")
set (CPACK_MODULE_PATH "")
set (CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/COPYING")
set (CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README.md")
set (CPACK_RESOURCE_FILE_WELCOME "${CMAKE_SOURCE_DIR}/README.md")
set (CPACK_SOURCE_GENERATOR "TGZ")
set (CPACK_SOURCE_TOPLEVEL_TAG "")
set (CPACK_SYSTEM_NAME "")
set (CPACK_TOPLEVEL_TAG "")

set (CPACK_PACKAGE_VERSION "${PROJECT_VERSION_STRING}${PROJECT_VERSION_GIT}")

set (CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}-${CPACK_PACKAGE_VERSION}")
set (CPACK_SOURCE_PACKAGE_FILE_NAME "${PROJECT_NAME}-${CPACK_PACKAGE_VERSION}")
set (CPACK_PACKAGE_VENDOR "Greenbone Networks GmbH")
set (CPACK_SOURCE_IGNORE_FILES
"${CMAKE_BINARY_DIR}"
"/.git/"
"swp$"
)
message("GLIB ld flags: ${GLIB_LDFLAGS}")
add_library (${CMAKE_PROJECT_NAME} MODULE ${SRCS})

add_custom_command(
	OUTPUT ${CONTROLOUT}
	COMMAND cat ${CONTROLIN} | sed -e's/LIBNAME/${CMAKE_PROJECT_NAME}/'  > ${CMAKE_BINARY_DIR}/${CONTROLOUT}
	WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
	DEPENDS ${CONTROLIN})

add_custom_command(
    OUTPUT ${SQLOUT}
    COMMAND cat ical.in.sql > ${CMAKE_BINARY_DIR}/${SQLOUT}
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    DEPENDS ${SQL})

install(TARGETS ${CMAKE_PROJECT_NAME} DESTINATION "${PostgreSQL_EXTLIB_DIR}")
install(FILES "${CMAKE_BINARY_DIR}/${CONTROLOUT}" DESTINATION "${PostgreSQL_SHARE_DIR}/extension")
install(FILES "${CMAKE_BINARY_DIR}/${SQLOUT}" DESTINATION "${PostgreSQL_SHARE_DIR}/extension")