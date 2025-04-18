#include <Common/Memory/New.h>

#include <Common/Tests/UnitTest.h>

#include <Common/Math/CoreNumericTypes.h>

#include <Common/Memory/Allocators/DynamicInlineStorageAllocator.h>
#include <Common/Memory/Allocators/DynamicAllocator.h>
#include <Common/Memory/Allocators/FixedAllocator.h>

namespace ngine::Memory
{
	// Explicit instantiations to make sure the whole classes compile
	template struct FixedAllocator<int, 1>;
	template struct DynamicAllocator<int, uint32, uint32>;
	template struct DynamicInlineStorageAllocator<int, 1, uint32, uint32>;
}

namespace ngine::Tests
{
	// Dynamic allocator tests
	UNIT_TEST(FixedAllocator, DefaultConstruct)
	{
		Memory::FixedAllocator<int, 5> allocator;
		EXPECT_EQ(allocator.GetCapacity(), 5u);
		EXPECT_NE(allocator.GetData(), nullptr);
		EXPECT_EQ(allocator.GetView().GetSize(), 5u);
	}

	UNIT_TEST(FixedAllocator, ReserveConstruct)
	{
		Memory::FixedAllocator<int, 5> allocator(Memory::Reserve, 4u);
		EXPECT_EQ(allocator.GetCapacity(), 5u);
		EXPECT_NE(allocator.GetData(), nullptr);
		EXPECT_EQ(allocator.GetView().GetSize(), 5u);
	}

	UNIT_TEST(FixedAllocator, WriteFullMemory)
	{
		Memory::FixedAllocator<int, 5> allocator;
		for (int& value : allocator.GetView())
		{
			value = static_cast<int>(&value - allocator.GetData());
		}

		for (int i = 0; i < allocator.GetCapacity(); ++i)
		{
			EXPECT_EQ(allocator.GetData()[i], i);
		}
	}

	// Dynamic allocator tests
	UNIT_TEST(DynamicAllocator, DefaultConstruct)
	{
		Memory::DynamicAllocator<int, uint32> allocator;
		EXPECT_EQ(allocator.GetCapacity(), 0u);
		EXPECT_EQ(allocator.GetData(), nullptr);
		EXPECT_EQ(allocator.GetView(), decltype(allocator)::ConstView());
	}

	UNIT_TEST(DynamicAllocator, ReserveConstruct)
	{
		Memory::DynamicAllocator<int, uint32> allocator(Memory::Reserve, 10u);
		EXPECT_EQ(allocator.GetCapacity(), 10u);
		EXPECT_NE(allocator.GetData(), nullptr);
		EXPECT_EQ(allocator.GetView().GetSize(), 10u);
	}

	UNIT_TEST(DynamicAllocator, MoveConstruct)
	{
		Memory::DynamicAllocator<int, uint32> allocator(Memory::Reserve, 13u);
		Memory::DynamicAllocator<int, uint32> newAllocator(Move(allocator));
		EXPECT_EQ(allocator.GetCapacity(), 0u);
		EXPECT_EQ(allocator.GetData(), nullptr);
		EXPECT_EQ(allocator.GetView(), decltype(allocator)::ConstView());
		EXPECT_EQ(newAllocator.GetCapacity(), 13u);
		EXPECT_NE(newAllocator.GetData(), nullptr);
		EXPECT_EQ(newAllocator.GetView().GetSize(), 13u);
	}

	UNIT_TEST(DynamicAllocator, MoveAssign)
	{
		Memory::DynamicAllocator<int, uint32> allocator(Memory::Reserve, 13u);
		Memory::DynamicAllocator<int, uint32> newAllocator = Move(allocator);
		EXPECT_EQ(allocator.GetCapacity(), 0u);
		EXPECT_EQ(allocator.GetData(), nullptr);
		EXPECT_EQ(allocator.GetView(), decltype(allocator)::ConstView());
		EXPECT_EQ(newAllocator.GetCapacity(), 13u);
		EXPECT_NE(newAllocator.GetData(), nullptr);
		EXPECT_EQ(newAllocator.GetView().GetSize(), 13u);
	}

