#include <Common/Memory/New.h>

#include <Common/Tests/UnitTest.h>

#include <Common/Memory/Containers/PriorityQueue.h>

namespace ngine::Tests
{
	enum Priority
	{
		Highest,
		MediumHigh,
		Medium,
		MediumLow,
		Lowest
	};

	UNIT_TEST(PriorityQueue, EmplaceSingle)
	{
		TPriorityQueue<int, 255, int> queue;
		queue.Emplace(Priority::Highest, 256);

		{
			int element;
			const bool success = queue.TryPop(element);
			EXPECT_TRUE(success);
			EXPECT_EQ(element, 256);
		}

		{
			int element;
			const bool success = queue.TryPop(element);
			EXPECT_FALSE(success);
		}
	}

	UNIT_TEST(PriorityQueue, EmplaceDifferentPriorities)
	{
		TPriorityQueue<int, 255, int> queue;
		queue.Emplace(Priority::Medium, 1024);
		queue.Emplace(Priority::Lowest, 256);
		queue.Emplace(Priority::Highest, 2048);

		{
			int element;
			const bool success = queue.TryPop(element);
			EXPECT_TRUE(success);
			EXPECT_EQ(element, 2048);
		}

		{
			int element;
			const bool success = queue.TryPop(element);
			EXPECT_TRUE(success);
			EXPECT_EQ(element, 1024);
		}

		{
			int element;
			const bool success = queue.TryPop(element);
			EXPECT_TRUE(success);
			EXPECT_EQ(element, 256);
		}

		{
			int element;
			const bool success = queue.TryPop(element);
			EXPECT_FALSE(success);
		}
	}

	UNIT_TEST(PriorityQueue, EmplaceDifferentPrioritiesMultiple)
	{
		TPriorityQueue<int, 255, int> queue;
		queue.Emplace(Priority::Lowest, 128);
		queue.Emplace(Priority::Medium, 1024);
		queue.Emplace(Priority::Highest, 4096);
		queue.Emplace(Priority::Lowest, 256);
		queue.Emplace(Priority::Highest, 2048);
		queue.Emplace(Priority::MediumHigh, 1337);

		{
			int element;
			const bool success = queue.TryPop(element);
			EXPECT_TRUE(success);
			EXPECT_TRUE(element == 2048 || element == 4096);
		}

		{
			int element;
			const bool success = queue.TryPop(element);
			EXPECT_TRUE(success);
			EXPECT_TRUE(element == 2048 || element == 4096);
		}

		{
			int element;
			const bool success = queue.TryPop(element);
			EXPECT_TRUE(success);
			EXPECT_TRUE(element == 1337);
		}

		{
			int element;
			const bool success = queue.TryPop(element);
			EXPECT_TRUE(success);
			EXPECT_EQ(element, 1024);
		}

		{
			int element;
			const bool success = queue.TryPop(element);
			EXPECT_TRUE(success);
			EXPECT_TRUE(element == 256 || element == 128);
		}

		{
			int element;
			const bool success = queue.TryPop(element);
			EXPECT_TRUE(success);
			EXPECT_TRUE(element == 256 || element == 128);
		}

		{
			int element;
			const bool success = queue.TryPop(element);
			EXPECT_FALSE(success);
		}
	}

	UNIT_TEST(PriorityQueue, EmplaceLargeAmount)
	{
		TPriorityQueue<uint8, 255, uint8> queue;
		for (uint8 i = 0; i < 255; ++i)
		{
			queue.Emplace(i, i);
		}

		uint8 numPops = 0u;
		uint8 element;
		while (queue.TryPop(element))
		{
			EXPECT_EQ(element, numPops);
			numPops++;
		}

		EXPECT_EQ(numPops, 255);
	}

	UNIT_TEST(PriorityQueue, EmplaceLargeAmountReverse)
	{
		TPriorityQueue<uint8, 255, uint8> queue;
		for (uint8 i = 0; i < 255; ++i)
		{
			queue.Emplace(254 - i, i);
		}

		uint8 numPops = 0u;
		uint8 element;
		while (queue.TryPop(element))
		{
			EXPECT_EQ(element, 254 - numPops);
			numPops++;
		}

		EXPECT_EQ(numPops, 255);
	}

	UNIT_TEST(PriorityQueue, ShiftToOriginEmpty)
	{
		TPriorityQueue<uint8, 255, uint8> queue;
		for (uint8 i = 0; i < 200; ++i)
		{
			queue.Emplace(i, i);
		}

		uint8 numPops = 0u;
		for (uint8 i = 0; i < 200; ++i)
		{
			uint8 element;
			const bool success = queue.TryPop(element);
			EXPECT_TRUE(success);

			EXPECT_EQ(element, i);
			EXPECT_EQ(numPops, i);
			numPops++;
		}

		EXPECT_EQ(numPops, 200);
		numPops = 0u;

		queue.ShiftToOrigin();

		// Push back a few more
		for (uint8 i = 100; i < 255; ++i)
		{
			queue.Emplace(i, i);
		}

		for (uint8 i = 100; i < 255; ++i)
		{
			uint8 element;
			const bool success = queue.TryPop(element);
			EXPECT_TRUE(success);

			EXPECT_EQ(element, i);
			EXPECT_EQ(numPops, i - 100);
			numPops++;
		}

		uint8 element;
		const bool success = queue.TryPop(element);
		EXPECT_FALSE(success);

		EXPECT_EQ(numPops, 155);
	}

	UNIT_TEST(PriorityQueue, ShiftToOriginWithContents)
	{
		TPriorityQueue<uint8, 255, uint8> queue;
		for (uint8 i = 0; i < 200; ++i)
		{
			queue.Emplace(i, i);
		}

		// Pop half
		for (uint8 i = 0; i < 100; ++i)
		{
			uint8 element;
			const bool success = queue.TryPop(element);
			EXPECT_TRUE(success);

			EXPECT_EQ(element, i);
		}

		queue.ShiftToOrigin();

		for (uint8 i = 100; i < 200; ++i)
		{
			uint8 element;
			const bool success = queue.TryPop(element);
			EXPECT_TRUE(success);

			EXPECT_EQ(element, i);
		}

		uint8 element;
		const bool success = queue.TryPop(element);
		EXPECT_FALSE(success);
	}

	UNIT_TEST(PriorityQueue, ShiftToOriginWithContentsReinsert)
	{
		TPriorityQueue<uint8, 255, uint8> queue;
		for (uint8 i = 0; i < 200; ++i)
		{
			queue.Emplace(i, i);
		}

		// Pop half
		for (uint8 i = 0; i < 100; ++i)
		{
			uint8 element;
			const bool success = queue.TryPop(element);
			EXPECT_TRUE(success);

			EXPECT_EQ(element, i);
		}

		queue.ShiftToOrigin();

		// Push back until we're full
		for (uint8 i = 0; i < 100; ++i)
		{
			queue.Emplace(i, i);
		}

		{
			uint8 numPops = 0u;
			for (uint8 i = 0; i < 200; ++i)
			{
				uint8 element;
				const bool success = queue.TryPop(element);
				EXPECT_TRUE(success);

				EXPECT_EQ(element, i);
				EXPECT_EQ(numPops, i);
				numPops++;
			}

			EXPECT_EQ(numPops, 200);
		}

		uint8 element;
		const bool success = queue.TryPop(element);
		EXPECT_FALSE(success);
	}
}
