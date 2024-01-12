# Copyright (C) 2020-2021 Greenbone AG
#
# SPDX-License-Identifier: GPL-3.0-or-later
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as
# published by the Free Software Foundation, either version 3 of the
# License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

cmake_minimum_required(VERSION 3.0)

message ("-- Configuring PostgreSQL extension for GVMd functions...")

project(pg-gvm
        VERSION 22.6.4
        LANGUAGES C)

# List all sourcefiles
set(SRCS
    src/regexp.c
    src/ical.c
    src/ical_utils.c
    src/hosts.c
    src/array.c)

# List all sql input files
set(SQL
    sql/regexp.in.sql
    sql/hosts.in.sql
    sql/ical.in.sql)

message ("-- Install prefix: ${CMAKE_INSTALL_PREFIX}")

configure_file (VERSION.in VERSION)

if (POLICY CMP0005)
  cmake_policy (SET CMP0005 NEW)
endif (POLICY CMP0005)

SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

include (FindPkgConfig)

# Check if libical is installed
pkg_check_modules (LIBICAL REQUIRED libical>=1.00)
pkg_check_modules (GLIB REQUIRED glib-2.0>=2.42)
pkg_check_modules (LIBGVM_BASE REQUIRED libgvm_base>=22.6)

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
set (PROJECT_DEV_VERSION 0)

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

# All versions with this API are compatible
# If an sql update is required the minor version MUST be increased
set (PROJECT_API_VERSION "${CMAKE_PROJECT_VERSION_MAJOR}.${CMAKE_PROJECT_VERSION_MINOR}")

# Check if pg_config is available
find_program(PGCONFIG pg_config)

if (NOT PGCONFIG)
	message(FATAL_ERROR "Could not find pg_config")
endif ()

execute_process(COMMAND ${PGCONFIG} --version OUTPUT_VARIABLE PGVERSION OUTPUT_STRIP_TRAILING_WHITESPACE)
if (NOT "${PGVERSION}" MATCHES "${PG_REQUIRED_VERSION}")
	message(FATAL_ERROR "Wrong PostgreSQL version: found ${PGVERSION}, required ${PG_REQUIRED_VERSION}")
endif ()

# Retrieve postgres include directories
execute_process(COMMAND ${PGCONFIG} --includedir --includedir-server OUTPUT_VARIABLE PostgreSQL_ACTUAL_INCLUDE_DIR OUTPUT_STRIP_TRAILING_WHITESPACE)

find_package (PostgreSQL REQUIRED)
if (NOT PostgreSQL_FOUND)
  message (SEND_ERROR "The PostgreSQL library is required.")
endif (NOT PostgreSQL_FOUND)
string (REGEX MATCH "^[ \t]*\([0-9]+\)\\.\([0-9]+\)\(.*\)" TEMP ${PostgreSQL_VERSION_STRING})
if (NOT CMAKE_MATCH_1)
  message (SEND_ERROR "Error matching PostgreSQL version.")
elseif ((CMAKE_MATCH_1 EQUAL 9 AND CMAKE_MATCH_2 LESS 6)
        OR (CMAKE_MATCH_1 LESS 9))
  message (SEND_ERROR "PostgreSQL version >= 9.6 is required")
  message (STATUS "PostgreSQL version ${CMAKE_MATCH_1}.${CMAKE_MATCH_2}${CMAKE_MATCH_3}")
endif (NOT CMAKE_MATCH_1)

# Set include directories
include_directories(${PostgreSQL_ACTUAL_INCLUDE_DIR} ${GLIB_INCLUDE_DIRS} ${LIBGVM_BASE_INCLUDE_DIRS})
include_directories("include")
link_libraries(${LIBICAL_LIBRARIES} ${LIBGVM_BASE_LDFLAGS})
set (CMAKE_SHARED_LINKER_FLAGS "-Wl,--as-needed")
# Set control file for postgres extension definition
set(CONTROLIN "control.in")
set(CONTROLOUT "pg-gvm.control")

