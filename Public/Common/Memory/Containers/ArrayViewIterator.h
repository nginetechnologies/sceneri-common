#pragma once

#include <Common/Memory/Containers/ArrayView.h>

namespace ngine
{
	template<typename ViewType>
	struct ArrayViewIterator
	{
		using OptionalType = typename ViewType::StoredType;

		FORCE_INLINE ArrayViewIterator(const ViewType view)
			: m_view(view)
		{
			while (m_view.HasElements() && (!m_view[0].IsValid()))
			{
				operator++();
			}
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr auto& operator*() noexcept
		{
			return *m_view.begin();
		}

		FORCE_INLINE constexpr void operator++() noexcept
		{
			do
			{
				m_view++;
			} while (m_view.HasElements() && (!m_view[0].IsValid()));
		}

		FORCE_INLINE constexpr void operator++(int) noexcept
		{
			do
			{
				m_view++;
			} while (m_view.HasElements() && (!m_view[0].IsValid()));
		}

		FORCE_INLINE constexpr void operator--() noexcept
		{
			do
			{
				m_view--;
			} while (m_view.HasElements() && (!m_view[0].IsValid()));
		}

		FORCE_INLINE constexpr void operator--(int) noexcept
		{
			do
			{
				m_view--;
			} while (m_view.HasElements() && (!m_view[0].IsValid()));
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS bool operator==(const ArrayViewIterator other) const noexcept
		{
			return m_view.begin() == other.m_view.begin();
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS bool operator!=(const ArrayViewIterator other) const noexcept
		{
			return m_view.begin() != other.m_view.begin();
		}
	private:
		ViewType m_view;
	};
}
