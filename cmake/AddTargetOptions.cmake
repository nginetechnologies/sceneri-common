if(PLATFORM_APPLE)
	include("${ENGINE_CMAKE_DIRECTORY}/Apple/SetupTarget.cmake")
endif()

function(AddCoreTargetOptions target)
	# default size of the stack
	# make sure to change both values here
	set(STACK_SIZE "16777216")
	set(STACK_SIZE_HEX "0x1000000")

	if(COMPILER_MSVC OR COMPILER_CLANG_WINDOWS)
		target_compile_definitions(${target} PRIVATE _ITERATOR_DEBUG_LEVEL=0)

		if(NOT OPTION_EXCEPTIONS)
			target_compile_definitions(${target} PRIVATE _HAS_EXCEPTIONS=0)
		endif()

		# Enable fast floating-point precision math
		target_compile_options(${target} PRIVATE /fp:precise)
		# Use vector calling convention
		target_compile_options(${target} PRIVATE /Gv)

		if(USE_CCACHE)
			# CCache doesn't support the ProgramDatabase format
			target_compile_options(${target} PRIVATE /Z7)
		else()
			target_compile_options(${target} PRIVATE /Zi)
		endif()

		if (OPTION_CODE_HOT_RELOAD)
			target_compile_options(${target} PRIVATE $<$<COMPILE_LANGUAGE:OBJCXX>:/hotpatch>)
			# Enable function-level linking
			target_compile_options(${target} PRIVATE /Gy)
			# Optimize global data
			target_compile_options(${target} PRIVATE /Gw)

			if (COMPILER_CLANG_WINDOWS)
				target_compile_options(${target} PRIVATE -Z7)
				target_compile_options(${target} PRIVATE -hotpatch)
				target_compile_options(${target} PRIVATE -Gy)
				target_compile_options(${target} PRIVATE -Xclang -mno-constructor-aliases)
				target_compile_options(${target} PRIVATE -gcodeview)
				target_compile_options(${target} PRIVATE -fms-hotpatch)
				target_compile_options(${target} PRIVATE -ffunction-sections)
			endif()

			target_link_options(${target} PRIVATE /FUNCTIONPADMIN)
			target_link_options(${target} PRIVATE /OPT:NOREF)
			target_link_options(${target} PRIVATE /OPT:NOICF)
			target_link_options(${target} PRIVATE /DEBUG:FULL)
		else()
			target_link_options(${target} PRIVATE /DEBUG)
		endif()

		# Enable large object file support (largely for Unity builds)
		target_compile_options(${target} PRIVATE /bigobj)

		# Make sure __cplusplus macro reports the actual language version
		target_compile_options(${target} PRIVATE /Zc:__cplusplus)

		if(COMPILER_MSVC)
			# Don't use the default libcmt library
			target_link_options(${target} PRIVATE
				$<$<CONFIG:Debug>:/NODEFAULTLIB:libcmt>
				$<$<CONFIG:Profile>:/NODEFAULTLIB:libcmt>
				$<$<CONFIG:RelWithDebInfo>:/NODEFAULTLIB:libcmt>
			)
		endif()

		if(COMPILER_MSVC)
			target_compile_options(${target} PRIVATE "/MP")
			target_link_options(${target} PRIVATE "LINKER:/STACK:${STACK_SIZE}")
		endif()
		target_compile_options(${target} PRIVATE /std:c++17)

		target_compile_options(${target} PRIVATE
			$<$<CONFIG:Debug>:/Od /Oi>
			$<$<CONFIG:RelWithDebInfo>:/O2 /GL /Oi /Ot /Ob3>
		)
		target_link_options(${target} PRIVATE
			$<$<CONFIG:RelWithDebInfo>:/LTCG>
		)
		# Allow expansion of explicit inline and forceinline for debug, automatic for profile and aggressive for release
		# Enable intrinsics even in debug
		target_compile_options(${target} PRIVATE
			$<$<CONFIG:Debug>:/Ob1 /Oi>
			$<$<CONFIG:Profile>:/Ob2 /Oi>
		)

		# Remove timestamps from executables to ensure determinisic .exe builds
		target_link_options(${target} PRIVATE /Brepro)

		target_compile_definitions(${target} PRIVATE _ENABLE_EXTENDED_ALIGNED_STORAGE=1)

		if(COMPILER_CLANG_WINDOWS)
			# Make sure the 128 bit compare exchange instruction is available
			target_compile_options(${target} PRIVATE -mcx16)
		endif()

		if(OPTION_ADDRESS_SANITIZER)
			target_compile_options(${target} PRIVATE /fsanitize=address)
			target_link_options(${target} PRIVATE /INCREMENTAL:NO)
			target_compile_options(${target} PRIVATE /fsanitize-address-use-after-return)
			target_compile_definitions(${target} PRIVATE _DISABLE_STRING_ANNOTATION=1 _DISABLE_VECTOR_ANNOTATION=1)
		endif()

		if(USE_AVX2)
			target_compile_options(${target} PRIVATE /arch:AVX2)
		elseif(USE_AVX)
			target_compile_options(${target} PRIVATE /arch:AVX)
		elseif(USE_SSE2)
			target_compile_options(${target} PRIVATE /arch:SSE2)
		elseif(USE_SSE)
			target_compile_options(${target} PRIVATE /arch:SSE)
		endif()
	elseif(COMPILER_CLANG)
		# Enable debug symbols in all builds
		target_compile_options(${target} PRIVATE -g)
		
		target_compile_definitions(${target} PRIVATE _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING=1 __clang__=1)

		target_compile_options(${target} PRIVATE $<$<OR:$<COMPILE_LANGUAGE:CXX>,$<COMPILE_LANGUAGE:OBJCXX>>:-ffp-model=precise -ffp-contract=off>)
		
		set_target_properties(${target} PROPERTIES C_VISIBILITY_PRESET hidden)
		set_target_properties(${target} PROPERTIES CXX_VISIBILITY_PRESET hidden)
		set_target_properties(${target} PROPERTIES VISIBILITY_INLINES_HIDDEN ON)

		if (NOT PLATFORM_EMSCRIPTEN)
			target_compile_options(${target} PRIVATE
				$<$<AND:$<CONFIG:RelWithDebInfo>,$<OR:$<COMPILE_LANGUAGE:CXX>,$<COMPILE_LANGUAGE:OBJCXX>>>:-flto -fwhole-program-vtables>
			)
			target_link_options(${target} PRIVATE
				$<$<CONFIG:RelWithDebInfo>:-flto -fwhole-program-vtables>
			)
		endif()

		if(PLATFORM_EMSCRIPTEN)
			target_link_options(${target} PRIVATE -sEXPORTED_FUNCTIONS=['_main','_emscripten_proxy_get_system_queue','_emscripten_proxy_sync','_malloc','_free','_onGoogleSignIn','_onAppleSignIn'])
			target_link_options(${target} PRIVATE -sEXPORTED_RUNTIME_METHODS=['ccall','stringToNewUTF8','UTF16ToString'])

			target_compile_options(${target} PRIVATE
				$<$<CONFIG:Debug>:-Og>
				$<$<CONFIG:Profile>:-O1>
				$<$<CONFIG:RelWithDebInfo>:-Os -fomit-frame-pointer -fmerge-all-constants -ffunction-sections -fdata-sections -fno-split-lto-unit -fno-unroll-loops -finline-functions>
			)
			target_link_options(${target} PRIVATE
				# Focus on code size when generating the wasm module
				$<$<CONFIG:RelWithDebInfo>:-Oz --gc-sections>
			)

     			# Disabled due to an Emscripten bug, enable when our fix is merged into their mainline
			# target_link_options(${target} PRIVATE
			#	# Focus on code size when generating JavaScript
			#	$<$<CONFIG:RelWithDebInfo>:--closure 1>
			#)
			target_link_options(${target} PRIVATE
			$<$<CONFIG:Debug>:-sASSERTIONS=2>
				$<$<CONFIG:Profile>:-sASSERTIONS=2>
			)

			target_compile_options(${target} PRIVATE 
				$<$<CONFIG:Debug>:-gsource-map>
				$<$<CONFIG:Profile>:-gsource-map>
				$<$<CONFIG:RelWithDebInfo>:-gsource-map>
			)
			target_link_options(${target} PRIVATE
				$<$<CONFIG:Debug>:-gsource-map>
				$<$<CONFIG:Profile>:-gsource-map>
				$<$<CONFIG:RelWithDebInfo>:-gsource-map>
			)

			target_compile_options(${target} PRIVATE 
				$<$<CONFIG:Debug>:-g3 -gseparate-dwarf>
				$<$<CONFIG:Profile>:-g3 -gseparate-dwarf>
				$<$<CONFIG:RelWithDebInfo>:-g3 -gseparate-dwarf>
			)
			target_link_options(${target} PRIVATE 
				$<$<CONFIG:Debug>:-g3>
				$<$<CONFIG:Profile>:-g3>
				$<$<CONFIG:RelWithDebInfo>:-g3>
			)

			target_link_options(${target} PRIVATE "-sSTACK_SIZE=${STACK_SIZE}")
		else()
			# Make sure the 128 bit compare exchange instruction is available
			target_compile_options(${target} PRIVATE $<$<OR:$<COMPILE_LANGUAGE:CXX>,$<COMPILE_LANGUAGE:OBJCXX>>:-mcx16>)

			target_compile_options(${target} PRIVATE
				$<$<CONFIG:Debug>:-O0>
				$<$<CONFIG:Profile>:-O1>
				$<$<AND:$<CONFIG:RelWithDebInfo>,$<COMPILE_LANGUAGE:CXX>>:-O3 -funroll-loops -fomit-frame-pointer>
			)
		endif()

		if(NOT OPTION_EXCEPTIONS)
			target_compile_options(${target} PRIVATE $<$<OR:$<COMPILE_LANGUAGE:CXX>,$<COMPILE_LANGUAGE:OBJCXX>>:-fno-exceptions>)
		endif()
		target_compile_options(${target} PRIVATE $<$<OR:$<COMPILE_LANGUAGE:CXX>,$<COMPILE_LANGUAGE:OBJCXX>>:-fno-rtti>)

		if(OPTION_ADDRESS_SANITIZER)
			if(PLATFORM_APPLE)
				set(CMAKE_XCODE_SCHEME_ADDRESS_SANITIZER ON)
				set(CMAKE_XCODE_SCHEME_ADDRESS_SANITIZER_USE_AFTER_RETURN ON)
			else()
				target_compile_options(${target} PRIVATE -fsanitize=address)
				target_link_options(${target} PRIVATE -fsanitize=address)
				target_compile_options(${target} PRIVATE -fsanitize-address-use-after-return)
				target_compile_options(${target} PRIVATE -fsanitize-address-use-after-scope)
			endif()
		endif()
		
		target_compile_definitions(${target} PRIVATE _LIBCPP_REMOVE_TRANSITIVE_INCLUDES=1)

		if(USE_AVX512)
			target_compile_options(${target} PRIVATE $<$<OR:$<COMPILE_LANGUAGE:CXX>,$<COMPILE_LANGUAGE:OBJCXX>>:-mavx512f>)
		elseif(USE_AVX2)
			target_compile_options(${target} PRIVATE $<$<OR:$<COMPILE_LANGUAGE:CXX>,$<COMPILE_LANGUAGE:OBJCXX>>:-mavx2>)
		elseif(USE_AVX)
			target_compile_options(${target} PRIVATE $<$<OR:$<COMPILE_LANGUAGE:CXX>,$<COMPILE_LANGUAGE:OBJCXX>>:-mavx>)
		elseif(USE_SSE4_2)
			target_compile_options(${target} PRIVATE $<$<OR:$<COMPILE_LANGUAGE:CXX>,$<COMPILE_LANGUAGE:OBJCXX>>:-msse4.2>)
		elseif(USE_SSE4_1)
			target_compile_options(${target} PRIVATE $<$<OR:$<COMPILE_LANGUAGE:CXX>,$<COMPILE_LANGUAGE:OBJCXX>>:-msse4.1>)
		elseif(USE_SSSE3)
			target_compile_options(${target} PRIVATE $<$<OR:$<COMPILE_LANGUAGE:CXX>,$<COMPILE_LANGUAGE:OBJCXX>>:-mssse3>)
		elseif(USE_SSE3)
			target_compile_options(${target} PRIVATE $<$<OR:$<COMPILE_LANGUAGE:CXX>,$<COMPILE_LANGUAGE:OBJCXX>>:-msse3>)
		elseif(USE_SSE2)
			target_compile_options(${target} PRIVATE $<$<OR:$<COMPILE_LANGUAGE:CXX>,$<COMPILE_LANGUAGE:OBJCXX>>:-msse2>)
		elseif(USE_SSE)
			target_compile_options(${target} PRIVATE $<$<OR:$<COMPILE_LANGUAGE:CXX>,$<COMPILE_LANGUAGE:OBJCXX>>:-msse>)
		endif()

		if(PLATFORM_EMSCRIPTEN)
			target_compile_options(${target} PRIVATE -mtail-call)
			target_compile_options(${target} PRIVATE -mextended-const)
			# Multi-memory not supported in Edge, Firefox or Safari yet
			#target_compile_options(${target} PRIVATE -mmultimemory)
			# Relaxed SIMD not supported in Safari or Firefox yet
			#target_compile_options(${target} PRIVATE -mrelaxed-simd)

			target_compile_options(${target} PRIVATE -msimd128)
			target_compile_options(${target} PUBLIC -matomics)
			target_compile_options(${target} PRIVATE -mbulk-memory)
			target_compile_options(${target} PRIVATE -mnontrapping-fptoint)
			target_compile_options(${target} PRIVATE -msign-ext)
			# Disabling -mmultivalue for Debug currently as it's not compatible with DWARF debug info
			# Also causes some symbol issues in latest Emscripten
			#target_compile_options(${target} PRIVATE $<$<NOT:$<CONFIG:Debug>>:-mmultivalue>)
			target_compile_options(${target} PRIVATE -mmutable-globals)
			target_compile_options(${target} PRIVATE -mreference-types)

			if(OPTION_EXCEPTIONS)
				target_compile_options(${target} PRIVATE -mexception-handling)
			else()
				target_compile_options(${target} PRIVATE -mno-exception-handling)
			endif()
		endif()
	elseif(COMPILER_GCC)
		# Enable debug symbols in all builds
		target_compile_options(${target} PRIVATE -g)
		
		set_target_properties(${target} PROPERTIES C_VISIBILITY_PRESET hidden)
		set_target_properties(${target} PROPERTIES CXX_VISIBILITY_PRESET hidden)
		set_target_properties(${target} PROPERTIES VISIBILITY_INLINES_HIDDEN ON)

		target_compile_options(${target} PRIVATE
			$<$<AND:$<CONFIG:RelWithDebInfo>,$<OR:$<COMPILE_LANGUAGE:CXX>,$<COMPILE_LANGUAGE:OBJCXX>>>:-flto>
		)

		# Make sure the 128 bit compare exchange instruction is available
		target_compile_options(${target} PRIVATE $<$<OR:$<COMPILE_LANGUAGE:CXX>,$<COMPILE_LANGUAGE:OBJCXX>>:-mcx16>)

		target_compile_options(${target} PRIVATE -ffp-contract=off)

		target_compile_options(${target} PRIVATE
			$<$<CONFIG:Debug>:-O0>
			$<$<CONFIG:Profile>:-O1>
			$<$<AND:$<CONFIG:RelWithDebInfo>,$<COMPILE_LANGUAGE:CXX>>:-O3 -funroll-loops -fomit-frame-pointer>
		)

		if(NOT OPTION_EXCEPTIONS)
			target_compile_options(${target} PRIVATE $<$<OR:$<COMPILE_LANGUAGE:CXX>,$<COMPILE_LANGUAGE:OBJCXX>>:-fno-exceptions>)
		endif()
		target_compile_options(${target} PRIVATE $<$<OR:$<COMPILE_LANGUAGE:CXX>,$<COMPILE_LANGUAGE:OBJCXX>>:-fno-rtti>)

		if(OPTION_ADDRESS_SANITIZER)
			target_compile_options(${target} PRIVATE -fsanitize=address)
			target_link_options(${target} PRIVATE -fsanitize=address)
			target_compile_options(${target} PRIVATE -fsanitize-address-use-after-return)
			target_compile_options(${target} PRIVATE -fsanitize-address-use-after-scope)
		endif()
		
		if(USE_AVX512)
			target_compile_options(${target} PRIVATE $<$<OR:$<COMPILE_LANGUAGE:CXX>,$<COMPILE_LANGUAGE:OBJCXX>>:-mavx512f>)
		elseif(USE_AVX2)
			target_compile_options(${target} PRIVATE $<$<OR:$<COMPILE_LANGUAGE:CXX>,$<COMPILE_LANGUAGE:OBJCXX>>:-mavx2>)
		elseif(USE_AVX)
			target_compile_options(${target} PRIVATE $<$<OR:$<COMPILE_LANGUAGE:CXX>,$<COMPILE_LANGUAGE:OBJCXX>>:-mavx>)
		elseif(USE_SSE4_2)
			target_compile_options(${target} PRIVATE $<$<OR:$<COMPILE_LANGUAGE:CXX>,$<COMPILE_LANGUAGE:OBJCXX>>:-msse4.2>)
		elseif(USE_SSE4_1)
			target_compile_options(${target} PRIVATE $<$<OR:$<COMPILE_LANGUAGE:CXX>,$<COMPILE_LANGUAGE:OBJCXX>>:-msse4.1>)
		elseif(USE_SSSE3)
			target_compile_options(${target} PRIVATE $<$<OR:$<COMPILE_LANGUAGE:CXX>,$<COMPILE_LANGUAGE:OBJCXX>>:-mssse3>)
		elseif(USE_SSE3)
			target_compile_options(${target} PRIVATE $<$<OR:$<COMPILE_LANGUAGE:CXX>,$<COMPILE_LANGUAGE:OBJCXX>>:-msse3>)
		elseif(USE_SSE2)
			target_compile_options(${target} PRIVATE $<$<OR:$<COMPILE_LANGUAGE:CXX>,$<COMPILE_LANGUAGE:OBJCXX>>:-msse2>)
		elseif(USE_SSE)
			target_compile_options(${target} PRIVATE $<$<OR:$<COMPILE_LANGUAGE:CXX>,$<COMPILE_LANGUAGE:OBJCXX>>:-msse>)
		endif()
	endif()

	if(PLATFORM_EMSCRIPTEN)
		target_compile_options(${target} PRIVATE -pthread)
		target_link_options(${target} PRIVATE -pthread)
		target_link_options(${target} PRIVATE -sPTHREAD_POOL_SIZE=8)
		target_link_options(${target} PRIVATE -sPTHREAD_POOL_SIZE_STRICT=0)
		target_link_options(${target} PRIVATE -sPTHREAD_POOL_DELAY_LOAD=1)
		target_link_options(${target} PRIVATE -sPROXY_TO_PTHREAD=1)
		target_compile_definitions(${target} PRIVATE SUPPORT_PTHREADS=1)
		target_link_options(${target} PRIVATE -sWASM_WORKERS=2)

		target_link_options(${target} PRIVATE -sINITIAL_MEMORY=600Mb)
		target_link_options(${target} PRIVATE -sABORTING_MALLOC=0)
		target_link_options(${target} PRIVATE -sMAXIMUM_MEMORY=4GB)
		target_link_options(${target} PRIVATE -sALLOW_MEMORY_GROWTH=1)
		target_link_options(${target} PRIVATE -sENVIRONMENT=web,worker)
		target_link_options(${target} PRIVATE -sWASM=1)
		target_link_options(${target} PRIVATE -sASYNCIFY=0)
		target_link_options(${target} PRIVATE -sDISABLE_EXCEPTION_THROWING=1)
		target_link_options(${target} PRIVATE -sDISABLE_EXCEPTION_CATCHING=1)
		target_compile_options(${target} PRIVATE -sDISABLE_EXCEPTION_THROWING=1)
		target_compile_options(${target} PRIVATE -sDISABLE_EXCEPTION_CATCHING=1)

		target_link_options(${target} PRIVATE -sSTRICT=1)
		target_link_options(${target} PRIVATE -sINCOMING_MODULE_JS_API=['wasmMemory','buffer','instantiateWasm','wasm','mainScriptUrlOrBlob','mainScriptUrlOrBlobPromise'])
		target_link_options(${target} PRIVATE -sWASM_BIGINT=1)
		target_link_options(${target} PRIVATE -sUSE_WEBGPU=1)
		target_link_options(${target} PRIVATE -sUSE_SDL=0)
		target_link_options(${target} PRIVATE -sFETCH=1)
		target_link_options(${target} PRIVATE -sFETCH_SUPPORT_INDEXEDDB=1)
		target_link_options(${target} PRIVATE -sFILESYSTEM=1)
		target_link_options(${target} PRIVATE -sWASMFS=1)
		target_link_options(${target} PRIVATE -sMALLOC=none)
		target_link_options(${target} PRIVATE -sPROXY_POSIX_SOCKETS=1)
		target_link_options(${target} PRIVATE $<$<CONFIG:Debug>:-sSAFE_HEAP=0>)
		target_link_options(${target} PRIVATE $<$<CONFIG:Debug>:-sSTACK_OVERFLOW_CHECK=2>)
		target_link_options(${target} PRIVATE $<$<CONFIG:Profile>:-sSAFE_HEAP=0>)
		target_link_options(${target} PRIVATE $<$<CONFIG:Profile>:-sSTACK_OVERFLOW_CHECK=2>)
		target_link_options(${target} PRIVATE $<$<CONFIG:RelWithDebInfo>:-sSAFE_HEAP=0>)
		target_link_options(${target} PRIVATE $<$<CONFIG:RelWithDebInfo>:-sSTACK_OVERFLOW_CHECK=2>)

		# Disable use of the TextDecoder in multithreaded contexts
		# See https://github.com/emscripten-core/emscripten/issues/18034
		target_link_options(${target} PRIVATE -sTEXTDECODER=0)
		
		target_link_libraries(${target} PRIVATE html5)
		target_link_libraries(${target} PRIVATE html5.js)
		target_link_libraries(${target} PRIVATE html5_webgpu.js)
		target_link_libraries(${target} PRIVATE websocket.js)
		target_link_libraries(${target} PRIVATE stubs)

		# Still in the proposal stage, not default in any browser
		#target_compile_options(${target} PRIVATE -sMEMORY64=1)
		#target_link_options(${target} PRIVATE -sMEMORY64=1)
		#target_compile_options(${target} PRIVATE -Wno-experimental)
	endif()

	if(PLATFORM_APPLE)
		target_link_options(${target} PRIVATE "LINKER:-stack_size,${STACK_SIZE_HEX}")
	endif()
