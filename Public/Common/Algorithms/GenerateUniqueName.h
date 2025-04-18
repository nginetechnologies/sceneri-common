#pragma once

#include <Common/Memory/Containers/ArrayView.h>
#include <Common/Memory/Containers/Format/StringView.h>
#include <Common/Memory/Containers/String.h>
#include <Common/Memory/Containers/OrderedSet.h>

namespace ngine::Algorithms
{
	[[nodiscard]] static inline UnicodeString GenerateUniqueName(ConstUnicodeStringView name, ArrayView<ConstUnicodeStringView> existingNames)
	{
		if (existingNames.IsEmpty())
		{
			return UnicodeString(name);
		}

		OrderedSet<uint32> matchedNumbers;
		for (ConstUnicodeStringView existingName : existingNames)
		{
			ConstUnicodeStringView search = existingName.FindLastRange(name);
			if (search.HasElements())
			{
				// Exact same name as we searched for
				if (existingName.GetSize() == name.GetSize())
				{
					// First name has no number so add 1 as the first entry.
					matchedNumbers.Insert(1u);
					continue;
				}

				const uint32 startCommentPosition = existingName.GetIteratorIndex(&search[0]);
				const uint32 searchEndPos = startCommentPosition + search.GetSize() + 1;
				ConstUnicodeStringView subView = existingName.GetSubstring(searchEndPos, existingName.GetSize() - searchEndPos);

				if (auto value = subView.TryToIntegral<uint32>(); value.success)
				{
					matchedNumbers.Insert(value.value);
				}
			}
		}

		// 1 represets just the name itself
		// If that name is not preset it can just be used.
		if (!matchedNumbers.Contains(1u))
		{
			return UnicodeString(name);
		}

		uint32 counter = 2;
		for (uint32 number : matchedNumbers)
		{
			// Ignore all numbers smaller than 2
			if (number < 2)
			{
				continue;
			}

			if (number == counter)
			{
				counter++;
			}
			else
			{
				break;
			}
		}

		UnicodeString result;
		result.Format("{0} {1}", name, counter);
		return result;
	}
}
