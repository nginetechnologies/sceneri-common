function(DeployBinaryDependency file)
	if (NOT EXISTS ${file})
		message(FATAL_ERROR "File " ${file} " did not exist")
	endif()
	get_filename_component(file_name "${file}" NAME)
	
	file(MAKE_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
	
	foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
		string( TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG_UPPER )
		file(MAKE_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG_UPPER}})

		file(COPY_FILE "${file}" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG_UPPER}}/${file_name}" ONLY_IF_DIFFERENT)

	endforeach()

	file(COPY_FILE "${file}" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${file_name}" ONLY_IF_DIFFERENT)

	if(OPTION_INSTALL)
		install(FILES ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${file_name}
			DESTINATION bin/
		)
	endif()

endfunction()