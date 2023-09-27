cmake_minimum_required(VERSION 3.16)

project(qqbot LANGUAGES CXX VERSION 1.0.0)



add_executable(qqbot)

target_compile_features(qqbot PRIVATE cxx_std_17)
set_target_properties(qqbot PROPERTIES CXX_STANDARD 17)
set_target_properties(qqbot PROPERTIES CXX_STANDARD_REQUIRED ON)



set(MIRAI_BUILD_UNIT_TESTS OFF CACHE BOOL "Build UnitTests" FORCE)
# set(MIRAI_GIT_USE_SSH ON CACHE BOOL "Use SSH for git download"  FORCE)
add_subdirectory(cpp-mirai-client)



target_compile_definitions(qqbot PRIVATE MIRAI_LOGGING_LEVEL=0)		# MIRAI_LOGGING_LEVELS_TRACE
target_sources(qqbot PRIVATE src/main.cpp)
target_link_libraries(qqbot PRIVATE cpp-mirai-client::cppmirai)