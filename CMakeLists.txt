cmake_minimum_required(VERSION 3.30)
project(SimpleChatProgram VERSION 1.0.0 LANGUAGES CXX)

find_package(Boost 1.87 REQUIRED COMPONENTS thread)
find_package(Curses 1.6 REQUIRED)

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

# set(SCP_CLIENT_CLI_SRC_FILES
#     src/ClientTest/TestMain.cpp
# )

add_library(SCPServer STATIC ${SCP_SERVER_SRC_FILES})
target_compile_features(SCPServer PUBLIC cxx_std_20)
target_include_directories(SCPServer PUBLIC include ${Boost_INCLUDE_DIRS})
target_link_libraries(SCPServer PUBLIC ${Boost_LIBRARIES})

add_executable(SCPServerCLI ${SCP_SERVER_CLI_SRC_FILES})
target_include_directories(SCPServerCLI PUBLIC ${Boost_INCLUDE_DIRS} ${CURSES_INCLUDE_DIRS})
target_link_libraries(SCPServerCLI PUBLIC SCPServer ${Boost_LIBRARIES} ${CURSES_LIBRARIES})

add_library(SCPClient STATIC ${SCP_CLIENT_SRC_FILES})
target_compile_features(SCPClient PUBLIC cxx_std_20)
target_include_directories(SCPClient PUBLIC include ${Boost_INCLUDE_DIRS})
target_link_libraries(SCPClient PUBLIC ${Boost_LIBRARIES})

# add_executable(SCPClientCLI ${SCP_CLIENT_CLI_SRC_FILES})
# target_include_directories(SCPClientCLI PUBLIC ${Boost_INCLUDE_DIRS})
# target_link_libraries(SCPClientCLI PUBLIC SCPClient ${Boost_LIBRARIES})