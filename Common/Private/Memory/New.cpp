#include "Memory/New.h"
#include "Memory/Allocators/Allocate.h"

#include <cstdlib>
#include <cstring>

#define USE_MIMALLOC (!PLATFORM_APPLE)
#if USE_MIMALLOC
#include "Common/3rdparty/mimalloc/mimalloc.h"
#elif PLATFORM_APPLE
#include <malloc/malloc.h>
#elif PLATFORM_POSIX
#include <malloc.h>
#endif

#if PLATFORM_EMSCRIPTEN
#include <emscripten/heap.h>
#endif

#include <Common/Assert/Assert.h>
#include <Common/TypeTraits/IsSame.h>
#include <Common/Math/Min.h>
#include <Common/Math/NumericLimits.h>
#include <Common/Platform/Assume.h>
#include <Common/Platform/Likely.h>

#include <Common/Threading/AtomicInteger.h>

static_assert(ngine::TypeTraits::IsSame<ngine::size, std::size_t>);

namespace ngine::Memory
{
	[[nodiscard]] RESTRICTED_RETURN void* AllocateOnStack(const size requestedSize) noexcept
	{
#if COMPILER_MSVC || COMPILER_CLANG_WINDOWS
		return _alloca(requestedSize);
#elif COMPILER_CLANG || COMPILER_GCC
		return __builtin_alloca(requestedSize);
#else
		return alloca(requestedSize);
#endif
	}

	static Threading::Atomic<size> memoryUsage = 0;

	[[nodiscard]] size GetDynamicMemoryUsage() noexcept
	{
		return memoryUsage;
	}

	static Threading::Atomic<size> biggestMemoryAllocation = 0;
	static Threading::Atomic<size> highestMemoryUsage = 0;

#define MAXIMUM_MEMORY_LIMIT_MB 0
#define MAXIMUM_INDIVIDUAL_ALLOCATION_SIZE_MB 0
#define TRACK_MEMORY_USAGE PROFILE_BUILD
#define TRACK_BIGGEST_MEMORY_ALLOCATION 0
#define TRACK_HIGHEST_MEMORY_USAGE 0
#define TRACK_MEMORY_USAGE_INTERNAL TRACK_MEMORY_USAGE || MAXIMUM_MEMORY_LIMIT_MB > 0

	[[nodiscard]] FORCE_INLINE bool TryRequestMemory(const size requestedSize)
	{
		if constexpr (MAXIMUM_INDIVIDUAL_ALLOCATION_SIZE_MB > 0)
		{
			constexpr size MaximumAllocationSize = MAXIMUM_INDIVIDUAL_ALLOCATION_SIZE_MB * 1024 * 1024;
			if (requestedSize > MaximumAllocationSize)
			{
				return false;
			}
		}

		if constexpr (TRACK_BIGGEST_MEMORY_ALLOCATION)
		{
			biggestMemoryAllocation.AssignMax(requestedSize);
		}

		size currentMemoryUsage = memoryUsage;
		size requestedNewMemoryUsage = currentMemoryUsage + requestedSize;

		while (true)
		{
			if constexpr (MAXIMUM_MEMORY_LIMIT_MB > 0)
			{
				constexpr size MaximumMemoryUsage = 1024 * 1024 * MAXIMUM_MEMORY_LIMIT_MB;
				if (requestedNewMemoryUsage > MaximumMemoryUsage)
				{
					return false;
				}
			}

			if (memoryUsage.CompareExchangeWeak(currentMemoryUsage, requestedNewMemoryUsage))
			{
				if constexpr (TRACK_HIGHEST_MEMORY_USAGE)
				{
					highestMemoryUsage.AssignMax(requestedNewMemoryUsage);
				}

				return true;
			}
			requestedNewMemoryUsage = currentMemoryUsage + requestedSize;
		}

		ExpectUnreachable();
	}