	UNIT_TEST(DynamicAllocator, Allocate)
	{
		Memory::DynamicAllocator<int, uint32> allocator;
		EXPECT_EQ(allocator.GetCapacity(), 0u);
		EXPECT_EQ(allocator.GetData(), nullptr);
		allocator.Allocate(12u);
		EXPECT_EQ(allocator.GetCapacity(), 12u);
		EXPECT_NE(allocator.GetData(), nullptr);
		allocator.Allocate(16u);
		EXPECT_EQ(allocator.GetCapacity(), 16u);
		EXPECT_NE(allocator.GetData(), nullptr);
		allocator.Allocate(1u);
		EXPECT_EQ(allocator.GetCapacity(), 1u);
		EXPECT_NE(allocator.GetData(), nullptr);
		allocator.Free();
		EXPECT_EQ(allocator.GetCapacity(), 0u);
		EXPECT_EQ(allocator.GetData(), nullptr);
	}

	UNIT_TEST(DynamicAllocator, WriteFullMemory)
	{
		Memory::DynamicAllocator<int, uint32> allocator;
		allocator.Allocate(10);
		EXPECT_EQ(allocator.GetCapacity(), 10u);

		for (int& value : allocator.GetView())
		{
			value = static_cast<int>(&value - allocator.GetData());
		}

		for (int i = 0; i < 10; ++i)
		{
			EXPECT_EQ(allocator.GetData()[i], i);
		}
	}

	// Dynamic inline storage allocator tests
	UNIT_TEST(DynamicInlineStorageAllocator, DefaultConstruct)
	{
		Memory::DynamicInlineStorageAllocator<int, 10, uint32> allocator;
		EXPECT_EQ(allocator.GetCapacity(), 0u);
		EXPECT_EQ(allocator.GetData(), nullptr);
		EXPECT_EQ(allocator.GetView(), decltype(allocator)::ConstView());
		EXPECT_TRUE(allocator.IsInlineStored());
	}

	UNIT_TEST(DynamicInlineStorageAllocator, ReserveConstructInline)
	{
		Memory::DynamicInlineStorageAllocator<int, 10, uint32> allocator(Memory::Reserve, 10u);
		EXPECT_EQ(allocator.GetCapacity(), 10u);
		EXPECT_NE(allocator.GetData(), nullptr);
		EXPECT_EQ(allocator.GetView().GetSize(), 10u);
		EXPECT_TRUE(allocator.IsInlineStored());
	}

	UNIT_TEST(DynamicInlineStorageAllocator, ReserveConstructDynamic)
	{
		Memory::DynamicInlineStorageAllocator<int, 10, uint32> allocator(Memory::Reserve, 15u);
		EXPECT_EQ(allocator.GetCapacity(), 15u);
		EXPECT_NE(allocator.GetData(), nullptr);
		EXPECT_EQ(allocator.GetView().GetSize(), 15u);
		EXPECT_TRUE(allocator.IsDynamicallyStored());
	}

	UNIT_TEST(DynamicInlineStorageAllocator, MoveConstructInline)
	{
		Memory::DynamicInlineStorageAllocator<int, 10, uint32> allocator(Memory::Reserve, 9u);
		Memory::DynamicInlineStorageAllocator<int, 10, uint32> newAllocator(Move(allocator));
		EXPECT_EQ(allocator.GetCapacity(), 0u);
		EXPECT_EQ(allocator.GetData(), nullptr);
		EXPECT_EQ(allocator.GetView(), decltype(allocator)::ConstView());
		EXPECT_TRUE(allocator.IsInlineStored());
		EXPECT_EQ(newAllocator.GetCapacity(), 9u);
		EXPECT_NE(newAllocator.GetData(), nullptr);
		EXPECT_EQ(newAllocator.GetView().GetSize(), 9u);
		EXPECT_TRUE(newAllocator.IsInlineStored());
	}

	UNIT_TEST(DynamicInlineStorageAllocator, MoveConstructDynamic)
	{
		Memory::DynamicInlineStorageAllocator<int, 10, uint32> allocator(Memory::Reserve, 13u);
		Memory::DynamicInlineStorageAllocator<int, 10, uint32> newAllocator(Move(allocator));
		EXPECT_EQ(allocator.GetCapacity(), 0u);
		EXPECT_EQ(allocator.GetData(), nullptr);
		EXPECT_EQ(allocator.GetView(), decltype(allocator)::ConstView());
		EXPECT_TRUE(allocator.IsInlineStored());
		EXPECT_EQ(newAllocator.GetCapacity(), 13u);
		EXPECT_NE(newAllocator.GetData(), nullptr);
		EXPECT_EQ(newAllocator.GetView().GetSize(), 13u);
		EXPECT_TRUE(newAllocator.IsDynamicallyStored());
	}

