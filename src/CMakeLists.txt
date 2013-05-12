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
                                 File.cpp       File.h
                                 help.cpp
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
                                 Thread.cpp     Thread.h
                                 Timer.cpp      Timer.h
                                 TLSClient.cpp  TLSClient.h
                                 TLSServer.cpp  TLSServer.h
                                 utf8.cpp       utf8.h
                                 util.cpp       util.h
                                 wcwidth6.cpp)

#target_link_libraries (taskd_executable ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries (taskd_executable ${TASKD_LIBRARIES})
set_property (TARGET taskd_executable PROPERTY OUTPUT_NAME "taskd")

install (TARGETS taskd_executable DESTINATION ${TASKD_BINDIR})

set (CMAKE_BUILD_TYPE debug)
set (CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG}   -g -Wall")
set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -Wall")

