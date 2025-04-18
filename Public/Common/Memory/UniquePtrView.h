#pragma once

#include <Common/Platform/ForceInline.h>
#include <Common/Platform/Pure.h>
#include <Common/Platform/TrivialABI.h>

namespace ngine
{
	template<typename ElementType>
	struct TRIVIAL_ABI UniquePtrView
	{
		UniquePtrView() = default;
		UniquePtrView(ElementType* pElement)
			: m_pElement(pElement)
		{
		}
		UniquePtrView(const UniquePtrView&) = delete;
		UniquePtrView& operator=(const UniquePtrView&) = delete;
		UniquePtrView(UniquePtrView&& other) noexcept
			: m_pElement(other.m_pElement)
		{
			other.m_pElement = nullptr;
		}
		UniquePtrView& operator=(UniquePtrView&& other) noexcept
		{
			m_pElement = other.m_pElement;
			other.m_pElement = nullptr;
			return *this;
		}

		[[nodiscard]] PURE_STATICS operator ElementType*() noexcept
		{
			return m_pElement;
		}
		[[nodiscard]] PURE_STATICS operator const ElementType *() const noexcept
		{
			return m_pElement;
		}
	protected:
		ElementType* m_pElement = nullptr;
	};
}
