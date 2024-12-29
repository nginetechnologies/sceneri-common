#include <Common/Memory/New.h>

#include <Common/Tests/UnitTest.h>

#include <Common/Memory/Containers/Vector.h>
#include <Common/Memory/Containers/FlatVector.h>
#include <Common/Memory/Containers/Array.h>
#include <Common/Math/CoreNumericTypes.h>

namespace ngine
{
	// Explicit instantiations to make sure the whole classes compile
	template struct Array<int, 1>;
	template struct Vector<int>;
	template struct FixedCapacityVector<int>;
	template struct FixedSizeVector<int>;
	template struct FlatVector<int, 1>;
}

namespace ngine::Tests
{
	UNIT_TEST(Vector, ConstructUninitialized)
	{
		Vector<int> vector;
		EXPECT_TRUE(vector.IsEmpty());
		EXPECT_EQ(vector.GetCapacity(), 0u);
	}

	UNIT_TEST(Vector, ConstructReserve)
	{
		Vector<int> vector(Memory::Reserve, 10u);
		EXPECT_TRUE(vector.IsEmpty());
		EXPECT_EQ(vector.GetCapacity(), 10u);
	}

	UNIT_TEST(Vector, ConstructWithSizeDefaultInitializedPrimitive)
	{
		Vector<int> vector(Memory::ConstructWithSize, Memory::DefaultConstruct, 10u);
		EXPECT_EQ(vector.GetSize(), 10u);
		EXPECT_EQ(vector.GetCapacity(), 10u);
		for (uint32 i = 0; i < 10; ++i)
		{
			EXPECT_EQ(vector[i], 0);
		}
	}

	UNIT_TEST(Vector, ConstructWithSizeDefaultInitializedComplex)
	{
		struct MyType
		{
			int m_value = 1337;
		};

		Vector<MyType> vector(Memory::ConstructWithSize, Memory::DefaultConstruct, 10u);
		EXPECT_EQ(vector.GetSize(), 10u);
		EXPECT_EQ(vector.GetCapacity(), 10u);
		for (uint32 i = 0; i < 10; ++i)
		{
			EXPECT_EQ(vector[i].m_value, 1337);
		}
	}

	UNIT_TEST(Vector, ConstructWithSizeUninitialized)
	{
		Vector<int> vector(Memory::ConstructWithSize, Memory::Uninitialized, 10u);
		EXPECT_EQ(vector.GetSize(), 10u);
		EXPECT_GE(vector.GetCapacity(), 10u);
	}

	UNIT_TEST(Vector, ConstructWithSizeZeroed)
	{
		Vector<int> vector(Memory::ConstructWithSize, Memory::Zeroed, 10u);
		EXPECT_EQ(vector.GetSize(), 10u);
		EXPECT_EQ(vector.GetCapacity(), 10u);
		for (uint32 i = 0; i < 10; ++i)
		{
			EXPECT_EQ(vector[i], 0);
		}
	}

	UNIT_TEST(Vector, ConstructImplicitMove)
	{
		struct BaseType
		{
			BaseType(int value)
				: m_value(value)
			{
			}

			int m_value;
		};

		struct MyType final : BaseType
		{
			MyType(BaseType&& baseType)
				: BaseType(Forward<BaseType>(baseType))
			{
			}
			MyType(const MyType&) = delete;
			MyType& operator=(const MyType&) = delete;
			MyType(MyType&& other)
				: BaseType(Forward<BaseType>(other))
			{
			}
			MyType& operator=(MyType&&) = default;
			MyType(int value) = delete;
		};

		Vector<MyType> vector = {BaseType(10), BaseType(11)};
		EXPECT_EQ(vector.GetSize(), 2u);
		EXPECT_EQ(vector[0].m_value, 10);
		EXPECT_EQ(vector[1].m_value, 11);
	}

	UNIT_TEST(Vector, ConstructImplicitCopy)
	{
		struct BaseType
		{
			BaseType(int value)
				: m_value(value)
			{
			}

			int m_value;
		};

		struct MyType final : BaseType
		{
			MyType(const BaseType& base)
				: BaseType(base)
			{
			}
			MyType(const MyType& other)
				: BaseType(other)
			{
			}
			MyType& operator=(const MyType& other) = default;
			MyType(MyType&& other) = delete;
			MyType& operator=(MyType&&) = delete;
			MyType(int value) = delete;
		};

		const BaseType a(10);
		const BaseType b(11);

		Vector<MyType> vector = {a, b};
		EXPECT_EQ(vector.GetSize(), 2u);
		EXPECT_EQ(vector[0].m_value, 10);
		EXPECT_EQ(vector[1].m_value, 11);
	}

