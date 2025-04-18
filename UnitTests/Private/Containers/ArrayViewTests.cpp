#include <Common/Memory/New.h>

#include <Common/Tests/UnitTest.h>

#include <Common/Memory/Containers/Array.h>
#include <Common/Memory/Containers/ArrayView.h>

namespace ngine::Tests
{
	UNIT_TEST(ArrayView, DefaultConstruct)
	{
		ArrayView<int> view;
		EXPECT_TRUE(view.IsEmpty());
		EXPECT_FALSE(view.HasElements());
		EXPECT_EQ(view.GetSize(), 0u);
		EXPECT_EQ(view.GetDataSize(), 0u);
		EXPECT_EQ(view.GetData(), nullptr);
		EXPECT_EQ(view.begin(), nullptr);
		EXPECT_EQ(view.end(), nullptr);
	}

	UNIT_TEST(ArrayView, ConstructSingleElement)
	{
		int element = 10;
		ArrayView<int> view(element);
		EXPECT_EQ(view.GetSize(), 1u);
		EXPECT_EQ(view.GetDataSize(), sizeof(int) * 1u);
		EXPECT_EQ(&view[0], &element);
		EXPECT_EQ(view[0], 10);
		EXPECT_EQ(*view.GetData(), 10);
		EXPECT_EQ(view.GetIteratorIndex(&element), 0u);
		EXPECT_EQ(view.GetLastElement(), 10);
		EXPECT_EQ(view.begin(), &element);
		EXPECT_EQ(view.end(), &element + 1u);
		EXPECT_TRUE(view.IsWithinBounds(&element));
		EXPECT_FALSE(view.IsWithinBounds(&element + 1u));
		EXPECT_FALSE(view.IsWithinBounds(&element - 1u));
	}

	UNIT_TEST(ArrayView, GetSubView)
	{
		Array<int, 5> elements = {1, 2, 3, 4, 5};
		const ArrayView<const int> view = elements.GetDynamicView();
		EXPECT_EQ(view.GetSize(), 5u);
		EXPECT_EQ(view[0], 1);
		EXPECT_EQ(view[1], 2);
		EXPECT_EQ(view[2], 3);
		EXPECT_EQ(view[3], 4);
		EXPECT_EQ(view[4], 5);
		EXPECT_EQ(view.GetIteratorIndex(&elements[2]), 2u);
		EXPECT_TRUE(view.Contains(3));
		EXPECT_FALSE(view.Contains(6));

		{
			const ArrayView<const int> subView = view.GetSubView(1u, 3u);
			EXPECT_EQ(subView.GetSize(), 3u);
			EXPECT_EQ(subView[0], 2);
			EXPECT_EQ(subView[1], 3);
			EXPECT_EQ(subView[2], 4);
			EXPECT_TRUE(view.IsWithinBounds(subView));
			EXPECT_TRUE(view.Overlaps(subView));
		}
	}

	UNIT_TEST(ArrayView, CopyFrom)
	{
		Array<int, 5> elements = {1, 2, 3, 4, 5};
		Array<int, 5> targetElements;
		targetElements.GetView().CopyFrom(elements.GetDynamicView());
		EXPECT_EQ(targetElements[0], 1);
		EXPECT_EQ(targetElements[1], 2);
		EXPECT_EQ(targetElements[2], 3);
		EXPECT_EQ(targetElements[3], 4);
		EXPECT_EQ(targetElements[4], 5);
	}

	UNIT_TEST(ArrayView, DefaultInitialize)
	{
		struct MyType
		{
			int m_value = 1337;
		};

		Array<MyType, 5> targetElements;
		targetElements.GetView().DefaultConstruct();

		EXPECT_EQ(targetElements[0].m_value, 1337);
		EXPECT_EQ(targetElements[1].m_value, 1337);
		EXPECT_EQ(targetElements[2].m_value, 1337);
		EXPECT_EQ(targetElements[3].m_value, 1337);
		EXPECT_EQ(targetElements[4].m_value, 1337);
	}

	UNIT_TEST(ArrayView, ZeroInitialize)
	{
		Array<int, 5> targetElements;
		targetElements.GetView().ZeroInitialize();

		EXPECT_EQ(targetElements[0], 0);
		EXPECT_EQ(targetElements[1], 0);
		EXPECT_EQ(targetElements[2], 0);
		EXPECT_EQ(targetElements[3], 0);
		EXPECT_EQ(targetElements[4], 0);
	}

	template<
		typename ContainedType,
		typename StoredType,
		typename InternalSizeType = uint32,
		typename InternalIndexType = InternalSizeType,
		uint8 Flags = 0>
	using StridedArrayView = ArrayView<ContainedType, InternalSizeType, InternalIndexType, StoredType, Flags>;
	UNIT_TEST(ArrayView, BaseView)
	{
		struct Foo
		{
			int m_data;
		};

		struct Bar final : public Foo
		{
			bool m_barData;
		};

		constexpr Array<Bar, 5> bars = {Bar(), Bar(), Bar(), Bar(), Bar()};

		const ArrayView<const Bar, uint8> barView = bars.GetView();
		const StridedArrayView<const Foo, const Bar, uint8> fooView = bars.GetDynamicView();

		EXPECT_EQ(barView.GetSize(), bars.GetSize());
		EXPECT_EQ(fooView.GetSize(), bars.GetSize());

		for (uint8 i = 0; i < bars.GetSize(); ++i)
		{
			EXPECT_EQ(&barView[i], &fooView[i]);
		}
	}

