cmake_minimum_required (VERSION 2.8)
include_directories (${CMAKE_SOURCE_DIR}
                     ${CMAKE_SOURCE_DIR}/src
                     ${CMAKE_SOURCE_DIR}/test
                     ${TASKD_INCLUDE_DIRS})

set (test_SRCS color.t config.t date.t directory.t duration.t file.t msg.t
               nibbler.t path.t rx.t text.t util.t utf8.t width.t json.t
               json_test)

message ("-- Configuring run_all")
if (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})

set (TESTBLOB "./*.t")
if (CYGWIN)
  set (TESTBLOB "./*.t ./*.t.exe")
endif (CYGWIN)

else (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})

set (TESTBLOB "${CMAKE_SOURCE_DIR}/test/*.t ${CMAKE_BINARY_DIR}/test/*.t")
if (CYGWIN)
  set (TESTBLOB "${CMAKE_SOURCE_DIR}/test/*.t ${CMAKE_BINARY_DIR}/test/*.t.exe")
endif (CYGWIN)

endif (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})

configure_file (run_all.in run_all)

add_custom_target (test ./run_all --verbose
                        DEPENDS ${test_SRCS}
                        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/test)

add_custom_target (build_tests DEPENDS ${test_SRCS}
                               WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/test)

foreach (src_FILE ${test_SRCS})
  add_executable (${src_FILE} "${src_FILE}.cpp"
                              ../src/Color.cpp
                              ../src/ConfigFile.cpp
                              ../src/Date.cpp
                              ../src/Directory.cpp
                              ../src/Duration.cpp
                              ../src/File.cpp
                              ../src/JSON.cpp
                              ../src/Msg.cpp
                              ../src/Nibbler.cpp
                              ../src/Path.cpp
                              ../src/RX.cpp
                              ../src/text.cpp
                              ../src/utf8.cpp
                              ../src/util.cpp
                              ../src/wcwidth6.cpp
                              test.cpp)
  target_link_libraries (${src_FILE} ${TASKD_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
endforeach (src_FILE)

