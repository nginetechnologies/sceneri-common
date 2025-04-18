#include <Common/Memory/New.h>
#include <Common/Memory/Allocators/Allocate.h>

#include <Common/Tests/UnitTest.h>

namespace ngine::Tests
{
	UNIT_TEST(Memory, Allocate)
	{
		[[maybe_unused]] const size initialMemoryUsage = Memory::GetDynamicMemoryUsage();

		{
			void* pMemory = Memory::Allocate(48);
			ASSERT_GE(Memory::GetAllocatedMemorySize(pMemory), 48ull);
			if constexpr (PROFILE_BUILD)
			{
				ASSERT_EQ(Memory::GetDynamicMemoryUsage(), initialMemoryUsage + Memory::GetAllocatedMemorySize(pMemory));
			}

			Memory::Deallocate(pMemory);
			if constexpr (PROFILE_BUILD)
			{
				ASSERT_EQ(Memory::GetDynamicMemoryUsage(), initialMemoryUsage);
			}
		}
	}

	UNIT_TEST(Memory, Reallocate)
	{
		[[maybe_unused]] const size initialMemoryUsage = Memory::GetDynamicMemoryUsage();

		{
			void* pMemory = Memory::Allocate(48);
			ASSERT_GE(Memory::GetAllocatedMemorySize(pMemory), 48ull);
			if constexpr (PROFILE_BUILD)
			{
				ASSERT_EQ(Memory::GetDynamicMemoryUsage(), initialMemoryUsage + Memory::GetAllocatedMemorySize(pMemory));
			}

			pMemory = Memory::Reallocate(pMemory, 96);
			ASSERT_GE(Memory::GetAllocatedMemorySize(pMemory), 96ull);
			if constexpr (PROFILE_BUILD)
			{
				ASSERT_EQ(Memory::GetDynamicMemoryUsage(), initialMemoryUsage + Memory::GetAllocatedMemorySize(pMemory));
			}

			Memory::Deallocate(pMemory);
			if constexpr (PROFILE_BUILD)
			{
				ASSERT_EQ(Memory::GetDynamicMemoryUsage(), initialMemoryUsage);
			}
		}
	}

	UNIT_TEST(Memory, AllocateAligned)
	{
		[[maybe_unused]] const size initialMemoryUsage = Memory::GetDynamicMemoryUsage();

		{
			void* pMemory = Memory::AllocateAligned(48, 16);
			ASSERT_GE(Memory::GetAllocatedAlignedMemorySize(pMemory, 16), 48ull);
			if constexpr (PROFILE_BUILD)
			{
				ASSERT_EQ(Memory::GetDynamicMemoryUsage(), initialMemoryUsage + Memory::GetAllocatedAlignedMemorySize(pMemory, 16));
			}

			Memory::DeallocateAligned(pMemory, 16);
			if constexpr (PROFILE_BUILD)
			{
				ASSERT_EQ(Memory::GetDynamicMemoryUsage(), initialMemoryUsage);
			}
		}
	}

	UNIT_TEST(Memory, ReallocateAligned)
	{
		[[maybe_unused]] const size initialMemoryUsage = Memory::GetDynamicMemoryUsage();

		{
			void* pMemory = Memory::AllocateAligned(48, 16);
			ASSERT_GE(Memory::GetAllocatedAlignedMemorySize(pMemory, 16), 48ull);
			if constexpr (PROFILE_BUILD)
			{
				ASSERT_EQ(Memory::GetDynamicMemoryUsage(), initialMemoryUsage + Memory::GetAllocatedAlignedMemorySize(pMemory, 16));
			}

			pMemory = Memory::ReallocateAligned(pMemory, 96, 16);
			ASSERT_GE(Memory::GetAllocatedAlignedMemorySize(pMemory, 16), 96ull);
			if constexpr (PROFILE_BUILD)
			{
				ASSERT_EQ(Memory::GetDynamicMemoryUsage(), initialMemoryUsage + Memory::GetAllocatedAlignedMemorySize(pMemory, 16));
			}

			Memory::DeallocateAligned(pMemory, 16);
			if constexpr (PROFILE_BUILD)
			{
				ASSERT_EQ(Memory::GetDynamicMemoryUsage(), initialMemoryUsage);
			}
		}
	}
}