	[[nodiscard]] PURE_STATICS size GetAllocatedMemorySize(void* pPointer) noexcept
	{
		if (pPointer == nullptr)
		{
			return 0;
		}

#if USE_MIMALLOC
		return mi_malloc_size(pPointer);
#elif COMPILER_MSVC
		return _msize(pPointer);
#elif PLATFORM_APPLE
		return malloc_size(pPointer);
#elif PLATFORM_POSIX
		return malloc_usable_size(pPointer);
#else
#error "Not implemented for platform"
#endif
	}

	[[nodiscard]] PURE_STATICS size GetAllocatedAlignedMemorySize(void* pPointer, [[maybe_unused]] size alignment) noexcept
	{
		if (pPointer == nullptr)
		{
			return 0;
		}

#if USE_MIMALLOC
		return mi_malloc_size(pPointer);
#elif COMPILER_MSVC
		return _aligned_msize(pPointer, alignment, 0);
#elif PLATFORM_APPLE
		return malloc_size(pPointer);
#elif PLATFORM_POSIX
		return malloc_usable_size(pPointer);
#else
#error "Not implemented for platform"
#endif
	}

	[[nodiscard]] RESTRICTED_RETURN void* Allocate(const size requestedSize) noexcept
	{
		if constexpr (TRACK_MEMORY_USAGE_INTERNAL)
		{
			if (!TryRequestMemory(requestedSize))
			{
				Assert(false, "Ran out of memory!");
				return nullptr;
			}
		}

#if USE_MIMALLOC
		void* pAllocation = mi_malloc(requestedSize);
#else
		void* pAllocation = malloc(requestedSize);
#endif
		Assert(pAllocation != nullptr || requestedSize == 0);

		if constexpr (TRACK_MEMORY_USAGE_INTERNAL)
		{
			memoryUsage += (GetAllocatedMemorySize(pAllocation) - requestedSize) * (pAllocation != nullptr);
			memoryUsage -= requestedSize * (pAllocation == nullptr);
		}

		return pAllocation;
	}

	[[nodiscard]] RESTRICTED_RETURN void* AllocateSmall(const size requestedSize) noexcept
	{
#if USE_MIMALLOC
		static_assert(MaximumSmallAllocationSize == MI_SMALL_SIZE_MAX);
#endif
		Assert(requestedSize <= MaximumSmallAllocationSize);
		Expect(requestedSize <= MaximumSmallAllocationSize);

		if constexpr (TRACK_MEMORY_USAGE_INTERNAL)
		{
			if (!TryRequestMemory(requestedSize))
			{
				Assert(false, "Ran out of memory!");
				return nullptr;
			}
		}

#if USE_MIMALLOC
		void* pAllocation = mi_malloc_small(requestedSize);
#else
		void* pAllocation = malloc(requestedSize);
#endif
		Assert(pAllocation != nullptr || requestedSize == 0);

		if constexpr (TRACK_MEMORY_USAGE_INTERNAL)
		{
			memoryUsage += (GetAllocatedMemorySize(pAllocation) - requestedSize) * (pAllocation != nullptr);
			memoryUsage -= requestedSize * (pAllocation == nullptr);
		}

		return pAllocation;
	}

	[[nodiscard]] RESTRICTED_RETURN void* Reallocate(void* pPointer, const size requestedSize) noexcept
	{
		if constexpr (TRACK_MEMORY_USAGE_INTERNAL)
		{
			if (!TryRequestMemory(requestedSize))
			{
				Assert(false, "Ran out of memory!");
				return nullptr;
			}
		}

		Expect(pPointer != nullptr);
#if TRACK_MEMORY_USAGE_INTERNAL
		const size previousAllocationSize = GetAllocatedMemorySize(pPointer);
#endif

#if USE_MIMALLOC
		void* pAllocation = mi_realloc(pPointer, requestedSize);
#else
		void* pAllocation = realloc(pPointer, requestedSize);
#endif
		Assert(pAllocation != nullptr || requestedSize == 0);

#if TRACK_MEMORY_USAGE_INTERNAL
		{
			memoryUsage += (GetAllocatedMemorySize(pAllocation) - requestedSize) * (pAllocation != nullptr);
			memoryUsage -= requestedSize * (pAllocation == nullptr);
			memoryUsage -= previousAllocationSize * (pAllocation != nullptr);
		}
#endif

		return pAllocation;
	}

