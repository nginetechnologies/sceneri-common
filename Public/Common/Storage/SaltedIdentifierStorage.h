#pragma once

#include <Common/Memory/Containers/Array.h>
#include <Common/Memory/ReferenceWrapper.h>
#include <Common/Memory/Containers/ArrayViewIterator.h>
#include <Common/Assert/Assert.h>
#include <Common/Math/Max.h>
#include <Common/Storage/Identifier.h>
#include <Common/Storage/FixedIdentifierArrayView.h>
#include <Common/Storage/IdentifierArrayView.h>
#include <Common/Threading/AtomicInteger.h>

namespace ngine
{
	template<typename IdentifierType>
	struct TSaltedIdentifierStorage final
	{
		using Container = Array<IdentifierType, IdentifierType::MaximumCount>;
		using ConstView = IdentifierArrayView<const IdentifierType, IdentifierType>;

		constexpr TSaltedIdentifierStorage()
		{
			Reset();
		}

		constexpr void Reset()
		{
			for (typename Container::iterator begin = m_container.begin(), it = begin, end = m_container.end(); it != end; ++it)
			{
				IdentifierType& __restrict element = *it;
				// Set the next free index
				element = IdentifierType::MakeFromIndex(static_cast<typename IdentifierType::IndexType>(it - begin) + 1);
			}

			m_maxUsedElementCount = 0;
			m_nextFreeElementIndex = 0;
		}

		TSaltedIdentifierStorage(TSaltedIdentifierStorage&& other)
			: m_container(Move(other.m_container))
			, m_maxUsedElementCount(other.m_maxUsedElementCount)
			, m_nextFreeElementIndex(other.m_nextFreeElementIndex)
		{
			// Note: other is in an invalid state
			// We could call Reset, but this is rather expensive and shouldn't be done on move.
		}
		TSaltedIdentifierStorage& operator=(TSaltedIdentifierStorage&& other)
		{
			m_container = Move(other.m_container);

			m_maxUsedElementCount = other.m_maxUsedElementCount;
			m_nextFreeElementIndex = other.m_nextFreeElementIndex;

			// Note: other is in an invalid state
			// We could call Reset, but this is rather expensive and shouldn't be done on move.
			return *this;
		}
		TSaltedIdentifierStorage(const TSaltedIdentifierStorage& other)
			: m_container(other.m_container)
			, m_maxUsedElementCount(other.m_maxUsedElementCount)
			, m_nextFreeElementIndex(other.m_nextFreeElementIndex)
		{
		}
		TSaltedIdentifierStorage& operator=(const TSaltedIdentifierStorage& other)
		{
			m_container = other.m_container;
			m_maxUsedElementCount = other.m_maxUsedElementCount;
			m_nextFreeElementIndex = other.m_nextFreeElementIndex;
			return *this;
		}

		[[nodiscard]] FORCE_INLINE bool IsEmpty() const
		{
			return m_maxUsedElementCount.Load() == (typename IdentifierType::IndexType)0;
		}
		[[nodiscard]] FORCE_INLINE bool IsFull() const
		{
			return m_nextFreeElementIndex.Load() == m_container.GetSize();
		}

		[[nodiscard]] FORCE_INLINE IdentifierType AcquireIdentifier()
		{
			typename IdentifierType::IndexType newElementIndex = m_nextFreeElementIndex;
			if (newElementIndex < m_container.GetSize())
			{
				ReferenceWrapper<IdentifierType> indexElement = m_container[newElementIndex];
				while (!m_nextFreeElementIndex.CompareExchangeWeak(newElementIndex, indexElement->GetIndex()))
				{
					indexElement = m_container[newElementIndex];
				}

				// Increment so that index 0 = invalid
				newElementIndex++;
				m_maxUsedElementCount.AssignMax(newElementIndex);

				const IdentifierType newIdentifier(newElementIndex, indexElement->GetIndexUseCount());

				indexElement->IncrementUseCount();

				return newIdentifier;
			}
			return {};
		}

