include("${ENGINE_CMAKE_DIRECTORY}/LinkStaticLibrary.cmake")

function(MakeLauncher target target_directory)
	if(PLATFORM_WINDOWS)
		MakeExecutable(${target} WIN32 "${target_directory}")
	elseif(PLATFORM_APPLE)
		MakeExecutable(${target} MACOSX_BUNDLE "${target_directory}")
	elseif(PLATFORM_ANDROID)
		MakeDynamicLibrary(${target} "${target_directory}" "${target}")
	else()
		MakeExecutable(${target} "" "${target_directory}")
	endif()

	LinkStaticLibrary(${target} Common)
	LinkStaticLibrary(${target} Engine)
	LinkStaticLibrary(${target} Renderer)

	set_target_properties(${target} PROPERTIES FOLDER Launchers)

	if(PLUGINS_IN_EXECUTABLE)
		set_target_properties(${target} PROPERTIES INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
	endif()

	if(PLATFORM_APPLE)
		set_target_properties(${target} PROPERTIES XCODE_ATTRIBUTE_DEAD_CODE_STRIPPING FALSE)
		set(EXECUTABLE_NAME "${target}")

		macro(ADD_BUNDLE_RESOURCES _target)
			set(_resources ${ARGN})
			foreach(_resource ${_resources})
					target_sources(${_target} PRIVATE ${_resource})
					set_source_files_properties(${_resource} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
			endforeach()
		endmacro()

		if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/Private/Apple/Assets.xcassets)
			ADD_BUNDLE_RESOURCES(${target}
				${CMAKE_CURRENT_SOURCE_DIR}/Private/Apple/Assets.xcassets
			)
		endif()

		if(PLATFORM_APPLE_IOS)
			set_target_properties(${target} PROPERTIES
				MACOSX_BUNDLE ON
				MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Private/Apple/Info_iOS.plist)
			
			ADD_BUNDLE_RESOURCES(${target}
				${ENGINE_CODE_DIRECTORY}/Launchers/Common/iOS/LaunchScreen.storyboard
				${ENGINE_CODE_DIRECTORY}/Launchers/Common/iOS/Main.storyboard
			)
		elseif(PLATFORM_APPLE_MACOS)
			set_target_properties(${target} PROPERTIES
				MACOSX_BUNDLE ON
				MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Private/Apple/Info_macOS.plist)

			set_target_properties(${target} PROPERTIES
				MACOSX_BUNDLE ON
			)

			ADD_BUNDLE_RESOURCES(${target}
				#${ENGINE_CODE_DIRECTORY}/Launchers/Common/macOS/LaunchScreen.storyboard
				${ENGINE_CODE_DIRECTORY}/Launchers/Common/macOS/Main.storyboard
			)
		elseif(PLATFORM_APPLE_VISIONOS)
			set_target_properties(${target} PROPERTIES
				MACOSX_BUNDLE ON
				MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Private/Apple/Info_visionOS.plist)
		endif()
	elseif(PLATFORM_ANDROID)
		# Import game-activity static lib inside the game-activity_static prefab module.
		find_package(game-activity REQUIRED CONFIG)
		target_link_libraries(${target} PUBLIC log android)

		LinkStaticLibrary(${target} game-activity::game-activity_static)
	elseif(PLATFORM_EMSCRIPTEN)
		target_link_libraries(${target} PUBLIC "--shell-file=${CMAKE_CURRENT_SOURCE_DIR}/Private/Web/shell.html")
		foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
			string( TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG_UPPER )
			file(MAKE_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG_UPPER}})
			file(COPY_FILE "${CMAKE_CURRENT_SOURCE_DIR}/Private/Web/icon.png" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG_UPPER}}/icon.png" ONLY_IF_DIFFERENT)

			file(MAKE_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG_UPPER}}/.well-known")
			file(COPY_FILE "${CMAKE_CURRENT_SOURCE_DIR}/Private/Web/apple-app-site-association" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG_UPPER}}/.well-known/apple-app-site-association" ONLY_IF_DIFFERENT)
		endforeach()
	endif()
endfunction()
