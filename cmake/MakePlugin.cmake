function(MakePlugin target directory label lib_directory bin_directory relative_config_path)
	if(PROJECT_BUILD)
		set(Engine_DIR ${ENGINE_CODE_DIRECTORY}/Intermediate${PLATFORM_NAME})
		find_package(Engine REQUIRED)
	endif()

	if(PLUGINS_IN_EXECUTABLE)
		MakeStaticLibrary(${target} ${directory} ${label})
	else()
		MakeDynamicLibrary(${target} ${directory} ${label})
	endif()

	get_filename_component(PLUGIN_DIRECTORY_PATH "${directory}/../" REALPATH)
	file(
		GLOB_RECURSE _${target}_private_assets_list
		LIST_DIRECTORIES false
		"${PLUGIN_DIRECTORY_PATH}/*.nplugin"
		"${PLUGIN_DIRECTORY_PATH}/**/*.nasset"
	)
	target_sources(${target} PRIVATE "${_${target}_private_assets_list}")

	#source_group(TREE "${PLUGIN_DIRECTORY_PATH}" FILES "${_${target}_private_assets_list}")

	foreach(_asset IN ITEMS ${_${target}_private_assets_list})
		get_filename_component(_asset_path "${_asset}" PATH)
		file(RELATIVE_PATH _asset_path_rel "${PLUGIN_DIRECTORY_PATH}" "${_asset_path}")
		string(REPLACE "/" "\\" _group_path "${_asset_path_rel}")
		source_group("${_group_path}" FILES "${_asset}")
	endforeach()

	source_group(Assets FILES "${_${target}_private_assets_list}")

	# Workarounds for IDEs that don't support PROJECT_LABEL
	# Note: Visual Studio does, but Rider doesn't read the label from the .vcxproj
	if (PLATFORM_APPLE OR GENERATOR_VISUAL_STUDIO)
		set_target_properties(${target} PROPERTIES FOLDER "Plugins/${label}")
	else()
		set_target_properties(${target} PROPERTIES FOLDER "Plugins")
	endif()

	set_target_properties(${target} PROPERTIES OUTPUT_NAME "${target}")

	foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
		string( TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG_UPPER )
		set_target_properties(${target}
			PROPERTIES
			LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG_UPPER} "${CMAKE_CURRENT_LIST_DIR}/../${bin_directory}/${PLATFORM_NAME}/${OUTPUTCONFIG}"
			RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG_UPPER} "${CMAKE_CURRENT_LIST_DIR}/../${bin_directory}/${PLATFORM_NAME}/${OUTPUTCONFIG}"
		)
	endforeach()

	target_link_libraries(${target} PRIVATE CommonAPI EngineAPI RendererAPI)
endfunction()