	void Deallocate(void* pPointer) noexcept
	{
#if TRACK_MEMORY_USAGE_INTERNAL
		const size allocationSize = GetAllocatedMemorySize(pPointer);
#endif

#if USE_MIMALLOC
		mi_free(pPointer);
#else
		free(pPointer);
#endif

#if TRACK_MEMORY_USAGE_INTERNAL
		{
			memoryUsage -= allocationSize;
		}
#endif
	}

	[[nodiscard]] RESTRICTED_RETURN void* AllocateAligned(const size requestedSize, const size alignment) noexcept
	{
		if constexpr (TRACK_MEMORY_USAGE_INTERNAL)
		{
			if (!TryRequestMemory(requestedSize))
			{
				Assert(false, "Ran out of memory!");
				return nullptr;
			}
		}

#if USE_MIMALLOC
		void* pAllocation = mi_new_aligned_nothrow(requestedSize, alignment);
#elif COMPILER_MSVC || COMPILER_CLANG_WINDOWS
		void* pAllocation = _aligned_malloc(requestedSize, alignment);
#elif PLATFORM_POSIX
		void* pAllocation;
		posix_memalign(&pAllocation, alignment, requestedSize);
#else
#pragma error "Not implemented for platform"
#endif

		Assert(pAllocation != nullptr || requestedSize == 0);

		if constexpr (TRACK_MEMORY_USAGE_INTERNAL)
		{
			memoryUsage += (GetAllocatedAlignedMemorySize(pAllocation, alignment) - requestedSize) * (pAllocation != nullptr);
			memoryUsage -= requestedSize * (pAllocation == nullptr);
		}
		return pAllocation;
	}

	[[nodiscard]] RESTRICTED_RETURN void* ReallocateAligned(void* pPointer, const size requestedSize, const size alignment) noexcept
	{
		ASSUME(pPointer != nullptr);

		if constexpr (TRACK_MEMORY_USAGE_INTERNAL)
		{
			if (!TryRequestMemory(requestedSize))
			{
				Assert(false, "Ran out of memory!");
				return nullptr;
			}
		}

#if TRACK_MEMORY_USAGE_INTERNAL || (PLATFORM_POSIX && !USE_MIMALLOC)
		const size previousAllocationSize = GetAllocatedAlignedMemorySize(pPointer, alignment);
#endif

#if USE_MIMALLOC
		void* pAllocation = mi_realloc_aligned(pPointer, requestedSize, alignment);
#elif COMPILER_MSVC || COMPILER_CLANG_WINDOWS
		void* pAllocation = _aligned_realloc(pPointer, requestedSize, alignment);
#elif PLATFORM_POSIX
		void* pAllocation;
		if (LIKELY(posix_memalign(&pAllocation, alignment, requestedSize) == 0 && pAllocation != nullptr))
		{
			memcpy(pAllocation, pPointer, Math::Min(previousAllocationSize, requestedSize));
			free(pPointer);
		}
		else
		{
			Assert(requestedSize == 0);
			pAllocation = nullptr;
		}
#else
#pragma error "Not implemented for platform"
#endif

#if TRACK_MEMORY_USAGE_INTERNAL
		{
			memoryUsage += (GetAllocatedAlignedMemorySize(pAllocation, alignment) - requestedSize) * (pAllocation != nullptr);
			memoryUsage -= requestedSize * (pAllocation == nullptr);
			memoryUsage -= previousAllocationSize * (pAllocation != nullptr);
		}
#endif

		return pAllocation;
	}

