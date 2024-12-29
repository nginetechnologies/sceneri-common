#pragma once

// Indicates that the function guarantees it does not change any non-local memory such as the return value
// The function is only allowed to depend on the parameters (no pointers including 'this'!)
#if COMPILER_CLANG || COMPILER_GCC
#define PURE_NOSTATICS __attribute__((const))
#elif COMPILER_MSVC
#define PURE_NOSTATICS __declspec(noalias)
#else
#define PURE_NOSTATICS
#endif

// Indicates that the function guarantees it does not change any non-local memory such as the return value
// The function is allowed to depend on the parameters and global variables (including pointers such as 'this')
#if COMPILER_CLANG || COMPILER_GCC
#define PURE_STATICS __attribute__((pure))
#else
#define PURE_STATICS
#endif

// Used to indicate that a function only depends on the locals and first-level indirections (pointer parameters)
// Indicates that the function guarantees it does not change any non-local memory such as the return value
#if COMPILER_CLANG || COMPILER_GCC
#define PURE_LOCALS_AND_POINTERS __attribute__((pure))
#elif COMPILER_MSVC
#define PURE_LOCALS_AND_POINTERS __declspec(noalias)
#else
#define PURE_LOCALS_AND_POINTERS
#endif

#if COMPILER_MSVC
#define RESTRICTED_RETURN __declspec(restrict)
#else
#define RESTRICTED_RETURN
#endif
