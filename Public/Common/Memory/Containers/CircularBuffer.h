#pragma once

#include <Common/Memory/Containers/Array.h>
#include <Common/Math/Wrap.h>
#include <Common/Math/Max.h>

namespace ngine
{
	template<typename ContainedType, size Size>
	struct FixedCircularBuffer
	{
		using SizeType = Memory::NumericSize<Size>;
		using IndexType = SizeType;

		using View = ArrayView<ContainedType, SizeType>;
		using ConstView = ArrayView<const ContainedType, SizeType>;
		using PointerType = typename View::PointerType;
		using ConstPointerType = typename View::ConstPointerType;
		using ReferenceType = typename View::ReferenceType;
		using IteratorType = typename View::IteratorType;
		using ReverseIteratorType = typename View::ReverseIteratorType;
		using ConstIteratorType = typename View::ConstIteratorType;
		using ConstReverseIteratorType = typename View::ConstReverseIteratorType;
		using iterator = typename View::iterator;
		using reverse_iterator = typename View::reverse_iterator;
		using const_iterator = typename View::const_iterator;
		using const_reverse_iterator = typename View::const_reverse_iterator;

		ContainedType& Emplace()
		{
			const SizeType capacity = GetCapacity();
			const SizeType nextIndex = m_nextIndex;
			m_nextIndex = SizeType(nextIndex + 1) % capacity;
			m_size = Math::Min(SizeType(m_size + 1), capacity);
			return m_elements[nextIndex];
		}

		[[nodiscard]] ContainedType& GetLastElement()
		{
			Expect(m_size > 0);
			const SizeType previousIndex = GetLastIndex();
			return m_elements[previousIndex];
		}
		[[nodiscard]] const ContainedType& GetLastElement() const
		{
			Expect(m_size > 0);
			const SizeType previousIndex = GetLastIndex();
			return m_elements[previousIndex];
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr SizeType GetSize() const noexcept
		{
			return m_size;
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr SizeType GetCapacity() const noexcept
		{
			return Size;
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr SizeType GetTheoreticalCapacity() const noexcept
		{
			return Size;
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr SizeType GetNextAvailableIndex() const noexcept
		{
			return m_nextIndex;
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr SizeType GetFirstIndex() const noexcept
		{
			return m_nextIndex % Math::Max(m_size, (SizeType)1);
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr SizeType GetLastIndex() const noexcept
		{
			return SizeType((int64(m_nextIndex) - 1 + m_size) % (int64)Math::Max(m_size, (SizeType)1));
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr bool IsEmpty() const noexcept
		{
			return m_size == 0;
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr bool HasElements() const noexcept
		{
			return m_size > 0;
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS operator View() noexcept LIFETIME_BOUND
		{
			return {begin(), end()};
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS operator ConstView() const noexcept LIFETIME_BOUND
		{
			return {begin(), end()};
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr View GetView() noexcept LIFETIME_BOUND
		{
			return {begin(), end()};
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr ConstView GetView() const noexcept LIFETIME_BOUND
		{
			return {begin(), end()};
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS PointerType GetData() noexcept LIFETIME_BOUND
		{
			return Memory::GetAddressOf(m_elements[0]);
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS ConstPointerType GetData() const noexcept LIFETIME_BOUND
		{
			return Memory::GetAddressOf(m_elements[0]);
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr ContainedType& operator[](const IndexType index) noexcept LIFETIME_BOUND
		{
			Expect((SizeType)index < m_size);
			return *(GetData() + (SizeType)index);
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr const ContainedType& operator[](const IndexType index) const noexcept LIFETIME_BOUND
		{
			Expect((SizeType)index < m_size);
			return *(GetData() + (SizeType)index);
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr IteratorType begin() noexcept LIFETIME_BOUND
		{
			return GetData();
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr ConstIteratorType begin() const noexcept LIFETIME_BOUND
		{
			return GetData();
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr IteratorType end() noexcept LIFETIME_BOUND
		{
			return GetData() + m_size;
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr ConstIteratorType end() const noexcept LIFETIME_BOUND
		{
			return GetData() + m_size;
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr ReverseIteratorType rbegin() noexcept LIFETIME_BOUND
		{
			return GetData() + m_size - 1;
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr ConstReverseIteratorType rbegin() const noexcept LIFETIME_BOUND
		{
			return GetData() + m_size - 1;
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr ReverseIteratorType rend() noexcept LIFETIME_BOUND
		{
			return GetData() - 1;
		}
		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr ConstReverseIteratorType rend() const noexcept LIFETIME_BOUND
		{
			return GetData() - 1;
		}
	private:
		Array<ContainedType, Size> m_elements;
		SizeType m_nextIndex{0};
		SizeType m_size{0};
	};
}
