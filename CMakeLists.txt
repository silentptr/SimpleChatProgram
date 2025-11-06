cmake_minimum_required(VERSION 3.30)
project(SimpleChatProgram VERSION 1.1.0 LANGUAGES CXX)

option(BUILD_CLIENT_CLI "Build the client CLI" OFF)
option(BUILD_SERVER_CLI "Build the server CLI" ON)
option(BUILD_CLIENT_GUI "Build the client GUI" ON)

message(STATUS "Build client CLI: ${BUILD_CLIENT_CLI}")
message(STATUS "Build server CLI: ${BUILD_SERVER_CLI}")
message(STATUS "Build client GUI: ${BUILD_CLIENT_GUI}")

find_package(Boost 1.87 REQUIRED)
message(STATUS "Boost version: ${Boost_VERSION}")

if(BUILD_CLIENT_CLI OR BUILD_SERVER_CLI)
    find_package(Curses REQUIRED)
endif()

if(BUILD_CLIENT_GUI)
    find_package(wxWidgets 3.28 REQUIRED COMPONENTS core base)
    if(wxWidgets_USE_FILE)
        include(${wxWidgets_USE_FILE})
    endif()
endif()

set(SCP_SERVER_SRC_FILES
    src/Server/ChatServer.cpp
)

set(SCP_SERVER_CLI_SRC_FILES
    src/ServerCLI/CLI.cpp
    src/ServerCLI/Main.cpp
)

set(SCP_CLIENT_SRC_FILES
    src/Client/ChatClient.cpp
)

set(SCP_CLIENT_CLI_SRC_FILES
    src/ClientCLI/CLI.cpp
    src/ClientCLI/Main.cpp
)

set(SCP_CLIENT_GUI_SRC_FILES
    src/ClientGUI/App.cpp
    src/ClientGUI/ConnectDialog.cpp
    src/ClientGUI/MainUI.cpp
)

add_library(SCPServer STATIC ${SCP_SERVER_SRC_FILES})
target_compile_features(SCPServer PUBLIC cxx_std_23)
target_include_directories(SCPServer PUBLIC include ${Boost_INCLUDE_DIRS})
target_link_libraries(SCPServer PUBLIC ${Boost_LIBRARIES})
if(WIN32)
    target_compile_definitions(SCPServer PUBLIC _WIN32_WINNT=0x0601)
    target_link_libraries(SCPServer PUBLIC Ws2_32.lib Mswsock.lib)
endif()

add_library(SCPClient STATIC ${SCP_CLIENT_SRC_FILES})
target_compile_features(SCPClient PUBLIC cxx_std_23)
target_include_directories(SCPClient PUBLIC include ${Boost_INCLUDE_DIRS})
target_link_libraries(SCPClient PUBLIC ${Boost_LIBRARIES})
if(WIN32)
    target_compile_definitions(SCPClient PUBLIC _WIN32_WINNT=0x0601)
    target_link_libraries(SCPClient PUBLIC Ws2_32.lib Mswsock.lib)
endif()

if(BUILD_SERVER_CLI)
    add_executable(SCPServerCLI ${SCP_SERVER_CLI_SRC_FILES})
    target_compile_features(SCPServerCLI PUBLIC cxx_std_23)
    target_include_directories(SCPServerCLI PUBLIC ${Boost_INCLUDE_DIRS} ${CURSES_INCLUDE_DIRS})
    target_link_libraries(SCPServerCLI PUBLIC SCPServer ${Boost_LIBRARIES} ${CURSES_LIBRARIES})
    target_compile_definitions(SCPServerCLI PUBLIC NCURSES_STATIC)
endif()

if(BUILD_CLIENT_CLI)
    add_executable(SCPClientCLI ${SCP_CLIENT_CLI_SRC_FILES})
    target_compile_features(SCPClientCLI PUBLIC cxx_std_23)
    target_include_directories(SCPClientCLI PUBLIC ${Boost_INCLUDE_DIRS} ${CURSES_INCLUDE_DIRS})
    target_link_libraries(SCPClientCLI PUBLIC SCPClient ${Boost_LIBRARIES} ${CURSES_LIBRARIES})
    target_compile_definitions(SCPClientCLI PUBLIC NCURSES_STATIC)
endif()

if(BUILD_CLIENT_GUI)
    add_executable(SCPClientGUI ${SCP_CLIENT_GUI_SRC_FILES})
    target_compile_features(SCPClientGUI PUBLIC cxx_std_23)
    target_include_directories(SCPClientGUI PUBLIC ${Boost_INCLUDE_DIRS})
    target_link_libraries(SCPClientGUI PUBLIC SCPClient ${Boost_LIBRARIES} ${wxWidgets_LIBRARIES})
endif()