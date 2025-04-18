
#pragma once

#include <Common/Platform/CompilerWarnings.h>
#include <Common/Math/CoreNumericTypes.h>
#include <Common/Platform/InternalLinkage.h>
#include <Common/Platform/ForceInline.h>
#include <Common/Platform/Pure.h>

#if COMPILER_GCC
#define __cdecl __attribute__((__cdecl__))
#endif

#include <new>

PUSH_MSVC_WARNINGS
DISABLE_MSVC_WARNINGS(28251)

[[nodiscard]] extern void* __cdecl operator new(const ngine::size size);
[[nodiscard]] extern void* __cdecl operator new[](const ngine::size size);
[[nodiscard]] extern void* __cdecl operator new(const ngine::size size, std::align_val_t);
[[nodiscard]] extern void* __cdecl operator new[](const ngine::size size, std::align_val_t);
[[nodiscard]] extern void* __cdecl operator new(const ngine::size size, const std::nothrow_t&) noexcept;
[[nodiscard]] extern void* __cdecl operator new[](const ngine::size size, const std::nothrow_t&) noexcept;
[[nodiscard]] extern void* __cdecl operator new(const ngine::size size, std::align_val_t, const std::nothrow_t&) noexcept;
[[nodiscard]] extern void* __cdecl operator new[](const ngine::size size, std::align_val_t, const std::nothrow_t&) noexcept;

extern void __cdecl operator delete(void* pPointer) noexcept;
extern void __cdecl operator delete[](void* pPointer) noexcept;
extern void __cdecl operator delete(void* pPointer, std::align_val_t) noexcept;
extern void __cdecl operator delete[](void* pPointer, std::align_val_t) noexcept;
extern void __cdecl operator delete(void* pPointer, ngine::size) noexcept;
extern void __cdecl operator delete[](void* pPointer, ngine::size) noexcept;
extern void __cdecl operator delete(void* pPointer, ngine::size, std::align_val_t) noexcept;
extern void __cdecl operator delete[](void* pPointer, ngine::size, std::align_val_t) noexcept;

extern void __cdecl operator delete(void* pPointer, const std::nothrow_t&) noexcept;
extern void __cdecl operator delete[](void* pPointer, const std::nothrow_t&) noexcept;
extern void __cdecl operator delete(void* pPointer, std::align_val_t, const std::nothrow_t&) noexcept;
extern void __cdecl operator delete[](void* pPointer, std::align_val_t, const std::nothrow_t&) noexcept;

#if _MSC_VER
PUSH_CLANG_WARNINGS
DISABLE_CLANG_WARNING("-Wreserved-id-macro");

#define __PLACEMENT_NEW_INLINE
#define __PLACEMENT_VEC_NEW_INLINE

POP_CLANG_WARNINGS
#endif

POP_MSVC_WARNINGS
