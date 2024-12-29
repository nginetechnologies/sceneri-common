include("${ENGINE_CMAKE_DIRECTORY}/MakeLauncher.cmake")

function(MakeConsole target target_directory)
	MakeLauncher(${ARGV})
	set_target_properties(${target} PROPERTIES WIN32_EXECUTABLE FALSE)
endfunction()