# Set SQL output file using the version to match postgres conventions
set(SQLOUT "pg-gvm--${PROJECT_API_VERSION}.sql")

# Determine the postgres lib dirs
execute_process(COMMAND ${PGCONFIG} --sharedir OUTPUT_VARIABLE PostgreSQL_SHARE_DIR OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND ${PGCONFIG} --pkglibdir OUTPUT_VARIABLE PostgreSQL_EXTLIB_DIR OUTPUT_STRIP_TRAILING_WHITESPACE)

# Custom targets to build SQL and control files
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
set (CPACK_PACKAGE_VENDOR "Greenbone AG")
set (CPACK_SOURCE_IGNORE_FILES
"${CMAKE_BINARY_DIR}"
"/.git/"
"swp$"
)

# Define the library
add_library (${CMAKE_PROJECT_NAME} MODULE ${SRCS})

# Prepare control file
configure_file (${CONTROLIN} ${CONTROLOUT})

# Prepare SQL file
add_custom_command(
    OUTPUT ${SQLOUT}
    COMMAND mkdir -p ${CMAKE_BINARY_DIR}/sqlin
    COMMAND cp ${SQL} ${CMAKE_BINARY_DIR}/sqlin/
    COMMAND cd ${CMAKE_BINARY_DIR}/sqlin/ && find -type f | xargs cat > ${CMAKE_BINARY_DIR}/${SQLOUT}
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    DEPENDS ${SQL})

# Define install targets
if (CMAKE_INSTALL_DEV_PREFIX)
  install(TARGETS ${CMAKE_PROJECT_NAME} DESTINATION "${CMAKE_INSTALL_DEV_PREFIX}/lib/postgresql")
  configure_file("${CMAKE_SOURCE_DIR}/pg-gvm-make-dev-links.in" "${CMAKE_BINARY_DIR}/pg-gvm-make-dev-links" @ONLY)
  install(
    FILES "${CMAKE_BINARY_DIR}/pg-gvm-make-dev-links"
    DESTINATION "${CMAKE_INSTALL_DEV_PREFIX}/sbin"
    PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
  )
  install(FILES "${CMAKE_BINARY_DIR}/${CONTROLOUT}" DESTINATION "${CMAKE_INSTALL_DEV_PREFIX}/share/postgresql/extension")
  install(FILES "${CMAKE_BINARY_DIR}/${SQLOUT}" DESTINATION "${CMAKE_INSTALL_DEV_PREFIX}/share/postgresql/extension")
  file(GLOB SQL_UPDATE_FILES ${CMAKE_SOURCE_DIR}/sql/update/*.sql)
  install(FILES ${SQL_UPDATE_FILES} DESTINATION "${CMAKE_INSTALL_DEV_PREFIX}/share/postgresql/extension")
  message(NOTICE "${CMAKE_PROJECT_NAME} installed with prefix. Run ${CMAKE_INSTALL_DEV_PREFIX}/sbin/pg-gvm-make-dev-links as root to create symlinks to this installation.")
else (CMAKE_INSTALL_DEV_PREFIX)
  install(TARGETS ${CMAKE_PROJECT_NAME} DESTINATION "${PostgreSQL_EXTLIB_DIR}")
  install(FILES "${CMAKE_BINARY_DIR}/${CONTROLOUT}" DESTINATION "${PostgreSQL_SHARE_DIR}/extension")
  install(FILES "${CMAKE_BINARY_DIR}/${SQLOUT}" DESTINATION "${PostgreSQL_SHARE_DIR}/extension")
  file(GLOB SQL_UPDATE_FILES ${CMAKE_SOURCE_DIR}/sql/update/*.sql)
  install(FILES ${SQL_UPDATE_FILES} DESTINATION "${PostgreSQL_SHARE_DIR}/extension")
endif (CMAKE_INSTALL_DEV_PREFIX)
