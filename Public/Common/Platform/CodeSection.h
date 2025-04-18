#pragma once

#if COMPILER_MSVC
#define CODE_SECTION(name) __declspec(code_seg(name))
#elif PLATFORM_APPLE
#define CODE_SECTION(name) __attribute__((section("__TEXT,__" name ",regular,pure_instructions")))
#elif COMPILER_CLANG /* || COMPILER_GCC */
#define CODE_SECTION(name) __attribute__((section(name)))
#else
#define CODE_SECTION(name)
#endif
