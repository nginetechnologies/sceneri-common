#include <Common/Memory/New.h>

#include <Common/Tests/UnitTest.h>
#include "../Timeout.h"

#include <Common/Memory/Allocators/Pool.h>
#include <Common/Memory/Containers/Vector.h>

namespace ngine::Tests
{
	UNIT_TEST(Pool, Create)
	{
		Memory::FixedPool<192, 32> pool;
		EXPECT_EQ(pool.GetUnusedFragmentCount(), 1u);

		[[maybe_unused]] void* pData = pool.Allocate(192 - 16);
		EXPECT_EQ(pool.GetUnusedFragmentCount(), 0u);
	}

	UNIT_TEST(Pool, AllocateAll)
	{
		Memory::FixedPool<192, 32> pool;

		EXPECT_EQ(pool.GetUnusedFragmentCount(), 1u);

		Vector<void*> allocatedEntries;

		{
			TEST_TIMEOUT_BEGIN
			while (true)
			{
				void* pEntry = pool.Allocate(8);
				if (pEntry == nullptr)
				{
					break;
				}

				allocatedEntries.EmplaceBack(pEntry);
			}
			TEST_TIMEOUT_FAIL_END(1000)
		}

		{
			TEST_TIMEOUT_BEGIN
			pool.VerifyIntegrity();
			TEST_TIMEOUT_FAIL_END(1000)
		}

		EXPECT_EQ(pool.GetUnusedFragmentCount(), 0u);

		{
			TEST_TIMEOUT_BEGIN
			for (void* pEntry : allocatedEntries)
			{
				pool.Deallocate(pEntry);
			}
			TEST_TIMEOUT_FAIL_END(1000)
		}

		{
			TEST_TIMEOUT_BEGIN
			pool.VerifyIntegrity();
			TEST_TIMEOUT_FAIL_END(1000)
		}

		EXPECT_EQ(pool.GetUnusedFragmentCount(), 1u);
	}
}