	UNIT_TEST(Vector, ConstructFromView)
	{
		const Array<int, 3> elements = {1, 2, 3};
		Vector<int> vector(elements.GetDynamicView());

		EXPECT_EQ(vector.GetSize(), 3u);
		EXPECT_EQ(vector[0], 1);
		EXPECT_EQ(vector[1], 2);
		EXPECT_EQ(vector[2], 3);
	}

	UNIT_TEST(Vector, MoveConstruct)
	{
		struct MyType
		{
			MyType(int element)
				: m_element(element)
			{
			}
			MyType(const MyType&) = delete;
			MyType& operator=(const MyType&) = delete;
			MyType(MyType&& other)
				: m_element(other.m_element)
			{
				other.m_element = 0;
			}
			MyType& operator=(MyType&& other)
			{
				m_element = other.m_element;
				other.m_element = 0;
				return *this;
			}

			operator int() const
			{
				return m_element;
			}

			int m_element;
		};

		Vector<MyType> vector = {1, 2, 3, 4, 5};
		EXPECT_EQ(vector.GetSize(), 5u);

		const Vector<MyType> otherVector(Move(vector));

		EXPECT_TRUE(vector.IsEmpty());
		EXPECT_TRUE(vector.GetCapacity() == 0);

		EXPECT_EQ(otherVector.GetSize(), 5u);
		EXPECT_EQ(otherVector[0], 1);
		EXPECT_EQ(otherVector[1], 2);
		EXPECT_EQ(otherVector[2], 3);
		EXPECT_EQ(otherVector[3], 4);
		EXPECT_EQ(otherVector[4], 5);
	}

	UNIT_TEST(Vector, MoveAssignment)
	{
		struct MyType
		{
			MyType(int element)
				: m_element(element)
			{
			}
			MyType(const MyType&) = delete;
			MyType& operator=(const MyType&) = delete;
			MyType(MyType&& other)
				: m_element(other.m_element)
			{
				other.m_element = 0;
			}
			MyType& operator=(MyType&& other)
			{
				m_element = other.m_element;
				other.m_element = 0;
				return *this;
			}

			operator int() const
			{
				return m_element;
			}

			int m_element;
		};

		Vector<MyType> vector = {1, 2, 3, 4, 5};
		EXPECT_EQ(vector.GetSize(), 5u);

		const Vector<MyType> otherVector = Move(vector);

		EXPECT_TRUE(vector.IsEmpty());
		EXPECT_TRUE(vector.GetCapacity() == 0);

		EXPECT_EQ(otherVector.GetSize(), 5u);
		EXPECT_EQ(otherVector[0], 1);
		EXPECT_EQ(otherVector[1], 2);
		EXPECT_EQ(otherVector[2], 3);
		EXPECT_EQ(otherVector[3], 4);
		EXPECT_EQ(otherVector[4], 5);
	}

	UNIT_TEST(Vector, Destructor)
	{
		static int numElements = 0;
		numElements = 0;

		struct Type
		{
			Type()
			{
				++numElements;
			}
			~Type()
			{
				--numElements;
			}
		};

		EXPECT_TRUE(numElements == 0);

		{
			Vector<Type> vector;
			EXPECT_EQ(numElements, 0);

			vector.Reserve(15);
			EXPECT_EQ(numElements, 0);

			vector.Resize(10);
			EXPECT_EQ(numElements, 10);

			vector.Resize(15);
			EXPECT_EQ(numElements, 15);
		}

		EXPECT_EQ(numElements, 0);
	}