	void DeallocateAligned(void* pPointer, [[maybe_unused]] const size alignment) noexcept
	{
#if TRACK_MEMORY_USAGE_INTERNAL
		[[maybe_unused]] const size allocationSize = GetAllocatedAlignedMemorySize(pPointer, alignment);
#endif

#if USE_MIMALLOC
		mi_free_aligned(pPointer, alignment);
#elif COMPILER_MSVC || COMPILER_CLANG_WINDOWS
		_aligned_free(pPointer);
#else
		free(pPointer);
#endif

#if TRACK_MEMORY_USAGE_INTERNAL
		{
			memoryUsage -= allocationSize;
		}
#endif
	}

	static Threading::Atomic<size> graphicsMemoryUsage = 0;

	static Threading::Atomic<size> biggestGraphicsMemoryAllocation = 0;
	static Threading::Atomic<size> highestGraphicsMemoryUsage = 0;

	void ReportGraphicsAllocation(const size requestedSize) noexcept
	{
		if constexpr (TRACK_BIGGEST_MEMORY_ALLOCATION)
		{
			biggestGraphicsMemoryAllocation.AssignMax(requestedSize);
		}

		size currentMemoryUsage = graphicsMemoryUsage;
		size requestedNewMemoryUsage = currentMemoryUsage + requestedSize;

		while (true)
		{
			if (graphicsMemoryUsage.CompareExchangeWeak(currentMemoryUsage, requestedNewMemoryUsage))
			{
				if constexpr (TRACK_HIGHEST_MEMORY_USAGE)
				{
					highestGraphicsMemoryUsage.AssignMax(requestedNewMemoryUsage);
				}
				return;
			}
			requestedNewMemoryUsage = currentMemoryUsage + requestedSize;
		}

		ExpectUnreachable();
	}

	void ReportGraphicsDeallocation(const size size) noexcept
	{
		graphicsMemoryUsage -= size;
	}

	[[nodiscard]] PURE_STATICS size GetGraphicsMemoryUsage() noexcept
	{
		return graphicsMemoryUsage;
	}
}

[[nodiscard]] void* __cdecl operator new(const ngine::size size)
{
	return ngine::Memory::Allocate(size);
}

[[nodiscard]] void* __cdecl operator new[](const ngine::size size)
{
	return ngine::Memory::Allocate(size);
}

[[nodiscard]] void* __cdecl operator new(const ngine::size size, std::align_val_t align)
{
	return ngine::Memory::AllocateAligned(size, static_cast<ngine::size>(align));
}
[[nodiscard]] void* __cdecl operator new[](const ngine::size size, std::align_val_t align)
{
	return ngine::Memory::AllocateAligned(size, static_cast<ngine::size>(align));
}

[[nodiscard]] void* __cdecl operator new(const ngine::size size, const std::nothrow_t&) noexcept
{
	return ngine::Memory::Allocate(size);
}

[[nodiscard]] void* __cdecl operator new[](const ngine::size size, const std::nothrow_t&) noexcept
{
	return ngine::Memory::Allocate(size);
}

[[nodiscard]] void* __cdecl operator new(const ngine::size size, std::align_val_t align, const std::nothrow_t&) noexcept
{
	return ngine::Memory::AllocateAligned(size, static_cast<ngine::size>(align));
}
[[nodiscard]] void* __cdecl operator new[](const ngine::size size, std::align_val_t align, const std::nothrow_t&) noexcept
{
	return ngine::Memory::AllocateAligned(size, static_cast<ngine::size>(align));
}

void __cdecl operator delete(void* pPointer) noexcept
{
	ngine::Memory::Deallocate(pPointer);
}

void __cdecl operator delete[](void* pPointer) noexcept
{
	ngine::Memory::Deallocate(pPointer);
}

void __cdecl operator delete(void* pPointer, std::align_val_t align) noexcept
{
	ngine::Memory::DeallocateAligned(pPointer, static_cast<ngine::size>(align));
}