	UNIT_TEST(ArrayView, RangeComparisons)
	{
		constexpr Array<int, 8> elements{1, 1, 2, 3, 4, 5, 1, 1};

		EXPECT_TRUE(elements.GetDynamicView() == elements.GetDynamicView());
		{
			Array<int, 8> otherElements{1, 1, 2, 3, 4, 5, 1, 1};
			EXPECT_TRUE(otherElements.GetDynamicView() == elements.GetDynamicView());
			EXPECT_FALSE(otherElements.GetDynamicView() != elements.GetDynamicView());
		}
		{
			Array<int, 7> otherElements{1, 1, 2, 3, 4, 1, 1};
			EXPECT_TRUE(otherElements.GetDynamicView() != elements.GetDynamicView());
			EXPECT_FALSE(otherElements.GetDynamicView() == elements.GetDynamicView());
		}
		{
			Array<int, 9> otherElements{1, 1, 0, 2, 3, 4, 5, 1, 1};
			EXPECT_TRUE(otherElements.GetDynamicView() != elements.GetDynamicView());
			EXPECT_FALSE(otherElements.GetDynamicView() == elements.GetDynamicView());
		}

		{
			Array<int, 9> otherElements{1, 1, 1, 2, 3, 4, 4, 1, 1};
			EXPECT_TRUE(otherElements.GetDynamicView() != elements.GetDynamicView());
			EXPECT_FALSE(otherElements.GetDynamicView() == elements.GetDynamicView());
		}

		EXPECT_TRUE(elements.GetDynamicView().ContainsRange(Array<int, 3>{2, 3, 4}.GetDynamicView()));
		EXPECT_FALSE(elements.GetDynamicView().ContainsRange(Array<int, 3>{2, 3, 5}.GetDynamicView()));

		{
			const ArrayView<const int> foundRange = elements.GetDynamicView().FindFirstRange(Array<int, 2>{1, 1}.GetDynamicView());
			EXPECT_TRUE(foundRange.HasElements());
			EXPECT_EQ(foundRange.GetSize(), 2u);
			EXPECT_EQ(foundRange[0], 1);
			EXPECT_EQ(foundRange[1], 1);
			EXPECT_EQ(&foundRange[0], &elements[0]);
		}

		{
			const ArrayView<const int> foundRange = elements.GetDynamicView().FindLastRange(Array<int, 2>{1, 1}.GetDynamicView());
			EXPECT_TRUE(foundRange.HasElements());
			EXPECT_EQ(foundRange.GetSize(), 2u);
			EXPECT_EQ(foundRange[0], 1);
			EXPECT_EQ(foundRange[1], 1);
			EXPECT_EQ(&foundRange[0], &elements[elements.GetSize() - 2u]);
		}

		EXPECT_TRUE(elements.GetDynamicView().ContainsAny(Array<int, 1>{1}.GetDynamicView()));
		EXPECT_TRUE(elements.GetDynamicView().ContainsAny(Array<int, 2>{2, 1}.GetDynamicView()));
		EXPECT_TRUE(elements.GetDynamicView().ContainsAny(Array<int, 3>{2, 1, 3}.GetDynamicView()));
		EXPECT_TRUE(elements.GetDynamicView().ContainsAny(Array<int, 3>{7, 1, 11}.GetDynamicView()));

		EXPECT_FALSE(elements.GetDynamicView().IsWithinBounds(Array<int, 3>{2, 3, 4}.GetDynamicView()));
		EXPECT_FALSE(elements.GetDynamicView().Overlaps(Array<int, 3>{2, 3, 4}.GetDynamicView()));
		EXPECT_FALSE(elements.GetDynamicView().Contains(Array<int, 3>{2, 3, 4}.GetDynamicView()));

		EXPECT_TRUE(elements.GetDynamicView().IsWithinBounds(elements.GetSubView(0, 2)));
		EXPECT_TRUE(elements.GetDynamicView().IsWithinBounds(elements.GetSubView(1, 2)));
		EXPECT_TRUE(elements.GetDynamicView().IsWithinBounds(elements.GetSubView(2, 2)));
		EXPECT_TRUE(elements.GetDynamicView().IsWithinBounds(elements.GetSubView(0, 1)));
		EXPECT_TRUE(elements.GetDynamicView().IsWithinBounds(elements.GetSubView(4, 1)));
		EXPECT_FALSE(elements.GetDynamicView().IsWithinBounds(ArrayView<const int>{elements.GetData() - 1, 2}));
		EXPECT_FALSE(elements.GetDynamicView().IsWithinBounds(ArrayView<const int>{elements.end() - 2, 3}));
		EXPECT_FALSE(elements.GetDynamicView().IsWithinBounds(ArrayView<const int>{elements.begin() - 1, 1}));
		EXPECT_FALSE(elements.GetDynamicView().IsWithinBounds(ArrayView<const int>{elements.end(), 2}));

		EXPECT_TRUE(elements.GetDynamicView().Overlaps(elements.GetSubView(0, 2)));
		EXPECT_TRUE(elements.GetDynamicView().Overlaps(elements.GetSubView(1, 2)));
		EXPECT_TRUE(elements.GetDynamicView().Overlaps(elements.GetSubView(2, 2)));
		EXPECT_TRUE(elements.GetDynamicView().Overlaps(elements.GetSubView(0, 1)));
		EXPECT_TRUE(elements.GetDynamicView().Overlaps(elements.GetSubView(4, 1)));
		EXPECT_TRUE(elements.GetDynamicView().Overlaps(ArrayView<const int>{elements.GetData() - 1, 2}));
		EXPECT_TRUE(elements.GetDynamicView().Overlaps(ArrayView<const int>{elements.end() - 2, 3}));
		EXPECT_FALSE(elements.GetDynamicView().Overlaps(ArrayView<const int>{elements.begin() - 1, 1}));
		EXPECT_FALSE(elements.GetDynamicView().Overlaps(ArrayView<const int>{elements.end(), 2}));
	}
}
