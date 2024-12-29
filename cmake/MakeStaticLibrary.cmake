function(MakeStaticLibrary target directory label)
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
	
	if(PLATFORM_APPLE)
		file(
			GLOB_RECURSE _${target}_private_source_list_apple
			LIST_DIRECTORIES false
			"${_${target}_private_src_root_path}/*.mm"
			"${_${target}_private_src_root_path}/*.swift"
		)
		set(_${target}_private_source_list "${_${target}_private_source_list};${_${target}_private_source_list_apple}")
	endif()

	set(_${target}_public_src_root_path "${directory}/Public/${label}")
	file(
		GLOB_RECURSE _${target}_public_source_list
		LIST_DIRECTORIES false
		"${directory}/Public/${label}/*.h*"
		"${directory}/Public/${label}/*.inl*"
		"${directory}/Public/${label}/*.natvis*"
	)

	add_library(${target} STATIC ${_${target}_private_source_list} ${_${target}_public_source_list})
	AddTargetOptions(${target})

	if(OPTION_PRECOMPILED_HEADERS)
		set(_${target}_pch_include "${directory}/Public/${label}/PrecompiledHeaders.h")
		if(EXISTS "${_${target}_pch_include}")
			target_precompile_headers(${target} PUBLIC $<$<OR:$<COMPILE_LANGUAGE:CXX>,$<COMPILE_LANGUAGE:OBJCXX>>:"${_${target}_pch_include}">)
		endif()
	endif()

	set_target_properties(${target} PROPERTIES PROJECT_LABEL "${label}")

	target_include_directories(${target} PUBLIC "${directory}/Public")
	target_include_directories(${target} INTERFACE "${directory}/Public")
	target_include_directories(${target} PRIVATE "${directory}/Private")
	target_include_directories(${target} PRIVATE "${directory}/Public/${label}")

	string(TOUPPER "${label}" TARGET_NAME_UPPER)
	target_compile_definitions(${target} INTERFACE "-D${TARGET_NAME_UPPER}_EXPORT_API=DLL_IMPORT")
	target_compile_definitions(${target} PRIVATE "-D${TARGET_NAME_UPPER}_EXPORT_API=DLL_EXPORT")

	foreach(_source IN ITEMS ${_${target}_private_source_list})
		get_filename_component(_source_path "${_source}" PATH)
		file(RELATIVE_PATH _source_path_rel "${_${target}_private_src_root_path}" "${_source_path}")
		string(REPLACE "/" "\\" _group_path "${_source_path_rel}")
		source_group("${_group_path}" FILES "${_source}")
	endforeach()

	foreach(_source IN ITEMS ${_${target}_public_source_list})
		get_filename_component(_source_path "${_source}" PATH)
		file(RELATIVE_PATH _source_path_rel "${_${target}_public_src_root_path}" "${_source_path}")
		string(REPLACE "/" "\\" _group_path "${_source_path_rel}")
		source_group("${_group_path}" FILES "${_source}")
	endforeach()

	MakeInterfaceLibrary(${target} ${directory})
endfunction()