void __cdecl operator delete[](void* pPointer, std::align_val_t align) noexcept
{
	ngine::Memory::DeallocateAligned(pPointer, static_cast<ngine::size>(align));
}

void __cdecl operator delete(void* pPointer, const ngine::size) noexcept
{
	ngine::Memory::Deallocate(pPointer);
}

void __cdecl operator delete[](void* pPointer, const ngine::size) noexcept
{
	ngine::Memory::Deallocate(pPointer);
}

void __cdecl operator delete(void* pPointer, const ngine::size, std::align_val_t align) noexcept
{
	ngine::Memory::DeallocateAligned(pPointer, static_cast<ngine::size>(align));
}

void __cdecl operator delete[](void* pPointer, const ngine::size, std::align_val_t align) noexcept
{
	ngine::Memory::DeallocateAligned(pPointer, static_cast<ngine::size>(align));
}

void __cdecl operator delete(void* pPointer, const std::nothrow_t&) noexcept
{
	ngine::Memory::Deallocate(pPointer);
}

void __cdecl operator delete[](void* pPointer, const std::nothrow_t&) noexcept
{
	ngine::Memory::Deallocate(pPointer);
}

void __cdecl operator delete(void* pPointer, std::align_val_t align, const std::nothrow_t&) noexcept
{
	ngine::Memory::DeallocateAligned(pPointer, static_cast<ngine::size>(align));
}

void __cdecl operator delete[](void* pPointer, std::align_val_t align, const std::nothrow_t&) noexcept
{
	ngine::Memory::DeallocateAligned(pPointer, static_cast<ngine::size>(align));
}

#if PLATFORM_EMSCRIPTEN && USE_MIMALLOC
extern "C"
{
#if !__has_feature(address_sanitizer) && !defined(__SANITIZE_ADDRESS__)
	void* malloc(size_t size)
	{
		return ngine::Memory::Allocate(size);
	}
	void* memalign(size_t alignment, size_t size)
	{
		return ngine::Memory::AllocateAligned(size, alignment);
	}
	void* realloc(void* pPointer, size_t size)
	{
		if (pPointer != nullptr)
		{
			return ngine::Memory::Reallocate(pPointer, size);
		}
		else
		{
			return ngine::Memory::Allocate(size);
		}
	}
	void* calloc(size_t count, size_t size)
	{
		void* pData = ngine::Memory::Allocate(count * size);
		ngine::Memory::Set(pData, 0, count * size);
		return pData;
	}
	void free(void* pPointer)
	{
		return ngine::Memory::Deallocate(pPointer);
	}
#endif
	void* __libc_malloc(size_t size)
	{
		return ngine::Memory::Allocate(size);
	}
	void* __libc_calloc(size_t count, size_t size)
	{
		void* pData = ngine::Memory::Allocate(count * size);
		ngine::Memory::Set(pData, 0, count * size);
		return pData;
	}
	void __libc_free(void* pPointer)
	{
		return ngine::Memory::Deallocate(pPointer);
	}

	void* emscripten_builtin_malloc(size_t size)
	{
		return ngine::Memory::Allocate(size);
	}
	void* emscripten_builtin_realloc(void* p, size_t size)
	{
		return ngine::Memory::Reallocate(p, size);
	}
	void* emscripten_builtin_memalign(size_t alignment, size_t size)
	{
		return ngine::Memory::AllocateAligned(size, alignment);
	}
	void emscripten_builtin_free(void* pPointer)
	{
		return ngine::Memory::Deallocate(pPointer);
	}
	void* emscripten_builtin_calloc(size_t count, size_t size)
	{
		void* pData = ngine::Memory::Allocate(count * size);
		ngine::Memory::Set(pData, 0, count * size);
		return pData;
	}

	int posix_memalign(void** pPointer, size_t alignment, size_t size)
	{
		void* pResult = ngine::Memory::AllocateAligned(size, alignment);
		*pPointer = pResult;
		return pResult != nullptr ? 0 : ENOMEM;
	}
}
#endif

