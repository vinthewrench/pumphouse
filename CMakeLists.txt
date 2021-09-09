# export CC=/usr/local/clang_9.0.0/bin/clang
# export CXX=/usr/local/clang_9.0.0/bin/clang++
SET (CMAKE_C_COMPILER             "/usr/lib/llvm/10/bin/clang")
SET (CMAKE_CXX_COMPILER           "/usr/lib/llvm/10/bin/clang++")

cmake_minimum_required(VERSION 3.0)

project(pumphouse)      

set(SOURCES
	src/main.cpp
	src/PumpHouseMgr.cpp
	src/PumpHouseDB.cpp
	src/SmartShunt.cpp
	src/SigineerInverter.cpp
	src/SerialStream.cpp
	src/LogMgr.cpp
	src/TimeStamp.cpp
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
target_link_libraries(pumphouse PRIVATE Threads::Threads)

target_include_directories(pumphouse
	PRIVATE 
	src
	)

set(CMAKE_BINARY_DIR "bin")
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
add_custom_target(copy_assets
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/assets ${CMAKE_BINARY_DIR}
)

add_dependencies(pumphouse copy_assets)
