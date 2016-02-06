cmake_minimum_required (VERSION 2.8)
if(POLICY CMP0037)
  cmake_policy(SET CMP0037 OLD)
endif()
include_directories (${CMAKE_SOURCE_DIR}
                     ${CMAKE_SOURCE_DIR}/src
                     ${CMAKE_SOURCE_DIR}/test
                     ${TASKD_INCLUDE_DIRS})

set (test_SRCS color.t config.t fs.t iso8601d.t iso8601p.t lexer.t msg.t
               nibbler.t rx.t text.t util.t utf8.t json.t json_test)

message ("-- Configuring tests")
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

add_custom_target (test ./run_all --verbose
                        DEPENDS ${test_SRCS} taskd_executable
                        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/test)

add_custom_target (build_tests DEPENDS ${test_SRCS}
                               WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/test)

foreach (src_FILE ${test_SRCS})
  add_executable (${src_FILE} "${src_FILE}.cpp"
                              ../src/Color.cpp
                              ../src/ConfigFile.cpp
                              ../src/FS.cpp
                              ../src/JSON.cpp
                              ../src/ISO8601.cpp
                              ../src/Lexer.cpp
                              ../src/Msg.cpp
                              ../src/Nibbler.cpp
                              ../src/RX.cpp
                              ../src/text.cpp
                              ../src/utf8.cpp
                              ../src/util.cpp
                              ../src/wcwidth6.cpp
                              test.cpp)
  target_link_libraries (${src_FILE} ${TASKD_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
endforeach (src_FILE)

configure_file(run_all run_all COPYONLY)
configure_file(problems problems COPYONLY)