		[[nodiscard]] FORCE_INLINE bool IsIdentifierPotentiallyValid(const IdentifierType identifier) const
		{
			return identifier.GetFirstValidIndex() < m_maxUsedElementCount &&
			       m_container[identifier.GetFirstValidIndex()].GetIndexUseCount() > 0 && IsIdentifierActive(identifier);
		}

		[[nodiscard]] FORCE_INLINE bool IsIdentifierActive(const IdentifierType identifier) const
		{
			return identifier.GetIndex() != 0 &&
			       m_container[identifier.GetFirstValidIndex()].GetIndexUseCount() - 1 == identifier.GetIndexUseCount();
		}

		[[nodiscard]] IdentifierType GetActiveIdentifier(const IdentifierType identifier) const
		{
			if (UNLIKELY(identifier.GetFirstValidIndex() >= m_maxUsedElementCount))
			{
				return {};
			}
			const IdentifierType& __restrict element = m_container[identifier.GetFirstValidIndex()];
			if (UNLIKELY(element.GetIndexUseCount() == 0))
			{
				return {};
			}

			return IdentifierType(identifier.GetIndex(), element.GetIndexUseCount() - 1u);
		}

		[[nodiscard]] ConstView GetView() const
		{
			return GetValidElementView(ConstView{m_container.GetDynamicView()});
		}

		FORCE_INLINE void ReturnIdentifier(const IdentifierType identifier)
		{
			ReturnIdentifier(identifier.GetFirstValidIndex());
		}

		template<typename ElementType>
		[[nodiscard]] FORCE_INLINE IdentifierArrayView<ElementType, IdentifierType>
		GetValidElementView(IdentifierArrayView<ElementType, IdentifierType> view) const
		{
			return view.GetSubViewUpTo(m_maxUsedElementCount);
		}

		template<typename ElementType>
		[[nodiscard]] FORCE_INLINE IdentifierArrayView<ElementType, IdentifierType>
		GetValidElementView(FixedIdentifierArrayView<ElementType, IdentifierType> view) const
		{
			return view.GetSubViewUpTo(m_maxUsedElementCount);
		}

		template<typename OutputType, typename Callback, typename ElementType>
		inline Optional<OutputType> FindInElements(IdentifierArrayView<ElementType, IdentifierType> view, Callback&& callback) const
		{
			view = GetValidElementView(view);
			for (ElementType& element : view)
			{
				if (const Optional<OutputType> result = callback(element))
				{
					return result;
				}
			}

			return Invalid;
		}

		template<typename OutputType, typename Callback, typename ElementType>
		inline Optional<OutputType> FindInElements(FixedIdentifierArrayView<ElementType, IdentifierType> view, Callback&& callback) const
		{
			auto dynamicView = GetValidElementView(view);
			for (ElementType& element : dynamicView)
			{
				if (const Optional<OutputType> result = callback(element))
				{
					return result;
				}
			}

			return Invalid;
		}

		// Specific iterator implementation that will only go through valid elements
		// Assumes that the view element is Optional<T>
		template<typename ViewType>
		using ValidElementIterator = ArrayViewIterator<ViewType>;

		[[nodiscard]] FORCE_INLINE typename IdentifierType::IndexType GetMaximumUsedElementCount() const
		{
			return m_maxUsedElementCount;
		}
	protected:
		void ReturnIdentifier(typename IdentifierType::IndexType index)
		{
			IdentifierType& __restrict element = m_container[index];

			typename IdentifierType::IndexType previousNextElementIndex = m_nextFreeElementIndex;
			element = IdentifierType(previousNextElementIndex, element.GetIndexUseCount());
			while (!m_nextFreeElementIndex.CompareExchangeWeak(previousNextElementIndex, index))
			{
				element = IdentifierType(previousNextElementIndex, element.GetIndexUseCount());
			}

			index++;
			m_maxUsedElementCount.CompareExchangeStrong(index, (typename IdentifierType::IndexType)(index - 1u));
		}
	protected:
		Container m_container;

		Threading::Atomic<typename IdentifierType::IndexType> m_maxUsedElementCount = 0;
		Threading::Atomic<typename IdentifierType::IndexType> m_nextFreeElementIndex = 1;
	};
}
