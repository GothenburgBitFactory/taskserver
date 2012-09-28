cmake_minimum_required (VERSION 2.8)

include (CheckFunctionExists)
include (CheckStructHasMember)

set (HAVE_CMAKE true)

project (taskd)
set (PROJECT_VERSION "0.7.0")

SET (TASKD_MAN1DIR share/man/man1  CACHE STRING "Installation directory for man pages, section 1")
SET (TASKD_DOCDIR  share/doc/taskd CACHE STRING "Installation directory for doc files")
SET (TASKD_EXTDIR  libexec/taskd   CACHE STRING "Installation directory for the extensions")
SET (TASKD_BINDIR  bin             CACHE STRING "Installation directory for the program")

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set (LINUX true)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set (DARWIN true)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
  set (FREEBSD true)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "OpenBSD")
  set (OPENBSD true)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "NetBSD")
  set (NETBSD true)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "SunOS")
  set (SOLARIS true)
else (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set (UNKNOWN true)
endif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

message ("-- Looking for SHA1 references")
if (EXISTS ${CMAKE_SOURCE_DIR}/.git/index)
  set (HAVE_COMMIT true)
  execute_process (COMMAND git log -1 --pretty=format:%h
                   WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                   OUTPUT_VARIABLE COMMIT)
  configure_file ( ${CMAKE_SOURCE_DIR}/commit.h.in
                   ${CMAKE_SOURCE_DIR}/commit.h)
  message ("-- Found SHA1 reference: ${COMMIT}")
endif (EXISTS ${CMAKE_SOURCE_DIR}/.git/index)

set (PACKAGE "${PROJECT_NAME}")
set (VERSION "${PROJECT_VERSION}")
set (PACKAGE_BUGREPORT "support@taskwarrior.org")
set (PACKAGE_NAME "${PACKAGE}")
set (PACKAGE_TARNAME "${PACKAGE}")
set (PACKAGE_VERSION "${VERSION}")
set (PACKAGE_STRING "${PACKAGE} ${VERSION}")

message ("-- Looking for pthread")
find_path (PTHREAD_INCLUDE_DIR pthread.h)
find_library (PTHREAD_LIBRARY NAMES pthread)
if (PTHREAD_INCLUDE_DIR AND PTHREAD_LIBRARY)
  message ("-- Found pthread: ${PTHREAD_LIBRARY}")
  set (HAVE_LIBPTHREAD true)
  set (TASKD_INCLUDE_DIRS ${TASKD_INCLUDE_DIRS} ${PTHREAD_INCLUDE_DIR})
  set (TASKD_LIBRARIES    ${TASKD_LIBRARIES}    ${PTHREAD_LIBRARIES})
endif (PTHREAD_INCLUDE_DIR AND PTHREAD_LIBRARY)

check_function_exists (timegm HAVE_TIMEGM)

check_struct_has_member ("struct tm"   tm_gmtoff    time.h                   HAVE_TM_GMTOFF)
check_struct_has_member ("struct stat" st_birthtime "sys/types.h;sys/stat.h" HAVE_ST_BIRTHTIME)

message ("-- Configuring cmake.h")
configure_file (
  ${CMAKE_SOURCE_DIR}/cmake.h.in
  ${CMAKE_SOURCE_DIR}/cmake.h)

add_subdirectory (src)
add_subdirectory (doc)
if (EXISTS test)
  add_subdirectory (test EXCLUDE_FROM_ALL)
endif (EXISTS test)

set (doc_FILES NEWS ChangeLog INSTALL AUTHORS COPYING)
foreach (doc_FILE ${doc_FILES})
  install (FILES ${doc_FILE}  DESTINATION ${TASKD_DOCDIR})
endforeach (doc_FILE)

# ---

set (CPACK_SOURCE_GENERATOR "TGZ")
set (CPACK_SOURCE_PACKAGE_FILE_NAME ${PACKAGE_NAME}-${PACKAGE_VERSION})
set (CPACK_SOURCE_IGNORE_FILES  "CMakeCache" "CMakeFiles" "CPackConfig" "CPackSourceConfig"
                                "_CPack_Packages" "cmake_install" "install_manifest"
                                "Makefile$" "test" "package-config"
                                "src/taskd$" "/\\.gitignore" "/\\.git/" "swp$")
include (CPack)
