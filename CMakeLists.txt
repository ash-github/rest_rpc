cmake_minimum_required(VERSION 2.8)

project(rest_rpc)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -std=c++14")

if (CMAKE_BUILD_TYPE STREQUAL Debug)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_DEBUG")
    #add_definitions(
    #    -D_DEBUG
    #)
endif ()

add_definitions(-DFMT_HEADER_ONLY)

find_package(Boost COMPONENTS coroutine context system thread chrono REQUIRED)

set(REST_RPC_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(
		${Boost_INCLUDE_DIRS}
		${REST_RPC_DIRECTORY}
		${REST_RPC_DIRECTORY}/iguana
		${REST_RPC_DIRECTORY}/iguana/third_party/msgpack/include
		${REST_RPC_DIRECTORY}/spdlog/include
		${REST_RPC_DIRECTORY}/msgpack/include
		)
SET(EXTRA_LIBS ${EXTRA_LIBS} ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})


set(ASYNC_CLIENT_SOURCE 
	${REST_RPC_DIRECTORY}/example/async_client_example/async_client_example.cpp
)
set(BENCH_CLIENT_SOURCE
	${REST_RPC_DIRECTORY}/example/bench_client/bench_client.cpp
)
set(BENCH_SERVER_SOURCE
	${REST_RPC_DIRECTORY}/example/bench_server/bench_server.cpp
)
set(KEY_VALUE_CLIENT_SOURCE
	${REST_RPC_DIRECTORY}/example/key_value_client/key_value_client.cpp
)
set(KEY_VALUE_SERVER_SOURCE
	${REST_RPC_DIRECTORY}/example/key_value_server/key_value_server.cpp
)
set(SERVER_EXAMPLE_SOURCE
	${REST_RPC_DIRECTORY}/example/server_example/server_example.cpp
)
set(TIMAX_BIND_SOURCE
	${REST_RPC_DIRECTORY}/example/timax_bind/timax_bind.cpp
)


add_executable(async_client ${ASYNC_CLIENT_SOURCE})
target_link_libraries(async_client ${EXTRA_LIBS})

add_executable(bench_client ${BENCH_CLIENT_SOURCE})
target_link_libraries(bench_client ${EXTRA_LIBS})

add_executable(bench_server ${BENCH_SERVER_SOURCE})
target_link_libraries(bench_server ${EXTRA_LIBS})

add_executable(key_value_client ${KEY_VALUE_CLIENT_SOURCE})
target_link_libraries(key_value_client ${EXTRA_LIBS})

add_executable(key_value_server ${KEY_VALUE_SERVER_SOURCE})
target_link_libraries(key_value_server ${EXTRA_LIBS})

add_executable(server_example ${SERVER_EXAMPLE_SOURCE})
target_link_libraries(server_example ${EXTRA_LIBS})

add_executable(tiamx_bind ${TIMAX_BIND_SOURCE})
target_link_libraries(tiamx_bind ${EXTRA_LIBS})

include (InstallRequiredSystemLibraries)
set (CPACK_PACKAGE_VERSION_MAJOR "1")
set (CPACK_PACKAGE_VERSION_MINOR "0")
SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "David Doria")
include (CPack)
