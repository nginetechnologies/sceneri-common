#pragma once

#include <Common/Math/CoreNumericTypes.h>
#include <Common/Platform/Pure.h>

namespace ngine::Memory
{
	inline static constexpr size MaximumSmallAllocationSize = 128 * sizeof(void*);

	[[nodiscard]] RESTRICTED_RETURN extern void* Allocate(const size size) noexcept;
	[[nodiscard]] RESTRICTED_RETURN extern void* AllocateSmall(const size size) noexcept;
	[[nodiscard]] RESTRICTED_RETURN extern void* Reallocate(void* pPointer, const size size) noexcept;
	extern void Deallocate(void* pPointer) noexcept;
	[[nodiscard]] RESTRICTED_RETURN extern void* AllocateAligned(const size requestedSize, const size alignment) noexcept;
	[[nodiscard]] RESTRICTED_RETURN extern void* ReallocateAligned(void* pPointer, const size requestedSize, const size alignment) noexcept;
	extern void DeallocateAligned(void* pPointer, const size alignment) noexcept;
	[[nodiscard]] PURE_STATICS size GetAllocatedMemorySize(void* pPointer) noexcept;
	[[nodiscard]] PURE_STATICS size GetAllocatedAlignedMemorySize(void* pPointer, const size alignment) noexcept;
	[[nodiscard]] PURE_STATICS size GetDynamicMemoryUsage() noexcept;
	[[nodiscard]] RESTRICTED_RETURN extern void* AllocateOnStack(const size size) noexcept;

	void ReportGraphicsAllocation(const size size) noexcept;
	void ReportGraphicsDeallocation(const size size) noexcept;
	[[nodiscard]] PURE_STATICS size GetGraphicsMemoryUsage() noexcept;
}