	UNIT_TEST(Vector, BasicGetters)
	{
		Vector<int> vector;
		EXPECT_FALSE(vector.HasElements());
		EXPECT_TRUE(vector.IsEmpty());
		EXPECT_EQ(vector.GetSize(), 0u);
		EXPECT_EQ(vector.GetCapacity(), 0u);
		EXPECT_EQ(vector.GetNextAvailableIndex(), 0u);
		EXPECT_EQ(vector.GetDataSize(), 0u);
		EXPECT_TRUE(vector.ReachedCapacity());
		EXPECT_EQ(vector.GetData(), nullptr);
		EXPECT_EQ(vector.begin(), nullptr);
		EXPECT_EQ(vector.end(), nullptr);

		vector.EmplaceBack(1);

		EXPECT_TRUE(vector.HasElements());
		EXPECT_FALSE(vector.IsEmpty());
		EXPECT_EQ(vector.GetSize(), 1u);
		EXPECT_GE(vector.GetCapacity(), 1u);
		EXPECT_EQ(vector.GetNextAvailableIndex(), 1u);
		EXPECT_EQ(vector.GetDataSize(), sizeof(int) * 1u);
		EXPECT_NE(vector.GetData(), nullptr);
		EXPECT_NE(vector.begin(), nullptr);
		EXPECT_EQ(vector.end(), vector.begin() + 1u);
		EXPECT_EQ(vector[0], 1);
		EXPECT_EQ(*vector.GetData(), 1);
		EXPECT_EQ(vector.GetIteratorIndex(vector.begin()), 0u);

		EXPECT_TRUE(vector.IsWithinBounds(vector.begin()));
		EXPECT_FALSE(vector.IsWithinBounds(vector.end()));
		EXPECT_FALSE(vector.IsWithinBounds(vector.begin() - 1u));

		EXPECT_EQ(vector.GetLastElement(), 1);
	}

	UNIT_TEST(Vector, GetView)
	{
		Vector<int> vector = {1, 2, 3, 4, 5};
		{
			const ArrayView<const int> view = vector;
			EXPECT_EQ(view.GetSize(), 5u);
			EXPECT_EQ(view[0], vector[0]);
			EXPECT_EQ(view[1], vector[1]);
			EXPECT_EQ(view[2], vector[2]);
			EXPECT_EQ(view[3], vector[3]);
			EXPECT_EQ(view[4], vector[4]);
		}

		{
			const ArrayView<const int> view = vector.GetSubView(1, 2);
			EXPECT_EQ(view.GetSize(), 2u);
			EXPECT_EQ(view[0], vector[1]);
			EXPECT_EQ(view[1], vector[2]);
		}
	}

	UNIT_TEST(Vector, Emplace)
	{
		struct MyType
		{
			MyType()
				: m_value(0)
			{
				// Default constructor should never be reached
				EXPECT_TRUE(false);
			}
			MyType(int value)
				: m_value(value)
			{
			}
			MyType(const MyType&)
			{
				// Copy constructor should never be reached
				EXPECT_TRUE(false);
			}
			MyType& operator=(const MyType&)
			{
				// Copy assignment operator should never be reached
				EXPECT_TRUE(false);
				return *this;
			}
			MyType(MyType&& other) = default;
			MyType& operator=(MyType&&) = default;
			~MyType()
			{
			}

			int m_value;
		};

		Vector<MyType> vector;
		{
			const MyType& insertedElement = vector.EmplaceBack(MyType(150));
			EXPECT_EQ(vector.GetSize(), 1u);
			EXPECT_GE(vector.GetCapacity(), 1u);
			EXPECT_EQ(insertedElement.m_value, 150);
			EXPECT_EQ(&insertedElement, &vector[0]);
		}

		{
			const MyType& insertedElement2 = vector.EmplaceBack(MyType(237));
			EXPECT_EQ(vector.GetSize(), 2u);
			EXPECT_GE(vector.GetCapacity(), 2u);
			EXPECT_EQ(insertedElement2.m_value, 237);
			EXPECT_EQ(&insertedElement2, &vector[1]);
			EXPECT_EQ(vector[0].m_value, 150);
		}

		{
			vector.Emplace(vector.begin() + 1, Memory::Uninitialized, MyType(121));
			EXPECT_EQ(vector.GetSize(), 3u);
			EXPECT_GE(vector.GetCapacity(), 3u);
			EXPECT_EQ(vector[0].m_value, 150);
			EXPECT_EQ(vector[1].m_value, 121);
			EXPECT_EQ(vector[2].m_value, 237);
		}

		{
			vector.Emplace(vector.begin() + 4, Memory::Uninitialized, MyType(127));
			EXPECT_EQ(vector.GetSize(), 5u);
			EXPECT_EQ(vector[0].m_value, 150);
			EXPECT_EQ(vector[1].m_value, 121);
			EXPECT_EQ(vector[2].m_value, 237);
			EXPECT_EQ(vector[4].m_value, 127);
		}

		{
			vector.MoveEmplaceRange(vector.begin() + 7, Memory::Uninitialized, Array<MyType, 2>{MyType(129), MyType(256)}.GetDynamicView());
			EXPECT_EQ(vector.GetSize(), 9u);
			EXPECT_EQ(vector[0].m_value, 150);
			EXPECT_EQ(vector[1].m_value, 121);
			EXPECT_EQ(vector[2].m_value, 237);
			EXPECT_EQ(vector[4].m_value, 127);
			EXPECT_EQ(vector[7].m_value, 129);
			EXPECT_EQ(vector[8].m_value, 256);
		}
	}

