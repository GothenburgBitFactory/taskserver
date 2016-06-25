cmake_minimum_required (VERSION 2.8)

# See this CMake issue before complaining about the following.
# https://cmake.org/Bug/view.php?id=16062
if(POLICY CMP0037)
  cmake_policy(SET CMP0037 OLD)
endif()

include_directories (${CMAKE_SOURCE_DIR}
                     ${CMAKE_SOURCE_DIR}/src
                     ${CMAKE_SOURCE_DIR}/test
                     ${TASKD_INCLUDE_DIRS})

set (test_SRCS color.t config.t fs.t iso8601d.t iso8601p.t lexer.t msg.t
               nibbler.t text.t util.t utf8.t json.t json_test)

add_custom_target (test ./run_all --verbose
                        DEPENDS ${test_SRCS} taskd_executable
                        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/test)

add_custom_target (build_tests DEPENDS ${test_SRCS}
                               WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/test)

foreach (src_FILE ${test_SRCS})
  add_executable (${src_FILE} "${src_FILE}.cpp" test.cpp)
  target_link_libraries (${src_FILE} taskd ${TASKD_LIBRARIES})
endforeach (src_FILE)

configure_file(run_all run_all COPYONLY)
configure_file(problems problems COPYONLY)
