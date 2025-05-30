# 1 Set version and project name
project(Deferred)
cmake_minimum_required(VERSION 3.12)
set(CMAKE_CXX_STANDARD 17)
add_definitions (-DDEBUG)

# 2 Copy assets and .dll
file(GLOB copyResources "./assets" "./thirdParty/bin/assimp/assimp-vc143-mtd.dll")
file(COPY ${copyResources} DESTINATION ${CMAKE_BINARY_DIR})

# 3 Third party
# 3.1 Include third party .h
include_directories(
	SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/include
)

# 3.2 Link third party .lib
link_directories(
	SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/lib
)

# 4 Add subdirectory
add_subdirectory(wrapper)
add_subdirectory(application)
add_subdirectory(glframework)
add_subdirectory(imgui)

# 5 Main directory
add_executable(deferred "main.cpp" "glad.c")

# 6 Link subdirectory and third party .lib
target_link_libraries(deferred glfw3.lib assimp-vc143-mtd.lib zlibstaticd.lib wrapper app fw imguilib)