	UNIT_TEST(Vector, PopBack)
	{
		Vector<int> vector = {1, 2, 3, 4, 5};
		EXPECT_EQ(vector.GetSize(), 5u);
		EXPECT_EQ(vector.GetLastElement(), 5);
		vector.PopBack();
		EXPECT_EQ(vector.GetSize(), 4u);
		EXPECT_EQ(vector.GetLastElement(), 4);
		vector.PopBack();
		EXPECT_EQ(vector.GetSize(), 3u);
		EXPECT_EQ(vector.GetLastElement(), 3);
	}

	UNIT_TEST(Vector, MoveFrom)
	{
		Vector<int> vector = {1, 2, 3};
		Vector<int> otherVector = {4, 5};

		vector.MoveFrom(vector.end(), otherVector);
		EXPECT_EQ(vector.GetSize(), 5u);
		EXPECT_TRUE(otherVector.IsEmpty());
		EXPECT_EQ(vector[0], 1);
		EXPECT_EQ(vector[1], 2);
		EXPECT_EQ(vector[2], 3);
		EXPECT_EQ(vector[3], 4);
		EXPECT_EQ(vector[4], 5);
	}

	UNIT_TEST(Vector, CopyFrom)
	{
		Vector<int> vector = {1, 2, 3};
		const Array<int, 2> other = {4, 5};

		vector.CopyFrom(vector.end(), other.GetDynamicView());
		EXPECT_EQ(vector.GetSize(), 5u);
		EXPECT_EQ(vector[0], 1);
		EXPECT_EQ(vector[1], 2);
		EXPECT_EQ(vector[2], 3);
		EXPECT_EQ(vector[3], 4);
		EXPECT_EQ(vector[4], 5);
	}

	UNIT_TEST(Vector, CopyAssign)
	{
		Vector<int> vector = {50, 37, 60};
		const Array<int, 5> other = {1, 2, 3, 4, 5};

		vector = other.GetDynamicView();
		EXPECT_EQ(vector.GetSize(), 5u);
		EXPECT_EQ(vector[0], 1);
		EXPECT_EQ(vector[1], 2);
		EXPECT_EQ(vector[2], 3);
		EXPECT_EQ(vector[3], 4);
		EXPECT_EQ(vector[4], 5);
	}

	UNIT_TEST(Vector, Clear)
	{
		static int numElements = 0;
		numElements = 0;

		struct Type
		{
			Type()
			{
				++numElements;
			}
			~Type()
			{
				--numElements;
			}
		};

		EXPECT_EQ(numElements, 0);
		Vector<Type> vector;
		vector.Resize(15);
		EXPECT_EQ(numElements, 15);

		vector.Clear();
		EXPECT_EQ(numElements, 0);
		EXPECT_TRUE(vector.IsEmpty());
		EXPECT_GE(vector.GetCapacity(), 15u);
	}

	UNIT_TEST(Vector, Remove)
	{
		Vector<int> vector = {0, 1, 2, 3, 4, 5};
		EXPECT_EQ(vector.GetSize(), 6u);
		EXPECT_EQ(vector.GetCapacity(), 6u);

		vector.Remove(vector.begin() + 4);
		EXPECT_EQ(vector.GetSize(), 5u);
		EXPECT_EQ(vector.GetCapacity(), 6u);
		EXPECT_EQ(vector[0], 0);
		EXPECT_EQ(vector[1], 1);
		EXPECT_EQ(vector[2], 2);
		EXPECT_EQ(vector[3], 3);
		EXPECT_EQ(vector[4], 5);
	}

	UNIT_TEST(Vector, RemoveView)
	{
		Vector<int> vector = {1, 2, 3, 4, 5};
		vector.Remove(vector.GetSubView(1, 2));
		EXPECT_EQ(vector.GetSize(), 3u);
		EXPECT_EQ(vector[0], 1);
		EXPECT_EQ(vector[1], 4);
		EXPECT_EQ(vector[2], 5);
	}

