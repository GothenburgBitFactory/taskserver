cmake_minimum_required (VERSION 2.8)
message ("-- Configuring man pages")
set (man_FILES taskd.1)
foreach (man_FILE ${man_FILES})
  configure_file (
    man/${man_FILE}.in
    man/${man_FILE})
  install (FILES ${CMAKE_CURRENT_BINARY_DIR}/man/${man_FILE} DESTINATION ${TASKD_MAN1DIR})
endforeach (man_FILE)
