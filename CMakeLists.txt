cmake_minimum_required (VERSION 3.21)
set(COMMON_ROOT_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}")

include("${COMMON_ROOT_DIRECTORY}/cmake/InitialSettings.cmake")

if(${CMAKE_HOST_SYSTEM_NAME} MATCHES "Darwin")
	project("sceneri-common" VERSION 0.8.0 LANGUAGES CXX C Swift OBJC OBJCXX)
else()
	project("sceneri-common" VERSION 0.8.0 LANGUAGES CXX C)
endif()

include("${COMMON_ROOT_DIRECTORY}/cmake/InitialConfiguration.cmake")

option(OPTION_BUILD_UNIT_TESTS "Build unit tests" OFF)
option(OPTION_BUILD_FEATURE_TESTS "Build feature tests" OFF)
option(BUILD_STATIC_LIBS "Build static libraries" ON)

if(OPTION_BUILD_UNIT_TESTS OR OPTION_BUILD_FEATURE_TESTS)
	set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
	set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
	add_subdirectory(${ENGINE_CODE_DIRECTORY}/Public/Common/3rdparty/googletest EXCLUDE_FROM_ALL)

	set_target_properties(gtest PROPERTIES FOLDER "Tests/gtest")
	set_target_properties(gtest_main PROPERTIES FOLDER "Tests/gtest")

	enable_testing()
endif()

include("${COMMON_ROOT_DIRECTORY}/CommonLibrary.cmake")