	UNIT_TEST(Vector, RemoveAllOccurrencesPredicate)
	{
		struct Entry
		{
			int m_value;
			int m_originalIndex;
		};

		Vector<Entry> vector = {
			Entry{1, 0},
			Entry{2, 1},
			Entry{2, 2},
			Entry{3, 3},
			Entry{3, 4},
			Entry{4, 5},
			Entry{4, 6},
		};
		vector.RemoveAllOccurrencesPredicate(
			[](const Entry value) -> ErasePredicateResult
			{
				return value.m_value == 3 ? ErasePredicateResult::Remove : ErasePredicateResult::Continue;
			}
		);
		EXPECT_EQ(vector.GetSize(), 5u);
		EXPECT_EQ(vector[0].m_value, 1);
		EXPECT_EQ(vector[0].m_originalIndex, 0);
		EXPECT_EQ(vector[1].m_value, 2);
		EXPECT_EQ(vector[1].m_originalIndex, 1);
		EXPECT_EQ(vector[2].m_value, 2);
		EXPECT_EQ(vector[2].m_originalIndex, 2);
		EXPECT_EQ(vector[3].m_value, 4);
		EXPECT_EQ(vector[3].m_originalIndex, 5);
		EXPECT_EQ(vector[4].m_value, 4);
		EXPECT_EQ(vector[4].m_originalIndex, 6);
	}

	UNIT_TEST(Vector, RemoveAllOccurrences)
	{
		struct Entry
		{
			int m_value;
			int m_originalIndex;

			bool operator==(const Entry other) const
			{
				return m_value == other.m_value;
			}
		};

		Vector<Entry> vector = {
			Entry{1, 0},
			Entry{2, 1},
			Entry{2, 2},
			Entry{3, 3},
			Entry{3, 4},
			Entry{4, 5},
			Entry{4, 6},
		};

		vector.RemoveAllOccurrences(Entry{3, 0});
		EXPECT_EQ(vector.GetSize(), 5u);
		EXPECT_EQ(vector[0].m_value, 1);
		EXPECT_EQ(vector[0].m_originalIndex, 0);
		EXPECT_EQ(vector[1].m_value, 2);
		EXPECT_EQ(vector[1].m_originalIndex, 1);
		EXPECT_EQ(vector[2].m_value, 2);
		EXPECT_EQ(vector[2].m_originalIndex, 2);
		EXPECT_EQ(vector[3].m_value, 4);
		EXPECT_EQ(vector[3].m_originalIndex, 5);
		EXPECT_EQ(vector[4].m_value, 4);
		EXPECT_EQ(vector[4].m_originalIndex, 6);
	}

	UNIT_TEST(Vector, RemoveFirstOccurrencePredicate)
	{
		struct Entry
		{
			int m_value;
			int m_originalIndex;
		};

		Vector<Entry> vector = {
			Entry{1, 0},
			Entry{2, 1},
			Entry{2, 2},
			Entry{3, 3},
			Entry{3, 4},
			Entry{4, 5},
			Entry{4, 6},
		};
		vector.RemoveFirstOccurrencePredicate(
			[](const Entry value) -> ErasePredicateResult
			{
				return value.m_value == 3 ? ErasePredicateResult::Remove : ErasePredicateResult::Continue;
			}
		);
		EXPECT_EQ(vector.GetSize(), 6u);
		EXPECT_EQ(vector[0].m_value, 1);
		EXPECT_EQ(vector[0].m_originalIndex, 0);
		EXPECT_EQ(vector[1].m_value, 2);
		EXPECT_EQ(vector[1].m_originalIndex, 1);
		EXPECT_EQ(vector[2].m_value, 2);
		EXPECT_EQ(vector[2].m_originalIndex, 2);
		EXPECT_EQ(vector[3].m_value, 3);
		EXPECT_EQ(vector[3].m_originalIndex, 4);
		EXPECT_EQ(vector[4].m_value, 4);
		EXPECT_EQ(vector[4].m_originalIndex, 5);
		EXPECT_EQ(vector[5].m_value, 4);
		EXPECT_EQ(vector[5].m_originalIndex, 6);
	}

