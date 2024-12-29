function(MakeInterfaceLibrary target directory)
	add_library(${target}API INTERFACE)
	target_include_directories(${target}API INTERFACE "${directory}/Public")

	string(TOUPPER "${target}" TARGET_NAME_UPPER)
	target_compile_definitions(${target}API INTERFACE "-D${TARGET_NAME_UPPER}_EXPORT_API=DLL_IMPORT")
endfunction()
