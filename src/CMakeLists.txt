cmake_minimum_required (VERSION 2.8)
include_directories (${CMAKE_SOURCE_DIR}
                     ${CMAKE_SOURCE_DIR}/src
                     ${TASKD_INCLUDE_DIRS})

add_executable (taskd_executable taskd.cpp
                                 admin.cpp
                                 api.cpp
                                 client.cpp
                                 Color.cpp      Color.h
                                 ConfigFile.cpp ConfigFile.h
                                 config.cpp
                                 daemon.cpp
                                 diag.cpp
                                 Database.cpp   Database.h
                                 Date.cpp       Date.h
                                 Directory.cpp  Directory.h
                                 Duration.cpp   Duration.h
                                 l10n/eng-USA.h
                                 File.cpp       File.h
                                 help.cpp
                                 i18n.h
                                 init.cpp
                                 JSON.cpp       JSON.h
                                 Log.cpp        Log.h
                                 Msg.cpp        Msg.h
                                 Nibbler.cpp    Nibbler.h
                                 Path.cpp       Path.h
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

target_link_libraries (taskd_executable ${TASKD_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

set_property (TARGET taskd_executable PROPERTY OUTPUT_NAME "taskd")

install (TARGETS taskd_executable DESTINATION ${TASKD_BINDIR})