	UNIT_TEST(DynamicInlineStorageAllocator, MoveAssignInline)
	{
		Memory::DynamicInlineStorageAllocator<int, 10, uint32> allocator(Memory::Reserve, 8u);
		Memory::DynamicInlineStorageAllocator<int, 10, uint32> newAllocator = Move(allocator);
		EXPECT_EQ(allocator.GetCapacity(), 0u);
		EXPECT_EQ(allocator.GetData(), nullptr);
		EXPECT_EQ(allocator.GetView(), decltype(allocator)::ConstView());
		EXPECT_TRUE(allocator.IsInlineStored());
		EXPECT_EQ(newAllocator.GetCapacity(), 8u);
		EXPECT_NE(newAllocator.GetData(), nullptr);
		EXPECT_EQ(newAllocator.GetView().GetSize(), 8u);
		EXPECT_TRUE(newAllocator.IsInlineStored());
	}

	UNIT_TEST(DynamicInlineStorageAllocator, MoveAssignDynamic)
	{
		Memory::DynamicInlineStorageAllocator<int, 10, uint32> allocator(Memory::Reserve, 13u);
		Memory::DynamicInlineStorageAllocator<int, 10, uint32> newAllocator = Move(allocator);
		EXPECT_EQ(allocator.GetCapacity(), 0u);
		EXPECT_EQ(allocator.GetData(), nullptr);
		EXPECT_EQ(allocator.GetView(), decltype(allocator)::ConstView());
		EXPECT_TRUE(allocator.IsInlineStored());
		EXPECT_EQ(newAllocator.GetCapacity(), 13u);
		EXPECT_NE(newAllocator.GetData(), nullptr);
		EXPECT_EQ(newAllocator.GetView().GetSize(), 13u);
		EXPECT_TRUE(newAllocator.IsDynamicallyStored());
	}

	UNIT_TEST(DynamicInlineStorageAllocator, Allocate)
	{
		Memory::DynamicInlineStorageAllocator<int, 10, uint32> allocator;
		EXPECT_TRUE(allocator.IsInlineStored());
		EXPECT_EQ(allocator.GetCapacity(), 0u);
		EXPECT_EQ(allocator.GetData(), nullptr);
		allocator.Allocate(5u);
		EXPECT_TRUE(allocator.IsInlineStored());
		EXPECT_EQ(allocator.GetCapacity(), 5u);
		EXPECT_NE(allocator.GetData(), nullptr);
		allocator.Allocate(10u);
		EXPECT_TRUE(allocator.IsInlineStored());
		EXPECT_EQ(allocator.GetCapacity(), 10u);
		EXPECT_NE(allocator.GetData(), nullptr);
		allocator.Allocate(1u);
		EXPECT_TRUE(allocator.IsInlineStored());
		EXPECT_EQ(allocator.GetCapacity(), 1u);
		EXPECT_NE(allocator.GetData(), nullptr);
		allocator.Allocate(11u);
		EXPECT_TRUE(allocator.IsDynamicallyStored());
		EXPECT_EQ(allocator.GetCapacity(), 11u);
		EXPECT_NE(allocator.GetData(), nullptr);
		allocator.Allocate(9u);
		EXPECT_TRUE(allocator.IsInlineStored());
		EXPECT_EQ(allocator.GetCapacity(), 9u);
		EXPECT_NE(allocator.GetData(), nullptr);
		allocator.Free();
		EXPECT_EQ(allocator.GetCapacity(), 0u);
		EXPECT_EQ(allocator.GetData(), nullptr);
		EXPECT_TRUE(allocator.IsInlineStored());
	}

	UNIT_TEST(DynamicInlineStorageAllocator, WriteFullMemory)
	{
		Memory::DynamicInlineStorageAllocator<int, 10, uint32> allocator;
		allocator.Allocate(10);
		EXPECT_EQ(allocator.GetCapacity(), 10u);

		for (int& value : allocator.GetView())
		{
			value = static_cast<int>(&value - allocator.GetData());
		}

		for (int i = 0; i < 10; ++i)
		{
			EXPECT_EQ(allocator.GetData()[i], i);
		}
	}
}
