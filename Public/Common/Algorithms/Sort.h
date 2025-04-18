#pragma once

#include <Common/Memory/Containers/ArrayView.h>

#include <algorithm>

namespace ngine::Algorithms
{
	template<typename Comparator, typename IteratorType>
	void Sort(IteratorType begin, IteratorType end, Comparator&& comparator)
	{
		std::sort(begin, end, Forward<Comparator>(comparator));
	}
}