	UNIT_TEST(Vector, RemoveFirstOccurrence)
	{
		struct Entry
		{
			int m_value;
			int m_originalIndex;

			bool operator==(const Entry other) const
			{
				return m_value == other.m_value;
			}
		};

		Vector<Entry> vector = {
			Entry{1, 0},
			Entry{2, 1},
			Entry{2, 2},
			Entry{3, 3},
			Entry{3, 4},
			Entry{4, 5},
			Entry{4, 6},
		};

		vector.RemoveFirstOccurrence(Entry{3, 0});
		EXPECT_EQ(vector.GetSize(), 6u);
		EXPECT_EQ(vector[0].m_value, 1);
		EXPECT_EQ(vector[0].m_originalIndex, 0);
		EXPECT_EQ(vector[1].m_value, 2);
		EXPECT_EQ(vector[1].m_originalIndex, 1);
		EXPECT_EQ(vector[2].m_value, 2);
		EXPECT_EQ(vector[2].m_originalIndex, 2);
		EXPECT_EQ(vector[3].m_value, 3);
		EXPECT_EQ(vector[3].m_originalIndex, 4);
		EXPECT_EQ(vector[4].m_value, 4);
		EXPECT_EQ(vector[4].m_originalIndex, 5);
		EXPECT_EQ(vector[5].m_value, 4);
		EXPECT_EQ(vector[5].m_originalIndex, 6);
	}

	UNIT_TEST(Vector, Reserve)
	{
		Vector<int> vector;
		vector.Reserve(5);
		EXPECT_GE(vector.GetCapacity(), 5u);
		EXPECT_EQ(vector.GetSize(), 0u);
	}

	UNIT_TEST(Vector, ReserveAndResize)
	{
		Vector<int> vector;

		vector.Reserve(12);
		EXPECT_TRUE(vector.IsEmpty());
		EXPECT_GE(vector.GetCapacity(), 12u);

		vector.Resize(11);
		EXPECT_FALSE(vector.IsEmpty());
		EXPECT_GE(vector.GetCapacity(), 12u);
		EXPECT_EQ(vector.GetSize(), 11u);
	}

	UNIT_TEST(Vector, Resize)
	{
		Vector<int> vector;
		vector.Resize(12u);
		EXPECT_EQ(vector.GetSize(), 12u);
		EXPECT_GE(vector.GetCapacity(), 12u);
	}

	UNIT_TEST(Vector, Grow)
	{
		Vector<int> vector = {1};
		vector.Reserve(5u);
		vector.Grow(5u);
		EXPECT_EQ(vector.GetSize(), 5u);
		EXPECT_EQ(vector[0], 1);
		EXPECT_EQ(vector[1], 0);
	}

	UNIT_TEST(Vector, Shrink)
	{
		Vector<int> vector = {1, 2, 3, 4};
		vector.Shrink(2u);
		EXPECT_EQ(vector.GetSize(), 2u);
		EXPECT_EQ(vector[0], 1);
		EXPECT_EQ(vector[1], 2);
	}

	UNIT_TEST(Vector, Swap)
	{
		Vector<int> vector1 = {1, 2, 3, 4};
		Vector<int> vector2 = {5, 6, 7, 8, 9};
		vector1.Swap(Move(vector2));

		EXPECT_EQ(vector1.GetSize(), 5u);
		EXPECT_EQ(vector2.GetSize(), 4u);
		EXPECT_EQ(vector1[0], 5);
		EXPECT_EQ(vector1[1], 6);
		EXPECT_EQ(vector1[2], 7);
		EXPECT_EQ(vector1[3], 8);
		EXPECT_EQ(vector1[4], 9);

		EXPECT_EQ(vector2[0], 1);
		EXPECT_EQ(vector2[1], 2);
		EXPECT_EQ(vector2[2], 3);
		EXPECT_EQ(vector2[3], 4);
	}

	UNIT_TEST(Vector, Find)
	{
		Vector<int> vector = {1, 2, 3, 4, 5};
		EXPECT_FALSE(vector.Contains(0));
		EXPECT_EQ(vector.Find(1), Iterator<int>(&vector[0]));
		EXPECT_EQ(vector.Find(2), Iterator<int>(&vector[1]));
		EXPECT_EQ(vector.Find(3), Iterator<int>(&vector[2]));
		EXPECT_EQ(vector.Find(4), Iterator<int>(&vector[3]));
		EXPECT_EQ(vector.Find(5), Iterator<int>(&vector[4]));
		EXPECT_FALSE(vector.Contains(6));
		EXPECT_TRUE(vector.ContainsIf(
			[](const int element) -> bool
			{
				return element == 3;
			}
		));
		EXPECT_FALSE(vector.ContainsIf(
			[](const int element) -> bool
			{
				return element == 6;
			}
		));
	}
}
