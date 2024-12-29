include("${ENGINE_CMAKE_DIRECTORY}/LinkStaticLibrary.cmake")

function(LinkPlugin target library)
	if(PLUGINS_IN_EXECUTABLE)
		LinkStaticLibrary(${target} ${library})
	else()
		target_link_libraries(${target} PUBLIC ${library})
	endif()
endfunction()
