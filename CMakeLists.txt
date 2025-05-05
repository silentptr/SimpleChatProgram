cmake_minimum_required(VERSION 3.30)
project(SimpleChatProgram VERSION 1.0.0 LANGUAGES CXX)

find_package(Boost 1.87 REQUIRED COMPONENTS thread)

set(SCP_SERVER_SRC_FILES
    src/Server/ChatServer.cpp
)

set(SCP_SERVER_TEST_SRC_FILES
    src/ServerTest/TestMain.cpp
)

set(SCP_CLIENT_SRC_FILES

)

set(SCP_UI_SRC_FILES

)

add_library(SCPServer STATIC ${SCP_SERVER_SRC_FILES})
target_compile_features(SCPServer PUBLIC cxx_std_20)
target_include_directories(SCPServer PUBLIC include ${Boost_INCLUDE_DIRS})
target_link_libraries(SCPServer PUBLIC ${Boost_LIBRARIES})

add_executable(SCPServerTest ${SCP_SERVER_TEST_SRC_FILES})
target_include_directories(SCPServerTest PUBLIC ${Boost_INCLUDE_DIRS})
target_link_libraries(SCPServerTest PUBLIC SCPServer ${Boost_LIBRARIES})