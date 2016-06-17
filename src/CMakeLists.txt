cmake_minimum_required (VERSION 2.8)
include_directories (${CMAKE_SOURCE_DIR}
                     ${CMAKE_SOURCE_DIR}/src
                     ${TASKD_INCLUDE_DIRS})

add_library (taskd admin.cpp
                   api.cpp
                   client.cpp
                   Color.cpp      Color.h
                   ConfigFile.cpp ConfigFile.h
                   config.cpp
                   daemon.cpp
                   diag.cpp
                   Database.cpp   Database.h
                   l10n/eng-USA.h
                   FS.cpp         FS.h
                   help.cpp
                   i18n.h
                   init.cpp
                   ISO8601.cpp    ISO8601.h
                   JSON.cpp       JSON.h
                   Lexer.cpp      Lexer.h
                   Log.cpp        Log.h
                   Msg.cpp        Msg.h
                   Nibbler.cpp    Nibbler.h
                   RX.cpp         RX.h
                   Server.cpp     Server.h
                   status.cpp
                   Task.cpp       Task.h
                   text.cpp       text.h
                   Timer.cpp      Timer.h
                   TLSClient.cpp  TLSClient.h
                   TLSServer.cpp  TLSServer.h
                   utf8.cpp       utf8.h
                   util.cpp       util.h
                   wcwidth6.cpp)

find_package(Threads REQUIRED)
add_executable (taskd_executable taskd.cpp)
target_link_libraries (taskd_executable taskd ${TASKD_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
set_property (TARGET taskd_executable PROPERTY OUTPUT_NAME "taskd")
install (TARGETS taskd_executable DESTINATION ${TASKD_BINDIR})

