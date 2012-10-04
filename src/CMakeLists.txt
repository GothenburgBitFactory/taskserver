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
                                 Socket.cpp     Socket.h
                                 status.cpp
                                 text.cpp       text.h
                                 Timer.cpp      Timer.h
                                 utf8.cpp       utf8.h
                                 util.cpp       util.h)

set_property (TARGET taskd_executable PROPERTY OUTPUT_NAME "taskd")

install (TARGETS taskd_executable DESTINATION ${TASKD_BINDIR})

set (CMAKE_BUILD_TYPE debug)
set (CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG}   -g -Wall")
set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -Wall")

