if (BUILD_STATIC_LIBS)
    MakeStaticModule(Common ${CMAKE_CURRENT_LIST_DIR} Common)
else()
    MakeDynamicLibrary(Common ${CMAKE_CURRENT_LIST_DIR} Common)
endif()

export(TARGETS CommonAPI Common FILE SceneriConfig.cmake)

MakeUnitTests(Common Common)

if(PLATFORM_APPLE) 
    set_source_files_properties("${CMAKE_CURRENT_LIST_DIR}/include/IO/Library.cpp" PROPERTIES LANGUAGE OBJCXX)
    set_source_files_properties("${CMAKE_CURRENT_LIST_DIR}/include/IO/Log.cpp" PROPERTIES LANGUAGE OBJCXX)
    set_source_files_properties("${CMAKE_CURRENT_LIST_DIR}/include/IO/Path.cpp" PROPERTIES LANGUAGE OBJCXX)

    target_link_libraries(Common PUBLIC "-framework CoreFoundation")
    target_link_libraries(Common PUBLIC "-framework Foundation")
elseif(PLATFORM_LINUX)
    target_link_libraries(Common PUBLIC "dl")
    target_link_libraries(Common PUBLIC "pthread")
endif()