#if PLATFORM_EMSCRIPTEN
PUSH_CLANG_WARNINGS
DISABLE_CLANG_WARNING("-Wc11-extensions")

#ifdef __EMSCRIPTEN_TRACING__
void emscripten_memprof_sbrk_grow(intptr_t old, intptr_t new);
#else
#define emscripten_memprof_sbrk_grow(...) ((void)0)
#endif

#include <emscripten/heap.h>

extern size_t __heap_base;

static uintptr_t sbrk_val = (uintptr_t)&__heap_base;

uintptr_t* emscripten_get_sbrk_ptr()
{
#ifdef __PIC__
	// In relocatable code we may call emscripten_get_sbrk_ptr() during startup,
	// potentially *before* the setup of the dynamically-linked __heap_base, when
	// using SAFE_HEAP. (SAFE_HEAP instruments *all* memory accesses, so even the
	// code doing dynamic linking itself ends up instrumented, which is why we can
	// get such an instrumented call before sbrk_val has its proper value.)
	if (sbrk_val == 0)
	{
		sbrk_val = (uintptr_t)&__heap_base;
	}
#endif
	return &sbrk_val;
}

// Enforce preserving a minimal alignof(maxalign_t) alignment for sbrk.
#define SBRK_ALIGNMENT (__alignof__(max_align_t))

#ifdef __EMSCRIPTEN_SHARED_MEMORY__
#define READ_SBRK_PTR(sbrk_ptr) (__c11_atomic_load((_Atomic(uintptr_t)*)(sbrk_ptr), __ATOMIC_SEQ_CST))
#else
#define READ_SBRK_PTR(sbrk_ptr) (*(sbrk_ptr))
#endif

void* sbrk(intptr_t increment_)
{
	uintptr_t increment = (uintptr_t)increment_;
	increment = (increment + (SBRK_ALIGNMENT - 1)) & ~(SBRK_ALIGNMENT - 1);
	uintptr_t* sbrk_ptr = (uintptr_t*)emscripten_get_sbrk_ptr();

	// To make sbrk thread-safe, implement a CAS loop to update the
	// value of sbrk_ptr.
	while (1)
	{
		uintptr_t old_brk = READ_SBRK_PTR(sbrk_ptr);
		uintptr_t new_brk = old_brk + increment;
		// Check for a) an overflow, which would indicate that we are trying to
		// allocate over maximum addressable memory. and b) if necessary,
		// increase the WebAssembly Memory size, and abort if that fails.
		if ((increment > 0 && new_brk <= old_brk) || (new_brk > emscripten_get_heap_size() && !emscripten_resize_heap(new_brk)))
		{
			errno = ENOMEM;
			return (void*)-1;
		}
#ifdef __EMSCRIPTEN_SHARED_MEMORY__
		// Attempt to update the dynamic top to new value. Another thread may have
		// beat this one to the update, in which case we will need to start over
		// by iterating the loop body again.
		uintptr_t expected = old_brk;

		__c11_atomic_compare_exchange_strong((_Atomic(uintptr_t)*)sbrk_ptr, &expected, new_brk, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);

		if (expected != old_brk)
			continue; // CAS failed, another thread raced in between.
#else
		*sbrk_ptr = new_brk;
#endif

		emscripten_memprof_sbrk_grow(old_brk, new_brk);
		return (void*)old_brk;
	}
}

int brk([[maybe_unused]] void* ptr)
{
#ifdef __EMSCRIPTEN_SHARED_MEMORY__
	// FIXME
	printf("brk() is not theadsafe yet, https://github.com/emscripten-core/emscripten/issues/10006");
	abort();
#else
	uintptr_t last = (uintptr_t)sbrk(0);
	if (sbrk((uintptr_t)ptr - last) == (void*)-1)
	{
		return -1;
	}
	return 0;
#endif
}
POP_CLANG_WARNINGS
#endif
