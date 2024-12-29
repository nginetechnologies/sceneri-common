include(GoogleTest)
include("${ENGINE_CMAKE_DIRECTORY}/LinkStaticLibrary.cmake")

function(MakeUnitTests target target_name)
	if(OPTION_BUILD_UNIT_TESTS)
		project(${target_name}UnitTests CXX)

		set(_${target_name}UnitTests_src_root_path "${CMAKE_CURRENT_LIST_DIR}/UnitTests")
			file(
				GLOB_RECURSE _${target_name}UnitTests_source_list
				LIST_DIRECTORIES false
				"${_${target_name}UnitTests_src_root_path}/*.cpp*"
				"${_${target_name}UnitTests_src_root_path}/*.h*"
			)

		MakeExecutable(${target_name}UnitTests "" "${CMAKE_CURRENT_LIST_DIR}/UnitTests")
		AddTargetOptions(${target_name}UnitTests)
		LinkStaticLibrary(${target_name}UnitTests ${target})
		target_link_libraries(${target_name}UnitTests PRIVATE gtest)
		set_target_properties(${target_name}UnitTests PROPERTIES FOLDER Tests/Unit)

		gtest_add_tests(${target_name}UnitTests "" AUTO)

		foreach(_source IN ITEMS ${_${target_name}UnitTests_source_list})
			get_filename_component(_source_path "${_source}" PATH)
			file(RELATIVE_PATH _source_path_rel "${_${target_name}UnitTests_src_root_path}" "${_source_path}")
			string(REPLACE "/" "\\" _group_path "${_source_path_rel}")
			source_group("${_group_path}" FILES "${_source}")
		endforeach()

		AddTargetOptions(gtest)
	endif()
endfunction()

function(MakeFeatureTests target target_name)
	if(OPTION_BUILD_FEATURE_TESTS)
		project(${target_name}FeatureTests CXX)

		set(_${target_name}FeatureTests_src_root_path "${CMAKE_CURRENT_LIST_DIR}/FeatureTests")
			file(
				GLOB_RECURSE _${target_name}FeatureTests_source_list
				LIST_DIRECTORIES false
				"${_${target_name}FeatureTests_src_root_path}/*.cpp*"
				"${_${target_name}FeatureTests_src_root_path}/*.h*"
			)

		MakeExecutable(${target_name}FeatureTests "" "${CMAKE_CURRENT_LIST_DIR}/FeatureTests")
		AddTargetOptions(${target_name}FeatureTests)
		LinkStaticLibrary(${target_name}FeatureTests ${target})
		target_link_libraries(${target_name}FeatureTests PRIVATE gtest)
		set_target_properties(${target_name}FeatureTests PROPERTIES FOLDER Tests/Feature)

		gtest_add_tests(${target_name}FeatureTests "" AUTO)

		foreach(_source IN ITEMS ${_${target_name}FeatureTests_source_list})
			get_filename_component(_source_path "${_source}" PATH)
			file(RELATIVE_PATH _source_path_rel "${_${target_name}FeatureTests_src_root_path}" "${_source_path}")
			string(REPLACE "/" "\\" _group_path "${_source_path_rel}")
			source_group("${_group_path}" FILES "${_source}")
		endforeach()

		AddTargetOptions(gtest)
	endif()
endfunction()