endfunction()

function(AddTargetOptions target)
	AddCoreTargetOptions(${target})

	target_compile_definitions(${target} PRIVATE CONTINUOUS_INTEGRATION=${CONTINUOUS_INTEGRATION})
	target_compile_definitions(${target} PRIVATE TARGET_DISTRIBUTION="${OPTION_DISTRIBUTION}")

	# Temporary while migrating preprocessor defines, TODO: make sure files include this
	if(MSVC)
		target_compile_options(${target} PRIVATE /FICommon/Common.h)
	else()
		target_compile_options(${target} PRIVATE -include Common/Common.h)
	endif()

	# Enable Asserts in all builds for now
	target_compile_definitions(${target} PRIVATE ENABLE_ASSERTS=1)

 	if (PLATFORM_APPLE)
		target_compile_options(${target} PRIVATE $<$<OR:$<COMPILE_LANGUAGE:CXX>,$<COMPILE_LANGUAGE:OBJCXX>>:-xobjective-c++>)
  	endif()

	string(REPLACE ";" "," BUILD_CONFIG_TYPES_DELIMITED "${BUILD_CONFIG_TYPES}")
	string(REPLACE ";" "," PLATFORM_TYPES_DELIMITED "${PLATFORM_TYPES}")

	target_compile_definitions(${target} PRIVATE PLATFORM_CONFIGURATION_TYPES="${BUILD_CONFIG_TYPES_DELIMITED}")
	target_compile_definitions(${target} PRIVATE PLATFORM_TYPES="${PLATFORM_TYPES_DELIMITED}")

	foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
		string( TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG_UPPER )
		target_compile_definitions(${target} PRIVATE $<$<CONFIG:${OUTPUTCONFIG}>:CONFIGURATION_NAME="${OUTPUTCONFIG}">)
		target_compile_definitions(${target} PRIVATE $<$<CONFIG:${OUTPUTCONFIG}>:CONFIGURATION_${OUTPUTCONFIG_UPPER}=1>)
	endforeach()

	string(REPLACE ";" "," TARGET_ENVIRONMENTS_DELIMITED "${TARGET_ENVIRONMENTS}")

	target_compile_definitions(${target} PRIVATE TARGET_ENVIRONMENT="${TARGET_ENVIRONMENT}")
	target_compile_definitions(${target} PRIVATE TARGET_ENVIRONMENTS="${TARGET_ENVIRONMENTS_DELIMITED}")

	target_compile_definitions(${target} PRIVATE
		$<$<CONFIG:Debug>:NDEBUG>
		$<$<CONFIG:Profile>:NDEBUG>
		$<$<CONFIG:RelWithDebInfo>:NDEBUG>)
	target_compile_definitions(${target} PRIVATE
		$<$<CONFIG:Debug>:DEBUG_BUILD=1>
		$<$<CONFIG:Profile>:DEBUG_BUILD=0>
		$<$<CONFIG:RelWithDebInfo>:DEBUG_BUILD=0>)
	target_compile_definitions(${target} PRIVATE
		$<$<CONFIG:Debug>:PROFILE_BUILD=1>
		$<$<CONFIG:Profile>:PROFILE_BUILD=1>
		$<$<CONFIG:RelWithDebInfo>:PROFILE_BUILD=0>)
	target_compile_definitions(${target} PRIVATE
		$<$<CONFIG:Debug>:RELEASE_BUILD=0>
		$<$<CONFIG:Profile>:RELEASE_BUILD=0>
		$<$<CONFIG:RelWithDebInfo>:RELEASE_BUILD=1>)

	if (OPTION_PACKAGE)
		target_compile_definitions(${target} PRIVATE PACKAGED_BUILD=1)
		if (DEFINED PROJECT_FILE)
			get_filename_component(PROJECT_FILE_RELATIVE_PATH "${PROJECT_FILE}" NAME)
			target_compile_definitions(${target} PRIVATE PACKAGED_PROJECT_FILE_PATH="${PROJECT_FILE_RELATIVE_PATH}")
		else()
			target_compile_definitions(${target} PRIVATE PACKAGED_PROJECT_FILE_PATH="")
		endif()
	else()
		target_compile_definitions(${target} PRIVATE PACKAGED_BUILD=0)
		target_compile_definitions(${target} PRIVATE PACKAGED_PROJECT_FILE_PATH="")
	endif()

	target_compile_definitions(${target} PRIVATE PLUGINS_IN_EXECUTABLE=1)

	target_compile_definitions(${target} PRIVATE USE_SVML=${USE_SVML})

	if (COMPILER_MSVC)
		if (USE_SSE)
			target_compile_definitions(${target} PRIVATE __SSE__)
		endif()
		if (USE_SSE2)
			target_compile_definitions(${target} PRIVATE __SSE2__)
		endif()
		if (USE_SSE3)
			target_compile_definitions(${target} PRIVATE __SSE3__)
		endif()
		if (USE_SSSE3)
			target_compile_definitions(${target} PRIVATE __SSSE3__)
		endif()
		if (USE_SSE4_1)
			target_compile_definitions(${target} PRIVATE __SSE4_1__)
		endif()
		if (USE_SSE4_2)
			target_compile_definitions(${target} PRIVATE __SSE4_2__)
		endif()
	endif()

	target_compile_definitions(${target} PRIVATE USE_SDL=${USE_SDL})

	if(OPTION_EXCEPTIONS)
		target_compile_definitions(${target} PRIVATE ENABLE_EXCEPTIONS=1)
	else()
		target_compile_definitions(${target} PRIVATE ENABLE_EXCEPTIONS=0)
	endif()

	set_target_properties(${target} PROPERTIES OPTIMIZE_DEPENDENCIES ON)
	set_target_properties(${target} PROPERTIES 
		UNITY_BUILD ${OPTION_UNITY_BUILD}
		UNITY_BUILD_BATCH_SIZE 4
	)

	if(COMPILER_MSVC OR COMPILER_CLANG_WINDOWS)
		# Enable most warnings and treat them as errors
		target_compile_options(${target} PRIVATE /Wall /WX)
		target_link_options(${target} PRIVATE /WX)

		# Enable type conversion rules
		target_compile_options(${target} PRIVATE /Zc:rvalueCast)
		# Enable standards conformance
		target_compile_options(${target} PRIVATE /permissive-)

		# Enable nameless struct
		target_compile_options(${target} PRIVATE /wd4201)
		# Enable empty controlled statements (for Assert compiling out in Release mode)
		target_compile_options(${target} PRIVATE /wd4390)
		# Disable extra padding warning
		target_compile_options(${target} PRIVATE /wd4324)
		# Disable preprocessor "is not defined as a preprocessor macro, replacing with '0' for '#if/#elif'"
		target_compile_options(${target} PRIVATE /wd4668)
		# Disable ' 'noexcept' used with no exception handling mode specified; termination on exception is not guaranteed. '
		target_compile_options(${target} PRIVATE /wd4577)
		# Disable extra padding warning
		target_compile_options(${target} PRIVATE /wd4820)
		# Disable implicitly deleted constructors and assignments
		target_compile_options(${target} PRIVATE /wd4625)
		target_compile_options(${target} PRIVATE /wd4626)
		target_compile_options(${target} PRIVATE /wd4623)
		target_compile_options(${target} PRIVATE /wd5027)
		target_compile_options(${target} PRIVATE /wd5026)
		# Disable signed / unsigned mismatch (really really picky compared to clang)
		target_compile_options(${target} PRIVATE /wd4365)
		# Disable relative include path contains '..'
		target_compile_options(${target} PRIVATE /wd4464)
		# Disable constructor is not implicitly called  (triggers on inactive union members)
		target_compile_options(${target} PRIVATE /wd4582)
		# Disable required explicit handling of all enum switch cases
		target_compile_options(${target} PRIVATE /wd4061)
		# Disable unreferenced inline function has been removed
		target_compile_options(${target} PRIVATE /wd4514)
		# Disable 'class has virtual functions, but its trivial destructor is not virtual' (fails with constexpr)
		target_compile_options(${target} PRIVATE /wd5204)
		# Disable behavior change: constructor is no longer implicitly called
		target_compile_options(${target} PRIVATE /wd4587)
		# Disable layout of class may have changed from a previous version of the compiler due to better packing of member '
		target_compile_options(${target} PRIVATE /wd4371)
		# Disable Compiler will insert Spectre mitigation for memory load if /Qspectre switch specified
		target_compile_options(${target} PRIVATE /wd5045)
		# Disable union destructor is not implicitly called
		target_compile_options(${target} PRIVATE /wd4583)
		# Disable reinterpret_cast used between related classes (triggers in templates)
		target_compile_options(${target} PRIVATE /wd4946)
		# Disable compiler may not enforce left-to-right evaluation order in braced initializer list
		target_compile_options(${target} PRIVATE /wd4868)
		# Disable  function not inlined
		target_compile_options(${target} PRIVATE /wd4710)
		# Disable 'this': used in base member initializer list
		target_compile_options(${target} PRIVATE /wd4355)
		# Disable compiler may not enforce left-to-right evaluation order for call to type
		target_compile_options(${target} PRIVATE /wd4866)
		# Disable 'std::chrono::operator -': possible change in behavior, change in UDT return calling convention
		target_compile_options(${target} PRIVATE /wd4686)
		# Disable a non-static data member with a volatile qualified type no longer implies that compiler generated copy/move constructors and copy/move assignment operators are not trivial
		target_compile_options(${target} PRIVATE /wd5220)
		# Disable the initialization of a subobject should be wrapped in braces
		target_compile_options(${target} PRIVATE /wd5246)
		# Disable 'reinterpret_cast': unsafe conversion from  'X' to 'Y'
		target_compile_options(${target} PRIVATE /wd4191)
		# Disable functions selected for automatic inline expansion but not marked inline
		target_compile_options(${target} PRIVATE /wd4711)
		# Disable implicit fall-through occurs here; (this is bugged in VS compiler version 17.4)
		target_compile_options(${target} PRIVATE /wd5262)
		# Disable 'variable-name': 'const' variable is not used
		target_compile_options(${target} PRIVATE /wd5264)
		# MACRO is defined to be '0': did you mean to use '#if MACRO'?
		# Breaks in MSVC's own headers
		target_compile_options(${target} PRIVATE /wd4574)
		# command line argument number X does not match precompiled header
		target_compile_options(${target} PRIVATE /wd4599)

		# Disable LNK4099 The linker was unable to find your .pdb file for 3rdparty libs
		target_link_options(${target} PRIVATE /ignore:4099)
		# Disable LNK4075
		target_link_options(${target} PRIVATE /ignore:4075)

		if(COMPILER_CLANG_WINDOWS)
			target_compile_options(${target} PRIVATE -Weverything -Wno-long-long -Werror -Wno-missing-braces -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-newline-eof -Wno-undef -Wno-exit-time-destructors -Wno-global-constructors -Wno-covered-switch-default -Wno-switch-enum -Wno-macro-redefined -Wno-old-style-cast -Wno-reserved-id-macro -Wno-documentation-unknown-command -Wno-unused-member-function -Wno-missing-prototypes -Wno-gnu-zero-variadic-macro-arguments -Wno-shadow-uncaptured-local -Wno-sign-conversion -Wno-missing-field-initializers -Wunused-parameter -Wno-ignored-attributes -Wno-range-loop-analysis -Wstring-conversion -Wno-zero-as-null-pointer-constant -Wno-extra-semi-stmt -Wno-extra-semi -Wno-double-promotion -Wno-inconsistent-missing-destructor-override -Wno-gnu-anonymous-struct -Wno-nested-anon-types -Wno-suggest-destructor-override -Wno-comma -Wno-duplicate-enum -Wno-ctad-maybe-unsupported -Wno-undefined-reinterpret-cast -Wno-c++20-compat -Wno-float-equal -Wno-undefined-func-template -Wno-unused-template -Wno-return-std-move-in-c++11 -Wno-microsoft-enum-value -Wno-microsoft-cast -Wno-documentation -Wno-ignored-qualifiers)
		endif()

	elseif(COMPILER_CLANG)
		target_compile_options(${target} PRIVATE $<$<OR:$<COMPILE_LANGUAGE:C>,$<COMPILE_LANGUAGE:CXX>,$<COMPILE_LANGUAGE:OBJCXX>>:
			-Weverything 
			-Wno-long-long 
			-Werror 
			-Wno-missing-braces 
			-Wno-c++98-compat
			-Wno-c++98-compat-pedantic 
			-Wno-newline-eof 
			-Wno-undef 
			-Wno-exit-time-destructors 
			-Wno-global-constructors 
			-Wno-covered-switch-default 
			-Wno-switch-enum 
			-Wno-macro-redefined 
			-Wno-unknown-warning-option
			-Wno-old-style-cast 
			-Wno-reserved-id-macro 
			-Wno-documentation-unknown-command 
			-Wno-unused-member-function 
			-Wno-missing-prototypes 
			-Wno-gnu-zero-variadic-macro-arguments 
			-Wno-shadow-uncaptured-local 
			-Wno-sign-conversion 
			-Wno-missing-field-initializers 
			-Wunused-parameter 
			-Wno-ignored-attributes 
			-Wno-range-loop-analysis 
			-Wstring-conversion 
			-Wno-zero-as-null-pointer-constant 
			-Wno-extra-semi-stmt 
			-Wno-extra-semi 
			-Wno-double-promotion 
			-Wno-inconsistent-missing-destructor-override 
			-Wno-gnu-anonymous-struct 
			-Wno-nested-anon-types 
			-Wno-suggest-destructor-override 
			-Wno-comma 
			-Wno-duplicate-enum 
			-Wno-ctad-maybe-unsupported 
			-Wno-undefined-reinterpret-cast 
			-Wno-c++20-compat 
			-Wno-float-equal 
			-Wno-undefined-func-template 
			-Wno-unused-template 
			-Wno-return-std-move-in-c++11 
			-Wno-microsoft-enum-value 
			-Wno-microsoft-cast 
			-Wno-documentation 
			-Wno-ignored-qualifiers 
			-Wno-unknown-warning-option 
			-Wconsumed 
			-Wno-padded 
			-Wno-deprecated-copy-with-user-provided-copy 
			-Wno-deprecated-copy 
			-Wno-weak-vtables 
			-Wno-weak-template-vtables 
			-Wno-deprecated-copy-with-user-provided-dtor 
			-Wno-alloca 
			-Wno-direct-ivar-access 
			-Wno-unused-macros 
			-Wno-unreachable-code 
			-Wno-disabled-macro-expansion 
			-Wno-missing-variable-declarations 
			-Wno-objc-missing-property-synthesis 
			-Wno-unused-command-line-argument 
			-Wno-bitwise-instead-of-logical 
			-Wno-reserved-identifier 
			-Wno-non-virtual-dtor 
			-Wno-unsafe-buffer-usage 
			-Wno-switch-default 
			-Wno-c++20-extensions
			-Wno-unknown-warning-option
			-Wno-unique-object-duplication>
		)
		if (PLATFORM_EMSCRIPTEN)
			target_compile_options(${target} PRIVATE -Wno-overriding-option)
		endif()
	elseif(COMPILER_GCC)
		target_compile_options(${target} PRIVATE $<$<OR:$<COMPILE_LANGUAGE:CXX>,$<COMPILE_LANGUAGE:OBJCXX>>:
			-Wall 
			#-Werror
			-Wno-ignored-attributes
			-Wno-ignored-qualifiers
			-Wno-attributes
			-Wno-builtin-macro-redefined
			-Wno-macro-redefined
			-Wno-changes-meaning
			-Wno-missing-field-initializers
			-Wno-range-loop-construct
			-Wno-maybe-uninitialized
			-Wno-unknown-warning-option
		>)
	endif()

	if (PLATFORM_POSIX AND NOT PLATFORM_EMSCRIPTEN)
		target_compile_definitions(${target} PRIVATE SUPPORT_PTHREADS=1)
	endif()

	if(PLATFORM_APPLE)
		SetupAppleTarget(${target})
	endif()
endfunction()
