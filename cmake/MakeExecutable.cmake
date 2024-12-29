function(MakeExecutable target _platform directory)
	set(_${target}_private_src_root_path "${directory}/Private")
	file(
		GLOB_RECURSE _${target}_private_source_list
		LIST_DIRECTORIES false
		"${_${target}_private_src_root_path}/*.cpp"
		"${_${target}_private_src_root_path}/*.c"
		"${_${target}_private_src_root_path}/*.cc"
		"${_${target}_private_src_root_path}/*.h"
		"${_${target}_private_src_root_path}/*.hpp"
		"${_${target}_private_src_root_path}/*.inl"
	)

	if(PLATFORM_WINDOWS)
		file(
			GLOB_RECURSE _${target}_private_source_list_windows
			LIST_DIRECTORIES false
			"${_${target}_private_src_root_path}/*.rc"
			"${ENGINE_CMAKE_DIRECTORY}/Windows/Application.manifest"
		)
		set(_${target}_private_source_list "${_${target}_private_source_list};${_${target}_private_source_list_windows}")
	endif()

	if(PLATFORM_APPLE)
		file(
			GLOB_RECURSE _${target}_private_source_list_apple
			LIST_DIRECTORIES false
			"${_${target}_private_src_root_path}/*.mm"
			"${_${target}_private_src_root_path}/*.swift"
			"${_${target}_private_src_root_path}/*.plist"
			"${_${target}_private_src_root_path}/*.entitlements*"
			"${_${target}_private_src_root_path}/*.storyboard*"
			"${_${target}_private_src_root_path}/../../Common/*.mm"
			"${_${target}_private_src_root_path}/../../Common/*.swift"
			"${_${target}_private_src_root_path}/../../Common/*.storyboard"
		)
		set(_${target}_private_source_list "${_${target}_private_source_list};${_${target}_private_source_list_apple}")
	endif()

	add_executable(${target} ${_platform} ${_${target}_private_source_list})

	AddTargetOptions(${target})

	if(OPTION_PRECOMPILED_HEADERS)
		if(EXISTS "${_${target}_pch_include}")
			target_precompile_headers(${target} PUBLIC $<$<OR:$<COMPILE_LANGUAGE:CXX>,$<COMPILE_LANGUAGE:OBJCXX>>:"${_${target}_pch_include}">)
		endif()
	endif()

	set_property(TARGET ${target} PROPERTY VS_DPI_AWARE "PerMonitor")

	target_include_directories(${target} PRIVATE "${directory}/Private")

	string(TOUPPER "${target}" TARGET_NAME_UPPER)
	target_compile_definitions(${target} INTERFACE "-D${TARGET_NAME_UPPER}_EXPORT_API=DLL_IMPORT")
	target_compile_definitions(${target} PRIVATE "-D${TARGET_NAME_UPPER}_EXPORT_API=DLL_EXPORT")

	foreach(_source IN ITEMS ${_${target}_private_source_list})
		get_filename_component(_source_path "${_source}" PATH)
		file(RELATIVE_PATH _source_path_rel "${_${target}_private_src_root_path}" "${_source_path}")
		string(REPLACE "/" "\\" _group_path "${_source_path_rel}")
		source_group("${_group_path}" FILES "${_source}")
	endforeach()

	MakeInterfaceLibrary(${target} ${directory})
endfunction()
