# export CC=/usr/local/clang_9.0.0/bin/clang
# export CXX=/usr/local/clang_9.0.0/bin/clang++
#SET (CMAKE_C_COMPILER             "/usr/lib/llvm/10/bin/clang")
#SET (CMAKE_CXX_COMPILER           "/usr/lib/llvm/10/bin/clang++")
SET (CMAKE_C_COMPILER             "/usr/local/clang_12.0.1/bin/clang")
SET (CMAKE_CXX_COMPILER            "/usr/local/clang_12.0.1/bin/clang++")


cmake_minimum_required(VERSION 3.0)

project(pumphouse)      

set(SOURCES
	src/main.cpp
	src/ServerCommandsLineFunctions.cpp
	src/ServerNouns.cpp
	PumphouseMgr/PumpHouseMgr.cpp
	PumphouseMgr/PumpHouseDB.cpp
	PumphouseMgr/SmartShunt.cpp
	PumphouseMgr/SigineerInverter.cpp
	PumphouseMgr/SerialStream.cpp
	PumphouseMgr/PumpHouseAPISecretMgr.cpp
	PumphouseMgr/TempSensor.cpp
	PumphouseMgr/TankDepth.cpp
	PumphouseMgr/I2C.cpp
	PumphouseMgr/TMP102.cpp
	PumphouseMgr/CPUInfo.cpp
	PumphouseMgr/MCP3427.cpp
	Submodule/FooServer/src/RESTutils.cpp
	Submodule/FooServer/src/ServerCmdQueue.cpp
	Submodule/FooServer/src/TCPClientInfo.cpp
	Submodule/FooServer/src/TCPServer.cpp
	Submodule/FooServer/src/REST/http_parser.c
	Submodule/FooServer/src/REST/REST_URL.cpp
	Submodule/FooServer/src/REST/RESTServerConnection.cpp
 	Submodule/FooServer/src/REST/yuarel.c
 	Submodule/FooServer/src/Telnet/CmdLineBuffer.cpp
 	Submodule/FooServer/src/Telnet/CmdLineBuffer.cpp
 	Submodule/FooServer/src/Telnet/CmdLineHelp.cpp
 	Submodule/FooServer/src/Telnet/CmdLineMgr.cpp
 	Submodule/FooServer/src/Telnet/CmdLineRegistry.cpp
 	Submodule/FooServer/src/Telnet/TelnetServerConnection.cpp
 	Submodule/FooServer/src/Telnet/libtelnet.c
 	Submodule/FooServer/src/hash-library/sha256.cpp
	Submodule/FooServer/src/TimeStamp.cpp
	Submodule/FooServer/src/LogMgr.cpp
 	)

add_executable(pumphouse ${SOURCES})


set_target_properties(pumphouse PROPERTIES
            CXX_STANDARD 17
            CXX_EXTENSIONS OFF
            )
 
set_target_properties(pumphouse PROPERTIES 
			OUTPUT_NAME "pumphouse"
			)

           
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

include_directories(/usr/include)
link_directories(/usr/lib)
target_link_libraries(pumphouse  PRIVATE sqlite3 Threads::Threads)

target_include_directories(pumphouse
	PRIVATE 
	src
	PumphouseMgr
	Submodule/FooServer/src
	Submodule/FooServer/src/hash-library 
	Submodule/FooServer/src/REST
	Submodule/FooServer/src/Telnet

	)

set(CMAKE_BINARY_DIR "bin")
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
add_custom_target(copy_assets
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/assets ${CMAKE_BINARY_DIR}
)

add_dependencies(pumphouse copy_assets)
