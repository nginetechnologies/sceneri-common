#pragma once

#if COMPILER_MSVC
#define MUST_TAIL
#elif COMPILER_GCC
#define MUST_TAIL
#elif COMPILER_CLANG && (!PLATFORM_WEBASSEMBLY || defined(__wasm_tail_call__))
#if __has_cpp_attribute(clang::musttail) && !PLATFORM_LINUX
#define MUST_TAIL [[clang::musttail]]
#else
#define MUST_TAIL
#endif
#else
#define MUST_TAIL
#endif
