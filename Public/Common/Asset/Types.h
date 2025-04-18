#pragma once

#include "Guid.h"

#include <Common/Memory/Containers/InlineVector.h>

namespace ngine::Asset
{
	using TypeGuid = ngine::Guid;

	struct Types
	{
		explicit Types() = default;
		Types(const ArrayView<const TypeGuid, uint16> allowedTypeGuids)
			: m_allowedTypeGuids{allowedTypeGuids}
		{
		}
		Types(const TypeGuid allowedTypeGuid)
			: m_allowedTypeGuids(allowedTypeGuid)
		{
		}
		Types(const Types& other)
			: m_allowedTypeGuids(other.m_allowedTypeGuids.GetView())
		{
		}
		Types& operator=(const Types& other)
		{
			m_allowedTypeGuids = other.m_allowedTypeGuids.GetView();
			return *this;
		}

		[[nodiscard]] ArrayView<const TypeGuid, uint16> GetAllowedTypeGuids() const
		{
			return m_allowedTypeGuids.GetView();
		}

		using Container = InlineVector<TypeGuid, 2, uint16>;
		Container m_allowedTypeGuids;
	};
}
